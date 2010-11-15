#ifndef NME_SURFACE_H
#define NME_SURFACE_H

#include <Graphics.h>
#include <Utils.h>

// ---- Surface API --------------

namespace nme
{


void HintColourOrder(bool inRedFirst);

extern int gTextureContextVersion;

class Texture
{
public:
   Texture() : mContextVersion(gTextureContextVersion) { }
   virtual ~Texture() {};
   virtual void Bind(class Surface *inSurface,int inSlot)=0;
   virtual void BindFlags(bool inRepeat,bool inSmooth)=0;
   virtual UserPoint PixelToTex(const UserPoint &inPixels)=0;
   virtual UserPoint TexToPaddedTex(const UserPoint &inPixels)=0;

   void Dirty(const Rect &inRect);
   bool IsDirty() { return mDirtyRect.HasPixels(); }
   bool IsCurrentVersion() { return mContextVersion==gTextureContextVersion; }

   Rect mDirtyRect;
   int  mContextVersion;
};

class Surface : public Object
{
public:
   Surface() : mTexture(0), mVersion(0) { };

   // Implementation depends on platform.
   static Surface *Load(const OSChar *inFilename);
   static Surface *LoadFromBytes(const uint8 *inBytes,int inLen);

   Surface *IncRef() { mRefCount++; return this; }

   virtual int Width() const =0;
   virtual int Height() const =0;
   virtual PixelFormat Format()  const = 0;
   virtual const uint8 *GetBase() const = 0;
   virtual int GetStride() const = 0;

   virtual void Clear(uint32 inColour,const Rect *inRect=0) = 0;
   virtual void Zero() { Clear(0); }

   int BytesPP() const { return Format()==pfAlpha ? 1 : 4; }
   const uint8 *Row(int inY) const { return GetBase() + GetStride()*inY; }

   virtual RenderTarget BeginRender(const Rect &inRect)=0;
   virtual void EndRender()=0;

   virtual void BlitTo(const RenderTarget &outTarget, const Rect &inSrcRect,int inPosX, int inPosY,
                       BlendMode inBlend, const BitmapCache *inMask,
                       uint32 inTint=0xffffff ) const = 0;
   virtual void StretchTo(const RenderTarget &outTarget,
                          const Rect &inSrcRect, const Rect &inDestRect) const = 0;

   Texture *GetTexture() { return mTexture; }
   Texture *GetOrCreateTexture(HardwareContext &inHardware);
   void Bind(HardwareContext &inHardware,int inSlot=0);

   virtual Surface *clone() { return 0; }
   virtual void getPixels(const Rect &inRect,uint32 *outPixels,bool inIgnoreOrder=false) { }
   virtual void setPixels(const Rect &inRect,const uint32 *intPixels,bool inIgnoreOrder=false) { }
   virtual void getColorBoundsRect(int inMask, int inCol, bool inFind, Rect &outRect)
   {
      outRect = Rect(0,0,Width(),Height());
   }
   virtual uint32 getPixel(int inX,int inY) { return 0; }
   virtual void setPixel(int inX,int inY,uint32 inRGBA,bool inAlphaToo=false) { }
   virtual void scroll(int inDX,int inDY) { }
   int Version() const  { return mVersion; }

protected:
   mutable int   mVersion;
   Texture       *mTexture;
   virtual       ~Surface();
};

// Helper class....
class AutoSurfaceRender
{
   Surface *mSurface;
   RenderTarget mTarget;
public:
   AutoSurfaceRender(Surface *inSurface) : mSurface(inSurface),
       mTarget(inSurface->BeginRender( Rect(inSurface->Width(),inSurface->Height()) ) ) { }
   AutoSurfaceRender(Surface *inSurface,const Rect &inRect) : mSurface(inSurface),
       mTarget(inSurface->BeginRender(inRect)) { }
   ~AutoSurfaceRender() { mSurface->EndRender(); }
   const RenderTarget &Target() { return mTarget; }

};

class SimpleSurface : public Surface
{
public:
   SimpleSurface(int inWidth,int inHeight,PixelFormat inPixelFormat,int inByteAlign=4);

   int Width() const  { return mWidth; }
   int Height() const  { return mHeight; }
   PixelFormat Format() const  { return mPixelFormat; }
   void Clear(uint32 inColour,const Rect *inRect);
   void Zero();


   RenderTarget BeginRender(const Rect &inRect);
   void EndRender();

   virtual void BlitTo(const RenderTarget &outTarget, const Rect &inSrcRect,int inPosX, int inPosY,
                       BlendMode inBlend, const BitmapCache *inMask,
                       uint32 inTint=0xffffff ) const;

   virtual void StretchTo(const RenderTarget &outTarget,
                          const Rect &inSrcRect, const Rect &inDestRect) const;

   const uint8 *GetBase() const { return mBase; }
   int GetStride() const { return mStride; }
   Surface *clone();
   void getPixels(const Rect &inRect,uint32 *outPixels,bool inIgnoreOrder=false);
   void setPixels(const Rect &inRect,const uint32 *intPixels,bool inIgnoreOrder=false);
   void getColorBoundsRect(int inMask, int inCol, bool inFind, Rect &outRect);
   uint32 getPixel(int inX,int inY);
   void setPixel(int inX,int inY,uint32 inRGBA,bool inAlphaToo=false);
   void scroll(int inDX,int inDY);


protected:
   int           mWidth;
   int           mHeight;
   PixelFormat   mPixelFormat;
   int           mStride;
   uint8         *mBase;
   ~SimpleSurface();

private:
   SimpleSurface(const SimpleSurface &inRHS);
   void operator=(const SimpleSurface &inRHS);
};

class HardwareSurface : public Surface
{
public:
   HardwareSurface(HardwareContext *inContext);

   int Width() const { return mHardware->Width(); }
   int Height() const { return mHardware->Height(); }
   PixelFormat Format()  const { return pfHardware; }
   const uint8 *GetBase() const { return 0; }
   int GetStride() const { return 0; }
   void Clear(uint32 inColour,const Rect *inRect=0) { mHardware->Clear(inColour,inRect); }
   RenderTarget BeginRender(const Rect &inRect)
   {
      mHardware->BeginRender(inRect);
      return RenderTarget(inRect,mHardware);
   }
   void EndRender() { }

   void BlitTo(const RenderTarget &outTarget, const Rect &inSrcRect,int inPosX, int inPosY,
                       BlendMode inBlend, const BitmapCache *inMask,
                       uint32 inTint ) const { }
   void StretchTo(const RenderTarget &outTarget,
                          const Rect &inSrcRect, const Rect &inDestRect) const { }
   Surface *clone();
   void getPixels(const Rect &inRect,uint32 *outPixels,bool inIgnoreOrder=false);
   void setPixels(const Rect &inRect,const uint32 *intPixels,bool inIgnoreOrder=false);

   protected:
      ~HardwareSurface();
   private:
      HardwareContext *mHardware;
};


} // end namespace nme


#endif
