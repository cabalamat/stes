/* dataitem.cpp

Copyright (c) 2000 Philip Hunt

Implements the DataItem class

Last altered: 16-Jul-2000
History:
15-Jul-2000 PH: created

*/

#include <stdio.h>
#include "setsi.h"
#include "crypt.h"

#include "dataitem.h"

#define DEBUG 1

//--------------------------------------------------------------------

DataItem::DataItem(){
   key = "";
   loc = 0;
   dataSize = 0;
   data.clear();
   dataIsEncrypted = false;
}


void DataItem::loadFromPt(FILE* ptFile){
   data.clear();
   int bytesLoaded = 0;
   while (bytesLoaded < DI_SIZE && !feof(ptFile)){
      uchar b;
      fread(&b, 1, 1, ptFile);
      data.push_back(b);
      bytesLoaded++;
   }
   dataSize = bytesLoaded;
}


void DataItem::writeCtBytes(FILE* ctFile){
#if DEBUG
   printf("DataItem::writeCtBytes() loc=%d dataSize=%d\n", 
      loc, dataSize);
#endif
   if (dataSize > 0){
      if (!dataIsEncrypted) encryptData();
      fwrite(&data[0], 1, data.size(), ctFile);
   }
   for (int i = dataSize; i < DI_SIZE; i++){
      uchar b = randInt(0, 255);
      fwrite(&b, 1, 1, ctFile);
   }
}


void DataItem::encryptData(){
#if DEBUG
   printf("DataItem::encryptData() loc=%d size=%d\n", loc, (int)data.size());
#endif

  /* pad as necessary to size is a multiple of 8 */
   while (data.size() % 8 != 0) data.push_back(randInt(0, 255));
   dataSize = data.size();
   
   Crypt::Block blk;
   blk.l = 2; blk.r = loc;
   Crypt cry((const unsigned char*)(char*)key);
   cry.Encrypt(&data[0], &data[0], data.size(), blk);
}
   
 
//--------------------------------------------------------------------
 
void DataItem::loadFromCt(FILE* ctFile){
#if DEBUG
   printf("DataItem::loadFromCt(). dataSize=%d loc=%d\n", 
      dataSize, loc);
#endif
   data.clear();
   int bytesLoaded = 0;
   while (bytesLoaded < dataSize && !feof(ctFile)){
      uchar b;
      fread(&b, 1, 1, ctFile);
      data.push_back(b);
      bytesLoaded++;
   }
   
   /* pad to a multiple of 8 */
   while (bytesLoaded % 8 != 0){
      uchar b;
      fread(&b, 1, 1, ctFile);
      data.push_back(b);
      bytesLoaded++;
   }
}


void DataItem::decrypt(Crypt& decryptionEngine){
   Crypt::Block blk;
   blk.l = 2; blk.r = loc;
   decryptionEngine.Decrypt(&data[0], data.size(), blk);
}


void DataItem::writePtBytes(FILE* ptFile){
#if DEBUG
   printf("DataItem::writePtBytes() loc=%d dataSize=%d\n", 
      loc, dataSize);
#endif
   if (dataSize > 0){
      fwrite(&data[0], 1, dataSize, ptFile);
   }
} 
 
//--------------------------------------------------------------------
/* end dataitem.cpp */
