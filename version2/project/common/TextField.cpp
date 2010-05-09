#include <TextField.h>
#include <Tilesheet.h>
#include <Utils.h>
#include <Surface.h>
#include <KeyCodes.h>
#include "XML/tinyxml.h"
#include <ctype.h>
#include <time.h>

namespace nme
{

TextField::TextField(bool inInitRef) : DisplayObject(inInitRef),
   alwaysShowSelection(false),
   antiAliasType(aaNormal),
   autoSize(asNone),
   background(false),
   backgroundColor(0xffffffff),
   border(false),
   borderColor(0x00000000),
   condenseWhite(false),
   defaultTextFormat( TextFormat::Default() ),
   displayAsPassword(false),
   embedFonts(false),
   gridFitType(gftPixel),
   maxChars(0),
   mouseWheelEnabled(true),
   multiline(false),
   restrict(std::wstring()),
   scrollH(0),
   scrollV(0),
   selectable(true),
   sharpness(0),
   styleSheet(0),
   textColor(0x000000),
   thickness(0),
   useRichTextClipboard(false),
   wordWrap(false),
   isInput(false)
{
   mStringState = ssText;
   mLinesDirty = false;
   mGfxDirty = true;
   boundsWidth = 100.0;
   boundsHeight = 100.0;
   mActiveRect = Rect(100,100);
   mFontsDirty = false;
   mSelectMin = mSelectMax = 0;
   mSelectDownChar = 0;
   caretIndex = 0;
   mCaretGfx = 0;
   mHighlightGfx = 0;
   mLastCaretHeight = -1;
   mSelectKeyDown = -1;
   maxScrollH  = 0;
   maxScrollV  = 0;
   setText(L"");
   textWidth = textHeight = 0;
   mLastUpDownX = -1;
   mLayoutScaleH = mLayoutScaleV = -1.0;
   mLayoutRotation = gr0;
}

TextField::~TextField()
{
   if (mCaretGfx)
      mCaretGfx->DecRef();
   if (mHighlightGfx)
      mHighlightGfx->DecRef();
   defaultTextFormat->DecRef();
}

void TextField::setWidth(double inWidth)
{
   boundsWidth = inWidth;
   mLinesDirty = true;
   mGfxDirty = true;
}


void TextField::setHeight(double inHeight)
{
   boundsHeight = inHeight;
   mLinesDirty = true;
   mGfxDirty = true;
}


const TextFormat *TextField::getDefaultTextFormat()
{
   return defaultTextFormat;
}

void TextField::setDefaultTextFormat(TextFormat *inFmt)
{
   if (inFmt)
      inFmt->IncRef();
   if (defaultTextFormat)
      defaultTextFormat->DecRef();
   defaultTextFormat = inFmt;
   mLinesDirty = true;
   mGfxDirty = true;
}

void TextField::SplitGroup(int inGroup,int inPos)
{
   CharGroup &group = mCharGroups[inGroup];
   CharGroup extra = group;
   extra.mFormat->IncRef();
   extra.mFont->IncRef();
   group.mChar0 += inPos;
   extra.mString.Set(&group.mString[inPos], group.mString.size()-inPos);
   group.mString.resize(inPos);
   mCharGroups.InsertAt(inGroup+1,extra);
   mLinesDirty = true;
}

void TextField::setTextFormat(TextFormat *inFmt,int inStart,int inEnd)
{
   if (!inFmt)
      return;

   Layout(GetFullMatrix(true));

   if (inStart<0) inStart = 0;
   int max = mCharPos.size();
   if (inEnd>max || inEnd<0) inEnd = max;

   if (inEnd<=inStart)
      return;

   inFmt->IncRef();
   int g0 = GroupFromChar(inStart);
   int g1 = GroupFromChar(inEnd);
   int g0_ex = inStart-mCharGroups[g0].mChar0;
   if (g0_ex>0)
   {
      SplitGroup(g0,g0_ex);
      g0++;
      g1++;
   }
   if (inEnd<max)
   {
      int g1_ex = inEnd-mCharGroups[g1].mChar0;
      if (g1_ex>0)
      {
         SplitGroup(g1,g1_ex);
         g1++;
      }
   }

   for(int g=g0;g<g1;g++)
   {
      mCharGroups[g].ApplyFormat(inFmt);
   }

   inFmt->DecRef();

   mLinesDirty = true;
   mFontsDirty = true;
   mGfxDirty = true;
}





void TextField::setTextColor(int inCol)
{
   textColor = inCol;
}

void TextField::setIsInput(bool inIsInput)
{
   isInput = inIsInput;
}

void TextField::setBackground(bool inBackground)
{
   background = inBackground;
   mGfxDirty = true;
   DirtyCache();
}

void TextField::setBackgroundColor(int inBackgroundColor)
{
   backgroundColor = inBackgroundColor;
   mGfxDirty = true;
   DirtyCache();
}

void TextField::setBorder(bool inBorder)
{
   border = inBorder;
   mGfxDirty = true;
   DirtyCache();
}

void TextField::setBorderColor(int inBorderColor)
{
   borderColor = inBorderColor;
   mGfxDirty = true;
   DirtyCache();
}

void TextField::setMultiline(bool inMultiline)
{
   multiline = inMultiline;
   mLinesDirty = true;
   mGfxDirty = true;
   DirtyCache();
}

void TextField::setWordWrap(bool inWordWrap)
{
   wordWrap = inWordWrap;
   mLinesDirty = true;
   mGfxDirty = true;
   DirtyCache();
}

void TextField::setAutoSize(int inAutoSize)
{
   autoSize = (AutoSizeMode)inAutoSize;
   mLinesDirty = true;
   mGfxDirty = true;
   DirtyCache();
}



void TextField::Focus()
{
#ifdef IPHONE
  Stage *stage = getStage();
  if (stage)
     stage->EnablePopupKeyboard(true);
#endif
}

void TextField::Unfocus()
{
#ifdef IPHONE
  Stage *stage = getStage();
  if (stage)
     stage->EnablePopupKeyboard(false);
#endif
}

bool TextField::FinishEditOnEnter()
{
   // For iPhone really
   return !multiline;
}

int TextField::getBottomScrollV()
{
	Layout();
	int l = scrollV;
	int height = boundsHeight;
	while(height>0 && l<mLines.size())
   {
      Line &line = mLines[l++];
		height -= line.mMetrics.height;
	}
	return l;
}

void TextField::setScrollH(int inScrollH)
{
	scrollH = inScrollH;
	if (scrollH<0)
		scrollH = 0;
	if (scrollH>maxScrollH)
		scrollH = scrollH;
	// TODO: do we need to re-layout on scroll?
	mLinesDirty = true;
   mGfxDirty = true;
   DirtyCache();
}

void TextField::setScrollV(int inScrollV)
{
	scrollV = inScrollV;
	if (scrollV<0)
		scrollV = 0;
	if (scrollV>maxScrollV)
		scrollV = scrollV;
	// TODO: do we need to re-layout on scroll?
	mLinesDirty = true;
   mGfxDirty = true;
   DirtyCache();

}



int TextField::getLength()
{
   if (mLines.empty()) return 0;
   Line & l =  mLines[ mLines.size()-1 ];
   return l.mChar0 + l.mChars;
}

int TextField::PointToChar(int inX,int inY)
{
   if (mCharPos.empty())
      return 0;

   ImagePoint scroll = GetScrollPos();
   inX +=scroll.x;
   inY +=scroll.y;

   // Find the line ...
   for(int l=0;l<mLines.size();l++)
   {
      Line &line = mLines[l];
      if ( (line.mY0+line.mMetrics.height) > inY && line.mChars)
      {
         // Find the char
         for(int c=0; c<line.mChars;c++)
            if (mCharPos[line.mChar0 + c].x>inX)
               return c==0 ? line.mChar0 : line.mChar0+c-1;
         return line.mChar0 + line.mChars;
      }
   }

   return getLength();
}



int TextField::getSelectionBeginIndex()
{
   if (mSelectMax <= mSelectMin)
      return caretIndex;
   return mSelectMin;
}

int TextField::getSelectionEndIndex()
{
   if (mSelectMax <= mSelectMin)
      return caretIndex;
   return mSelectMax;
}


bool TextField::CaptureDown(Event &inEvent)
{
   if (selectable || isInput)
   {
      if (selectable)
         getStage()->EnablePopupKeyboard(true);

      UserPoint point = TargetToRect(GetFullMatrix(true), UserPoint( inEvent.x, inEvent.y) );
      int pos = PointToChar(point.x,point.y);
      caretIndex = pos;
      if (selectable)
      {
         mSelectDownChar = pos;
         mSelectMin = mSelectMax = pos;
         mGfxDirty = true;
         DirtyCache();
      }
   }
   return true;
}

UserPoint TextField::TargetToRect(const Matrix &inMatrix,const UserPoint &inPoint)
{
   UserPoint p = (inPoint - UserPoint(inMatrix.mtx, inMatrix.mty));
   switch(mLayoutRotation)
   {
      case gr90: return UserPoint(-p.y,p.x);
      case gr180: return UserPoint(-p.x,-p.y);
      case gr270: return UserPoint(p.y,-p.x);
   }
   return p;
}

UserPoint TextField::RectToTarget(const Matrix &inMatrix,const UserPoint &inPoint)
{
   UserPoint trans(inMatrix.mtx, inMatrix.mty);
   switch(mLayoutRotation)
   {
      case gr90: return UserPoint(inPoint.y,-inPoint.x) + trans;
      case gr180: return UserPoint(-inPoint.x,-inPoint.y) + trans;
      case gr270: return UserPoint(-inPoint.y,inPoint.x) + trans;
   }
   return inPoint + trans;

}



void TextField::Drag(Event &inEvent)
{
   if (selectable)
   {
      mSelectKeyDown = -1;
      UserPoint point = TargetToRect(GetFullMatrix(true),UserPoint( inEvent.x, inEvent.y) );
      int pos = PointToChar(point.x,point.y);
      if (pos>mSelectDownChar)
      {
         mSelectMin = mSelectDownChar;
         mSelectMax = pos;
      }
      else
      {
         mSelectMin = pos;
         mSelectMax = mSelectDownChar;
      }
      caretIndex = pos;
      ShowCaret();
      //printf("%d(%d) -> %d,%d\n", pos, mSelectDownChar, mSelectMin , mSelectMax);
      mGfxDirty = true;
      DirtyCache();
   }
}

void TextField::EndDrag(Event &inEvent)
{
}


void TextField::OnKey(Event &inEvent)
{
   if (isInput && inEvent.type==etKeyDown && inEvent.code<0xffff )
   {
      int code = inEvent.code;
      bool shift = inEvent.flags & efShiftDown;

      switch(inEvent.value)
      {
         case keyBACKSPACE:
            if (mSelectMin<mSelectMax)
            {
               DeleteSelection();
            }
            else if (caretIndex>0)
            {
               DeleteChars(caretIndex-1,caretIndex);
               caretIndex--;
            }
            else if (mCharGroups.size())
               DeleteChars(0,1);
            ShowCaret();
            return;

         case keySHIFT:
            mSelectKeyDown = -1;
            mGfxDirty = true;
            return;

         case keyRIGHT:
         case keyLEFT:
         case keyHOME:
         case keyEND:
            mLastUpDownX = -1;
         case keyUP:
         case keyDOWN:
            if (mSelectKeyDown<0 && shift) mSelectKeyDown = caretIndex;
            switch(inEvent.value)
            {
               case keyLEFT: if (caretIndex>0) caretIndex--; break;
               case keyRIGHT: if (caretIndex<mCharPos.size()) caretIndex++; break;
               case keyHOME: caretIndex = 0; break;
               case keyEND: caretIndex = getLength(); break;
               case keyUP:
               case keyDOWN:
               {
                  int l= LineFromChar(caretIndex);
                  //printf("caret line : %d\n",l);
                  if (l==0 && inEvent.value==keyUP) return;
                  if (l==mLines.size()-1 && inEvent.value==keyDOWN) return;
                  l += (inEvent.value==keyUP) ? -1 : 1;
                  Line &line = mLines[l];
                  if (mLastUpDownX<0)
                     mLastUpDownX  = GetCursorPos().x + 1;
                  int c;
                  for(c=0; c<line.mChars;c++)
                     if (mCharPos[line.mChar0 + c].x>mLastUpDownX)
                        break;
                  caretIndex =  c==0 ? line.mChar0 : line.mChar0+c-1;
                  break;
               }
            }

            if (mSelectKeyDown>=0)
            {
               mSelectMin = std::min(mSelectKeyDown,caretIndex);
               mSelectMax = std::max(mSelectKeyDown,caretIndex);
               mGfxDirty = true;
            }
            ShowCaret();
            return;

         // TODO: top/bottom

         case keyENTER:
            code = '\n';
            break;
      }

      if ( (multiline && code=='\n') || (code>27 && code<63000))
      {
         DeleteSelection();
         wchar_t str[2] = {code,0};
         InsertString(str);
         ShowCaret();
      }
   }
   else
   {
      if (inEvent.type==etKeyUp && inEvent.value==keySHIFT)
         mSelectKeyDown = -1;
   }
}


void TextField::ShowCaret()
{
   ImagePoint pos(0,0);
   bool changed = false;

   if (caretIndex < mCharPos.size())
      pos = mCharPos[caretIndex];
   else if (mLines.size())
   {
      pos.x = EndOfLineX( mLines.size()-1 );
      pos.y = mLines[ mLines.size() -1].mY0;
   }
   if (pos.x-scrollH >= mActiveRect.w)
   {
      changed = true;
      scrollH = pos.x - mActiveRect.w + 1;
   }
   else if (pos.x-scrollH < 0)
   {
      changed = true;
      scrollH = pos.x;
   }
   if (scrollH<0)
   {
      changed = true;
      scrollH = 0;
   }
   if (scrollV<0)
   {
      changed = true;
      scrollV = 0;
   }
   if (scrollH>maxScrollH)
   {
      scrollH = maxScrollH;
      changed = true;
      if (scrollV<0) scrollV = 0;
   }
   // TODO: -ve scroll for right/aligned/centred?
   //printf("Scroll %d/%d\n", scrollH, maxScrollH);
   if (scrollV>maxScrollV)
   {
      scrollV = maxScrollV;
      changed = true;
   }


   if (changed)
   {
      DirtyCache();
      if (mSelectMax > mSelectMin)
      {
         mGfxDirty = true;
      }
   }
}


void TextField::Clear()
{
   for(int i=0;i<mCharGroups.size();i++)
      mCharGroups[i].Clear();
   mCharGroups.resize(0);
   mLines.resize(0);
   maxScrollH = maxScrollV = 0;
   scrollV = scrollH = 0;
}

void TextField::setText(const std::wstring &inString)
{
   Clear();
   CharGroup chars;
   chars.mString.Set(inString.c_str(),inString.length());
   chars.mFormat = defaultTextFormat->IncRef();
   chars.mFont = 0;
   chars.mFontHeight = 0;
   mCharGroups.push_back(chars);
   mLinesDirty = true;
   mFontsDirty = true;
   mGfxDirty = true;
}

std::wstring TextField::getText()
{
   std::wstring result;
   for(int i=0;i<mCharGroups.size();i++)
      result += std::wstring(mCharGroups[i].mString.mPtr,mCharGroups[i].Chars());
   return result;
}

// TODO:
std::wstring TextField::getHTMLText()
{
   std::wstring result;
   for(int i=0;i<mCharGroups.size();i++)
      result += std::wstring(mCharGroups[i].mString.mPtr,mCharGroups[i].Chars());
   return result;
}


void TextField::AddNode(const TiXmlNode *inNode, TextFormat *inFormat,int &ioCharCount,
                        int inLineSkips,bool inBeginParagraph)
{
   for(const TiXmlNode *child = inNode->FirstChild(); child; child = child->NextSibling() )
   {
      const TiXmlText *text = child->ToText();
      if (text)
      {
         CharGroup chars;
         chars.mFormat = inFormat->IncRef();
         chars.mFont = 0;
         chars.mFontHeight = 0;
         UTF8ToWideVec(chars.mString,text->Value());
         for(int i=0;i<inLineSkips;i++)
            chars.mString.push_back('\n');
         chars.mBeginParagraph = inBeginParagraph;
         ioCharCount += chars.Chars();

         mCharGroups.push_back(chars);
         //printf(" %s %d\n", text->Value(), inLineSkips );
         inLineSkips = 0;
         inBeginParagraph = 0;
      }
      else
      {
         const TiXmlElement *el = child->ToElement();
         if (el)
         {
            inFormat->IncRef();
            TextFormat *fmt = inFormat;

            if (el->ValueTStr()=="font")
            {
               for (const TiXmlAttribute *att = el->FirstAttribute(); att;
                          att = att->Next())
               {
                  const char *val = att->Value();
                  if (att->NameTStr()=="color" && val[0]=='#')
                  {
                     int col;
                     if (sscanf(val+1,"%x",&col))
                     {
                        fmt = fmt->COW();
                        fmt->color = col;
                     }
                  }
                  else if (att->NameTStr()=="face")
                  {
                     fmt = fmt->COW();
                     fmt->font = UTF8ToWide(val);
                  }
                  else if (att->NameTStr()=="size")
                  {
                     int size;
                     if (sscanf(att->Value(),"%d",&size))
                     {
                        fmt = fmt->COW();
                        if (val[0]=='-' || val[0]=='+')
                           fmt->size = std::max( (int)fmt->size + size, 0 );
                        else
                           fmt->size = size;
                     }
                  }
               }
            }
            else if (el->ValueTStr()=="b")
            {
               if (!fmt->bold)
               {
                  fmt = fmt->COW();
                  fmt->bold = true;
               }
            }
            else if (el->ValueTStr()=="i")
            {
               if (!fmt->italic)
               {
                  fmt = fmt->COW();
                  fmt->italic = true;
               }
            }
            else if (el->ValueTStr()=="u")
            {
               if (!fmt->underline)
               {
                  fmt = fmt->COW();
                  fmt->underline = true;
               }
            }
            else if (el->ValueTStr()=="br")
               inLineSkips++;
            else if (el->ValueTStr()=="p")
            {
               if (inBeginParagraph)
                  inLineSkips++;
               else
                  inBeginParagraph = true;
            }

            for (const TiXmlAttribute *att = el->FirstAttribute(); att; att = att->Next())
            {
               //const char *val = att->Value();
               if (att->NameTStr()=="align")
               {
                  fmt = fmt->COW();
                  if (att->ValueTStr()=="left")
                     fmt->align = tfaLeft;
                  else if (att->ValueTStr()=="right")
                     fmt->align = tfaRight;
                  else if (att->ValueTStr()=="center")
                     fmt->align = tfaCenter;
                  else if (att->ValueTStr()=="justify")
                     fmt->align = tfaJustify;
               }
            }


            AddNode(child,fmt,ioCharCount,inLineSkips,inBeginParagraph);

            inFormat->DecRef();
         }
      }
   }
}


void TextField::setHTMLText(const std::wstring &inString)
{
   Clear();
   mLinesDirty = true;
   mFontsDirty = true;
   std::string str = "<top>" + WideToUTF8(inString) + "</top>";

   TiXmlNode::SetCondenseWhiteSpace(condenseWhite);
   TiXmlDocument doc;
   const char *err = doc.Parse(str.c_str(),0,TIXML_ENCODING_UTF8);
   const TiXmlNode *top =  doc.FirstChild();
   if (top)
   {
      int chars = 0;
      AddNode(top,defaultTextFormat,chars,0,0);
   }
   if (mCharGroups.empty())
      setText(L"");
}


int TextField::LineFromChar(int inChar)
{
   int min = 0;
   int max = mLines.size();

   while(min+1<max)
   {
      int mid = (min+max)/2;
      if (mLines[mid].mChar0>inChar)
         max = mid;
      else
         min = mid;
   }
   return min;
}

int TextField::GroupFromChar(int inChar)
{
   if (mCharGroups.empty()) return 0;

   int min = 0;
   int max = mCharGroups.size();
   CharGroup &last = mCharGroups[max-1];
   if (inChar>=last.mChar0)
   {
      if (inChar>=last.mChar0 + last.Chars())
         return max;
      return max-1;
   }

   while(min+1<max)
   {
      int mid = (min+max)/2;
      if (mCharGroups[mid].mChar0>inChar)
         max = mid;
      else
         min = mid;
   }
   while(min<max && mCharGroups[min].Chars()==0)
      min++;
   return min;
}



int TextField::EndOfLineX(int inLine)
{
   Line &line = mLines[inLine];
   return mCharPos[ line.mChar0 ].x + line.mMetrics.width;
}

int TextField::EndOfCharX(int inChar,int inLine)
{
   if (inLine<0 || inLine>=mLines.size() || inChar<0 || inChar>=mCharPos.size())
      return 0;
   Line &line = mLines[inLine];
   // Not last character on line?
   if (inChar < line.mChar0 + line.mChars -1 )
      return mCharPos[inChar+1].x;
   // Return end-of-line
   return mCharPos[ line.mChar0 ].x + line.mMetrics.width;
}


ImagePoint TextField::GetScrollPos()
{
   return ImagePoint(scrollH,mLines[scrollV].mY0);
}

ImagePoint TextField::GetCursorPos()
{
   ImagePoint pos(0,0);
   if (caretIndex < mCharPos.size())
      pos = mCharPos[caretIndex];
   else if (mLines.size())
   {
      pos.x = EndOfLineX( mLines.size()-1 );
      pos.y = mLines[ mLines.size() -1].mY0;
   }
   return pos;
}

void TextField::BuildBackground()
{
   Graphics &gfx = GetGraphics();
   if (mGfxDirty)
   {
      gfx.clear();
      if (mHighlightGfx)
         mHighlightGfx->clear();
      if (background || border)
      {
         int b=2;
         if (background)
            gfx.beginFill( backgroundColor, 1 );
         if (border)
            gfx.lineStyle(1, borderColor );
         gfx.moveTo(-b,-b);
         gfx.lineTo(b+boundsWidth,-b);
         gfx.lineTo(b+boundsWidth,b+boundsHeight);
         gfx.lineTo(-b,b+boundsHeight);
         gfx.lineTo(-b,-b);
      }
      //printf("%d,%d\n", mSelectMin , mSelectMax);
      if (mSelectMin < mSelectMax)
      {
         ImagePoint scroll = GetScrollPos();
         if (!mHighlightGfx)
            mHighlightGfx = new Graphics(true);

         int l0 = LineFromChar(mSelectMin);
         int l1 = LineFromChar(mSelectMax-1);
         ImagePoint pos = mCharPos[mSelectMin] - scroll;
         int height = mLines[l1].mMetrics.height;
         int x1 = EndOfCharX(mSelectMax-1,l1) - scroll.x;
         mHighlightGfx->lineStyle(-1);
         mHighlightGfx->beginFill( 0x101060, 1);
         // Special case of begin/end on same line ...
         if (l0==l1)
         {
            mHighlightGfx->drawRect(pos.x,pos.y,x1-pos.x,height);
         }
         else
         {
            mHighlightGfx->drawRect(pos.x,pos.y,EndOfLineX(l0)- scroll.x-pos.x,height);
            for(int y=l0+1;y<l1;y++)
            {
               Line &line = mLines[y];
               pos = mCharPos[line.mChar0]-scroll;
               mHighlightGfx->drawRect(pos.x,pos.y,EndOfLineX(y)-scroll.x-pos.x,line.mMetrics.height);
            }
            Line &line = mLines[l1];
            pos = mCharPos[line.mChar0]-scroll;
            mHighlightGfx->drawRect(pos.x,pos.y,x1-pos.x,line.mMetrics.height);
         }
      }
      mGfxDirty = false;
   }
}


void TextField::Render( const RenderTarget &inTarget, const RenderState &inState )
{
   if (inTarget.mPixelFormat==pfAlpha || inState.mPhase==rpBitmap)
      return;

   const Matrix &matrix = *inState.mTransform.mMatrix;
   Layout(matrix);


   RenderState state(inState);

   Rect r = mActiveRect.Rotated(mLayoutRotation).Translated(matrix.mtx,matrix.mty);
   state.mClipRect = r.Intersect(inState.mClipRect);
   if (inState.mMask)
      state.mClipRect = inState.mClipRect.Intersect(inState.mMask->GetRect());

   if (!state.mClipRect.HasPixels())
      return;


   if (inState.mPhase==rpHitTest)
   {
      // Convert destination pixels to local pixels...
      UserPoint pos =  TargetToRect(matrix,UserPoint(inState.mClipRect.x, inState.mClipRect.y) );
      if ( mActiveRect.Contains(pos) )
         inState.mHitResult = this;
      return;
   }

   ImagePoint scroll = GetScrollPos();

   BuildBackground();

   Graphics &gfx = GetGraphics();

   if (!gfx.empty())
      gfx.Render(inTarget,inState);

   UserPoint origin = RectToTarget( matrix, UserPoint(mActiveRect.x, mActiveRect.y) );
   UserPoint dPdX(1,0);
   UserPoint dPdY(0,1);
   switch(mLayoutRotation)
   {
      case 90: dPdX = UserPoint(0,-1); dPdY = UserPoint(1,0); break;
      case 180: dPdX = UserPoint(-1,0); dPdY = UserPoint(0,-1); break;
      case 270: dPdX = UserPoint(0,1); dPdY = UserPoint(-1,0); break;
   }

	bool highlight = mHighlightGfx && !mHighlightGfx->empty();
	bool caret = isInput && (( (int)(GetTimeStamp()*3)) & 1) && getStage()->GetFocusObject()==this;

	// Setup matrix for going from Rect to Target
	Matrix rect_to_target;
	if (highlight || caret)
	{
		rect_to_target.m00 = dPdX.x;
		rect_to_target.m01 = dPdY.x;
		rect_to_target.mtx = origin.x;
		rect_to_target.m10 = dPdX.y;
		rect_to_target.m11 = dPdY.y;
		rect_to_target.mty = origin.y;
		state.mTransform.mMatrix = &rect_to_target;

      if (highlight)
         mHighlightGfx->Render(inTarget,state);

		if (caret)
		{
			if (!mCaretGfx)
				mCaretGfx = new Graphics(true);
			int line = LineFromChar(caretIndex);
			if (line>=0)
			{
				int height = mLines[line].mMetrics.height;
				if (height!=mLastCaretHeight)
				{
					mLastCaretHeight = height;
					mCaretGfx->clear();
					mCaretGfx->lineStyle(1,0x000000);
					mCaretGfx->moveTo(0,0);
					mCaretGfx->lineTo(0,mLastCaretHeight);
				}

				ImagePoint pos = GetCursorPos() - scroll;
			  
				rect_to_target.TranslateData(pos.x,pos.y);
				mCaretGfx->Render(inTarget,state);
			}
		}
	}


   HardwareContext *hardware = inTarget.IsHardware() ? inTarget.mHardware : 0;

   RenderTarget clipped;
   if (hardware)
      hardware->SetViewport(state.mClipRect);
   else
      clipped = inTarget.ClipRect( state.mClipRect );


   int line = 0;
   int last_line = mLines.size()-1;
   for(int g=0;g<mCharGroups.size();g++)
   {
      CharGroup &group = mCharGroups[g];
      if (group.Chars() && group.mFont)
      {
         uint32 group_tint =
              inState.mColourTransform->Transform(group.mFormat->color(textColor) | 0xff000000);
         for(int c=0;c<group.Chars();c++)
         {
            int ch = group.mString[c];
            if (ch!='\n')
            {
               int cid = group.mChar0 + c;
               ImagePoint pos = mCharPos[cid]-scroll;
               if (pos.y>mActiveRect.h) break;
               while(line<last_line && mLines[line+1].mChar0 >= cid)
                  line++;
               pos.y += mLines[line].mMetrics.ascent;

               int a;
               Tile tile = group.mFont->GetGlyph( ch, a);
               UserPoint p = origin + dPdX*pos.x + dPdY*pos.y+ UserPoint(tile.mOx,tile.mOy);
               uint32 tint = cid>=mSelectMin && cid<mSelectMax ? 0xffffffff : group_tint;
               if (hardware)
               {
                  // todo - better to wizz though and do all of the same surface first?
                  // ok to call this multiple times with same data
                  hardware->BeginBitmapRender(tile.mSurface,tint);
                  hardware->RenderBitmap(tile.mRect, (int)p.x, (int)p.y);
               }
               else
               {
                  tile.mSurface->BlitTo(clipped,
                     tile.mRect, (int)p.x, (int)p.y,
                     bmTinted, 0,
                    (uint32)tint);
               }
            }
         }
      }
   }


   if (hardware)
      hardware->EndBitmapRender();
}

void TextField::GetExtent(const Transform &inTrans, Extent2DF &outExt,bool inForBitmap)
{
   Layout(*inTrans.mMatrix);

   if (inForBitmap && !border && !background)
   {
      Rect r = mActiveRect.Rotated(mLayoutRotation).
            Translated(inTrans.mMatrix->mtx, inTrans.mMatrix->mty);
      for(int corner=0;corner<4;corner++)
          outExt.Add( UserPoint(((corner & 1) ? r.x : r.x1()),((corner & 1) ? r.y : r.y1())));
   }
   else if (inForBitmap && border)
   {
      BuildBackground();
      return DisplayObject::GetExtent(inTrans,outExt,inForBitmap);
   }
   else
   {
      for(int corner=0;corner<4;corner++)
      {
         UserPoint pos((corner & 1) ? boundsWidth*mLayoutScaleH : 0,
                       (corner & 2) ? boundsHeight*mLayoutScaleV : 0);
         outExt.Add( RectToTarget(*inTrans.mMatrix,pos) );
      }
   }
}


void TextField::DeleteChars(int inFirst,int inEnd)
{
   inEnd = std::min(inEnd,getLength());
   if (inFirst>=inEnd)
      return;

   // Find CharGroup/Pos from char-id
   int g0 = GroupFromChar(inFirst);
   if (g0>=0 && g0<mCharGroups.size())
   {
      int g1 = GroupFromChar(inEnd-1);
      CharGroup &group0 = mCharGroups[g0];
      int del_g0 = inFirst==group0.mChar0 ? g0 : g0+1;
      group0.mString.erase( inFirst - group0.mChar0, inEnd-inFirst );
      CharGroup &group1 = mCharGroups[g1];
      int del_g1 = (inEnd ==group1.mChar0+group1.Chars())? g1+1 : g1;
      if (g0!=g1)
         group1.mString.erase( 0,inEnd - group1.mChar0);

      // Leave at least 1 group...
      if (del_g0==0 && del_g1==mCharGroups.size())
         del_g0=1;
      if (del_g0 < del_g1)
      {
         for(int g=del_g0; g<del_g1;g++)
            mCharGroups[g].Clear();
         mCharGroups.erase(del_g0, del_g1 - del_g0);
      }

      mLinesDirty = true;
      mGfxDirty = true;
      Layout(GetFullMatrix(true));
   }
}

void TextField::DeleteSelection()
{
   if (mSelectMin>=mSelectMax)
      return;
   DeleteChars(mSelectMin,mSelectMax);
   caretIndex = mSelectMin;
   mSelectMin = mSelectMax = 0;
   mSelectKeyDown = -1;
   mGfxDirty = true;
}

void TextField::InsertString(const std::wstring &inString)
{
   if (caretIndex<0) caretIndex = 0;
   caretIndex = std::min(caretIndex,getLength());

   if (caretIndex==0)
   {
      if (mCharGroups.empty())
      {
         setText(inString);
      }
      else
      {
         mCharGroups[0].mString.InsertAt(0,inString.c_str(),inString.length());
      }
   }
   else
   {
      int g = GroupFromChar(caretIndex-1);
      CharGroup &group = mCharGroups[g];
      group.mString.InsertAt( caretIndex-group.mChar0,inString.c_str(),inString.length());
   }
   caretIndex += inString.length();
   mLinesDirty = true;
   mGfxDirty = true;
   Layout(GetFullMatrix(true));
}

// Combine x,y scaling with rotation to calculate pixel coordinates for
//  each character.
void TextField::Layout(const Matrix &inMatrix)
{
   GlyphRotation rot;
   double scale_h;
   double scale_v;

   if (fabs(inMatrix.m00)> fabs(inMatrix.m01))
   {
      scale_h = fabs(inMatrix.m00);
      scale_v = fabs(inMatrix.m11);
      rot = inMatrix.m00 < 0 ? gr180 : gr0;
   }
   else
   {
      scale_h = fabs(inMatrix.m10);
      scale_v = fabs(inMatrix.m01);
      rot = inMatrix.m01 < 0 ? gr270 : gr90;
   }

   if (mFontsDirty || scale_h!=mLayoutScaleH || scale_v!=mLayoutScaleV ||
           rot!=mLayoutRotation)
   {
      for(int i=0;i<mCharGroups.size();i++)
         if (mCharGroups[i].UpdateFont(scale_v,rot,!embedFonts))
            mLinesDirty = true;
      mFontsDirty = false;
      mLayoutScaleV = scale_v;
      mLayoutScaleH = scale_h;
      mLayoutRotation = rot;
   }

   if (!mLinesDirty)
      return;

   //printf("Re-layout\n");
   // Now, layout into local co-ordinates...
   mLines.resize(0);
   mCharPos.resize(0);

   int y0 = 0;
   Line line;
   line.mY0 = y0;
   int char_count = 0;
   textHeight = 0;
   textWidth = 0;
   int x = 0;
   int y = 0;
   mLastUpDownX = -1;
   int max_x = boundsWidth * mLayoutScaleH;

   for(int i=0;i<mCharGroups.size();i++)
   {
      CharGroup &g = mCharGroups[i];
      g.mChar0 = char_count;
      int cid = 0;
      int last_word_cid = 0;
      int last_word_x = x;
      int last_word_line_chars = line.mChars;
      if ( g.mBeginParagraph && line.mChars && multiline)
      {
         g.UpdateMetrics(line.mMetrics);
         mLines.push_back(line);
         y += line.mMetrics.height;
         line.Clear();

         x = 0;
         line.mY0 = y;
         line.mChar0 = char_count;
         line.mCharGroup0 = i;
         line.mCharInGroup0 = cid;
      }


      g.UpdateMetrics(line.mMetrics);
      while(cid<g.Chars())
      {
         if (line.mChars==0)
         {
            x = 0;
            line.mY0 = y;
            line.mChar0 = char_count;
            line.mCharGroup0 = i;
            line.mCharInGroup0 = cid;
            last_word_line_chars = 0;
            last_word_cid = cid;
            last_word_x = 0;
         }

         int advance = 0;
         int ch = g.mString[cid];
         mCharPos.push_back( ImagePoint(x,y) );
         line.mChars++;
         char_count++;
         cid++;
         if ( !isalpha(ch) && !isdigit(ch) && ch!='_' )
         {
            if (isspace(ch) || line.mChars==1)
            {
               last_word_cid = cid;
               last_word_line_chars = line.mChars;
            }
            else
            {
               last_word_cid = cid-1;
               last_word_line_chars = line.mChars-1;
            }
            last_word_x = x;
         }

         if (ch=='\n')
         {
            // New line ...
            mLines.push_back(line);
            line.Clear();
            g.UpdateMetrics(line.mMetrics);
            y += g.Height();
            continue;
         }

         int ox = x;
         if (g.mFont)
            g.mFont->GetGlyph( ch, advance );
         else
            advance = 0;
         x+= advance;
         //printf(" Char %c (%d..%d,%d) %p\n", ch, ox, x, y, g.mFont);
         if ( (wordWrap||multiline) && (x > max_x) && line.mChars>1)
         {
            // No break on line so far - just back up 1 character....
            if (last_word_line_chars==0 || !wordWrap)
            {
               cid--;
               line.mChars--;
               char_count--;
               mCharPos.qpop();
               line.mMetrics.width = ox;
            }
            else
            {
               // backtrack to last break ...
               cid = last_word_cid;
               char_count-= line.mChars - last_word_line_chars;
               mCharPos.resize(char_count);
               line.mChars = last_word_line_chars;
               line.mMetrics.width = last_word_x;
            }
            mLines.push_back(line);
            y += g.Height();
            x = 0;
            line.Clear();
            g.UpdateMetrics(line.mMetrics);
            continue;
         }

         line.mMetrics.width = x;
         if (x>textWidth)
            textWidth = x;
      }
   }
   if (line.mChars || mLines.empty())
   {
      mCharGroups[mCharGroups.size()-1].UpdateMetrics(line.mMetrics);
      y += line.mMetrics.height;
      mLines.push_back(line);
   }

   textHeight = y;

   mActiveRect = Rect(0,0,boundsWidth*mLayoutScaleH+0.99,boundsHeight*mLayoutScaleV+0.99);
   int max_y = boundsHeight * mLayoutScaleV;
   if (autoSize != asNone)
   {
      if (!wordWrap)
      {
         switch(autoSize)
         {
            case asLeft: mActiveRect.w = textWidth;
                         break;
            case asRight: mActiveRect.x = mActiveRect.x1()-textWidth;
                         mActiveRect.w = textWidth;
                         break;
            case asCenter: mActiveRect.x = (mActiveRect.x+mActiveRect.x1()-textWidth)/2;
                         mActiveRect.w = textWidth;
                         break;
         }
      }
      max_y = mActiveRect.h = textHeight;
   }

   maxScrollH = std::max(0,textWidth-max_x);
   maxScrollV = 0;

   // Work out how many lines from the end fit in the rect, and
   //  therefore how many lines we can scroll...
   if (textHeight>max_y && mLines.size()>1)
   {
      int left = max_y;
      int line = mLines.size()-1;
      while(left>0 && line>0 )
        left -= mLines[line--].mMetrics.height;
      maxScrollV = line;
   }

   // Align rows ...
   for(int l=0;l<mLines.size();l++)
   {
      Line &line = mLines[l];
      int chars = line.mChars;
      if (chars>0)
      {
         CharGroup &group = mCharGroups[line.mCharGroup0];

         // Get alignment...
         int extra = (mActiveRect.w - line.mMetrics.width);
         switch(group.mFormat->align(tfaLeft))
         {
            case tfaLeft: extra = 0; break;
            case tfaCenter: extra/=2; break;
         }
         if (extra>0)
         {
            for(int c=0; c<line.mChars; c++)
               mCharPos[line.mChar0+c].x += extra;
         }
      }
   }

   mLinesDirty = false;
   int n = mCharPos.size();
   mSelectMin = std::min(mSelectMin,n);
   mSelectMax = std::min(mSelectMax,n);
   ShowCaret();
}




// --- TextFormat -----------------------------------

TextFormat::TextFormat() :
   align(tfaLeft),
   blockIndent(0),
   bold(false),
   bullet(false),
   color(0x00000000),
   font(L"Times"),
   indent(0),
   italic(false),
   kerning(false),
   leading(0),
   leftMargin(0),
   letterSpacing(0),
   rightMargin(0),
   size(12),
   tabStops( QuickVec<int>() ),
   target(L""),
   underline(false),
   url(L"")
{
}

TextFormat::~TextFormat()
{
}

TextFormat *TextFormat::COW()
{
   if (mRefCount<2)
      return this;
   TextFormat *result = new TextFormat(*this);
   result->mRefCount = 1;
   mRefCount --;
   return result;
}

TextFormat *TextFormat::Create(bool inInitRef)
{
   TextFormat *result = new TextFormat();
   if (inInitRef)
      result->IncRef();
   return result;
}

static TextFormat *sDefaultTextFormat = 0;

TextFormat *TextFormat::Default()
{
   if (!sDefaultTextFormat)
      sDefaultTextFormat = TextFormat::Create(true);
   sDefaultTextFormat->IncRef();
   return sDefaultTextFormat;
}



// --- TextFormat -----------------------------------

void CharGroup::Clear()
{
   mString.clear();
   mFormat->DecRef();
   if (mFont)
      mFont->DecRef();
}

bool CharGroup::UpdateFont(double inScale,GlyphRotation inRotation,bool inNative)
{
   int h = 0.5 + inScale*mFormat->size;
   if (!mFont || h!=mFontHeight || mFont->IsNative()!=inNative)
   {
      if (mFont)
         mFont->DecRef();
      mFont = Font::Create(*mFormat,inScale,inRotation,inNative,true);
      mFontHeight = h;
      return true;
   }
   return false;
}

void CharGroup::ApplyFormat(TextFormat *inFormat)
{
   mFormat = mFormat->COW();

   inFormat->align.Apply(mFormat->align);
   inFormat->blockIndent.Apply(mFormat->blockIndent);
   inFormat->bold.Apply(mFormat->bold);
   inFormat->bullet.Apply(mFormat->bullet);
   inFormat->color.Apply(mFormat->color);
   inFormat->font.Apply(mFormat->font);
   inFormat->indent.Apply(mFormat->indent);
   inFormat->italic.Apply(mFormat->italic);
   inFormat->kerning.Apply(mFormat->kerning);
   inFormat->leading.Apply(mFormat->leading);
   inFormat->leftMargin.Apply(mFormat->leftMargin);
   inFormat->letterSpacing.Apply(mFormat->letterSpacing);
   inFormat->rightMargin.Apply(mFormat->rightMargin);
   inFormat->size.Apply(mFormat->size);
   inFormat->tabStops.Apply(mFormat->tabStops);
   inFormat->target.Apply(mFormat->target);
   inFormat->underline.Apply(mFormat->underline);
   inFormat->url.Apply(mFormat->url);
}

} // end namespace nme

