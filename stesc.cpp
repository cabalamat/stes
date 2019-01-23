/* stesc.cpp

Copyright (c) 2000 Philip Hunt

Program to create a stes ciphertext file.

Usage:
   stesc ctf k1 pt1 k2 pt2 ... kn ptn
   
where:
   ctf = ciphertext filename (to be created)
   k1, pt1 = first key, filename of first plaintext
      (then next key/plaintext-filename pairs) 

Last altered: 16-Jul-2000
History:
12-Jul-2000 PH: created

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <vector.h>

#include "crypt.h"
#include "checkitem.h"
#include "dataitem.h"

#define DEBUG 0

/***** 
If set, this causes all CA information to be output, decrypted, 
to the file ``CA.plain''. This obviously renders encryption useless
& is intended for debugging only.
*****/
#define DEBUG_OUTPUT_CA 1

//--------------------------------------------------------------------
/* global variables */


int numKeys = 0;
char* cipherTextFilename;
vector<char*> keys;
vector<char*> plainTextFilenames;

/* which CIs are currently used/unused? */
SetSI usedCIs(CA_ITEMS);

/* information about CIs */
CheckItem ci[CA_ITEMS];

/* information about DIs */
DataItem di[DA_ITEMS];

/* which DIs are currently used/unused? */
SetSI usedDIs(DA_ITEMS);

/* which CI is key (k) using? */
int kUsesCI[CA_ITEMS];

//--------------------------------------------------------------------

void initializeVars(){
   //int i;
   
   usedCIs.unsetAll();
   usedDIs.unsetAll();


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
   cipherTextFilename = argv[1];
   
   int nextArg = 2;
   while (nextArg < argc){
      keys.push_back(argv[nextArg++]);
      plainTextFilenames.push_back(argv[nextArg++]);
#if DEBUG
      printf("nextArg=%d keys[%d]={%s} pTFn[%d]={%s}\n",
         nextArg, numKeys, keys[numKeys], 
         numKeys, plainTextFilenames[numKeys]);
#endif
      numKeys++;
   }//while
#if DEBUG
   printf("numKeys=%d\n", numKeys);
#endif
}

//--------------------------------------------------------------------

/* allocate a free CI. Return its index number. */
int allocCI(){
   int ciLoc;
   do {
      ciLoc = randInt_o(CA_ITEMS);
#if 0
      printf("allocCI(), ciLoc=%d (%d %d/%d)\n", ciLoc, CA_ITEMS,
         rand(), RAND_MAX);
#endif
   } while (usedCIs.isSet(ciLoc));
   usedCIs.set(ciLoc); // it's being used now
 
   return ciLoc;
}

/* allocate a free DI. Return its index number. */
int allocDI(){
   int diLoc;
   do {
      diLoc = randInt_o(DA_ITEMS);
   } while (usedDIs.isSet(diLoc));
   usedDIs.set(diLoc); // it's being used now
 
   return diLoc;
}


//--------------------------------------------------------------------
/* allocate all the DIs necessay for key (k) */

void allocDiForKey(int k){
   int diLoc;

   /* find out how big the file is */
   struct stat fileStatistics;
   stat(plainTextFilenames[k], &fileStatistics);
   int fileSize = fileStatistics.st_size;
   
   /* tell the CI this size */
   int ciLoc = kUsesCI[k];
   CheckItem& thisCI = ci[ciLoc];
   thisCI.dataSize = fileSize;
   
   /* work out how many DIs in the DA we need to allocate to
      hold this */
   int diNeeded = fileSize / DI_SIZE;
   int partialBytes  = fileSize % DI_SIZE;
   if (partialBytes > 0) diNeeded += 1;
#if DEBUG
   printf("allocDiForKey(%d), filename [%s] fileSize=%d diNeeded=%d\n",
      k, plainTextFilenames[k], fileSize, diNeeded);
#endif   
   
   /* allocate DIs; tell CI which DIs have been allocated */
   for (int i = 0; i < diNeeded; i++){
      diLoc = allocDI();
      thisCI.diset.set(diLoc);
      di[diLoc].key = keys[k];
      di[diLoc].loc = diLoc;
   }
   
   /* allocate data to the DIs for this key */
   if (fileSize <= 0) return;
   FILE* ptFile = fopen(plainTextFilenames[k], "r");
   int dataToAllocate = fileSize;
   int nextDI = 0; //the next unused DI
   while (dataToAllocate > 0){
      int allocNow = DI_SIZE;
      if (dataToAllocate < allocNow) allocNow = dataToAllocate;
      diLoc = thisCI.diset.getNext(nextDI);
      nextDI = diLoc + 1;
      di[diLoc].setDataSize(allocNow);
      di[diLoc].loadFromPt(ptFile);
       
      dataToAllocate -= DI_SIZE;
   }//while
   
}

//--------------------------------------------------------------------
/* write (n) random bytes to file (f). */

void fwriteRandom(int n, FILE* f){
   uchar b;
   for (int i = 0; i < n; i++){
      b = randInt(0, 255);
      fwrite(&b, 1, 1, f);
   }
}

//--------------------------------------------------------------------

int main(int argc, char** argv){
   int i;
 
   initializeVars();
   decodeArgs(argc, argv);
    
   for (int keyNum = 0; keyNum < numKeys; keyNum++){
#if DEBUG
      printf("main. (1) keyNum=%d\n", keyNum);
#endif
      /* get an unused CI location for the key */
      int ciLoc = allocCI();
      kUsesCI[keyNum] = ciLoc;
      
      /* tell the CI its key */
      ci[ciLoc].key = keys[keyNum];
#if DEBUG
      printf("main. (1.1) keyNum=%d\n", keyNum);
#endif

      /* allocate DIs */
      allocDiForKey(keyNum);
#if DEBUG
      printf("for keyNum=%d, ci is %d\n", keyNum, ciLoc);
#endif
      
#if DEBUG
      printf("main. (2)\n");
#endif
   
   }//for keyNum

   /* create ciphertext file */
   FILE* ctFile = fopen(cipherTextFilename, "w");
   
#if DEBUG_OUTPUT_CA
   OFile caPlain("CA.plain");
   caPlain << "*** writing debug info from stesc ***\n\n";
#endif   
   
   /* write CIs to ciphertext file */
   for (i = 0; i < CA_ITEMS;  i++){
#if DEBUG
      printf("main. (4) i=%d\n", i);
#endif
      if (usedCIs.isSet(i)){
         fwrite(ci[i].getCtBytes(), 1, CheckItem::byteSize(), ctFile);
      } else {
         /* unused CI, so do random bytes */
         fwriteRandom(CheckItem::byteSize(), ctFile);
      }
#if DEBUG_OUTPUT_CA
      caPlain << "\nCI #" << i << ":\n";
      ci[i].outDebugInfo(caPlain);
#endif   
   }//for
  

   /* write DIs to ciphertext file */
   for (i = 0; i < DA_ITEMS; i++){
      di[i].writeCtBytes(ctFile);
   }
   
   /* close ct file */
   fclose(ctFile);
   
#if DEBUG_OUTPUT_CA
   caPlain.flush();
#endif   
   
   return 0;
}

//--------------------------------------------------------------------

/* end stesc.cpp */
