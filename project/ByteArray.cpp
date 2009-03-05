#include "ByteArray.h"

DEFINE_KIND( k_byte_array );



void delete_byte_array( value ba )
{
   if ( val_is_kind( ba, k_byte_array ) )
   {
      val_gc( ba, NULL );

      delete BYTEARRAY(ba);
   }
}


value nme_create_byte_array()
{
   ByteArray *b = new ByteArray;
   return b->ToValue();
}

value ByteArray::ToValue()
{
   value v = alloc_abstract( k_byte_array, this );
   val_gc( v, delete_byte_array );
   return v;
}

value nme_byte_array_length(value inArray)
{
   ByteArray *ba = BYTEARRAY(inArray);
   return alloc_int(ba->mSize);
}

value nme_byte_array_get(value inArray,value inIndex)
{
   ByteArray *ba = BYTEARRAY(inArray);
   int idx = val_int(inIndex);
   if (idx<0 || idx>=ba->mSize)
      return val_null;
   return  alloc_int(ba->mPtr[idx]);
}

value nme_byte_array_set(value inArray,value inIndex,value inValue)
{
   ByteArray *ba = BYTEARRAY(inArray);
   int idx = val_int(inIndex);
   if (idx<0)
      return val_null;
   ba->set(idx,val_int(inValue));
   return val_null;
}

value nme_read_file(value inName)
{
   FILE *file = fopen(val_string(inName),"rb");
   if (!file)
   {
      return val_null;
   }
   fseek(file,0,SEEK_END);
   int len = ftell(file);
   fseek(file,0,SEEK_SET);

   ByteArray *result = new ByteArray(len);
   fread(result->mPtr,len,1,file);
   fclose(file);

   return result->ToValue();
}


DEFINE_PRIM(nme_create_byte_array,0);
DEFINE_PRIM(nme_byte_array_length,1);
DEFINE_PRIM(nme_byte_array_get,2);
DEFINE_PRIM(nme_byte_array_set,3);
DEFINE_PRIM(nme_read_file,1);

