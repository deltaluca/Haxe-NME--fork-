#ifndef QUICK_VEC
#define QUICK_VEC

#include <algorithm>

// Little vector/set class, optimised for small data and not using many malloc calls.
// Data are allocated with "malloc", so they should not rely on constructors etc.
template<typename T_,int QBUF_SIZE_=16>
class QuickVec
{
   enum { QBufSize = QBUF_SIZE_ };
public:
   typedef T_ *iterator;
   typedef const T_ * const_iterator;

public:
   QuickVec()
   {
      mPtr = mQBuf;
      mAlloc = QBufSize;
      mSize = 0;
   }
   QuickVec(const QuickVec<T_,QBUF_SIZE_> &inRHS)
   {
      if (inRHS.mSize<=QBufSize)
      {
         mAlloc = QBufSize;
         mPtr = mQBuf;
      }
      else
      {
         mAlloc = inRHS.mAlloc;
         mPtr = (T_ *)malloc(mAlloc * sizeof(T_));
      }
      mSize = inRHS.mSize;
      memcpy(mPtr,inRHS.mPtr,sizeof(T_)*mSize);
   }
   ~QuickVec()
   {
      if (mPtr!=mQBuf)
         free(mPtr);
   }

   // This assumes the values in the array are sorted.
   void Toggle(T_ inValue)
   {
      if (mSize==0)
      {
         mPtr[mSize++] = inValue;
         return;
      }

      int was = mPtr[0];

      // Before/at start
      if (inValue<=mPtr[0])
      {
         if (inValue==mPtr[0])
            EraseAt(0);
         else
            InsertAt(0,inValue);
      }
      else
      {
         int last = mSize-1;
         // After/on end
         if (inValue>=mPtr[last])
         {
            if (inValue==mPtr[last])
               EraseAt(last);
            else
            {
               Grow();
               mPtr[mSize] = inValue;
               ++mSize;
            }
         }
         else
         {
            // between 0 ... last
            int min = 0;
            int max = last;

            while(max>min+1)
            {
               int middle = (max+min+1)/2;
               T_ v = mPtr[middle];
               if (v==inValue)
               {
                  EraseAt(middle);
                  return;
               }
               if (v<inValue)
                  min = middle;
               else
                  max = middle;
            }
            // Not found, must be between min and max (=min+1)
            InsertAt(min+1,inValue);
         }
      }
   }

   inline void Grow()
   {
      if (mSize>=mAlloc)
      {
         if (mPtr==mQBuf)
         {
            mPtr = (T_ *)malloc(sizeof(T_)*(QBufSize*2));
            memcpy(mPtr, mQBuf, sizeof(mQBuf));
            mAlloc = QBufSize*2;
         }
         else
         {
            mAlloc *= 2;
            mPtr = (T_*)realloc(mPtr, sizeof(T_)*mAlloc);
         }
      }
   }
   void reserve(int inSize)
   {
      if (mAlloc<inSize && inSize>QBufSize)
      {
         mAlloc = inSize;

         if (mPtr!=mQBuf)
         {
            mPtr = (T_ *)realloc(mPtr,sizeof(T_)*mAlloc);
         }
         else
         {
            T_ *buf = (T_ *)malloc(sizeof(T_)*mAlloc);
            memcpy(buf,mPtr,mSize*sizeof(T_));
            mPtr = buf;
         }
      }
   }
   void resize(int inSize)
   {
      if (mAlloc<inSize)
      {
         if (mPtr==mQBuf)
         {
            mAlloc = inSize;
            mPtr = (T_ *)malloc(sizeof(T_)*(mAlloc));
            memcpy(mPtr, mQBuf, sizeof(T_)*mSize);
         }
         else
         {
            mAlloc = inSize;
            mPtr = (T_*)realloc(mPtr, sizeof(T_)*mAlloc);
         }

      }
      mSize = inSize;
   }
   inline void push_back(const T_ &inVal)
   {
      Grow();
      mPtr[mSize++] = inVal;
   }
   inline void EraseAt(int inPos)
   {
      memmove(mPtr + inPos, mPtr + inPos + 1, (mSize-inPos-1) * sizeof(T_) );
      --mSize;
   }
   inline void EraseAt(int inFirst,int inLast)
   {
      memmove(mPtr + inFirst, mPtr + inLast, (mSize-inLast) * sizeof(T_) );
      mSize -= inLast-inFirst;
   }

   inline void InsertAt(int inPos,const T_ &inValue)
   {
      Grow();
      memmove(mPtr + inPos + 1, mPtr + inPos, (mSize-inPos) * sizeof(T_) );
      mPtr[inPos] = inValue;
      ++mSize;
   }

   inline int size() const { return mSize; }
   inline bool empty() const { return mSize==0; }
   inline T_& operator[](int inIndex) { return mPtr[inIndex]; }
   inline const T_& operator[](int inIndex) const { return mPtr[inIndex]; }
   inline iterator begin() { return mPtr; }
   inline iterator rbegin() { return mPtr + mSize -1; }
   inline iterator end() { return mPtr + mSize; }
   inline const_iterator begin() const { return mPtr; }
   inline const_iterator rbegin() const { return mPtr + mSize - 1; }
   inline const_iterator end() const { return mPtr + mSize; }
   void swap( QuickVec<T_,QBUF_SIZE_> &inRHS )
   {
      if (mPtr!=mQBuf)
      {
         // Both "real" pointers - just swap them
         if (inRHS.mPtr!=inRHS.mQBuf)
         {
            std::swap(mPtr,inRHS.mPtr);
         }
         else
         {
            // RHS in in the qbuf, we have a pointer
            memcpy(mQBuf,inRHS.mQBuf,inRHS.mSize*sizeof(T_));
            inRHS.mPtr = mPtr;
            mPtr = mQBuf;
         }
      }
      else
      {
         // We have a qbuf, rhs has a pointer
         if (inRHS.mPtr!=inRHS.mQBuf)
         {
            memcpy(inRHS.mQBuf,mQBuf,mSize*sizeof(T_));
            mPtr = inRHS.mPtr;
            inRHS.mPtr = inRHS.mQBuf;
         }
         else
         {
            // Both using QBuf ...
            if (mSize && inRHS.mSize)
            {
               T_ tmp[QBufSize];
               memcpy(tmp,mPtr,mSize*sizeof(T_));
               memcpy(mPtr,inRHS.mPtr,inRHS.mSize*sizeof(T_));
               memcpy(inRHS.mPtr,tmp,mSize*sizeof(T_));
            }
            else if (mSize)
               memcpy(inRHS.mQBuf,mQBuf,mSize*sizeof(T_));
            else
               memcpy(mQBuf,inRHS.mQBuf,inRHS.mSize*sizeof(T_));
         }
      }

      std::swap(mAlloc,inRHS.mAlloc);
      std::swap(mSize,inRHS.mSize);
   }

   QuickVec<T_,QBUF_SIZE_> &operator=(const QuickVec<T_,QBUF_SIZE_> &inRHS)
   {
      if (mPtr!=mQBuf)
         free(mPtr);
      if (inRHS.mSize<=QBufSize)
      {
         mPtr = mQBuf;
         mAlloc = QBufSize;
      }
      else
      {
         mAlloc = inRHS.mAlloc;
         mPtr = (T_ *)malloc( mAlloc * sizeof(T_) );
      }
      mSize = inRHS.mSize;
      memcpy(mPtr,inRHS.mPtr,mSize*sizeof(T_));
      return *this;
   }


   T_  *mPtr;
   T_  mQBuf[QBufSize];
   int mAlloc;
   int mSize;

};

#endif