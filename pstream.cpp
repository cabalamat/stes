/* pstream.cpp
   ===========

(c)1994-2000 Philip Hunt

Phil's stream library

Last altered:  26-Jul-2000
History:
16-Jun-94: created

11-Sep-94 PH: added trim, cont functions to PhString.

3-Oct-98 PH: convert to Linux

30-Nov-98 PH: added singleQuote(), doubleQuote()

14-Jul-2000 PH: renamed pstream.cxx -> pstream.cpp
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "pstream.h"


#define DEB 0
//debugging this module?

#define self (*this)
/***** 
To make Smalltalk programmers feel at home. Also avoids messy
syntax, eg:
   self[index]
looks better than:
   (*this)[index]
*****/
             

//--------------------------------------------------------------------

bool isWordChar(char c)
{
   return (isalnum(c) || c=='_');
}


bool isWordStartChar(char c)
{
   return (isalpha(c) || c=='_');
}


bool startsWith(const char* s, const char* t)
{
   int i=0;
   while (s[i] == t[i] || s[i] == '\0') i++;
   return (t[i] == '\0');
}


//--------------------------------------------------------------------
/* OStream: abstract superclass for output streams */

void OStream::put(const char* s)
{
   while (*s != '\0') {
      putChar(*s);
      s++;
   }  
}


void OStream::flush(){
}


OStream& operator<<(OStream& os, int i)
{
   char buf[20];
   sprintf(buf, "%d", i);
   os.put(buf);
   return os;
}


OStream& operator<<(OStream& os, long i)
{
   char buf[40];
   sprintf(buf, "%ld", i);
   os.put(buf);
   return os;
}


OStream& operator<<(OStream& os, unsigned long i)
{
   char buf[40];
   sprintf(buf, "%ld", i);
   os.put(buf);
   return os;
}


OStream& operator<<(OStream& os, double f)
{
   char buf[50];
   sprintf(buf, "%G", f);
   os.put(buf);
   return os;
}


//--------------------------------------------------------------------
/* Tee class */

Tee::Tee(OStream* dst1, OStream* dst2)
{
   dest1 = dst1;
   dest2 = dst2;
}


void Tee::addDest(OStream* dst)
{
   if (dest1 == NULL)
      dest1 = dst;
   else
      dest2 = dst;
}


void Tee::removeDests()
{
   dest1 = NULL;
   dest2 = NULL;
}


void Tee::put(const char* s)
{
   if (dest1 != NULL) dest1->put(s);
   if (dest2 != NULL) dest2->put(s);
}


void Tee::putChar(char c)
{
   if (dest1 != NULL) dest1->putChar(c);
   if (dest2 != NULL) dest2->putChar(c);
}


//--------------------------------------------------------------------
/* Output file: */


OFile::OFile(char* pathname, bool append)
{
   if (pathname==NULL) {
      // create temporary filename
      pname = tmpnam(NULL);
   } else {
      pname = pathname;
   }

   f = fopen(pname, (append ? "a" : "w"));
#if DEB
   printf("OFile: Opened '%s'\n", (char*)pname);
#endif
   if (f == NULL) {
      fprintf(stderr, "Couldn't open file '%s' for output\n",
         (char*)pname);
   } 
}


OFile::OFile(FILE* ff, char* fakePathname)
{
   f = ff;
   if (fakePathname != NULL) {
      pname = fakePathname;
   }else{
      char buf[80];
      sprintf(buf, "%p", (void*)ff);
      pname << "*** (FILE*)" << buf << " ***";
   }
}


OFile::~OFile()
{
   fclose(f);
#if DEB
   printf("OFile: Closed '%s'\n", (char*)pname);
#endif

}


void OFile::put(const char* s)
{
   fputs(s, f);
#if DEB
   printf("OFile: '%s'<<[%s]\n", (char*)pname, s);
#endif
}


void OFile::putChar(char c)
{
   if (c != '\0') {
      fputc(c, f);
#if DEB
      printf("OFile: '%s'<<'%c'\n", (char*)pname, c);
#endif
   }
}


void OFile::flush()
{
   fflush(f); 
}


OFile chout(stdout, "**stdout**");
OFile cherr(stderr, "**stderr**");

 

//--------------------------------------------------------------------
/* IStream class */


PhString IStream::getLine()
{
   PhString s;
   char c;
   while (!eof()) {
      c = get();
      s &= c;
      if (c == '\n') break;
   }
   return s;
}


//--------------------------------------------------------------------
/* IFile: input file */


IFile::IFile(char* pathname)
{
   pname = pathname;
   f = fopen(pname, "r");

   /* if (f) is NULL the file couldn't be opened and therefore is assumed
      not to exist: */
   exist = (f != NULL);
}


IFile::~IFile()
{
   if (exist) fclose(f);
}


char IFile::get()
{
   if (exist) {
      int c = fgetc(f);
      if (c == EOF) 
         return '\0';
      else
         return c;
   } else {
      return '\0';
   }
}


bool IFile::eof()
{
   return (!exist) || feof(f);
}


OStream& operator<<(OStream& os, IFile& iFile)
{
   while (!iFile.eof()) {
      os << iFile.get();
   }
   return os;
}


//--------------------------------------------------------------------
/* PeekStream */


bool PeekStream::isNext(const char* s)
{
#if DEB
   printf("PeekStream::isNext(\"%s\").\n", s);
#endif
   int i;
   for (i = 0; s[i] != '\0' ; i++) { 
#if DEB
      printf("   peek(%d)='%c'\n", i, peek(i));
#endif
      if (peek(i) !=s[i]) return false;
   }
   return true;
}


bool PeekStream::isNextSkip(const char* s)
{
   int slen = strlen(s);
   int i;
   if (slen == 0) return true;
   for (i = 0; i < slen; i++) {
      if (peek(i) !=s[i]) return false;
   }
   for (i = 0; i < slen; i++) get();
   return true;
}


bool PeekStream::isNextWord()
{
   return isWordStartChar(peek());
}


PhString PeekStream::getWord()
{
#if DEB
   printf("PeekStream::getWord()\n");
#endif
   // go to word's beginning
   while(!isNextWord() && !eof()) {
      char dummy = get();
#if DEB
      printf("PeekStream::getWord(), get() returns '%c'\n", dummy);
#endif
   }
   if (eof()) return "";

   PhString s;
   while (isWordChar(peek())) {
      char c = get();
#if DEB
      printf("PeekStream::getWord(), c='%c'\n", c);
#endif
      s &= c;
   }
   return s;
}


PhString PeekStream::getSQString()
{
   PhString s;
   skipToAfter("\'");
   for(;;) {
      char c = get();
      if (c == '\0') return s;
      if (c == '\'') {
         if (peek()=='\'') {
            get();
            s &= "\'";
         }else{
            //end of string
            return s;
         }
      }else{
         s &= c;
      }
   }//for
}


PhString PeekStream::getDQString()
{
   PhString s;
   skipToAfter("\"");
   for(;;) {
      char c = get();
      if (c == '\0') return s;
      if (c == '\"') {
         if (peek()=='\"') {
            get();
            s &= "\"";
         }else{
            //end of string
            return s;
         }
      }else{
         s &= c;
      }
   }//for
}


bool PeekStream::isNextInteger()
{
   if (isdigit(peek())) return true;
   if (peek()=='-' && isdigit(peek(1))) return true;
   return false;
}


int PeekStream::getInteger()
{
   while(!isNextInteger() && !eof()) get();
   if (eof()) return 0;

   PhString s;
   if (peek()=='-') s &= get();
   while (isdigit(peek())) s &= get();
   return atoi(s);
}


bool PeekStream::skipPast(char c)
{
   while(peek()==c && !eof()) get();
   return !eof();
}


bool PeekStream::skipPast(char* s)
{
   while(strchr(s, peek())!=NULL && !eof()) get();
   return !eof();
}


bool PeekStream::skipToAfter(char* s)
{
   skipToBefore(s);
   return isNextSkip(s);
}


bool PeekStream::skipToBefore(char* s)
{
   while (!isNext(s) && !eof()) get();
   return !eof();
}

//--------------------------------------------------------------------
/* class PeekWrapper */

//-------- inherited:

char PeekWrapper::get()
{
   char c;
   c = buf.get();

   /* if (buf) is empty, (c) will be '\0' so look in the IStream: */
   if (c == '\0') return is->get();
   return c;
}


bool PeekWrapper::eof()
{
   return (buf.eof() && is->eof());
}


char PeekWrapper::peek(int lookahead)
{
   if (lookahead < 0) return '\0';

   int index = lookahead + 1;
   /* Indexes of a PhString start from [1] */

   if (index > buf.size()) {
      int charsToAdd = index - buf.size();
      while (charsToAdd--)
         buf << is->get(); 
   }
   return buf[index];
}


//--------

PeekWrapper::PeekWrapper(IStream* istr)
{
   init(istr);
}


void PeekWrapper::init(IStream* istr)
{
   is = istr;
   buf = "";
}


//--------------------------------------------------------------------
/* class PLWrapper */

//-------- overridden virtual functions:

char PLWrapper::get()
{
   if (unread() < 1) buf << is->get();

   char result = buf[curPos++];

   if (result == '\n') {
      /* start of new line of input */
      buf.butLeft(curPos - 1);
      curPos = 1;
      curLine ++;
   }
#if DEB
   printf("PLWrapper::get() curPos=%d buf=[%s] ?",
      curPos, (char*)buf);
   getc(stdin);
#endif

   return result;
}


bool PLWrapper::eof()
{
   return (unread()==0 && is->eof());
}


char PLWrapper::peek(int lookahead)
{
   int x = curPos + lookahead;

   while (buf.size() < x) {
      if (is->eof()) break;
      buf << is->get();
   }//while

   return buf[x]; 
}


PhString PLWrapper::getCurLine()
{
   int x = curPos;

   /* make (x) point to the end of the line (not including '\n'): */ 
   for(;;) {
      /* make sure buf[x] is in range: */
      while (buf.size() < x && !is->eof())
         buf << is->get();
      if (is->eof()) break;

      if (buf[x] == '\n') {
         x--;
         break;
      }

      x++;
   }//for

   return left(buf, x);
}


int PLWrapper::getLineNum()
{
   return curLine;
}


int PLWrapper::getColNum()
{
   return curPos;
}


//--------

PLWrapper::PLWrapper(IStream* istr)
{
   curPos = 1;
   curLine = 1;
   is = istr;
}


void PLWrapper::init(IStream* istr)
{
   is = istr;
   curPos = 1;
   curLine = 1;
   if (is != NULL) 
      buf = "";
}


int PLWrapper::unread()
/* return the size of the unread portion of (buf) */
{
   return buf.size() - curPos + 1;
}


//--------------------------------------------------------------------
/* class ScanString */

char ScanString::get()
{
   char c = at(cursor);
   if (c != '\0') cursor++;
   if (c == '\n') lineNum++;
   return c;
}


bool ScanString::eof()
{
   return (cursor > str->size());
}


char ScanString::peek(int lookahead)
{
   return at(cursor + lookahead);
}


PhString ScanString::getCurLine()
{
   int f, t;

   /* make (f) point to the start of the substring containing the 
      current line: */
   f = cursor;
   while (at(f-1)!='\0' && at(f-1)!='\n') f--;

   /* now make (t) point to the end of the substring containing the 
      current line (not including the '\n'): */
   t = cursor;
   if (at(t) != '\n') {
      while (at(t+1)!='\0' && at(t+1)!='\n')
         t++;
   }

   return mid(*str, f, t-f+1);
}


int ScanString::getLineNum()
{
   return lineNum;
}


int ScanString::getColNum()
{
   int f;

   /* make (f) point to the start of the substring containing the 
      current line: */
   f = cursor;
   while (at(f-1)!='\0' && at(f-1)!='\n') f--;

   return cursor-f+1;
}


ScanString::ScanString(PhString* pstr)
{
   assert(pstr != NULL);
   str = pstr;
   cursor = 1;
   lineNum = 1;
}


ScanString::~ScanString()
{
}


char ScanString::at(int index)
{
   return (*str)[index];
}


//--------------------------------------------------------------------
/* Phil's string class */


//----- definitions of virtual functions:

char PhString::get()
{
   char c = (*this)[1];
   butLeft(1);
   return c;
}


bool PhString::eof()
{
   return (size() == 0);
}


char PhString::peek(int lookahead)
{
   int index = lookahead+1;
   return (*this)[index];
}

//----- cons/dest:
   
PhString::PhString()
{
   data = NULL;
   alloc = 0;
   to = from = 0;
}


PhString::PhString(const char* s)
{
   int len = strlen(s);
   if (len>0) {
      alloc = len+1;
      data = new char[alloc];
      strcpy(data, s);
      from = 0;
      to = len; // make (to) point to the last char in data[]
   }else{
      data = NULL;
      alloc = 0;
      to = from = 0;
   }
}


PhString::PhString(char c)
{
   alloc = allocLump;
   data = new char[alloc];
   data[0] = c;
   data[1] = '\0';
   from = 0;
   to = 1;
}


PhString::PhString(const PhString& s)
{
   int len = s.size();
   if (len>0) {
      alloc = len+1;
      data = new char[alloc];
      strcpy(data, s);
      from = 0;
      to = len;
   }else{
      data = NULL;
      alloc = 0;
      to = from = 0;
   }
}


PhString::~PhString()
{
   if (alloc > 0) delete[] data;
}


PhString& PhString::operator=(const PhString& s)
// assignment
{
   /* check we are not assigning to self.
      Note that the condition is true if either:
      1. (*this) and (s) are the same string
      2. both data areas are empty, therefore both strings are ""
         therefore no copying need take place.
   */
   if (alloc == s.alloc && data == s.data) return *this;

   int need = s.size() + 1;
   /* the number of bytes that (*this)'s data area needs to have */

   if (alloc < need) {
      /* data[] is not big enough */
      from = 0;
      to = 0;

      makeRoomToAdd(need-1);
      /* (need) is the number of bytes needed which is 1 more than the 
         number of characters needed. */

      strcpy(data, (char*)s);
      to = need-1;
   }else{
      /* data[] is big enough */

      /*...add code here to reduce size of data[] if it is too big... */

      from = 0;
      strcpy(data, (char*)s);
      to = need-1;
   }
   return *this;
}


PhString::operator char*() const
// convert to char*
{
   return (char*)(alloc>0 ? &data[from] : "");
}


void PhString::operator&=(const PhString& s)
{
   /* Warning: This routine will not work if a string is appended
      to itself (ie:   PhString s("abc"); s &= s;   does not work).
   */
#if 1
   printf("PhString::operator&=(const PhString& s)\n"
      "s=[%s] alloc=%d from=%d to=%d\n",
      (char*)s, alloc, from, to
   );
#endif
   int slen = s.size();
   makeRoomToAdd(slen);
#if DEB
   printf("slen=%d alloc=%d from=%d to=%d\n",
      slen, alloc, from, to
   );
#endif
   //strcpy(&data[to], (char*)s);
#if DEB
   char* sptr = s; 
   for(int ix = to; ix <= to+slen; ix++){
      data[ix] = *sptr++;
      printf("%d'%c'%d ", ix, data[ix], (int)data[ix]);
   }
#endif
   to += slen;
}


void PhString::operator&=(const char* s)
{
   /* Warning: This routine will not work if a string is appended
      to itself (ie:   PhString s("abc"); s &= (char*)s;   does not work).
   */
#if DEB
   printf("PhString::operator&=(const char* s)\n"
      "s=[%s] alloc=%d from=%d to=%d\n",
      s, alloc, from, to
   );
#endif
   int slen = strlen(s);
   makeRoomToAdd(slen);
   strcpy(&data[to], s);
   to += slen;
}


void PhString::operator&=(const char c)
{
   makeRoomToAdd(1);
   data[to++] = c;
   data[to] = '\0';
}


PhString PhString::operator&(const PhString& s)
{
   PhString result;
   result = *this;
   result &= s;
   return result;
}


PhString PhString::operator&(const char* s)
{
   PhString result;
   result = *this;
   result &= s;
   return result;
}


char PhString::operator[](int index) const
{
   if (index >= 1 && index <= size())
      return data[index+from-1];
   else
      return '\0';
}


//----- destructive operators on the string:

void PhString::mid(int f, int len)
{
   if (size() == 0) return;

   // normalize (len):
   if (len == 9999 || len > size()) {
      len = size();
   } else if (len < 0) {
      len = 0;
   }

   if (len == 0) {
      /* we know that alloc>0 (see above) so data[] must exist so this
         is legitimate: */
      delete[] data;
      alloc = 0;
      return;
   }
   
   if (f < 1) {
      f = 1;
   }else if (f > size()){
      f = alloc;
   }

   from = from + f -1;
   to = from + len;
   data[to] = '\0';
}


void PhString::left(int i)
{
   if (size() == 0) return;
   if (i > size()) return;
   if (i < 0) i = 0;
   to = from + i;
   data[to] = '\0';
}


void PhString::butLeft(int i)
{
   if (size() == 0) return;
   if (i <= 0) return;
   if (i > size()) i = size();
#if DEB
   printf("PhString::butLeft(), to=%d from=%d", to, from);
#endif
   from += i;
#if DEB
   printf("->%d, string=[%s]\n", from, &data[from]);
#endif
   //data[to] = '\0';
}


void PhString::right(int i)
{
#if DEB
   printf("PhString::right(), i=%d to=%d from=%d data[from]=[%s]\n", 
      i, to, from, &data[from]);
   int jj;
   for (jj=0; jj<10; jj++)
      printf("[%d]%c(%d) ", jj, data[jj], (int)data[jj]);
#endif

   if (size() == 0) return;
   if (i > size()) return;
   if (i < 0) i = 0;
   from = to - i;
   //data[to] = '\0';
}


void PhString::butRight(int i)
{
   if (size() == 0) return;
   if (i < 0) return;
   if (i > size()) i = size();
   to -= i;
   data[to] = '\0';
}


void PhString::leftTrim(char c)
{
   if (c == '\0') return;
   while (self[1] == c) from++;
}


void PhString::rightTrim(char c)
{
   if (c == '\0') return;
   while (self[size()] == c) to--;
}


void PhString::trim(char c)
{
   if (c == '\0') return;
   while (self[1] == c) from++;
   while (self[size()] == c) to--;
}


void PhString::lower()
{
   if (size() == 0) return;
   int i;
   for (i = from; i < to; i++)
      data[i] = tolower(data[i]);
}


void PhString::upper()
{
   if (size() == 0) return;
   int i;
   for (i = from; i < to; i++)
      data[i] = toupper(data[i]);
}


//----- quoting methods:


PhString singleQuote(const char * s){
   PhString result = "\'";
   char c;
   for (;;){
      c = *s++;
      if (c == '\0') break;
      if (c == '\'')
         result &= "\'\'";
	 result &= c; 
   }//for
   result &= "\'";
   return result;
}


PhString doubleQuote(const char * s){
   PhString result = "\"";
   char c;
   for (;;){
      c = *s++;
      if (c == '\0') break;
      if (c == '\"')
         result &= "\"\"";
	 result &= c; 
   }//for
   result &= "\"";
   return result;
}


//----- comparison methods:

bool PhString::cont(const char* s)
{
   return (strstr(self, s) != NULL);
}


bool PhString::contw(const char* s)
{
   /* this is not particularly efficient */

   return (strstr(::lower(self), ::lower(s)) != NULL);
}

//----- protected methods:

void PhString::makeRoomToAdd(int i)
/* This adjusts the size of data[] so that on exit there is enough room
after &data[to] to add a string whose strlen() is (i). data[] might be
resized. (from) and (to) might be altered.
*/
{
   if (alloc == 0) {
      alloc = (i+1 > allocLump ? i+1 : allocLump);
      data = new char[alloc];
      from = 0;
      to = 0;
      return;
   }

   int free = alloc - to - 1; // the amount of free room at the end of data[]
   if (free >= i) return;

   /* Can we get space by using the part of data[] before 
      &data[from] ?
      Note that we only do this if the used space in data[] is small
      compared to the total space in data[] - otherwise we would be
      copying chars in data[] very often, if the string is being
      used as a buffer.
   */
   if (free+from >= i 
       && from*4 >= alloc){
      // move the active string to the start of data[]
      memmove(&data[0], &data[from], to-from+1);
      to = to - from;
      from = 0;
   }else{
      // make data[] bigger by allocating more memory to it.
      int newSizeNeeded = alloc + i - free;
      int newSizeMin = alloc*2 + allocLump;
      int newSize = ( newSizeMin>newSizeNeeded ?
	 newSizeMin : newSizeNeeded );

      /* Warning: this assumes realloc works for blocks allocated
         with new. */
      //data = (char*)realloc((void*)data, newSize);
      char* newData = new char[newSize];
      int newTo = to - from;
      for (int ix = 0; ix <= newTo; ix++)
         newData[ix] = data[from+ix];
      delete[] data;

      data = newData;
      alloc = newSize;
      from = 0;
      to = newTo;
   }
}


//-------- method for debugging:

void PhString::printDeb() const
{
   printf("{from=%d to=%d alloc=%d", from, to, alloc);
   if (alloc > 0 && data != 0){
      int ito = from+6;
      if (ito > alloc-1) ito = alloc-1; 
      for (int i = from; i <= ito; i++){
         char c = data[i]; int ci = c;
         printf(" %d", i);
         if (ci>=32 && ci<=127)
            printf(":%c", c);
         else
            printf("<%d>", ci);
      }
   }
   printf("}\n");
}


//--------------------------------------------------------------------
/* functions operating on PhString: */


PhString operator&(const char* s1, const PhString& s2)
/* returns (s1&s2) */
{
   PhString result(s1);
   result &= s2;
   return result;
}


PhString mid(const PhString& s, int f, int len)
{
   PhString result = s;
   result.mid(f, len);
   return result;
}


PhString left(const PhString& s, int i)
{
   PhString result = s;
   result.left(i);
   return result;
}


PhString butLeft(const PhString& s, int i)
{
   PhString result = s;
   result.butLeft(i);
   return result;
}


PhString right(const PhString& s, int i)
{
#if DEB
   printf("s="); s.printDeb();
#endif
   PhString result = s;
#if DEB
   printf("result="); result.printDeb();
#endif
   result.right(i);
   return result;
}


PhString butRight(const PhString& s, int i)
{
   PhString result = s;
   result.butRight(i);
   return result;
}


PhString leftTrim(const PhString& s, char c)
{
   /* PH 11-Sep-94: these trim functions are not particularly
      efficient. Quicker would be to calculate which part of (s) to
      go into the new string, then create (result) writing into it
      only that part of (s) which is necessary. */
   PhString result = s;
   result.leftTrim(c);
   return result;
}


PhString rightTrim(const PhString& s, char c)
{
   PhString result = s;
   result.rightTrim(c);
   return result;
}


PhString trim(const PhString& s, char c)
{
   PhString result = s;
   result.trim(c);
   return result;
}


PhString lower(const PhString& s)
{
   PhString result = s;
   result.lower();
   return result;
}


PhString upper(const PhString& s)
{
   PhString result = s;
   result.upper();
   return result;
}


bool operator==(const PhString& s1, const PhString& s2)
{
   return (strcmp(s1, s2) == 0);
}

bool operator==(const PhString& s1, const char* s2)
{
   return (strcmp(s1, s2) == 0);
}

bool operator==(const char* s1, const PhString& s2)
{
   return (strcmp(s1, s2) == 0);
}

bool operator!=(const PhString& s1, const PhString& s2)
{
   return (strcmp(s1, s2) != 0);
}

bool operator!=(const PhString& s1, const char* s2)
{
   return (strcmp(s1, s2) != 0);
}

bool operator!=(const char* s1, const PhString& s2)
{
   return (strcmp(s1, s2) != 0);
}


//--------------------------------------------------------------------






/* end pstream.c */
