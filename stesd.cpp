/* stesd.cpp

Copyright (c) 2000 Philip Hunt

Program to decrtpy a stes ciphertext file.

Usage:
   stesc ctf k pt
   
where:
   ctf = ciphertext filename (already exists)
   k = key to use to decode ctf
   pt = filename that rthe result will be upt into

Last altered: 26-Jul-2000
History:
21-Jul-2000 PH: created

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <vector.h>

#include "crypt.h"
#include "checkitem.h"
#include "dataitem.h"

#define DEBUG 1

/***** 
If set, this causes all CA information to be output, decrypted, 
to the file ``CA.plain''. This obviously renders encryption useless
& is intended for debugging only.
*****/
#define DEBUG_OUTPUT_CA 1

//--------------------------------------------------------------------
/* global variables */

char* ctf;
char* key;
char* ptf;



//--------------------------------------------------------------------

void initializeVars(){
   //int i;
 


}

//--------------------------------------------------------------------

void usage(){
   //...to do...
}


void decodeArgs(int argc, char** argv){
#if DEBUG
   for (int ix = 0; ix < argc; ix++){
      printf("argv[%d]={%s}\n", ix, argv[ix]);
   }
#endif
   ctf = argv[1];
   key = argv[2];
   ptf = argv[3];
}

//--------------------------------------------------------------------

int main(int argc, char** argv){
   int i;
 
   initializeVars();
   decodeArgs(argc, argv);
   
   FILE* ctFile;
   ctFile = fopen(ctf, "r");
   
   /* set up decryption engine */
   Crypt decrypt((uchar*)key);
   
#if DEBUG
   OFile debug("dDebug");
   debug << "*** writing debug info from stesd ***\n\n";
#endif      
   
   /* go through each CI in turn, until one is found that successfully
      decrypts: */
   CheckItem ci;
   bool valid;
   for (i = 0; i < CA_ITEMS; i++){
      valid = ci.loadFromCtf(ctFile, i, decrypt);
#if DEBUG
      debug << "\nCI #" << i << ":\n";
      ci.outDebugInfo(debug);
#endif      
      if (valid) break;
   }//for i
 
   if (!valid) {
      printf("stesd: key `%s' not valid, aborting.\n", key);
      exit(1);
   }
  
   /* we have the CI, now read it's DI's, one at a time, to produce
      the plaintext */    
   FILE* ptFile;
   ptFile = fopen(ptf, "w");    
   ci.decodePlaintextToFile(ctFile, ptFile, decrypt);
   
   fclose(ptFile);
   fclose(ctFile);
   exit(0);
}

//--------------------------------------------------------------------

/* end stesd.cpp */
