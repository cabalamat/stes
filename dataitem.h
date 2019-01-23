/* dataitem.h

Copyright (c) 2000 Philip Hunt

Part of stes.
Holds the DataItem class.

Last altered: 16-Jul-2000
History:
15-Jul-2000 PhilHunt: created

*/

#ifndef _dataitem_h_
#define _dataitem_h_

#include <stdio.h>
#include <vector.h>
#include "crypt.h"
#include "pstream.h"

#include "stesdefs.h"

//--------------------------------------------------------------------

class DataItem {
public:
   DataItem();

   void setDataSize(int bytes) {dataSize = bytes;};
   void loadFromPt(FILE* ptFile);
   void writeCtBytes(FILE* ctFile);
   
   //----- used in decryption:
   void loadFromCt(FILE* ctFile);
   void decrypt(Crypt& decryptionEngine);
   void writePtBytes(FILE* ptFile);
   
   PhString key;
   int loc; //location in DA
   int dataSize;
   
protected:  
   vector<uchar> data; 
   bool dataIsEncrypted;
   
   void encryptData();
};

//--------------------------------------------------------------------

#endif

/* end dataitem.h */
