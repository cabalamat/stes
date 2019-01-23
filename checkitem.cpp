/* checkitem.cpp

Copyright (c) 2000 Philip Hunt

Implementation for CheckItem class.

Last altered: 13-Jul-2000
History:
13-Jul-2000 PH: created

*/

#include <stdio.h>
#include "crypt.h"

#include "checkitem.h"
#include "dataitem.h"

#define DEBUG 1

//--------------------------------------------------------------------

/* multiplier numbers */

uint32 mult1 = 0x0000a423; 
uint32 mult2 = 0x0000e5b1; 
uint32 mult3 = 0x0000f0cd; 

//--------------------------------------------------------------------

CheckItem::CheckItem(){
   /* set up check numbers */
   check1 = mult1 * randInt(0xffff);
   check2 = mult2 * randInt(0xffff);
   check3 = mult3 * randInt(0xffff);
   
   /* set up diset */
   diset.setSize(DA_ITEMS);
}


void CheckItem::debugInfo(){
   printf("%08x ", check1);
   printf("%08x ", check2);
   printf("%08x\n", check3);
   printf("dataSize = %d\n", dataSize);
   
   for (int i = 0; i < DA_ITEMS; i++){
      printf("%s", (diset.isSet(i)?"1" : "0"));
      if (i % 8 == 7) printf(" ");  
   }
   printf("\n");
}


void CheckItem::outDebugInfo(OStream& os){
   /* yes I know this looks ugly; I'm going to add a sprintf()-like
      function to pstream when I get round to it */
   char buf[1000];
   sprintf(buf, "%08x ", check1); os << buf;
   sprintf(buf, "%08x ", check2); os << buf;
   sprintf(buf, "%08x\n", check3); os << buf;
   sprintf(buf, "dataSize = %d\n", dataSize); os << buf;
   
   for (int i = 0; i < DA_ITEMS; i++){
      os << (diset.isSet(i) ? "1" : "0");
      if (i % 8 == 7) os << " ";  
   }
   os << "\n";
   os << "pt: "; outRaw(os, ptByteData); os << "\n";
   os << "ct: "; outRaw(os, ctByteData); os << "\n";
}


void CheckItem::loadFromPtBytes(uchar* p){
   //...to do...
}


uchar* CheckItem::getPtBytes(){
   while (ptByteData.size() < byteSize()) ptByteData.push_back(0);
   uchar* p = &ptByteData[0];
   writeBytes(p, check1);
#if DEBUG
   printf("CheckItem::getPtBytes() "
      "check1=%08x, ptByteData: %02x %02x %02x %02x\n"
      , check1, ptByteData[0], ptByteData[1], ptByteData[2], ptByteData[3]
   );
#endif
   writeBytes(p+4, check2);
   writeBytes(p+8, check3);
   writeBytes(p+12, dataSize);
   
   /* convert (diset) into array of bytes. 
      diset[0] goes into MSB (bit7) of byte p+16, 
      diset[1] goes into bit6 of byte p+16,
      diset[8] goes into bit7 of byte p+17, etc
   */
   for (int q = 0; q < (DA_ITEMS+7)/8; q++){ 
      int v = 0;
      for (int di = q*8; di <= q*8+7; di++){
         v = v << 1;
         
         /* If (di) is outside the range of valid data items, then
            ordinarily we would have some known 0s in the plaintext that
            an adversary might be able to exploit; so use random instead.
         */
         if (di < DA_ITEMS) {
            v += (int)diset.isSet(di);
         } else {
            v += randInt(0,1);
         }
      }//for di
      p[q+16] = v;
   }//for q
}


uchar* CheckItem::getCtBytes(){
   getPtBytes();
#if DEBUG
   printf("CheckItem::getCtBytes() 1.\n");
#endif
  
   /* ctByteData must be big enough: */
   while (ctByteData.size() < byteSize()) ctByteData.push_back(0);
   
   Crypt cry((unsigned char*)(char*)key);
   Crypt::Block blk;
   blk.l = 1; blk.r = 1;
#if DEBUG
   printf("CheckItem::getCtBytes() 3. ct size %d, pt size %d\n",
      ctByteData.size(), ptByteData.size());
#endif
   cry.Encrypt(&ctByteData[0], &ptByteData[0], byteSize(), blk);
#if DEBUG
   printf("CheckItem::getCtBytes() 4.\n");
#endif
   return &ctByteData[0];
}


int CheckItem::byteSize(){
   int result = 4*3  /* check1, check2, check3 */
      + 4 /* dataSize */
      + (DA_ITEMS+7)/8; /* diset */
      
   /* make it a multiple of 8 */   
   result = ((result-1)/8+1)*8;
}

//--------------------------------------------------------------------
/* decryption functions */


bool CheckItem::loadFromCtf(FILE* ctFile, 
   int ciLoc, 
   Crypt& decryptionEngine)
{ 
#if DEBUG
   printf("CheckItem::loadFromCtf(--,%d,--)\n", ciLoc);
#endif
   /* ctByteData, ptByteData must be big enough: */
   while (ctByteData.size() < byteSize()) ctByteData.push_back(0);
   while (ptByteData.size() < byteSize()) ptByteData.push_back(0);
   
   /* load encrypted text */
   fseek(ctFile, ciLoc*byteSize(), SEEK_SET);
   fread(&ctByteData[0], 1, byteSize(), ctFile);
   
   /* perform decryption */
   Crypt::Block blk;
   blk.l = 1; blk.r = 1;
   decryptionEngine.Decrypt(&ptByteData[0], &ctByteData[0], byteSize(), blk);
   
   /* write ptByteData --> check1, check2, check3, dataSize, diset */
   unpackPtData();
      
   /* do checking numbers match? */   
   return checkNumbersMatch();
}


void CheckItem::decodePlaintextToFile(FILE* ctFile, FILE* ptFile,
   Crypt& decryptionEngine)
{
   int decryptedBytes = 0; // bytes so far decrypted
   int nextDI = 0; // next unread DI
   
   while (decryptedBytes < dataSize){
      int dataToAllocate = dataSize - decryptedBytes;
      int allocNow = DI_SIZE;
      if (dataToAllocate < allocNow) allocNow = dataToAllocate;
      int diLoc = diset.getNext(nextDI);
      nextDI = diLoc + 1;
      
      DataItem di;
      di.loc = diLoc;
      int offset = CA_ITEMS*byteSize() + diLoc*DI_SIZE;
      fseek(ctFile, offset, SEEK_SET);
#if DEBUG
      printf("decodePlaintext() diLoc=%d offset=%d\n", diLoc, offset);
#endif

      di.setDataSize(allocNow);
      di.loadFromCt(ctFile);
      di.decrypt(decryptionEngine);
      di.writePtBytes(ptFile); 
      
      decryptedBytes += allocNow;
   }//while
   
}


bool CheckItem::checkNumbersMatch(){
   uint32 d;
   
   d = check1 / mult1;
   if (mult1*d != check1) return false;
   d = check2 / mult2;
   if (mult2*d != check2) return false;
   d = check3 / mult3;
   if (mult3*d != check3) return false;
   return true;
}


/* write ptByteData --> check1, check2, check3, dataSize, diset */
void CheckItem::unpackPtData(){
   uchar* p = &ptByteData[0];
   check1 = bytesToUint32(p);
   check2 = bytesToUint32(p+4);
   check3 = bytesToUint32(p+8);
   dataSize = bytesToUint32(p+12);
   
   diset.unsetAll();
   for (int di = 0; di < DA_ITEMS; di++){
      int offset = di / 8;
      int byte = ptByteData[16+offset];
      int bitInByte = 7 - (di - offset*8);
      int bitMask = 1;
      for (int j = 0; j < bitInByte; j++) bitMask <<= 1;
      bool bitIsSet = (byte & bitMask);
      diset.set(di, bitIsSet);
   }//for di
}


//--------------------------------------------------------------------

void CheckItem::outRaw(OStream& os, vector<uchar>&bd){
   int i;
   os << "(" << (int)bd.size() << ") ";
   for (i = 0; i < bd.size(); i++){
      char buf[30];
      sprintf(buf, "%02x", (int)bd[i]);
      if (i > 0 && i%4 == 0) os << " ";
      os << buf;
   };
}

//--------------------------------------------------------------------

/* end checkitem.cpp */
