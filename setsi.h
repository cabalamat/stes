/* setsi.h

Copyright (c) 2000 Philip Hunt

A Set of Small Integers. Also some other functions.

Last altered: 16-Jul-2000
History:
13-Jul-2000 PhilHunt: created

*/

#ifndef _setsi_h_
#define _setsi_h_

#include <vector.h>
#include "stesdefs.h"

//--------------------------------------------------------------------

class SetSI {
public:
   //----- ctor, etc:
   SetSI();
   SetSI(int size);
   ~SetSI();
   void setSize(int size);
   
   //----- manipulation:
   bool isSet(int i);
   void set(int i, bool flag =true);
   void unset(int i);
   void unsetAll();
   bool inRange(int i){ return (i>=0 && i<dataSize);};
   int getNext(int afterThis =-1);
   
protected:   
   int dataSize;
   vector<bool> data;
};

/*****
(size) and (setSize) refer to the size of the set. If the size
is 10, then the set stores boolean information on integers 0 to 9.

*****/

//--------------------------------------------------------------------
/* random number generation */

int randInt(int to);
/* returns an integer in range 0..to */

int randInt(int from, int to);
/* returns an integer in range from..to */

int randInt_o(int over);
/* returns an integer in range 0..over-1 */

int randInt_o(int from, int over);
/* returns an integer in range from..over-1 */

//--------------------------------------------------------------------
/* convert to bytes */

void writeBytes(uchar* p, uint32 i);

uint32 bytesToUint32(uchar* p);


//--------------------------------------------------------------------

#endif

/* end setsi.h */
