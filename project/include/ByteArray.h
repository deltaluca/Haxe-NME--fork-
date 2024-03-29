#ifndef NME_BYTE_ARRAY_H
#define NME_BYTE_ARRAY_H

#include "Object.h"
#include "QuickVec.h"
#include "Utils.h"

namespace nme
{


// If you put this structure on the stack, then you do not have to worry about GC.
// If you store this in a heap structure, then you will need to use GC roots for mValue...
struct ByteArray
{
   ByteArray(int inSize);
   ByteArray(const ByteArray &inRHS);
   ByteArray();
   ByteArray(struct _value *Value);
   ByteArray(const QuickVec<unsigned char>  &inValue);

   void          Resize(int inSize);
   int           Size();
   unsigned char *Bytes();
   bool          Ok() { return mValue!=0; }


   struct _value *mValue;

   static ByteArray FromFile(const OSChar *inFilename);
   #ifdef HX_WINDOWS
   static ByteArray FromFile(const char *inFilename);
   #endif
};

#ifdef ANDROID
ByteArray AndroidGetAssetBytes(const char *);
#endif

}

#endif
