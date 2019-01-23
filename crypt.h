////////////////////////////////////////////////////////////////////////////
///
// crypt.h
//
//    Encryption and decryption of byte strings.
//
//    CAVEATS: The last (n % 8) bytes of *buf, if any, will be untouched, so 
//    pad accordingly.  Operates in cipher block chaining mode, so the Block 
//    argument and the buffer size must be the same for Encrypt and Decrypt.

/***** modified by P.Hunt, 26-July-2000 *****/

#ifndef _crypt_h_
#define _crypt_h_

struct Crypt {

   struct Block { 
      unsigned long l, r;
      Block& operator^=(Block& b) { l ^= b.l; r ^= b.r; return *this; } 
   };

   // initialize the P and S boxes
   Crypt(const unsigned char* szKey);

   // encrypt/decrypt buffer in place
   void Encrypt(unsigned char* buf,size_t n,Block&);
   void Decrypt(unsigned char* buf,size_t n,Block&);

   // encrypt/decrypt from input buffer to output
   void Encrypt(unsigned char* out,const unsigned char* buf,size_t n,Block&);
   void Decrypt(unsigned char* out,const unsigned char* buf,size_t n,Block&);

private:
   unsigned long F(unsigned long);
   void X(Block&,const int);
   void Encrypt(Block&);
   void Decrypt(Block&);
   unsigned long P[18];
   unsigned long S[4][256];
   static const unsigned long Init_P[18];
   static const unsigned long Init_S[4][256];
};

#endif
/* end crypt.h */
