/* setsi.cpp

Copyright (c) 2000 Philip Hunt

Implementation of SetSI class, which is a Set of Small Integers.
Uses the STL.

Last altered: 16-Jul-2000
History:
13-Jul-2000 PH: created

*/

#include <stdio.h>
#include <stdlib.h>
#include "setsi.h"

//--------------------------------------------------------------------


SetSI::SetSI(){
   dataSize = 0;
}

SetSI::SetSI(int size){
   setSize(size);
}

SetSI::~SetSI(){
}

void SetSI::setSize(int size){
   data.reserve(size);
   while (data.size() < size) data.push_back(false);
   dataSize = size;
}

bool SetSI::isSet(int i){
   if (!inRange(i)) return false;
   return data[i];
}

void SetSI::set(int i, bool flag){
   if (!inRange(i)) return;
   data[i] = flag;
}

void SetSI::unset(int i){
   if (!inRange(i)) return;
   data[i] = false;
}

void SetSI::unsetAll(){
   for (int i = 0; i < dataSize; i++)
      data[i] = false;
}

int SetSI::getNext(int afterThis){
   for (int i = afterThis+1; i < dataSize; i++){
      if (data[i] == true) return i; /* found next set integer */
   }//for

   /* didn't find it so return not-found code */
   return -1;
}

//--------------------------------------------------------------------

/* these random-number generators might have to be made more
random */

int randInt(int to){
   return randInt_o(to+1);
}

int randInt(int from, int to){
   return from + randInt_o(to+1-from);
}

int randInt_o(int over){
   return (int) (1.0*over*rand()/(RAND_MAX+1.0)); 
}

int randInt_o(int from, int over){
   return from + randInt_o(over-from);
}

//--------------------------------------------------------------------
/* convert to bytes - big endian */

void writeBytes(uchar* p, uint32 i){
   p[0] = i >> 24;
   p[1] = (i >> 16) & 0xff;
   p[2] = (i >> 8) & 0xff;
   p[3] = i & 0xff;
}


uint32 bytesToUint32(uchar* p){
   uint32 result = 0;
   result = p[0] << 24;
   result += p[1] << 16;
   result += p[2] << 8;
   result += p[3];
   return result;
}

//--------------------------------------------------------------------

/* end setsi.cpp */
