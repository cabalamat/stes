/* checkitem.h

Copyright (c) 2000 Philip Hunt

Interface for CheckItem class. This class implements a check item in
the check area.

Last altered: 26-Jul-2000
History:
13-Jul-2000 PhilHunt: created

26-Jul-2000 PH: added loadFromCtf() function, for decryption functionality
*/

#ifndef _checkitem_h_
#define _checkitem_h_

#include <stdio.h>

#include "stesdefs.h"
#include "pstream.h"
#include "setsi.h"

//--------------------------------------------------------------------


class CheckItem {
public:
   //----- ctor, etc:
   CheckItem();
   
   //----- debugging:
   void debugInfo();
   void outDebugInfo(OStream& os);
   
   //----- functions:
   void loadFromPtBytes(uchar* p);
   uchar* getPtBytes();
   uchar* getCtBytes();
   static int byteSize();
   
   uint32 getCheck1(){ return check1;};
   uint32 getCheck2(){ return check2;};
   uint32 getCheck3(){ return check3;};
   
   //----- functions for decryption:
   bool loadFromCtf(FILE* ctFile, int ciLoc,  
      Crypt& decryptionEngine);
   void decodePlaintextToFile(FILE* ctFile, FILE* ptFile, 
      Crypt& decryptionEngine);
   bool checkNumbersMatch();
   
   //----- data:
   SetSI diset;
   PhString key;
   uint32 dataSize;
  
protected:
   uint32 check1, check2, check3;
   vector<uchar> ptByteData;
   vector<uchar> ctByteData;
   
   void unpackPtData();
   void outRaw(OStream& os, vector<uchar>&bd);
};

/*****
storeAsBytes() returns a pointer to the start of the 
the number of bytes stored, i.e. the size
of the CheckItem in the ciphertext file.

byteSize() returns the size of the CheckItem in bytes (note that
all check items in a ciphertext file will be the same size, which
will be a multiple of 8 bytes).

*****/

//--------------------------------------------------------------------

#endif

/* end checkitem.h */
