/* pstream.h
   =========

(c)1994-2000 Philip Hunt

Phil's stream library

Last altered 26-Jul-2000
History:
14-Jun-94: created

4-Jul-94 PH: debugged PhString

11-Sep-94 PH: added trim, cont functions to PhString.

30-Nov-98 PH: added singleQuote(), doubleQuote() functions.

25-Dec-98 PH: added OStream::flush().

14-Jul-2000 PH: removed ph.h dependency

26-Jul-2000 PH: added OStream& operator<<(OStream&, unsigned long)
*/


#ifndef _pstream_h_
#define _pstream_h_

#include <stdio.h>

/* classes defined here (ab==abstract) */

class OStream; //ab
class    Tee;
class    OFile;

class IStream; //ab
class    IFile;
class    PeekStream; //ab
class       PeekWrapper;
class       PeekLine; //ab
class          PLWrapper; 
class          ScanString;
class       PhString; //also inherits from OStream


/* This list reflects the inheritance hierarchy not the order they are
defined - in the definitions OFile and IFile have been put at the end
decause they depend on PhString. */

//--------------------------------------------------------------------
/* utility functions */

bool isWordChar(char c);
bool isWordStartChar(char c);

bool startsWith(const char* s, const char* t);
/* Does the string (s) start with (t) ? */



//--------------------------------------------------------------------

class OStream {
public:
   virtual void put(const char* s);
   virtual void putChar(char c) =0;
   virtual void flush();
};


OStream& operator<<(OStream& os, int i);
OStream& operator<<(OStream& os, long i);
OStream& operator<<(OStream& os, unsigned long i);
OStream& operator<<(OStream& os, double f);


inline OStream& operator<<(OStream& os, const char* s)
{
   os.put(s); return os;
}


inline OStream& operator<<(OStream& os, char c)
{
   os.putChar(c); return os;
}

//--------------------------------------------------------------------

class Tee: public OStream {
public:
   Tee(OStream* dst1 =NULL, OStream* dst2 =NULL);
   void addDest(OStream* dst);
   void removeDests();

   virtual void put(const char* s);
   virtual void putChar(char c);

protected:
   OStream* dest1;
   OStream* dest2;
};

/*********
Tee is like the Unix program ``tee'': it sends its input to multiple
destinations.

You can add a destination for Tee either by putting it in the
constructor, or by addDest().

removeDests() removes all the destinations.

An instance of Tee sends its input to its destinations in the order that
the destinations were given to it.
put(s) sends the whole of (s) to the 1st destination, then the whole 
of (s) to the 2nd destination, etc.

This version of Tee works with a maximum of 2 destinations. it is 
undefined what happens if you try to use more destinations.
*********/

//--------------------------------------------------------------------

class IStream {
public:
   virtual char get()=0;
   virtual bool eof()=0;

   virtual PhString getLine();
};

/*********
IStream is the abstract superclass for input streams. Its subclasses
must implement get() and eof(). getLine() is defined in terms of these
functions.

get() returns the next character in the input, or '\0' if there are
no more chars.

eof() returns TRUE if at end-of-file (ie there are no more characters 
to be read in the input).
*********/

//--------------------------------------------------------------------

class PeekStream: public IStream {
public:
   virtual char peek(int lookahead =0)=0;

   bool isNext(const char* s);
   bool isNextSkip(const char* s);

   bool isNextWord();
   PhString getWord();

   PhString getSQString();
   PhString getDQString();

   bool isNextInteger();
   int getInteger();

   bool skipPast(char c);
   bool skipPast(char* s);

   bool skipToAfter(char* s);
   bool skipToBefore(char* s);
};

/*********
PeekStream is an abstract class which defines a protocol for Peekable
Streams (ie those input streams for which you can non-destructively look
ahead of the current position). Peekable streams are useful especially
when writing lexical analysers and file input routines.

A subclass of PeekStream must define peek() - a method to 
non-destructively look ahead of the current input buffer of an input
stream.

isNext(str) returns whether then next chars in the input stream 
are (str).

isNextSkip(str) does the same as isNext(str), but if there is a match
the matching chars are skipped.

isNextWord() returns whether the next chars are a C++-style identifier.

getSQString() gets a string enclosed in single quotes (eg 'a string').
If 2 adjacent single quotes appear inside the string they are output
as one ' in the return value.

getDQString() is like getSQString(), except that the string is enclosed
in double-quote characters (i.e. "); thus two double-quotes are used to
quote a double-quote.

isNextInteger() returns whether then next chars in the input stream 
are an integer.

skipPast(char c) calls get() repeatedly until peek(0) is not (c).
This and the following functions return TRUE unless reached end of input
while completing the function.

skipPast(char* s) calls get() repeatedly until peek(0) is not in (s).

skipToAfter(str) calls get() until the last chars got (after the
start of this call) were (str).

skipToBefore(str) calls get() until isNext(str) is TRUE. If isNext(str)
is already true, get() is not called.
*********/

//--------------------------------------------------------------------

class PeekLine: public PeekStream {
public:
   virtual PhString getCurLine() = 0;
   virtual int getLineNum() = 0;
   virtual int getColNum() = 0;
};

/*********
PeekLine means "a peekable stream where you can get the current line"
It is an extension of PeekStream, and offers the facility to return the
current line (ie all the characters in the line in the stream which 
includes the current position). You can also get the line number of the 
current line, and the current position's column number within the line. 
PeekLine is particularly useful if you want your program to print error 
messages like this:

   ERROR: missing operator in line 26, column 10:
   a = b + c d
   ---------^

(Note: In previous versions of this library this class was called
PeekBuffer.)

getCurLine()
returns the text of the current line, not including the '\n' at the
end of the line.

getLineNum(), getColNum() both number from 1.
*********/


//--------------------------------------------------------------------

class ScanString: public PeekLine {
public:
   //----- inherited virtual methods:
   virtual char get();
   virtual bool eof();
   virtual char peek(int lookahead =0);
   virtual PhString getCurLine();
   virtual int getLineNum();
   virtual int getColNum();

   //-----
   ScanString(PhString* pstr);
   ~ScanString();

protected:
   PhString* str;
   int cursor; // current position in (str)
   int lineNum; // current line in (str)

   // private methods:
   char at(int index);
   //{return (*str)[index];};
};

/*********
A ScanString ("string which can be scanned") is a wrapper over a String 
so you can use it like a PeekLine.

To create a ScanString:

   PhString s;
   ScanString scanner(&s);

Once (scanner) has been created, don't alter the status of (s). A
ScanString doesn't itself alter the status of the PhString it is scanning.
*********/

//--------------------------------------------------------------------
/* Phil's string class */

const int allocLump = 30; //how much data to allocate on heap


class PhString: public OStream, public PeekStream {
public:
   //----- inherited virtual methods:
   //from OStream:
   virtual void put(const char* s) { (*this) &= s; };
   virtual void putChar(char c) { (*this) &= c; };
   //from PeekStream:
   virtual char get();
   virtual bool eof();
   virtual char peek(int lookahead =0);


   //----- cons/dest:   
   PhString();
   PhString(const char* s);
   PhString(char c);
   PhString(const PhString& s);
   ~PhString();
   PhString& operator=(const PhString& s); // assignment

   operator char*() const;
   /* convert to char*. NB: do not alter the char*, or read it
      after you have changed the state of the PhString it came from. */

   int size() const
   { return (data==NULL ? 0 : to-from); };
   /* return the number of characters in the string */

   void operator&=(const PhString& s);
   void operator&=(const char* s);
   void operator&=(const char c);
   PhString operator&(const PhString& s);
   PhString operator&(const char* s);

   //----- get part of the string:
   char operator[](int index) const;
   char at(int index) const;

   //----- destructive operators on the string:
   void mid(int f, int len =9999);
   void left(int i);
   void butLeft(int i);
   void right(int i);
   void butRight(int i);

   void leftTrim(char c =' ');
   void rightTrim(char c =' ');
   void trim(char c =' ');

   void lower();
   void upper();

   //----- comparison
   bool cont(const char* s);
   bool contw(const char* s);

   //----- debugging:
   void printDeb() const;

protected:
   char* data; int alloc;
   int to, from;

   //----- internal methods:
   void makeRoomToAdd(int i);
};


PhString operator&(const char* s1, const PhString& s2);
/* returns (s1&s2) */

PhString mid(const PhString& s, int f, int len =9999);
PhString left(const PhString& s, int i);
PhString butLeft(const PhString& s, int i);
PhString right(const PhString& s, int i);
PhString butRight(const PhString& s, int i);

PhString leftTrim(const PhString& s, char c =' ');
PhString rightTrim(const PhString& s, char c =' ');
PhString trim(const PhString& s, char c =' ');

PhString lower(const PhString& s);
PhString upper(const PhString& s);

PhString singleQuote(const char * s);
PhString doubleQuote(const char * s);

bool operator==(const PhString& s1, const PhString& s2);
bool operator==(const PhString& s1, const char* s2);
bool operator==(const char* s1, const PhString& s2);

bool operator!=(const PhString& s1, const PhString& s2);
bool operator!=(const PhString& s1, const char* s2);
bool operator!=(const char* s1, const PhString& s2);


/*********
PhString - a variable-length string class

Note that the string is indexed from 1, so s[1] is the leftmost character.

Destructive operations on string
--------------------------------

Each of these methods alters its receiver. Their function is:
s.mid(f, len) changes the value to string starting at s[f], with size 
   (len). If (len) is omitted, continues to end of string. If f<1,
   (f) is treated as being 1.
s.left(i) makes it the leftmost (i) characters.
s.butLeft(i) everything but the leftmost (i) characters.
s.right(i) the rightmost (i) characters.
s.butRight(i) everything but the rightmost (i) characters.

In the above, if i>s.length(), i is treated as being s.length().

There are also non-destructive functions equivalent to these methods.
Each function has as 1st argument a PhString and returns a PhString.
Eg:   PhString left(const PhString s, int i) - this returns a value 
consisting of the lerftmost (i) characters in (s). (s) remains unchanged.

s.leftTrim(c) removes leading (c)s from the left of (s) until the leftmost
character in (s) is no longer a (c).

s.rightTrim(c) removes trailing (c)s from the right.

s.trim(c) removes leading and trailing (c)s.

lower() - makes it lower case
upper() - makes it upper case


Comparison operators
--------------------

s.cont(s2) returns true if (s) contains (s2).

s.contw(s2) returns true if (s) contains (s2) without case sensitivity (ie
when all chars in (s), (s2) are first changed to lower case.)

operator==
operator!=
These allow PhStrings to be compared with other PhStrings and char*.  


singleQuote(), doubleQuote()
----------------------------
singleQuote() returns a string consisting of the input string, enclosed
in single-quote characters (i.e. ') and where every single-quote inside
the string is replaced by to single-quote characters.
doubleQuote() is similar but uses double-quote characters for quoting
and escaping in the string.

*********/

//--------------------------------------------------------------------
/* PeekWrapper */

class PeekWrapper: public PeekStream {
public:
   //----- inherited:
   virtual char get();
   virtual bool eof();
   virtual char peek(int lookahead =0);

   //-----
   PeekWrapper(IStream* istr =0);

   void init(IStream* istr);

protected:
   IStream* is;
   PhString buf;
};

/*********
A wrapper around an IStream that makes it behave like a PeekStream.

init(istr) tells the PeekWrapper to take input from (*istr).
*********/
//--------------------------------------------------------------------

class PLWrapper: public PeekLine {
public:
   //----- inherited virtual methods:
   virtual char get();
   virtual bool eof();
   virtual char peek(int lookahead =0);
   virtual PhString getCurLine();
   virtual int getLineNum();
   virtual int getColNum();

   //-----
   PLWrapper(IStream* istr =0);

   void init(IStream* istr);

protected:
   IStream* is;
   PhString buf;
   int curPos;
   int curLine;

   int unread();
};

/*********
A wrapper around an IStream that makes it behave like a PeekLine.
Compare this class with PeekWrapper, which has the same functionality
except that PeekWrapper doesn't understand getCurLine(), getLineNum(),
or getColNum().

init(istr) tells the PLWrapper to take input from (*istr).
*********/


//--------------------------------------------------------------------
/* Output file: */

class OFile: public OStream {
public:
   OFile(char* pathname =NULL, bool append =false);
   OFile(FILE* ff, char* fakePathname =NULL);
   ~OFile();

   virtual void put(const char* s);
   virtual void putChar(char c);
   virtual void flush();

   char* pathname() {return pname;};
protected:
   FILE* f;
   PhString pname;
};

/*********
OFile is a file for output. The constructor takes as arguments the filename
(if no arg a temporary filename is used), and whether the file is opened
for append. if the file is not opened for append, and it already exists,
the existing file is deleted before the new one is written.

Note that a temporary file is not automatically deleted - you must do it 
manually.

~OFile() flushes any internal buffers.

pathname() returns the filename. This might be just the filename or
it might be the whole pathname. If it was created with just the filename,
that is what will be returned, else the full pathname will be returned.


OFile(FILE* ff, ...) constructor:
---------------------------------

OFile(FILE* ff, char* fakePathname =NULL) takes a pointer to an already 
existing file descriptor. All output is then sent to that file descriptor. 

pathname() will return whatever value was put into (fakePathname). 
If (fakePathname) is NULL, pathname() returns a message like 
"*** (FILE*)xxx ***" where xxx is the address (ff).

This constructor is useful for the predefined OFiles going to stdout 
and stderr, i.e:

   OFile chout(stdout, "**stdout**");
   OFile cherr(stderr, "**stderr**");
*********/

extern OFile chout;
extern OFile cherr;


//--------------------------------------------------------------------
/* input file */

class IFile: public IStream {
public:
   IFile(char* pathname);
   ~IFile();

   virtual char get();
   virtual bool eof();

   bool exists() { return exist;};
   char* pathname() {return pname;};

protected:
   FILE* f;
   bool exist;
   PhString pname;
};


OStream& operator<<(OStream& os, IFile& iFile);


/*********
IFile is a file for input. If the filename given in the constructor is
not a valid file, eof() will return TRUE and exists() will return FALSE.

Note the << syntax which writes everything so-far-unread in the file to
an output stream. Copying a file can be done by:

   OFile("dest") << IFile("source");

*********/

//--------------------------------------------------------------------



#endif
/* end pstream.h */
