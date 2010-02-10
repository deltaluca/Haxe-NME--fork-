#ifndef NME_TEXT_FIELD_H
#define NME_TEXT_FIELD_H

#include "Graphics.h"
#include "QuickVec.h"
#include "Font.h"
#include "Display.h"

class TiXmlNode;

namespace nme
{

enum TextFieldType { tftDynamic, tftInput };


class TextField : public DisplayObject
{
public:
   TextField(bool inInitRef=false);

   void appendText(std::wstring inString);
   Rect getCharBoundaries(int inCharIndex);
   int getCharIndexAtPoint(double x, double y);
   int getFirstCharInParagraph(int inCharIndex);
   int getLineIndexAtPoint(double x,double y);
   int getLineIndexOfChar(int inCharIndex);
   int getLineLength(int inLineIndex);
   const TextLineMetrics &getLineMetrics(int inLineIndex);
   int getLineOffset(int inLineIndex);
   std::wstring getLineText();
   int getParagraphLength(int inCharIndex);
   TextFormat *getTextFormat(int inFirstChar=-1, int inEndChar=-1);
   bool isFontCompatible(const std::wstring &inFont, const std::wstring &inStyle);
   void replaceSelectedText(const std::wstring &inText);
   void replaceText(int inBeginIndex, int inEndIndex, const std::wstring &inText);
   int  setSelection(int inFirst, int inLast);
   void setTextFormat(const TextFormat *inFormat,int inFirstChar=-1, int inLastChar = -1);
   bool getSelectable() { return selectable; }
   void setSelectable(bool inSelectable) { selectable = inSelectable; }
   void setTextColor(int inColor);
   int  getTextColor() { return textColor; }

   const TextFormat *getDefaultTextFormat();
   void setDefaultTextFormat(TextFormat *inFormat);

   double getWidth();
   void setWidth(double inWidth);
   double getHeight();
   void setHeight(double inHeight);

   std::wstring getHTMLText();
   void setHTMLText(const std::wstring &inString);
   std::wstring getText();
   void setText(const std::wstring &inString);

   int   getBottomScrollV();
   int   getCaretIndex();
   int   getLength();
   int   getMaxScrollH();
   int   getMaxScrollV();
   int   getNumLines();
   int   getSelectionBeginIndex();
   int   getSelectionEndIndex();
   int   getTextHeight();
   int   getTextWidth();

   bool  alwaysShowSelection;
   AntiAliasType antiAliasType;
   AutoSizeMode autoSize;
   bool  background;
   ARGB  backgroundColor;
   bool  border;
   ARGB  borderColor;
   bool  condenseWhite;

   TextFormat *defaultTextFormat;
   bool  displayAsPassword;
   bool  embedFonts;
   GridFitType gridFitType;
   int  maxChars;
   bool mouseWheelEnabled;
   bool multiline;
   std::wstring restrict;
   int  scrollH;
   int  scrollV;
   bool selectable;
   float sharpness;
   struct StyleSheet *styleSheet;
   int textColor;
   float  thickness;
   TextFieldType type;
   bool useRichTextClipboard;
   bool  wordWrap;

   void Render( const RenderTarget &inTarget, const RenderState &inState );

   // Display-object like properties
   Rect mRect;

   void GetExtent(const Transform &inTrans, Extent2DF &outExt,bool inForBitmap);



protected:
   ~TextField();

private:
   TextField(const TextField &);
   void operator=(const TextField &);
   void Layout();

   void Clear();
   void AddNode(const TiXmlNode *inNode, TextFormat *inFormat, int &ioCharCount,int inLineSkips);
   void UpdateFonts(const Transform &inTransform);

   enum StringState { ssNone, ssText, ssHTML };
   StringState mStringState;
   std::wstring mUserString;

   bool mLinesDirty;
   bool mGfxDirty;
   bool mFontsDirty;
   double mLastUpdateScale;
	GlyphRotation mLastUpdateRotation;

   CharGroups mCharGroups;
   Lines mLines;
};

} // end namespace nme


#endif
