/* stesdefs.h

Copyright (c) 2000 Philip Hunt

Common defintions for stes project.

Last altered: 13-Jul-2000
History:
13-Jul-2000 PhilHunt: created

*/

#ifndef _stesdefs_h_
#define _stesdefs_h_

//--------------------------------------------------------------------

/* the number of items in the Check Area */
const int CA_ITEMS = 30;

/* the number of items in the Data Area */
const int DA_ITEMS = 50;

/* the size, in bytes, of an item in the Data Area (a "Data Item").
Note that this must be a multiple of 8 (a requirement of the blowfish
algorithm).
*/
const int DI_SIZE = 1024;


typedef unsigned long uint32;
typedef unsigned char uchar;

//--------------------------------------------------------------------
#endif
/* end stesdefs.h */
