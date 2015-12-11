/*
* Botan 1.11.25 Amalgamation
* (C) 1999-2013,2014,2015 Jack Lloyd and others
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "botan_all.h"
#include "botan_all_internal.h"

/*
* (C) 2013,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_AEAD_CCM)
#endif

#if defined(BOTAN_HAS_AEAD_CHACHA20_POLY1305)
#endif

#if defined(BOTAN_HAS_AEAD_EAX)
#endif

#if defined(BOTAN_HAS_AEAD_GCM)
#endif

#if defined(BOTAN_HAS_AEAD_OCB)
#endif

#if defined(BOTAN_HAS_AEAD_SIV)
#endif

namespace Botan {

AEAD_Mode::~AEAD_Mode() {}

#if defined(BOTAN_HAS_AEAD_CCM)
BOTAN_REGISTER_BLOCK_CIPHER_MODE_LEN2(CCM_Encryption, CCM_Decryption, 16, 3);
#endif

#if defined(BOTAN_HAS_AEAD_CHACHA20_POLY1305)
BOTAN_REGISTER_TRANSFORM_NOARGS(ChaCha20Poly1305_Encryption);
BOTAN_REGISTER_TRANSFORM_NOARGS(ChaCha20Poly1305_Decryption);
#endif

#if defined(BOTAN_HAS_AEAD_EAX)
BOTAN_REGISTER_BLOCK_CIPHER_MODE_LEN(EAX_Encryption, EAX_Decryption, 0);
#endif

#if defined(BOTAN_HAS_AEAD_GCM)
BOTAN_REGISTER_BLOCK_CIPHER_MODE_LEN(GCM_Encryption, GCM_Decryption, 16);
#endif

#if defined(BOTAN_HAS_AEAD_OCB)
BOTAN_REGISTER_BLOCK_CIPHER_MODE_LEN(OCB_Encryption, OCB_Decryption, 16);
#endif

#if defined(BOTAN_HAS_AEAD_SIV)
BOTAN_REGISTER_BLOCK_CIPHER_MODE(SIV_Encryption, SIV_Decryption);
#endif

AEAD_Mode* get_aead(const std::string& algo_spec, Cipher_Dir direction)
   {
   std::unique_ptr<Cipher_Mode> mode(get_cipher_mode(algo_spec, direction));

   if(AEAD_Mode* aead = dynamic_cast<AEAD_Mode*>(mode.get()))
      {
      mode.release();
      return aead;
      }

   return nullptr;
   }

}
/*
* AES
* (C) 1999-2010,2015 Jack Lloyd
*
* Based on the public domain reference implementation by Paulo Baretto
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


/*
* This implementation is based on table lookups which are known to be
* vulnerable to timing and cache based side channel attacks. Some
* countermeasures are used which may be helpful in some situations:
*
* - Small tables are used in the first and last rounds.
*
* - The TE and TD tables are computed at runtime to avoid flush+reload
*   attacks using clflush. As different processes will not share the
*   same underlying table data, an attacker can't manipulate another
*   processes cache lines via their shared reference to the library
*   read only segment.
*
* - Each cache line of the lookup tables is accessed at the beginning
*   of each call to encrypt or decrypt. (See the Z variable below)
*
* If available SSSE3 or AES-NI are used instead of this version, as both
* are faster and immune to side channel attacks.
*
* Some AES cache timing papers for reference:
*
* "Software mitigations to hedge AES against cache-based software side
* channel vulnerabilities" https://eprint.iacr.org/2006/052.pdf
*
* "Cache Games - Bringing Access-Based Cache Attacks on AES to Practice"
* http://www.ieee-security.org/TC/SP2011/PAPERS/2011/paper031.pdf
*
* "Cache-Collision Timing Attacks Against AES" Bonneau, Mironov
* http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.88.4753
*/

namespace Botan {

namespace {

const byte SE[256] = {
   0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B,
   0xFE, 0xD7, 0xAB, 0x76, 0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0,
   0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0, 0xB7, 0xFD, 0x93, 0x26,
   0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
   0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2,
   0xEB, 0x27, 0xB2, 0x75, 0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0,
   0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84, 0x53, 0xD1, 0x00, 0xED,
   0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
   0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F,
   0x50, 0x3C, 0x9F, 0xA8, 0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5,
   0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2, 0xCD, 0x0C, 0x13, 0xEC,
   0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
   0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14,
   0xDE, 0x5E, 0x0B, 0xDB, 0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C,
   0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79, 0xE7, 0xC8, 0x37, 0x6D,
   0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
   0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F,
   0x4B, 0xBD, 0x8B, 0x8A, 0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E,
   0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E, 0xE1, 0xF8, 0x98, 0x11,
   0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
   0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F,
   0xB0, 0x54, 0xBB, 0x16 };

const byte SD[256] = {
   0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E,
   0x81, 0xF3, 0xD7, 0xFB, 0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87,
   0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB, 0x54, 0x7B, 0x94, 0x32,
   0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
   0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49,
   0x6D, 0x8B, 0xD1, 0x25, 0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16,
   0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92, 0x6C, 0x70, 0x48, 0x50,
   0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
   0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05,
   0xB8, 0xB3, 0x45, 0x06, 0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02,
   0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B, 0x3A, 0x91, 0x11, 0x41,
   0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
   0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8,
   0x1C, 0x75, 0xDF, 0x6E, 0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89,
   0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B, 0xFC, 0x56, 0x3E, 0x4B,
   0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
   0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59,
   0x27, 0x80, 0xEC, 0x5F, 0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D,
   0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF, 0xA0, 0xE0, 0x3B, 0x4D,
   0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
   0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63,
   0x55, 0x21, 0x0C, 0x7D };

inline byte xtime(byte s) { return (s << 1) ^ ((s >> 7) * 0x1B); }
inline byte xtime4(byte s) { return xtime(xtime(s)); }
inline byte xtime8(byte s) { return xtime(xtime(xtime(s))); }

inline byte xtime3(byte s) { return xtime(s) ^ s; }
inline byte xtime9(byte s) { return xtime8(s) ^ s; }
inline byte xtime11(byte s) { return xtime8(s) ^ xtime(s) ^ s; }
inline byte xtime13(byte s) { return xtime8(s) ^ xtime4(s) ^ s; }
inline byte xtime14(byte s) { return xtime8(s) ^ xtime4(s) ^ xtime(s); }

const std::vector<u32bit>& AES_TE()
   {
   auto compute_TE = []() {
      std::vector<u32bit> TE(1024);
      for(size_t i = 0; i != 256; ++i)
         {
         const byte s = SE[i];
         const u32bit x = make_u32bit(xtime(s), s, s, xtime3(s));

         TE[i] = x;
         TE[i+256] = rotate_right(x, 8);
         TE[i+512] = rotate_right(x, 16);
         TE[i+768] = rotate_right(x, 24);
         }
      return TE;
   };

   static std::vector<u32bit> TE = compute_TE();
   return TE;
   }

const std::vector<u32bit>& AES_TD()
   {
   auto compute_TD = []() {
      std::vector<u32bit> TD(1024);
      for(size_t i = 0; i != 256; ++i)
         {
         const byte s = SD[i];
         const u32bit x = make_u32bit(xtime14(s), xtime9(s), xtime13(s), xtime11(s));

         TD[i] = x;
         TD[i+256] = rotate_right(x, 8);
         TD[i+512] = rotate_right(x, 16);
         TD[i+768] = rotate_right(x, 24);
         }
      return TD;
   };
   static std::vector<u32bit> TD = compute_TD();
   return TD;
   }

/*
* AES Encryption
*/
void aes_encrypt_n(const byte in[], byte out[],
                   size_t blocks,
                   const secure_vector<u32bit>& EK,
                   const secure_vector<byte>& ME)
   {
   BOTAN_ASSERT(EK.size() && ME.size() == 16, "Key was set");

   const size_t cache_line_size = CPUID::cache_line_size();

   const std::vector<u32bit>& TE = AES_TE();

   // Hit every cache line of TE
   u32bit Z = 0;
   for(size_t i = 0; i < TE.size(); i += cache_line_size / sizeof(u32bit))
      {
      Z |= TE[i];
      }
   Z &= TE[82]; // this is zero, which hopefully the compiler cannot deduce

   for(size_t i = 0; i != blocks; ++i)
      {
      u32bit T0 = load_be<u32bit>(in, 0) ^ EK[0];
      u32bit T1 = load_be<u32bit>(in, 1) ^ EK[1];
      u32bit T2 = load_be<u32bit>(in, 2) ^ EK[2];
      u32bit T3 = load_be<u32bit>(in, 3) ^ EK[3];

      T0 ^= Z;

      /* Use only the first 256 entries of the TE table and do the
      * rotations directly in the code. This reduces the number of
      * cache lines potentially used in the first round from 64 to 16
      * (assuming a typical 64 byte cache line), which makes timing
      * attacks a little harder; the first round is particularly
      * vulnerable.
      */

      u32bit B0 = TE[get_byte(0, T0)] ^
                  rotate_right(TE[get_byte(1, T1)],  8) ^
                  rotate_right(TE[get_byte(2, T2)], 16) ^
                  rotate_right(TE[get_byte(3, T3)], 24) ^ EK[4];

      u32bit B1 = TE[get_byte(0, T1)] ^
                  rotate_right(TE[get_byte(1, T2)],  8) ^
                  rotate_right(TE[get_byte(2, T3)], 16) ^
                  rotate_right(TE[get_byte(3, T0)], 24) ^ EK[5];

      u32bit B2 = TE[get_byte(0, T2)] ^
                  rotate_right(TE[get_byte(1, T3)],  8) ^
                  rotate_right(TE[get_byte(2, T0)], 16) ^
                  rotate_right(TE[get_byte(3, T1)], 24) ^ EK[6];

      u32bit B3 = TE[get_byte(0, T3)] ^
                  rotate_right(TE[get_byte(1, T0)],  8) ^
                  rotate_right(TE[get_byte(2, T1)], 16) ^
                  rotate_right(TE[get_byte(3, T2)], 24) ^ EK[7];

      for(size_t r = 2*4; r < EK.size(); r += 2*4)
         {
         T0 = EK[r  ] ^ TE[get_byte(0, B0)      ] ^ TE[get_byte(1, B1) + 256] ^
                        TE[get_byte(2, B2) + 512] ^ TE[get_byte(3, B3) + 768];
         T1 = EK[r+1] ^ TE[get_byte(0, B1)      ] ^ TE[get_byte(1, B2) + 256] ^
                        TE[get_byte(2, B3) + 512] ^ TE[get_byte(3, B0) + 768];
         T2 = EK[r+2] ^ TE[get_byte(0, B2)      ] ^ TE[get_byte(1, B3) + 256] ^
                        TE[get_byte(2, B0) + 512] ^ TE[get_byte(3, B1) + 768];
         T3 = EK[r+3] ^ TE[get_byte(0, B3)      ] ^ TE[get_byte(1, B0) + 256] ^
                        TE[get_byte(2, B1) + 512] ^ TE[get_byte(3, B2) + 768];

         B0 = EK[r+4] ^ TE[get_byte(0, T0)      ] ^ TE[get_byte(1, T1) + 256] ^
                        TE[get_byte(2, T2) + 512] ^ TE[get_byte(3, T3) + 768];
         B1 = EK[r+5] ^ TE[get_byte(0, T1)      ] ^ TE[get_byte(1, T2) + 256] ^
                        TE[get_byte(2, T3) + 512] ^ TE[get_byte(3, T0) + 768];
         B2 = EK[r+6] ^ TE[get_byte(0, T2)      ] ^ TE[get_byte(1, T3) + 256] ^
                        TE[get_byte(2, T0) + 512] ^ TE[get_byte(3, T1) + 768];
         B3 = EK[r+7] ^ TE[get_byte(0, T3)      ] ^ TE[get_byte(1, T0) + 256] ^
                        TE[get_byte(2, T1) + 512] ^ TE[get_byte(3, T2) + 768];
         }

      out[ 0] = SE[get_byte(0, B0)] ^ ME[0];
      out[ 1] = SE[get_byte(1, B1)] ^ ME[1];
      out[ 2] = SE[get_byte(2, B2)] ^ ME[2];
      out[ 3] = SE[get_byte(3, B3)] ^ ME[3];
      out[ 4] = SE[get_byte(0, B1)] ^ ME[4];
      out[ 5] = SE[get_byte(1, B2)] ^ ME[5];
      out[ 6] = SE[get_byte(2, B3)] ^ ME[6];
      out[ 7] = SE[get_byte(3, B0)] ^ ME[7];
      out[ 8] = SE[get_byte(0, B2)] ^ ME[8];
      out[ 9] = SE[get_byte(1, B3)] ^ ME[9];
      out[10] = SE[get_byte(2, B0)] ^ ME[10];
      out[11] = SE[get_byte(3, B1)] ^ ME[11];
      out[12] = SE[get_byte(0, B3)] ^ ME[12];
      out[13] = SE[get_byte(1, B0)] ^ ME[13];
      out[14] = SE[get_byte(2, B1)] ^ ME[14];
      out[15] = SE[get_byte(3, B2)] ^ ME[15];

      in += 16;
      out += 16;
      }
   }

/*
* AES Decryption
*/
void aes_decrypt_n(const byte in[], byte out[], size_t blocks,
                   const secure_vector<u32bit>& DK,
                   const secure_vector<byte>& MD)
   {
   BOTAN_ASSERT(DK.size() && MD.size() == 16, "Key was set");

   const size_t cache_line_size = CPUID::cache_line_size();
   const std::vector<u32bit>& TD = AES_TD();

   u32bit Z = 0;
   for(size_t i = 0; i < TD.size(); i += cache_line_size / sizeof(u32bit))
      {
      Z |= TD[i];
      }
   Z &= TD[99]; // this is zero, which hopefully the compiler cannot deduce

   for(size_t i = 0; i != blocks; ++i)
      {
      u32bit T0 = load_be<u32bit>(in, 0) ^ DK[0];
      u32bit T1 = load_be<u32bit>(in, 1) ^ DK[1];
      u32bit T2 = load_be<u32bit>(in, 2) ^ DK[2];
      u32bit T3 = load_be<u32bit>(in, 3) ^ DK[3];

      T0 ^= Z;

      u32bit B0 = TD[get_byte(0, T0)] ^
                  rotate_right(TD[get_byte(1, T3)],  8) ^
                  rotate_right(TD[get_byte(2, T2)], 16) ^
                  rotate_right(TD[get_byte(3, T1)], 24) ^ DK[4];

      u32bit B1 = TD[get_byte(0, T1)] ^
                  rotate_right(TD[get_byte(1, T0)],  8) ^
                  rotate_right(TD[get_byte(2, T3)], 16) ^
                  rotate_right(TD[get_byte(3, T2)], 24) ^ DK[5];

      u32bit B2 = TD[get_byte(0, T2)] ^
                  rotate_right(TD[get_byte(1, T1)],  8) ^
                  rotate_right(TD[get_byte(2, T0)], 16) ^
                  rotate_right(TD[get_byte(3, T3)], 24) ^ DK[6];

      u32bit B3 = TD[get_byte(0, T3)] ^
                  rotate_right(TD[get_byte(1, T2)],  8) ^
                  rotate_right(TD[get_byte(2, T1)], 16) ^
                  rotate_right(TD[get_byte(3, T0)], 24) ^ DK[7];

      for(size_t r = 2*4; r < DK.size(); r += 2*4)
         {
         T0 = DK[r  ] ^ TD[get_byte(0, B0)      ] ^ TD[get_byte(1, B3) + 256] ^
                        TD[get_byte(2, B2) + 512] ^ TD[get_byte(3, B1) + 768];
         T1 = DK[r+1] ^ TD[get_byte(0, B1)      ] ^ TD[get_byte(1, B0) + 256] ^
                        TD[get_byte(2, B3) + 512] ^ TD[get_byte(3, B2) + 768];
         T2 = DK[r+2] ^ TD[get_byte(0, B2)      ] ^ TD[get_byte(1, B1) + 256] ^
                        TD[get_byte(2, B0) + 512] ^ TD[get_byte(3, B3) + 768];
         T3 = DK[r+3] ^ TD[get_byte(0, B3)      ] ^ TD[get_byte(1, B2) + 256] ^
                        TD[get_byte(2, B1) + 512] ^ TD[get_byte(3, B0) + 768];

         B0 = DK[r+4] ^ TD[get_byte(0, T0)      ] ^ TD[get_byte(1, T3) + 256] ^
                        TD[get_byte(2, T2) + 512] ^ TD[get_byte(3, T1) + 768];
         B1 = DK[r+5] ^ TD[get_byte(0, T1)      ] ^ TD[get_byte(1, T0) + 256] ^
                        TD[get_byte(2, T3) + 512] ^ TD[get_byte(3, T2) + 768];
         B2 = DK[r+6] ^ TD[get_byte(0, T2)      ] ^ TD[get_byte(1, T1) + 256] ^
                        TD[get_byte(2, T0) + 512] ^ TD[get_byte(3, T3) + 768];
         B3 = DK[r+7] ^ TD[get_byte(0, T3)      ] ^ TD[get_byte(1, T2) + 256] ^
                        TD[get_byte(2, T1) + 512] ^ TD[get_byte(3, T0) + 768];
         }

      out[ 0] = SD[get_byte(0, B0)] ^ MD[0];
      out[ 1] = SD[get_byte(1, B3)] ^ MD[1];
      out[ 2] = SD[get_byte(2, B2)] ^ MD[2];
      out[ 3] = SD[get_byte(3, B1)] ^ MD[3];
      out[ 4] = SD[get_byte(0, B1)] ^ MD[4];
      out[ 5] = SD[get_byte(1, B0)] ^ MD[5];
      out[ 6] = SD[get_byte(2, B3)] ^ MD[6];
      out[ 7] = SD[get_byte(3, B2)] ^ MD[7];
      out[ 8] = SD[get_byte(0, B2)] ^ MD[8];
      out[ 9] = SD[get_byte(1, B1)] ^ MD[9];
      out[10] = SD[get_byte(2, B0)] ^ MD[10];
      out[11] = SD[get_byte(3, B3)] ^ MD[11];
      out[12] = SD[get_byte(0, B3)] ^ MD[12];
      out[13] = SD[get_byte(1, B2)] ^ MD[13];
      out[14] = SD[get_byte(2, B1)] ^ MD[14];
      out[15] = SD[get_byte(3, B0)] ^ MD[15];

      in += 16;
      out += 16;
      }
   }

void aes_key_schedule(const byte key[], size_t length,
                      secure_vector<u32bit>& EK,
                      secure_vector<u32bit>& DK,
                      secure_vector<byte>& ME,
                      secure_vector<byte>& MD)
   {
   static const u32bit RC[10] = {
      0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000,
      0x20000000, 0x40000000, 0x80000000, 0x1B000000, 0x36000000 };

   const size_t rounds = (length / 4) + 6;

   secure_vector<u32bit> XEK(length + 32), XDK(length + 32);

   const size_t X = length / 4;
   for(size_t i = 0; i != X; ++i)
      XEK[i] = load_be<u32bit>(key, i);

   for(size_t i = X; i < 4*(rounds+1); i += X)
      {
      XEK[i] = XEK[i-X] ^ RC[(i-X)/X] ^
               make_u32bit(SE[get_byte(1, XEK[i-1])],
                           SE[get_byte(2, XEK[i-1])],
                           SE[get_byte(3, XEK[i-1])],
                           SE[get_byte(0, XEK[i-1])]);

      for(size_t j = 1; j != X; ++j)
         {
         XEK[i+j] = XEK[i+j-X];

         if(X == 8 && j == 4)
            XEK[i+j] ^= make_u32bit(SE[get_byte(0, XEK[i+j-1])],
                                    SE[get_byte(1, XEK[i+j-1])],
                                    SE[get_byte(2, XEK[i+j-1])],
                                    SE[get_byte(3, XEK[i+j-1])]);
         else
            XEK[i+j] ^= XEK[i+j-1];
         }
      }

   const std::vector<u32bit>& TD = AES_TD();

   for(size_t i = 0; i != 4*(rounds+1); i += 4)
      {
      XDK[i  ] = XEK[4*rounds-i  ];
      XDK[i+1] = XEK[4*rounds-i+1];
      XDK[i+2] = XEK[4*rounds-i+2];
      XDK[i+3] = XEK[4*rounds-i+3];
      }

   for(size_t i = 4; i != length + 24; ++i)
      XDK[i] = TD[SE[get_byte(0, XDK[i])] +   0] ^
               TD[SE[get_byte(1, XDK[i])] + 256] ^
               TD[SE[get_byte(2, XDK[i])] + 512] ^
               TD[SE[get_byte(3, XDK[i])] + 768];

   ME.resize(16);
   MD.resize(16);

   for(size_t i = 0; i != 4; ++i)
      {
      store_be(XEK[i+4*rounds], &ME[4*i]);
      store_be(XEK[i], &MD[4*i]);
      }

   EK.resize(length + 24);
   DK.resize(length + 24);
   copy_mem(EK.data(), XEK.data(), EK.size());
   copy_mem(DK.data(), XDK.data(), DK.size());
   }

}

void AES_128::encrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   aes_encrypt_n(in, out, blocks, EK, ME);
   }

void AES_128::decrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   aes_decrypt_n(in, out, blocks, DK, MD);
   }

void AES_128::key_schedule(const byte key[], size_t length)
   {
   aes_key_schedule(key, length, EK, DK, ME, MD);
   }

void AES_128::clear()
   {
   zap(EK);
   zap(DK);
   zap(ME);
   zap(MD);
   }

void AES_192::encrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   aes_encrypt_n(in, out, blocks, EK, ME);
   }

void AES_192::decrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   aes_decrypt_n(in, out, blocks, DK, MD);
   }

void AES_192::key_schedule(const byte key[], size_t length)
   {
   aes_key_schedule(key, length, EK, DK, ME, MD);
   }

void AES_192::clear()
   {
   zap(EK);
   zap(DK);
   zap(ME);
   zap(MD);
   }

void AES_256::encrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   aes_encrypt_n(in, out, blocks, EK, ME);
   }

void AES_256::decrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   aes_decrypt_n(in, out, blocks, DK, MD);
   }

void AES_256::key_schedule(const byte key[], size_t length)
   {
   aes_key_schedule(key, length, EK, DK, ME, MD);
   }

void AES_256::clear()
   {
   zap(EK);
   zap(DK);
   zap(ME);
   zap(MD);
   }

}
/*
* Algorithm Identifier
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Create an AlgorithmIdentifier
*/
AlgorithmIdentifier::AlgorithmIdentifier(const OID& alg_id,
                                         const std::vector<byte>& param)
   {
   oid = alg_id;
   parameters = param;
   }

/*
* Create an AlgorithmIdentifier
*/
AlgorithmIdentifier::AlgorithmIdentifier(const std::string& alg_id,
                                         const std::vector<byte>& param)
   {
   oid = OIDS::lookup(alg_id);
   parameters = param;
   }

/*
* Create an AlgorithmIdentifier
*/
AlgorithmIdentifier::AlgorithmIdentifier(const OID& alg_id,
                                         Encoding_Option option)
   {
   const byte DER_NULL[] = { 0x05, 0x00 };

   oid = alg_id;

   if(option == USE_NULL_PARAM)
      parameters += std::pair<const byte*, size_t>(DER_NULL, sizeof(DER_NULL));
   }

/*
* Create an AlgorithmIdentifier
*/
AlgorithmIdentifier::AlgorithmIdentifier(const std::string& alg_id,
                                         Encoding_Option option)
   {
   const byte DER_NULL[] = { 0x05, 0x00 };

   oid = OIDS::lookup(alg_id);

   if(option == USE_NULL_PARAM)
      parameters += std::pair<const byte*, size_t>(DER_NULL, sizeof(DER_NULL));
   }

/*
* Compare two AlgorithmIdentifiers
*/
namespace {

bool param_null_or_empty(const std::vector<byte>& p)
   {
   if(p.size() == 2 && (p[0] == 0x05) && (p[1] == 0x00))
      return true;
   return p.empty();
   }

}

bool operator==(const AlgorithmIdentifier& a1, const AlgorithmIdentifier& a2)
   {
   if(a1.oid != a2.oid)
      return false;

   if(param_null_or_empty(a1.parameters) &&
      param_null_or_empty(a2.parameters))
      return true;

   return (a1.parameters == a2.parameters);
   }

/*
* Compare two AlgorithmIdentifiers
*/
bool operator!=(const AlgorithmIdentifier& a1, const AlgorithmIdentifier& a2)
   {
   return !(a1 == a2);
   }

/*
* DER encode an AlgorithmIdentifier
*/
void AlgorithmIdentifier::encode_into(DER_Encoder& codec) const
   {
   codec.start_cons(SEQUENCE)
      .encode(oid)
      .raw_bytes(parameters)
   .end_cons();
   }

/*
* Decode a BER encoded AlgorithmIdentifier
*/
void AlgorithmIdentifier::decode_from(BER_Decoder& codec)
   {
   codec.start_cons(SEQUENCE)
      .decode(oid)
      .raw_bytes(parameters)
   .end_cons();
   }

}
/*
* AlternativeName
* (C) 1999-2007 Jack Lloyd
*     2007 Yves Jerschow
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

/*
* Check if type is a known ASN.1 string type
*/
bool is_string_type(ASN1_Tag tag)
   {
   return (tag == NUMERIC_STRING ||
           tag == PRINTABLE_STRING ||
           tag == VISIBLE_STRING ||
           tag == T61_STRING ||
           tag == IA5_STRING ||
           tag == UTF8_STRING ||
           tag == BMP_STRING);
   }

}

/*
* Create an AlternativeName
*/
AlternativeName::AlternativeName(const std::string& email_addr,
                                 const std::string& uri,
                                 const std::string& dns,
                                 const std::string& ip)
   {
   add_attribute("RFC822", email_addr);
   add_attribute("DNS", dns);
   add_attribute("URI", uri);
   add_attribute("IP", ip);
   }

/*
* Add an attribute to an alternative name
*/
void AlternativeName::add_attribute(const std::string& type,
                                    const std::string& str)
   {
   if(type == "" || str == "")
      return;

   auto range = alt_info.equal_range(type);
   for(auto j = range.first; j != range.second; ++j)
      if(j->second == str)
         return;

   multimap_insert(alt_info, type, str);
   }

/*
* Add an OtherName field
*/
void AlternativeName::add_othername(const OID& oid, const std::string& value,
                                    ASN1_Tag type)
   {
   if(value == "")
      return;
   multimap_insert(othernames, oid, ASN1_String(value, type));
   }

/*
* Get the attributes of this alternative name
*/
std::multimap<std::string, std::string> AlternativeName::get_attributes() const
   {
   return alt_info;
   }

/*
* Get the otherNames
*/
std::multimap<OID, ASN1_String> AlternativeName::get_othernames() const
   {
   return othernames;
   }

/*
* Return all of the alternative names
*/
std::multimap<std::string, std::string> AlternativeName::contents() const
   {
   std::multimap<std::string, std::string> names;

   for(auto i = alt_info.begin(); i != alt_info.end(); ++i)
      multimap_insert(names, i->first, i->second);

   for(auto i = othernames.begin(); i != othernames.end(); ++i)
      multimap_insert(names, OIDS::lookup(i->first), i->second.value());

   return names;
   }

/*
* Return if this object has anything useful
*/
bool AlternativeName::has_items() const
   {
   return (alt_info.size() > 0 || othernames.size() > 0);
   }

namespace {

/*
* DER encode an AlternativeName entry
*/
void encode_entries(DER_Encoder& encoder,
                    const std::multimap<std::string, std::string>& attr,
                    const std::string& type, ASN1_Tag tagging)
   {
   auto range = attr.equal_range(type);

   for(auto i = range.first; i != range.second; ++i)
      {
      if(type == "RFC822" || type == "DNS" || type == "URI")
         {
         ASN1_String asn1_string(i->second, IA5_STRING);
         encoder.add_object(tagging, CONTEXT_SPECIFIC, asn1_string.iso_8859());
         }
      else if(type == "IP")
         {
         const u32bit ip = string_to_ipv4(i->second);
         byte ip_buf[4] = { 0 };
         store_be(ip, ip_buf);
         encoder.add_object(tagging, CONTEXT_SPECIFIC, ip_buf, 4);
         }
      }
   }

}

/*
* DER encode an AlternativeName extension
*/
void AlternativeName::encode_into(DER_Encoder& der) const
   {
   der.start_cons(SEQUENCE);

   encode_entries(der, alt_info, "RFC822", ASN1_Tag(1));
   encode_entries(der, alt_info, "DNS", ASN1_Tag(2));
   encode_entries(der, alt_info, "URI", ASN1_Tag(6));
   encode_entries(der, alt_info, "IP", ASN1_Tag(7));

   for(auto i = othernames.begin(); i != othernames.end(); ++i)
      {
      der.start_explicit(0)
         .encode(i->first)
         .start_explicit(0)
            .encode(i->second)
         .end_explicit()
      .end_explicit();
      }

   der.end_cons();
   }

/*
* Decode a BER encoded AlternativeName
*/
void AlternativeName::decode_from(BER_Decoder& source)
   {
   BER_Decoder names = source.start_cons(SEQUENCE);

   while(names.more_items())
      {
      BER_Object obj = names.get_next_object();
      if((obj.class_tag != CONTEXT_SPECIFIC) &&
         (obj.class_tag != (CONTEXT_SPECIFIC | CONSTRUCTED)))
         continue;

      const ASN1_Tag tag = obj.type_tag;

      if(tag == 0)
         {
         BER_Decoder othername(obj.value);

         OID oid;
         othername.decode(oid);
         if(othername.more_items())
            {
            BER_Object othername_value_outer = othername.get_next_object();
            othername.verify_end();

            if(othername_value_outer.type_tag != ASN1_Tag(0) ||
               othername_value_outer.class_tag !=
                   (CONTEXT_SPECIFIC | CONSTRUCTED)
               )
               throw Decoding_Error("Invalid tags on otherName value");

            BER_Decoder othername_value_inner(othername_value_outer.value);

            BER_Object value = othername_value_inner.get_next_object();
            othername_value_inner.verify_end();

            const ASN1_Tag value_type = value.type_tag;

            if(is_string_type(value_type) && value.class_tag == UNIVERSAL)
               add_othername(oid, ASN1::to_string(value), value_type);
            }
         }
      else if(tag == 1 || tag == 2 || tag == 6)
         {
         const std::string value = Charset::transcode(ASN1::to_string(obj),
                                                      LATIN1_CHARSET,
                                                      LOCAL_CHARSET);

         if(tag == 1) add_attribute("RFC822", value);
         if(tag == 2) add_attribute("DNS", value);
         if(tag == 6) add_attribute("URI", value);
         }
      else if(tag == 7)
         {
         if(obj.value.size() == 4)
            {
            const u32bit ip = load_be<u32bit>(&obj.value[0], 0);
            add_attribute("IP", ipv4_to_string(ip));
            }
         }

      }
   }

}
/*
* Attribute
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Create an Attribute
*/
Attribute::Attribute(const OID& attr_oid, const std::vector<byte>& attr_value)
   {
   oid = attr_oid;
   parameters = attr_value;
   }

/*
* Create an Attribute
*/
Attribute::Attribute(const std::string& attr_oid,
                     const std::vector<byte>& attr_value)
   {
   oid = OIDS::lookup(attr_oid);
   parameters = attr_value;
   }

/*
* DER encode a Attribute
*/
void Attribute::encode_into(DER_Encoder& codec) const
   {
   codec.start_cons(SEQUENCE)
      .encode(oid)
      .start_cons(SET)
         .raw_bytes(parameters)
      .end_cons()
   .end_cons();
   }

/*
* Decode a BER encoded Attribute
*/
void Attribute::decode_from(BER_Decoder& codec)
   {
   codec.start_cons(SEQUENCE)
      .decode(oid)
      .start_cons(SET)
         .raw_bytes(parameters)
      .end_cons()
   .end_cons();
   }

}
/*
* ASN.1 Internals
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* BER Decoding Exceptions
*/
BER_Decoding_Error::BER_Decoding_Error(const std::string& str) :
   Decoding_Error("BER: " + str) {}

BER_Bad_Tag::BER_Bad_Tag(const std::string& str, ASN1_Tag tag) :
      BER_Decoding_Error(str + ": " + std::to_string(tag)) {}

BER_Bad_Tag::BER_Bad_Tag(const std::string& str,
                         ASN1_Tag tag1, ASN1_Tag tag2) :
   BER_Decoding_Error(str + ": " + std::to_string(tag1) + "/" + std::to_string(tag2)) {}

namespace ASN1 {

/*
* Put some arbitrary bytes into a SEQUENCE
*/
std::vector<byte> put_in_sequence(const std::vector<byte>& contents)
   {
   return DER_Encoder()
      .start_cons(SEQUENCE)
         .raw_bytes(contents)
      .end_cons()
   .get_contents_unlocked();
   }

/*
* Convert a BER object into a string object
*/
std::string to_string(const BER_Object& obj)
   {
   return to_string(obj.value);
   }

/*
* Do heuristic tests for BER data
*/
bool maybe_BER(DataSource& source)
   {
   byte first_byte;
   if(!source.peek_byte(first_byte))
      throw Stream_IO_Error("ASN1::maybe_BER: Source was empty");

   if(first_byte == (SEQUENCE | CONSTRUCTED))
      return true;
   return false;
   }

}

}
/*
* ASN.1 OID
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* ASN.1 OID Constructor
*/
OID::OID(const std::string& oid_str)
   {
   if(oid_str != "")
      {
      try
         {
         id = parse_asn1_oid(oid_str);
         }
      catch(...)
         {
         throw Invalid_OID(oid_str);
         }

      if(id.size() < 2 || id[0] > 2)
         throw Invalid_OID(oid_str);
      if((id[0] == 0 || id[0] == 1) && id[1] > 39)
         throw Invalid_OID(oid_str);
      }
   }

/*
* Clear the current OID
*/
void OID::clear()
   {
   id.clear();
   }

/*
* Return this OID as a string
*/
std::string OID::as_string() const
   {
   std::string oid_str;
   for(size_t i = 0; i != id.size(); ++i)
      {
      oid_str += std::to_string(id[i]);
      if(i != id.size() - 1)
         oid_str += ".";
      }
   return oid_str;
   }

/*
* OID equality comparison
*/
bool OID::operator==(const OID& oid) const
   {
   if(id.size() != oid.id.size())
      return false;
   for(size_t i = 0; i != id.size(); ++i)
      if(id[i] != oid.id[i])
         return false;
   return true;
   }

/*
* Append another component to the OID
*/
OID& OID::operator+=(u32bit component)
   {
   id.push_back(component);
   return (*this);
   }

/*
* Append another component to the OID
*/
OID operator+(const OID& oid, u32bit component)
   {
   OID new_oid(oid);
   new_oid += component;
   return new_oid;
   }

/*
* OID inequality comparison
*/
bool operator!=(const OID& a, const OID& b)
   {
   return !(a == b);
   }

/*
* Compare two OIDs
*/
bool operator<(const OID& a, const OID& b)
   {
   const std::vector<u32bit>& oid1 = a.get_id();
   const std::vector<u32bit>& oid2 = b.get_id();

   if(oid1.size() < oid2.size())
      return true;
   if(oid1.size() > oid2.size())
      return false;
   for(size_t i = 0; i != oid1.size(); ++i)
      {
      if(oid1[i] < oid2[i])
         return true;
      if(oid1[i] > oid2[i])
         return false;
      }
   return false;
   }

/*
* DER encode an OBJECT IDENTIFIER
*/
void OID::encode_into(DER_Encoder& der) const
   {
   if(id.size() < 2)
      throw Invalid_Argument("OID::encode_into: OID is invalid");

   std::vector<byte> encoding;
   encoding.push_back(40 * id[0] + id[1]);

   for(size_t i = 2; i != id.size(); ++i)
      {
      if(id[i] == 0)
         encoding.push_back(0);
      else
         {
         size_t blocks = high_bit(id[i]) + 6;
         blocks = (blocks - (blocks % 7)) / 7;

         for(size_t j = 0; j != blocks - 1; ++j)
            encoding.push_back(0x80 | ((id[i] >> 7*(blocks-j-1)) & 0x7F));
         encoding.push_back(id[i] & 0x7F);
         }
      }
   der.add_object(OBJECT_ID, UNIVERSAL, encoding);
   }

/*
* Decode a BER encoded OBJECT IDENTIFIER
*/
void OID::decode_from(BER_Decoder& decoder)
   {
   BER_Object obj = decoder.get_next_object();
   if(obj.type_tag != OBJECT_ID || obj.class_tag != UNIVERSAL)
      throw BER_Bad_Tag("Error decoding OID, unknown tag",
                        obj.type_tag, obj.class_tag);
   if(obj.value.size() < 2)
      throw BER_Decoding_Error("OID encoding is too short");


   clear();
   id.push_back(obj.value[0] / 40);
   id.push_back(obj.value[0] % 40);

   size_t i = 0;
   while(i != obj.value.size() - 1)
      {
      u32bit component = 0;
      while(i != obj.value.size() - 1)
         {
         ++i;

         if(component >> (32-7))
            throw Decoding_Error("OID component overflow");

         component = (component << 7) + (obj.value[i] & 0x7F);

         if(!(obj.value[i] & 0x80))
            break;
         }
      id.push_back(component);
      }
   }

}
/*
* Simple ASN.1 String Types
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

/*
* Choose an encoding for the string
*/
ASN1_Tag choose_encoding(const std::string& str,
                         const std::string& type)
   {
   static const byte IS_PRINTABLE[256] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01,
      0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
      0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
      0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
      0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
      0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
      0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00 };

   for(size_t i = 0; i != str.size(); ++i)
      {
      if(!IS_PRINTABLE[static_cast<byte>(str[i])])
         {
         if(type == "utf8")   return UTF8_STRING;
         if(type == "latin1") return T61_STRING;
         throw Invalid_Argument("choose_encoding: Bad string type " + type);
         }
      }
   return PRINTABLE_STRING;
   }

}

/*
* Create an ASN1_String
*/
ASN1_String::ASN1_String(const std::string& str, ASN1_Tag t) : tag(t)
   {
   iso_8859_str = Charset::transcode(str, LOCAL_CHARSET, LATIN1_CHARSET);

   if(tag == DIRECTORY_STRING)
      tag = choose_encoding(iso_8859_str, "latin1");

   if(tag != NUMERIC_STRING &&
      tag != PRINTABLE_STRING &&
      tag != VISIBLE_STRING &&
      tag != T61_STRING &&
      tag != IA5_STRING &&
      tag != UTF8_STRING &&
      tag != BMP_STRING)
      throw Invalid_Argument("ASN1_String: Unknown string type " +
                             std::to_string(tag));
   }

/*
* Create an ASN1_String
*/
ASN1_String::ASN1_String(const std::string& str)
   {
   iso_8859_str = Charset::transcode(str, LOCAL_CHARSET, LATIN1_CHARSET);
   tag = choose_encoding(iso_8859_str, "latin1");
   }

/*
* Return this string in ISO 8859-1 encoding
*/
std::string ASN1_String::iso_8859() const
   {
   return iso_8859_str;
   }

/*
* Return this string in local encoding
*/
std::string ASN1_String::value() const
   {
   return Charset::transcode(iso_8859_str, LATIN1_CHARSET, LOCAL_CHARSET);
   }

/*
* Return the type of this string object
*/
ASN1_Tag ASN1_String::tagging() const
   {
   return tag;
   }

/*
* DER encode an ASN1_String
*/
void ASN1_String::encode_into(DER_Encoder& encoder) const
   {
   std::string value = iso_8859();
   if(tagging() == UTF8_STRING)
      value = Charset::transcode(value, LATIN1_CHARSET, UTF8_CHARSET);
   encoder.add_object(tagging(), UNIVERSAL, value);
   }

/*
* Decode a BER encoded ASN1_String
*/
void ASN1_String::decode_from(BER_Decoder& source)
   {
   BER_Object obj = source.get_next_object();

   Character_Set charset_is;

   if(obj.type_tag == BMP_STRING)
      charset_is = UCS2_CHARSET;
   else if(obj.type_tag == UTF8_STRING)
      charset_is = UTF8_CHARSET;
   else
      charset_is = LATIN1_CHARSET;

   *this = ASN1_String(
      Charset::transcode(ASN1::to_string(obj), charset_is, LOCAL_CHARSET),
      obj.type_tag);
   }

}
/*
* X.509 Time Types
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <sstream>
#include <iomanip>

namespace Botan {

X509_Time::X509_Time(const std::chrono::system_clock::time_point& time)
   {
   calendar_point cal = calendar_value(time);

   m_year   = cal.year;
   m_month  = cal.month;
   m_day    = cal.day;
   m_hour   = cal.hour;
   m_minute = cal.minutes;
   m_second = cal.seconds;

   m_tag = (m_year >= 2050) ? GENERALIZED_TIME : UTC_TIME;
   }

X509_Time::X509_Time(const std::string& t_spec, ASN1_Tag tag)
   {
   set_to(t_spec, tag);
   }

void X509_Time::encode_into(DER_Encoder& der) const
   {
   if(m_tag != GENERALIZED_TIME && m_tag != UTC_TIME)
      throw Invalid_Argument("X509_Time: Bad encoding tag");

   der.add_object(m_tag, UNIVERSAL,
                  Charset::transcode(to_string(),
                                     LOCAL_CHARSET,
                                     LATIN1_CHARSET));
   }

void X509_Time::decode_from(BER_Decoder& source)
   {
   BER_Object ber_time = source.get_next_object();

   set_to(Charset::transcode(ASN1::to_string(ber_time),
                             LATIN1_CHARSET,
                             LOCAL_CHARSET),
          ber_time.type_tag);
   }

std::string X509_Time::to_string() const
   {
   if(time_is_set() == false)
      throw Invalid_State("X509_Time::as_string: No time set");

   u32bit full_year = m_year;

   if(m_tag == UTC_TIME)
      {
      if(m_year < 1950 || m_year >= 2050)
         throw Encoding_Error("X509_Time: The time " + readable_string() +
                              " cannot be encoded as a UTCTime");

      full_year = (m_year >= 2000) ? (m_year - 2000) : (m_year - 1900);
      }

   const auto factor_y = uint64_t{10000000000ull}; // literal exceeds 32bit int range
   const auto factor_m = uint64_t{100000000ull};
   const auto factor_d = uint64_t{1000000ull};
   const auto factor_h = uint64_t{10000ull};
   const auto factor_i = uint64_t{100ull};

   std::string repr = std::to_string(factor_y * full_year +
                                     factor_m * m_month +
                                     factor_d * m_day +
                                     factor_h * m_hour +
                                     factor_i * m_minute +
                                     m_second) + "Z";

   u32bit desired_size = (m_tag == UTC_TIME) ? 13 : 15;

   while(repr.size() < desired_size)
      repr = "0" + repr;

   return repr;
   }

std::string X509_Time::readable_string() const
   {
   if(time_is_set() == false)
      throw Invalid_State("X509_Time::readable_string: No time set");

   // desired format: "%04d/%02d/%02d %02d:%02d:%02d UTC"
   std::stringstream output;
      {
      using namespace std;
      output << setfill('0')
             << setw(4) << m_year << "/" << setw(2) << m_month << "/" << setw(2) << m_day
             << " "
             << setw(2) << m_hour << ":" << setw(2) << m_minute << ":" << setw(2) << m_second
             << " UTC";
      }
   return output.str();
   }

bool X509_Time::time_is_set() const
   {
   return (m_year != 0);
   }

s32bit X509_Time::cmp(const X509_Time& other) const
   {
   if(time_is_set() == false)
      throw Invalid_State("X509_Time::cmp: No time set");

   const s32bit EARLIER = -1, LATER = 1, SAME_TIME = 0;

   if(m_year < other.m_year)     return EARLIER;
   if(m_year > other.m_year)     return LATER;
   if(m_month < other.m_month)   return EARLIER;
   if(m_month > other.m_month)   return LATER;
   if(m_day < other.m_day)       return EARLIER;
   if(m_day > other.m_day)       return LATER;
   if(m_hour < other.m_hour)     return EARLIER;
   if(m_hour > other.m_hour)     return LATER;
   if(m_minute < other.m_minute) return EARLIER;
   if(m_minute > other.m_minute) return LATER;
   if(m_second < other.m_second) return EARLIER;
   if(m_second > other.m_second) return LATER;

   return SAME_TIME;
   }

void X509_Time::set_to(const std::string& t_spec, ASN1_Tag spec_tag)
   {
   if(spec_tag == UTC_OR_GENERALIZED_TIME)
      {
      try
         {
         set_to(t_spec, GENERALIZED_TIME);
         return;
         }
      catch(Invalid_Argument) {} // Not a generalized time. Continue

      try
         {
         set_to(t_spec, UTC_TIME);
         return;
         }
      catch(Invalid_Argument) {} // Not a UTC time. Continue

      throw Invalid_Argument("Time string could not be parsed as GeneralizedTime or UTCTime.");
      }

   BOTAN_ASSERT(spec_tag == UTC_TIME || spec_tag == GENERALIZED_TIME, "Invalid tag.");

   if(t_spec.empty())
      throw Invalid_Argument("Time string must not be empty.");

   if(t_spec.back() != 'Z')
      throw Unsupported_Argument("Botan does not support times with timezones other than Z: " + t_spec);

   if(spec_tag == GENERALIZED_TIME)
      {
      if(t_spec.size() != 13 && t_spec.size() != 15)
         throw Invalid_Argument("Invalid GeneralizedTime string: '" + t_spec + "'");
      }
   else if(spec_tag == UTC_TIME)
      {
      if(t_spec.size() != 11 && t_spec.size() != 13)
         throw Invalid_Argument("Invalid UTCTime string: '" + t_spec + "'");
      }

   const size_t YEAR_SIZE = (spec_tag == UTC_TIME) ? 2 : 4;

   std::vector<std::string> params;
   std::string current;

   for(size_t j = 0; j != YEAR_SIZE; ++j)
      current += t_spec[j];
   params.push_back(current);
   current.clear();

   for(size_t j = YEAR_SIZE; j != t_spec.size() - 1; ++j)
      {
      current += t_spec[j];
      if(current.size() == 2)
         {
         params.push_back(current);
         current.clear();
         }
      }

   m_year   = to_u32bit(params[0]);
   m_month  = to_u32bit(params[1]);
   m_day    = to_u32bit(params[2]);
   m_hour   = to_u32bit(params[3]);
   m_minute = to_u32bit(params[4]);
   m_second = (params.size() == 6) ? to_u32bit(params[5]) : 0;
   m_tag    = spec_tag;

   if(spec_tag == UTC_TIME)
      {
      if(m_year >= 50) m_year += 1900;
      else             m_year += 2000;
      }

   if(!passes_sanity_check())
      throw Invalid_Argument("Time did not pass sanity check: " + t_spec);
   }

/*
* Do a general sanity check on the time
*/
bool X509_Time::passes_sanity_check() const
   {
   if(m_year < 1950 || m_year > 2100)
      return false;
   if(m_month == 0 || m_month > 12)
      return false;
   if(m_day == 0 || m_day > 31)
      return false;
   if(m_hour >= 24 || m_minute > 60 || m_second > 60)
      return false;

   if (m_tag == UTC_TIME)
      {
      /*
      UTCTime limits the value of components such that leap seconds
      are not covered. See "UNIVERSAL 23" in "Information technology
      Abstract Syntax Notation One (ASN.1): Specification of basic notation"

      http://www.itu.int/ITU-T/studygroups/com17/languages/
      */
      if (m_hour > 23 || m_minute > 59 || m_second > 59)
         {
         return false;
         }
      }

   return true;
   }

/*
* Compare two X509_Times for in various ways
*/
bool operator==(const X509_Time& t1, const X509_Time& t2)
   { return (t1.cmp(t2) == 0); }
bool operator!=(const X509_Time& t1, const X509_Time& t2)
   { return (t1.cmp(t2) != 0); }

bool operator<=(const X509_Time& t1, const X509_Time& t2)
   { return (t1.cmp(t2) <= 0); }
bool operator>=(const X509_Time& t1, const X509_Time& t2)
   { return (t1.cmp(t2) >= 0); }

bool operator<(const X509_Time& t1, const X509_Time& t2)
   { return (t1.cmp(t2) < 0); }
bool operator>(const X509_Time& t1, const X509_Time& t2)
   { return (t1.cmp(t2) > 0); }

}

/*
* BER Decoder
* (C) 1999-2008,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

/*
* BER decode an ASN.1 type tag
*/
size_t decode_tag(DataSource* ber, ASN1_Tag& type_tag, ASN1_Tag& class_tag)
   {
   byte b;
   if(!ber->read_byte(b))
      {
      class_tag = type_tag = NO_OBJECT;
      return 0;
      }

   if((b & 0x1F) != 0x1F)
      {
      type_tag = ASN1_Tag(b & 0x1F);
      class_tag = ASN1_Tag(b & 0xE0);
      return 1;
      }

   size_t tag_bytes = 1;
   class_tag = ASN1_Tag(b & 0xE0);

   size_t tag_buf = 0;
   while(true)
      {
      if(!ber->read_byte(b))
         throw BER_Decoding_Error("Long-form tag truncated");
      if(tag_buf & 0xFF000000)
         throw BER_Decoding_Error("Long-form tag overflowed 32 bits");
      ++tag_bytes;
      tag_buf = (tag_buf << 7) | (b & 0x7F);
      if((b & 0x80) == 0) break;
      }
   type_tag = ASN1_Tag(tag_buf);
   return tag_bytes;
   }

/*
* Find the EOC marker
*/
size_t find_eoc(DataSource*);

/*
* BER decode an ASN.1 length field
*/
size_t decode_length(DataSource* ber, size_t& field_size)
   {
   byte b;
   if(!ber->read_byte(b))
      throw BER_Decoding_Error("Length field not found");
   field_size = 1;
   if((b & 0x80) == 0)
      return b;

   field_size += (b & 0x7F);
   if(field_size == 1) return find_eoc(ber);
   if(field_size > 5)
      throw BER_Decoding_Error("Length field is too large");

   size_t length = 0;

   for(size_t i = 0; i != field_size - 1; ++i)
      {
      if(get_byte(0, length) != 0)
         throw BER_Decoding_Error("Field length overflow");
      if(!ber->read_byte(b))
         throw BER_Decoding_Error("Corrupted length field");
      length = (length << 8) | b;
      }
   return length;
   }

/*
* BER decode an ASN.1 length field
*/
size_t decode_length(DataSource* ber)
   {
   size_t dummy;
   return decode_length(ber, dummy);
   }

/*
* Find the EOC marker
*/
size_t find_eoc(DataSource* ber)
   {
   secure_vector<byte> buffer(DEFAULT_BUFFERSIZE), data;

   while(true)
      {
      const size_t got = ber->peek(buffer.data(), buffer.size(), data.size());
      if(got == 0)
         break;

      data += std::make_pair(buffer.data(), got);
      }

   DataSource_Memory source(data);
   data.clear();

   size_t length = 0;
   while(true)
      {
      ASN1_Tag type_tag, class_tag;
      size_t tag_size = decode_tag(&source, type_tag, class_tag);
      if(type_tag == NO_OBJECT)
         break;

      size_t length_size = 0;
      size_t item_size = decode_length(&source, length_size);
      source.discard_next(item_size);

      length += item_size + length_size + tag_size;

      if(type_tag == EOC && class_tag == UNIVERSAL)
         break;
      }
   return length;
   }

}

/*
* Check a type invariant on BER data
*/
void BER_Object::assert_is_a(ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   if(this->type_tag != type_tag || this->class_tag != class_tag)
      throw BER_Decoding_Error("Tag mismatch when decoding got " +
                               std::to_string(this->type_tag) + "/" +
                               std::to_string(this->class_tag) + " expected " +
                               std::to_string(type_tag) + "/" +
                               std::to_string(class_tag));
   }

/*
* Check if more objects are there
*/
bool BER_Decoder::more_items() const
   {
   if(source->end_of_data() && (pushed.type_tag == NO_OBJECT))
      return false;
   return true;
   }

/*
* Verify that no bytes remain in the source
*/
BER_Decoder& BER_Decoder::verify_end()
   {
   if(!source->end_of_data() || (pushed.type_tag != NO_OBJECT))
      throw Invalid_State("BER_Decoder::verify_end called, but data remains");
   return (*this);
   }

/*
* Save all the bytes remaining in the source
*/
BER_Decoder& BER_Decoder::raw_bytes(secure_vector<byte>& out)
   {
   out.clear();
   byte buf;
   while(source->read_byte(buf))
      out.push_back(buf);
   return (*this);
   }

BER_Decoder& BER_Decoder::raw_bytes(std::vector<byte>& out)
   {
   out.clear();
   byte buf;
   while(source->read_byte(buf))
      out.push_back(buf);
   return (*this);
   }

/*
* Discard all the bytes remaining in the source
*/
BER_Decoder& BER_Decoder::discard_remaining()
   {
   byte buf;
   while(source->read_byte(buf))
      ;
   return (*this);
   }

/*
* Return the BER encoding of the next object
*/
BER_Object BER_Decoder::get_next_object()
   {
   BER_Object next;

   if(pushed.type_tag != NO_OBJECT)
      {
      next = pushed;
      pushed.class_tag = pushed.type_tag = NO_OBJECT;
      return next;
      }

   decode_tag(source, next.type_tag, next.class_tag);
   if(next.type_tag == NO_OBJECT)
      return next;

   const size_t length = decode_length(source);
   if(!source->check_available(length))
      throw BER_Decoding_Error("Value truncated");

   next.value.resize(length);
   if(source->read(next.value.data(), length) != length)
      throw BER_Decoding_Error("Value truncated");

   if(next.type_tag == EOC && next.class_tag == UNIVERSAL)
      return get_next_object();

   return next;
   }

BER_Decoder& BER_Decoder::get_next(BER_Object& ber)
   {
   ber = get_next_object();
   return (*this);
   }

/*
* Push a object back into the stream
*/
void BER_Decoder::push_back(const BER_Object& obj)
   {
   if(pushed.type_tag != NO_OBJECT)
      throw Invalid_State("BER_Decoder: Only one push back is allowed");
   pushed = obj;
   }

/*
* Begin decoding a CONSTRUCTED type
*/
BER_Decoder BER_Decoder::start_cons(ASN1_Tag type_tag,
                                    ASN1_Tag class_tag)
   {
   BER_Object obj = get_next_object();
   obj.assert_is_a(type_tag, ASN1_Tag(class_tag | CONSTRUCTED));

   BER_Decoder result(obj.value.data(), obj.value.size());
   result.parent = this;
   return result;
   }

/*
* Finish decoding a CONSTRUCTED type
*/
BER_Decoder& BER_Decoder::end_cons()
   {
   if(!parent)
      throw Invalid_State("BER_Decoder::end_cons called with NULL parent");
   if(!source->end_of_data())
      throw Decoding_Error("BER_Decoder::end_cons called with data left");
   return (*parent);
   }

/*
* BER_Decoder Constructor
*/
BER_Decoder::BER_Decoder(DataSource& src)
   {
   source = &src;
   owns = false;
   pushed.type_tag = pushed.class_tag = NO_OBJECT;
   parent = nullptr;
   }

/*
* BER_Decoder Constructor
 */
BER_Decoder::BER_Decoder(const byte data[], size_t length)
   {
   source = new DataSource_Memory(data, length);
   owns = true;
   pushed.type_tag = pushed.class_tag = NO_OBJECT;
   parent = nullptr;
   }

/*
* BER_Decoder Constructor
*/
BER_Decoder::BER_Decoder(const secure_vector<byte>& data)
   {
   source = new DataSource_Memory(data);
   owns = true;
   pushed.type_tag = pushed.class_tag = NO_OBJECT;
   parent = nullptr;
   }

/*
* BER_Decoder Constructor
*/
BER_Decoder::BER_Decoder(const std::vector<byte>& data)
   {
   source = new DataSource_Memory(data.data(), data.size());
   owns = true;
   pushed.type_tag = pushed.class_tag = NO_OBJECT;
   parent = nullptr;
   }

/*
* BER_Decoder Copy Constructor
*/
BER_Decoder::BER_Decoder(const BER_Decoder& other)
   {
   source = other.source;
   owns = false;
   if(other.owns)
      {
      other.owns = false;
      owns = true;
      }
   pushed.type_tag = pushed.class_tag = NO_OBJECT;
   parent = other.parent;
   }

/*
* BER_Decoder Destructor
*/
BER_Decoder::~BER_Decoder()
   {
   if(owns)
      delete source;
   source = nullptr;
   }

/*
* Request for an object to decode itself
*/
BER_Decoder& BER_Decoder::decode(ASN1_Object& obj,
                                 ASN1_Tag, ASN1_Tag)
   {
   obj.decode_from(*this);
   return (*this);
   }

/*
* Decode a BER encoded NULL
*/
BER_Decoder& BER_Decoder::decode_null()
   {
   BER_Object obj = get_next_object();
   obj.assert_is_a(NULL_TAG, UNIVERSAL);
   if(obj.value.size())
      throw BER_Decoding_Error("NULL object had nonzero size");
   return (*this);
   }

/*
* Decode a BER encoded BOOLEAN
*/
BER_Decoder& BER_Decoder::decode(bool& out)
   {
   return decode(out, BOOLEAN, UNIVERSAL);
   }

/*
* Decode a small BER encoded INTEGER
*/
BER_Decoder& BER_Decoder::decode(size_t& out)
   {
   return decode(out, INTEGER, UNIVERSAL);
   }

/*
* Decode a BER encoded INTEGER
*/
BER_Decoder& BER_Decoder::decode(BigInt& out)
   {
   return decode(out, INTEGER, UNIVERSAL);
   }

BER_Decoder& BER_Decoder::decode_octet_string_bigint(BigInt& out)
   {
   secure_vector<byte> out_vec;
   decode(out_vec, OCTET_STRING);
   out = BigInt::decode(out_vec.data(), out_vec.size());
   return (*this);
   }

std::vector<byte> BER_Decoder::get_next_octet_string()
   {
   std::vector<byte> out_vec;
   decode(out_vec, OCTET_STRING);
   return out_vec;
   }

/*
* Decode a BER encoded BOOLEAN
*/
BER_Decoder& BER_Decoder::decode(bool& out,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   BER_Object obj = get_next_object();
   obj.assert_is_a(type_tag, class_tag);

   if(obj.value.size() != 1)
      throw BER_Decoding_Error("BER boolean value had invalid size");

   out = (obj.value[0]) ? true : false;
   return (*this);
   }

/*
* Decode a small BER encoded INTEGER
*/
BER_Decoder& BER_Decoder::decode(size_t& out,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   BigInt integer;
   decode(integer, type_tag, class_tag);

   if(integer.bits() > 32)
      throw BER_Decoding_Error("Decoded integer value larger than expected");

   out = 0;
   for(size_t i = 0; i != 4; ++i)
      out = (out << 8) | integer.byte_at(3-i);

   return (*this);
   }

/*
* Decode a small BER encoded INTEGER
*/
u64bit BER_Decoder::decode_constrained_integer(ASN1_Tag type_tag,
                                               ASN1_Tag class_tag,
                                               size_t T_bytes)
   {
   if(T_bytes > 8)
      throw BER_Decoding_Error("Can't decode small integer over 8 bytes");

   BigInt integer;
   decode(integer, type_tag, class_tag);

   if(integer.bits() > 8*T_bytes)
      throw BER_Decoding_Error("Decoded integer value larger than expected");

   u64bit out = 0;
   for(size_t i = 0; i != 8; ++i)
      out = (out << 8) | integer.byte_at(7-i);

   return out;
   }

/*
* Decode a BER encoded INTEGER
*/
BER_Decoder& BER_Decoder::decode(BigInt& out,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   BER_Object obj = get_next_object();
   obj.assert_is_a(type_tag, class_tag);

   if(obj.value.empty())
      out = 0;
   else
      {
      const bool negative = (obj.value[0] & 0x80) ? true : false;

      if(negative)
         {
         for(size_t i = obj.value.size(); i > 0; --i)
            if(obj.value[i-1]--)
               break;
         for(size_t i = 0; i != obj.value.size(); ++i)
            obj.value[i] = ~obj.value[i];
         }

      out = BigInt(&obj.value[0], obj.value.size());

      if(negative)
         out.flip_sign();
      }

   return (*this);
   }

/*
* BER decode a BIT STRING or OCTET STRING
*/
BER_Decoder& BER_Decoder::decode(secure_vector<byte>& out, ASN1_Tag real_type)
   {
   return decode(out, real_type, real_type, UNIVERSAL);
   }

/*
* BER decode a BIT STRING or OCTET STRING
*/
BER_Decoder& BER_Decoder::decode(std::vector<byte>& out, ASN1_Tag real_type)
   {
   return decode(out, real_type, real_type, UNIVERSAL);
   }

/*
* BER decode a BIT STRING or OCTET STRING
*/
BER_Decoder& BER_Decoder::decode(secure_vector<byte>& buffer,
                                 ASN1_Tag real_type,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   if(real_type != OCTET_STRING && real_type != BIT_STRING)
      throw BER_Bad_Tag("Bad tag for {BIT,OCTET} STRING", real_type);

   BER_Object obj = get_next_object();
   obj.assert_is_a(type_tag, class_tag);

   if(real_type == OCTET_STRING)
      buffer = obj.value;
   else
      {
      if(obj.value.empty())
         throw BER_Decoding_Error("Invalid BIT STRING");
      if(obj.value[0] >= 8)
         throw BER_Decoding_Error("Bad number of unused bits in BIT STRING");

      buffer.resize(obj.value.size() - 1);
      copy_mem(buffer.data(), &obj.value[1], obj.value.size() - 1);
      }
   return (*this);
   }

BER_Decoder& BER_Decoder::decode(std::vector<byte>& buffer,
                                 ASN1_Tag real_type,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   if(real_type != OCTET_STRING && real_type != BIT_STRING)
      throw BER_Bad_Tag("Bad tag for {BIT,OCTET} STRING", real_type);

   BER_Object obj = get_next_object();
   obj.assert_is_a(type_tag, class_tag);

   if(real_type == OCTET_STRING)
      buffer = unlock(obj.value);
   else
      {
      if(obj.value.empty())
         throw BER_Decoding_Error("Invalid BIT STRING");
      if(obj.value[0] >= 8)
         throw BER_Decoding_Error("Bad number of unused bits in BIT STRING");

      buffer.resize(obj.value.size() - 1);
      copy_mem(buffer.data(), &obj.value[1], obj.value.size() - 1);
      }
   return (*this);
   }

}
/*
* DER Encoder
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

/*
* DER encode an ASN.1 type tag
*/
secure_vector<byte> encode_tag(ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   if((class_tag | 0xE0) != 0xE0)
      throw Encoding_Error("DER_Encoder: Invalid class tag " +
                           std::to_string(class_tag));

   secure_vector<byte> encoded_tag;
   if(type_tag <= 30)
      encoded_tag.push_back(static_cast<byte>(type_tag | class_tag));
   else
      {
      size_t blocks = high_bit(type_tag) + 6;
      blocks = (blocks - (blocks % 7)) / 7;

      encoded_tag.push_back(class_tag | 0x1F);
      for(size_t i = 0; i != blocks - 1; ++i)
         encoded_tag.push_back(0x80 | ((type_tag >> 7*(blocks-i-1)) & 0x7F));
      encoded_tag.push_back(type_tag & 0x7F);
      }

   return encoded_tag;
   }

/*
* DER encode an ASN.1 length field
*/
secure_vector<byte> encode_length(size_t length)
   {
   secure_vector<byte> encoded_length;
   if(length <= 127)
      encoded_length.push_back(static_cast<byte>(length));
   else
      {
      const size_t top_byte = significant_bytes(length);

      encoded_length.push_back(static_cast<byte>(0x80 | top_byte));

      for(size_t i = sizeof(length) - top_byte; i != sizeof(length); ++i)
         encoded_length.push_back(get_byte(i, length));
      }
   return encoded_length;
   }

}

/*
* Return the encoded SEQUENCE/SET
*/
secure_vector<byte> DER_Encoder::DER_Sequence::get_contents()
   {
   const ASN1_Tag real_class_tag = ASN1_Tag(class_tag | CONSTRUCTED);

   if(type_tag == SET)
      {
      std::sort(set_contents.begin(), set_contents.end());
      for(size_t i = 0; i != set_contents.size(); ++i)
         contents += set_contents[i];
      set_contents.clear();
      }

   secure_vector<byte> result;
   result += encode_tag(type_tag, real_class_tag);
   result += encode_length(contents.size());
   result += contents;
   contents.clear();

   return result;
   }

/*
* Add an encoded value to the SEQUENCE/SET
*/
void DER_Encoder::DER_Sequence::add_bytes(const byte data[], size_t length)
   {
   if(type_tag == SET)
      set_contents.push_back(secure_vector<byte>(data, data + length));
   else
      contents += std::make_pair(data, length);
   }

/*
* Return the type and class taggings
*/
ASN1_Tag DER_Encoder::DER_Sequence::tag_of() const
   {
   return ASN1_Tag(type_tag | class_tag);
   }

/*
* DER_Sequence Constructor
*/
DER_Encoder::DER_Sequence::DER_Sequence(ASN1_Tag t1, ASN1_Tag t2) :
   type_tag(t1), class_tag(t2)
   {
   }

/*
* Return the encoded contents
*/
secure_vector<byte> DER_Encoder::get_contents()
   {
   if(subsequences.size() != 0)
      throw Invalid_State("DER_Encoder: Sequence hasn't been marked done");

   secure_vector<byte> output;
   std::swap(output, contents);
   return output;
   }

/*
* Start a new ASN.1 SEQUENCE/SET/EXPLICIT
*/
DER_Encoder& DER_Encoder::start_cons(ASN1_Tag type_tag,
                                     ASN1_Tag class_tag)
   {
   subsequences.push_back(DER_Sequence(type_tag, class_tag));
   return (*this);
   }

/*
* Finish the current ASN.1 SEQUENCE/SET/EXPLICIT
*/
DER_Encoder& DER_Encoder::end_cons()
   {
   if(subsequences.empty())
      throw Invalid_State("DER_Encoder::end_cons: No such sequence");

   secure_vector<byte> seq = subsequences[subsequences.size()-1].get_contents();
   subsequences.pop_back();
   raw_bytes(seq);
   return (*this);
   }

/*
* Start a new ASN.1 EXPLICIT encoding
*/
DER_Encoder& DER_Encoder::start_explicit(u16bit type_no)
   {
   ASN1_Tag type_tag = static_cast<ASN1_Tag>(type_no);

   if(type_tag == SET)
      throw Internal_Error("DER_Encoder.start_explicit(SET); cannot perform");

   return start_cons(type_tag, CONTEXT_SPECIFIC);
   }

/*
* Finish the current ASN.1 EXPLICIT encoding
*/
DER_Encoder& DER_Encoder::end_explicit()
   {
   return end_cons();
   }

/*
* Write raw bytes into the stream
*/
DER_Encoder& DER_Encoder::raw_bytes(const secure_vector<byte>& val)
   {
   return raw_bytes(val.data(), val.size());
   }

DER_Encoder& DER_Encoder::raw_bytes(const std::vector<byte>& val)
   {
   return raw_bytes(val.data(), val.size());
   }

/*
* Write raw bytes into the stream
*/
DER_Encoder& DER_Encoder::raw_bytes(const byte bytes[], size_t length)
   {
   if(subsequences.size())
      subsequences[subsequences.size()-1].add_bytes(bytes, length);
   else
      contents += std::make_pair(bytes, length);

   return (*this);
   }

/*
* Encode a NULL object
*/
DER_Encoder& DER_Encoder::encode_null()
   {
   return add_object(NULL_TAG, UNIVERSAL, nullptr, 0);
   }

/*
* DER encode a BOOLEAN
*/
DER_Encoder& DER_Encoder::encode(bool is_true)
   {
   return encode(is_true, BOOLEAN, UNIVERSAL);
   }

/*
* DER encode a small INTEGER
*/
DER_Encoder& DER_Encoder::encode(size_t n)
   {
   return encode(BigInt(n), INTEGER, UNIVERSAL);
   }

/*
* DER encode a small INTEGER
*/
DER_Encoder& DER_Encoder::encode(const BigInt& n)
   {
   return encode(n, INTEGER, UNIVERSAL);
   }

/*
* DER encode an OCTET STRING or BIT STRING
*/
DER_Encoder& DER_Encoder::encode(const secure_vector<byte>& bytes,
                                 ASN1_Tag real_type)
   {
   return encode(bytes.data(), bytes.size(),
                 real_type, real_type, UNIVERSAL);
   }

/*
* DER encode an OCTET STRING or BIT STRING
*/
DER_Encoder& DER_Encoder::encode(const std::vector<byte>& bytes,
                                 ASN1_Tag real_type)
   {
   return encode(bytes.data(), bytes.size(),
                 real_type, real_type, UNIVERSAL);
   }

/*
* Encode this object
*/
DER_Encoder& DER_Encoder::encode(const byte bytes[], size_t length,
                                 ASN1_Tag real_type)
   {
   return encode(bytes, length, real_type, real_type, UNIVERSAL);
   }

/*
* DER encode a BOOLEAN
*/
DER_Encoder& DER_Encoder::encode(bool is_true,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   byte val = is_true ? 0xFF : 0x00;
   return add_object(type_tag, class_tag, &val, 1);
   }

/*
* DER encode a small INTEGER
*/
DER_Encoder& DER_Encoder::encode(size_t n,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   return encode(BigInt(n), type_tag, class_tag);
   }

/*
* DER encode an INTEGER
*/
DER_Encoder& DER_Encoder::encode(const BigInt& n,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   if(n == 0)
      return add_object(type_tag, class_tag, 0);

   bool extra_zero = (n.bits() % 8 == 0);
   secure_vector<byte> contents(extra_zero + n.bytes());
   BigInt::encode(&contents[extra_zero], n);
   if(n < 0)
      {
      for(size_t i = 0; i != contents.size(); ++i)
         contents[i] = ~contents[i];
      for(size_t i = contents.size(); i > 0; --i)
         if(++contents[i-1])
            break;
      }

   return add_object(type_tag, class_tag, contents);
   }

/*
* DER encode an OCTET STRING or BIT STRING
*/
DER_Encoder& DER_Encoder::encode(const secure_vector<byte>& bytes,
                                 ASN1_Tag real_type,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   return encode(bytes.data(), bytes.size(),
                 real_type, type_tag, class_tag);
   }

/*
* DER encode an OCTET STRING or BIT STRING
*/
DER_Encoder& DER_Encoder::encode(const std::vector<byte>& bytes,
                                 ASN1_Tag real_type,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   return encode(bytes.data(), bytes.size(),
                 real_type, type_tag, class_tag);
   }

/*
* DER encode an OCTET STRING or BIT STRING
*/
DER_Encoder& DER_Encoder::encode(const byte bytes[], size_t length,
                                 ASN1_Tag real_type,
                                 ASN1_Tag type_tag, ASN1_Tag class_tag)
   {
   if(real_type != OCTET_STRING && real_type != BIT_STRING)
      throw Invalid_Argument("DER_Encoder: Invalid tag for byte/bit string");

   if(real_type == BIT_STRING)
      {
      secure_vector<byte> encoded;
      encoded.push_back(0);
      encoded += std::make_pair(bytes, length);
      return add_object(type_tag, class_tag, encoded);
      }
   else
      return add_object(type_tag, class_tag, bytes, length);
   }

/*
* Conditionally write some values to the stream
*/
DER_Encoder& DER_Encoder::encode_if(bool cond, DER_Encoder& codec)
   {
   if(cond)
      return raw_bytes(codec.get_contents());
   return (*this);
   }

DER_Encoder& DER_Encoder::encode_if(bool cond, const ASN1_Object& obj)
   {
   if(cond)
      encode(obj);
   return (*this);
   }

/*
* Request for an object to encode itself
*/
DER_Encoder& DER_Encoder::encode(const ASN1_Object& obj)
   {
   obj.encode_into(*this);
   return (*this);
   }

/*
* Write the encoding of the byte(s)
*/
DER_Encoder& DER_Encoder::add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                                     const byte rep[], size_t length)
   {
   secure_vector<byte> buffer;
   buffer += encode_tag(type_tag, class_tag);
   buffer += encode_length(length);
   buffer += std::make_pair(rep, length);

   return raw_bytes(buffer);
   }

/*
* Write the encoding of the byte(s)
*/
DER_Encoder& DER_Encoder::add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                                     const std::string& rep_str)
   {
   const byte* rep = reinterpret_cast<const byte*>(rep_str.data());
   const size_t rep_len = rep_str.size();
   return add_object(type_tag, class_tag, rep, rep_len);
   }

/*
* Write the encoding of the byte
*/
DER_Encoder& DER_Encoder::add_object(ASN1_Tag type_tag,
                                     ASN1_Tag class_tag, byte rep)
   {
   return add_object(type_tag, class_tag, &rep, 1);
   }

}
/*
* X509_DN
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <ostream>

namespace Botan {

/*
* Create an empty X509_DN
*/
X509_DN::X509_DN()
   {
   }

/*
* Create an X509_DN
*/
X509_DN::X509_DN(const std::multimap<OID, std::string>& args)
   {
   for(auto i = args.begin(); i != args.end(); ++i)
      add_attribute(i->first, i->second);
   }

/*
* Create an X509_DN
*/
X509_DN::X509_DN(const std::multimap<std::string, std::string>& args)
   {
   for(auto i = args.begin(); i != args.end(); ++i)
      add_attribute(OIDS::lookup(i->first), i->second);
   }

/*
* Add an attribute to a X509_DN
*/
void X509_DN::add_attribute(const std::string& type,
                            const std::string& str)
   {
   OID oid = OIDS::lookup(type);
   add_attribute(oid, str);
   }

/*
* Add an attribute to a X509_DN
*/
void X509_DN::add_attribute(const OID& oid, const std::string& str)
   {
   if(str == "")
      return;

   auto range = dn_info.equal_range(oid);
   for(auto i = range.first; i != range.second; ++i)
      if(i->second.value() == str)
         return;

   multimap_insert(dn_info, oid, ASN1_String(str));
   dn_bits.clear();
   }

/*
* Get the attributes of this X509_DN
*/
std::multimap<OID, std::string> X509_DN::get_attributes() const
   {
   std::multimap<OID, std::string> retval;
   for(auto i = dn_info.begin(); i != dn_info.end(); ++i)
      multimap_insert(retval, i->first, i->second.value());
   return retval;
   }

/*
* Get the contents of this X.500 Name
*/
std::multimap<std::string, std::string> X509_DN::contents() const
   {
   std::multimap<std::string, std::string> retval;
   for(auto i = dn_info.begin(); i != dn_info.end(); ++i)
      multimap_insert(retval, OIDS::lookup(i->first), i->second.value());
   return retval;
   }

/*
* Get a single attribute type
*/
std::vector<std::string> X509_DN::get_attribute(const std::string& attr) const
   {
   const OID oid = OIDS::lookup(deref_info_field(attr));

   auto range = dn_info.equal_range(oid);

   std::vector<std::string> values;
   for(auto i = range.first; i != range.second; ++i)
      values.push_back(i->second.value());
   return values;
   }

/*
* Return the BER encoded data, if any
*/
std::vector<byte> X509_DN::get_bits() const
   {
   return dn_bits;
   }

/*
* Deref aliases in a subject/issuer info request
*/
std::string X509_DN::deref_info_field(const std::string& info)
   {
   if(info == "Name" || info == "CommonName") return "X520.CommonName";
   if(info == "SerialNumber")                 return "X520.SerialNumber";
   if(info == "Country")                      return "X520.Country";
   if(info == "Organization")                 return "X520.Organization";
   if(info == "Organizational Unit" || info == "OrgUnit")
      return "X520.OrganizationalUnit";
   if(info == "Locality")                     return "X520.Locality";
   if(info == "State" || info == "Province")  return "X520.State";
   if(info == "Email")                        return "RFC822";
   return info;
   }

/*
* Compare two X509_DNs for equality
*/
bool operator==(const X509_DN& dn1, const X509_DN& dn2)
   {
   auto attr1 = dn1.get_attributes();
   auto attr2 = dn2.get_attributes();

   if(attr1.size() != attr2.size()) return false;

   auto p1 = attr1.begin();
   auto p2 = attr2.begin();

   while(true)
      {
      if(p1 == attr1.end() && p2 == attr2.end())
         break;
      if(p1 == attr1.end())      return false;
      if(p2 == attr2.end())      return false;
      if(p1->first != p2->first) return false;
      if(!x500_name_cmp(p1->second, p2->second))
         return false;
      ++p1;
      ++p2;
      }
   return true;
   }

/*
* Compare two X509_DNs for inequality
*/
bool operator!=(const X509_DN& dn1, const X509_DN& dn2)
   {
   return !(dn1 == dn2);
   }

/*
* Induce an arbitrary ordering on DNs
*/
bool operator<(const X509_DN& dn1, const X509_DN& dn2)
   {
   auto attr1 = dn1.get_attributes();
   auto attr2 = dn2.get_attributes();

   if(attr1.size() < attr2.size()) return true;
   if(attr1.size() > attr2.size()) return false;

   for(auto p1 = attr1.begin(); p1 != attr1.end(); ++p1)
      {
      auto p2 = attr2.find(p1->first);
      if(p2 == attr2.end())       return false;
      if(p1->second > p2->second) return false;
      if(p1->second < p2->second) return true;
      }
   return false;
   }

namespace {

/*
* DER encode a RelativeDistinguishedName
*/
void do_ava(DER_Encoder& encoder,
            const std::multimap<OID, std::string>& dn_info,
            ASN1_Tag string_type, const std::string& oid_str,
            bool must_exist = false)
   {
   const OID oid = OIDS::lookup(oid_str);
   const bool exists = (dn_info.find(oid) != dn_info.end());

   if(!exists && must_exist)
      throw Encoding_Error("X509_DN: No entry for " + oid_str);
   if(!exists) return;

   auto range = dn_info.equal_range(oid);

   for(auto i = range.first; i != range.second; ++i)
      {
      encoder.start_cons(SET)
         .start_cons(SEQUENCE)
            .encode(oid)
            .encode(ASN1_String(i->second, string_type))
         .end_cons()
      .end_cons();
      }
   }

}

/*
* DER encode a DistinguishedName
*/
void X509_DN::encode_into(DER_Encoder& der) const
   {
   auto dn_info = get_attributes();

   der.start_cons(SEQUENCE);

   if(!dn_bits.empty())
      der.raw_bytes(dn_bits);
   else
      {
      do_ava(der, dn_info, PRINTABLE_STRING, "X520.Country");
      do_ava(der, dn_info, DIRECTORY_STRING, "X520.State");
      do_ava(der, dn_info, DIRECTORY_STRING, "X520.Locality");
      do_ava(der, dn_info, DIRECTORY_STRING, "X520.Organization");
      do_ava(der, dn_info, DIRECTORY_STRING, "X520.OrganizationalUnit");
      do_ava(der, dn_info, DIRECTORY_STRING, "X520.CommonName");
      do_ava(der, dn_info, PRINTABLE_STRING, "X520.SerialNumber");
      }

   der.end_cons();
   }

/*
* Decode a BER encoded DistinguishedName
*/
void X509_DN::decode_from(BER_Decoder& source)
   {
   std::vector<byte> bits;

   source.start_cons(SEQUENCE)
      .raw_bytes(bits)
   .end_cons();

   BER_Decoder sequence(bits);

   while(sequence.more_items())
      {
      BER_Decoder rdn = sequence.start_cons(SET);

      while(rdn.more_items())
         {
         OID oid;
         ASN1_String str;

         rdn.start_cons(SEQUENCE)
            .decode(oid)
            .decode(str)
            .verify_end()
        .end_cons();

         add_attribute(oid, str.value());
         }
      }

   dn_bits = bits;
   }

namespace {

std::string to_short_form(const std::string& long_id)
   {
   if(long_id == "X520.CommonName")
      return "CN";

   if(long_id == "X520.Organization")
      return "O";

   if(long_id == "X520.OrganizationalUnit")
      return "OU";

   return long_id;
   }

}

std::ostream& operator<<(std::ostream& out, const X509_DN& dn)
   {
   std::multimap<std::string, std::string> contents = dn.contents();

   for(std::multimap<std::string, std::string>::const_iterator i = contents.begin();
       i != contents.end(); ++i)
      {
      out << to_short_form(i->first) << "=" << i->second << ' ';
      }
   return out;
   }

}
/*
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

//static
void LibraryInitializer::initialize(const std::string&)
   {
   // none needed currently
   }

//static
void LibraryInitializer::deinitialize()
   {
   // none needed currently
   }

}
/*
* SCAN Name Abstraction
* (C) 2008-2009,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

std::string make_arg(
   const std::vector<std::pair<size_t, std::string> >& name, size_t start)
   {
   std::string output = name[start].second;
   size_t level = name[start].first;

   size_t paren_depth = 0;

   for(size_t i = start + 1; i != name.size(); ++i)
      {
      if(name[i].first <= name[start].first)
         break;

      if(name[i].first > level)
         {
         output += "(" + name[i].second;
         ++paren_depth;
         }
      else if(name[i].first < level)
         {
         output += ")," + name[i].second;
         --paren_depth;
         }
      else
         {
         if(output[output.size() - 1] != '(')
            output += ",";
         output += name[i].second;
         }

      level = name[i].first;
      }

   for(size_t i = 0; i != paren_depth; ++i)
      output += ")";

   return output;
   }

std::pair<size_t, std::string>
deref_aliases(const std::pair<size_t, std::string>& in)
   {
   return std::make_pair(in.first,
                         SCAN_Name::deref_alias(in.second));
   }

}

SCAN_Name::SCAN_Name(std::string algo_spec, const std::string& extra) : SCAN_Name(algo_spec)
   {
   alg_name += extra;
   }

SCAN_Name::SCAN_Name(const char* algo_spec) : SCAN_Name(std::string(algo_spec))
   {
   }

SCAN_Name::SCAN_Name(std::string algo_spec)
   {
   orig_algo_spec = algo_spec;

   std::vector<std::pair<size_t, std::string> > name;
   size_t level = 0;
   std::pair<size_t, std::string> accum = std::make_pair(level, "");

   const std::string decoding_error = "Bad SCAN name '" + algo_spec + "': ";

   algo_spec = SCAN_Name::deref_alias(algo_spec);

   for(size_t i = 0; i != algo_spec.size(); ++i)
      {
      char c = algo_spec[i];

      if(c == '/' || c == ',' || c == '(' || c == ')')
         {
         if(c == '(')
            ++level;
         else if(c == ')')
            {
            if(level == 0)
               throw Decoding_Error(decoding_error + "Mismatched parens");
            --level;
            }

         if(c == '/' && level > 0)
            accum.second.push_back(c);
         else
            {
            if(accum.second != "")
               name.push_back(deref_aliases(accum));
            accum = std::make_pair(level, "");
            }
         }
      else
         accum.second.push_back(c);
      }

   if(accum.second != "")
      name.push_back(deref_aliases(accum));

   if(level != 0)
      throw Decoding_Error(decoding_error + "Missing close paren");

   if(name.size() == 0)
      throw Decoding_Error(decoding_error + "Empty name");

   alg_name = name[0].second;

   bool in_modes = false;

   for(size_t i = 1; i != name.size(); ++i)
      {
      if(name[i].first == 0)
         {
         mode_info.push_back(make_arg(name, i));
         in_modes = true;
         }
      else if(name[i].first == 1 && !in_modes)
         args.push_back(make_arg(name, i));
      }
   }

std::string SCAN_Name::all_arguments() const
   {
   std::string out;
   if(arg_count())
      {
      out += "(";
      for(size_t i = 0; i != arg_count(); ++i)
         {
         out += arg(i);
         if(i != arg_count() - 1)
            out += ",";
         }
      out += ")";
      }
   return out;
   }

std::string SCAN_Name::arg(size_t i) const
   {
   if(i >= arg_count())
      throw std::range_error("SCAN_Name::arg " + std::to_string(i) +
                             " out of range for '" + as_string() + "'");
   return args[i];
   }

std::string SCAN_Name::arg(size_t i, const std::string& def_value) const
   {
   if(i >= arg_count())
      return def_value;
   return args[i];
   }

size_t SCAN_Name::arg_as_integer(size_t i, size_t def_value) const
   {
   if(i >= arg_count())
      return def_value;
   return to_u32bit(args[i]);
   }

std::mutex SCAN_Name::g_alias_map_mutex;
std::map<std::string, std::string> SCAN_Name::g_alias_map = {
   { "3DES",            "TripleDES" },
   { "ARC4",            "RC4" },
   { "CAST5",           "CAST-128" },
   { "DES-EDE",         "TripleDES" },
   { "EME-OAEP",        "OAEP" },
   { "EME-PKCS1-v1_5",  "PKCS1v15" },
   { "EME1",            "OAEP" },
   { "EMSA-PKCS1-v1_5", "EMSA_PKCS1" },
   { "EMSA-PSS",        "PSSR" },
   { "EMSA2",           "EMSA_X931" },
   { "EMSA3",           "EMSA_PKCS1" },
   { "EMSA4",           "PSSR" },
   { "GOST-34.11",      "GOST-R-34.11-94" },
   { "MARK-4",          "RC4(256)" },
   { "OMAC",            "CMAC" },
   { "PSS-MGF1",        "PSSR" },
   { "SHA-1",           "SHA-160" },
   { "SHA1",            "SHA-160" },
   { "X9.31",           "EMSA2" }
};

void SCAN_Name::add_alias(const std::string& alias, const std::string& basename)
   {
   std::lock_guard<std::mutex> lock(g_alias_map_mutex);

   if(g_alias_map.find(alias) == g_alias_map.end())
      g_alias_map[alias] = basename;
   }

std::string SCAN_Name::deref_alias(const std::string& alias)
   {
   std::lock_guard<std::mutex> lock(g_alias_map_mutex);

   std::string name = alias;

   for(auto i = g_alias_map.find(name); i != g_alias_map.end(); i = g_alias_map.find(name))
      name = i->second;

   return name;
   }

}
/*
* OctetString
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Create an OctetString from RNG output
*/
OctetString::OctetString(RandomNumberGenerator& rng,
                         size_t length)
   {
   bits = rng.random_vec(length);
   }

/*
* Create an OctetString from a hex string
*/
OctetString::OctetString(const std::string& hex_string)
   {
   bits.resize(1 + hex_string.length() / 2);
   bits.resize(hex_decode(bits.data(), hex_string));
   }

/*
* Create an OctetString from a byte string
*/
OctetString::OctetString(const byte in[], size_t n)
   {
   bits.assign(in, in + n);
   }

/*
* Set the parity of each key byte to odd
*/
void OctetString::set_odd_parity()
   {
   const byte ODD_PARITY[256] = {
      0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x07, 0x07, 0x08, 0x08, 0x0B, 0x0B,
      0x0D, 0x0D, 0x0E, 0x0E, 0x10, 0x10, 0x13, 0x13, 0x15, 0x15, 0x16, 0x16,
      0x19, 0x19, 0x1A, 0x1A, 0x1C, 0x1C, 0x1F, 0x1F, 0x20, 0x20, 0x23, 0x23,
      0x25, 0x25, 0x26, 0x26, 0x29, 0x29, 0x2A, 0x2A, 0x2C, 0x2C, 0x2F, 0x2F,
      0x31, 0x31, 0x32, 0x32, 0x34, 0x34, 0x37, 0x37, 0x38, 0x38, 0x3B, 0x3B,
      0x3D, 0x3D, 0x3E, 0x3E, 0x40, 0x40, 0x43, 0x43, 0x45, 0x45, 0x46, 0x46,
      0x49, 0x49, 0x4A, 0x4A, 0x4C, 0x4C, 0x4F, 0x4F, 0x51, 0x51, 0x52, 0x52,
      0x54, 0x54, 0x57, 0x57, 0x58, 0x58, 0x5B, 0x5B, 0x5D, 0x5D, 0x5E, 0x5E,
      0x61, 0x61, 0x62, 0x62, 0x64, 0x64, 0x67, 0x67, 0x68, 0x68, 0x6B, 0x6B,
      0x6D, 0x6D, 0x6E, 0x6E, 0x70, 0x70, 0x73, 0x73, 0x75, 0x75, 0x76, 0x76,
      0x79, 0x79, 0x7A, 0x7A, 0x7C, 0x7C, 0x7F, 0x7F, 0x80, 0x80, 0x83, 0x83,
      0x85, 0x85, 0x86, 0x86, 0x89, 0x89, 0x8A, 0x8A, 0x8C, 0x8C, 0x8F, 0x8F,
      0x91, 0x91, 0x92, 0x92, 0x94, 0x94, 0x97, 0x97, 0x98, 0x98, 0x9B, 0x9B,
      0x9D, 0x9D, 0x9E, 0x9E, 0xA1, 0xA1, 0xA2, 0xA2, 0xA4, 0xA4, 0xA7, 0xA7,
      0xA8, 0xA8, 0xAB, 0xAB, 0xAD, 0xAD, 0xAE, 0xAE, 0xB0, 0xB0, 0xB3, 0xB3,
      0xB5, 0xB5, 0xB6, 0xB6, 0xB9, 0xB9, 0xBA, 0xBA, 0xBC, 0xBC, 0xBF, 0xBF,
      0xC1, 0xC1, 0xC2, 0xC2, 0xC4, 0xC4, 0xC7, 0xC7, 0xC8, 0xC8, 0xCB, 0xCB,
      0xCD, 0xCD, 0xCE, 0xCE, 0xD0, 0xD0, 0xD3, 0xD3, 0xD5, 0xD5, 0xD6, 0xD6,
      0xD9, 0xD9, 0xDA, 0xDA, 0xDC, 0xDC, 0xDF, 0xDF, 0xE0, 0xE0, 0xE3, 0xE3,
      0xE5, 0xE5, 0xE6, 0xE6, 0xE9, 0xE9, 0xEA, 0xEA, 0xEC, 0xEC, 0xEF, 0xEF,
      0xF1, 0xF1, 0xF2, 0xF2, 0xF4, 0xF4, 0xF7, 0xF7, 0xF8, 0xF8, 0xFB, 0xFB,
      0xFD, 0xFD, 0xFE, 0xFE };

   for(size_t j = 0; j != bits.size(); ++j)
      bits[j] = ODD_PARITY[bits[j]];
   }

/*
* Hex encode an OctetString
*/
std::string OctetString::as_string() const
   {
   return hex_encode(bits.data(), bits.size());
   }

/*
* XOR Operation for OctetStrings
*/
OctetString& OctetString::operator^=(const OctetString& k)
   {
   if(&k == this) { zeroise(bits); return (*this); }
   xor_buf(bits.data(), k.begin(), std::min(length(), k.length()));
   return (*this);
   }

/*
* Equality Operation for OctetStrings
*/
bool operator==(const OctetString& s1, const OctetString& s2)
   {
   return (s1.bits_of() == s2.bits_of());
   }

/*
* Unequality Operation for OctetStrings
*/
bool operator!=(const OctetString& s1, const OctetString& s2)
   {
   return !(s1 == s2);
   }

/*
* Append Operation for OctetStrings
*/
OctetString operator+(const OctetString& k1, const OctetString& k2)
   {
   secure_vector<byte> out;
   out += k1.bits_of();
   out += k2.bits_of();
   return OctetString(out);
   }

/*
* XOR Operation for OctetStrings
*/
OctetString operator^(const OctetString& k1, const OctetString& k2)
   {
   secure_vector<byte> ret(std::max(k1.length(), k2.length()));

   copy_mem(ret.data(), k1.begin(), k1.length());
   xor_buf(ret.data(), k2.begin(), k2.length());
   return OctetString(ret);
   }

}
/*
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

Transform* get_transform(const std::string& specstr,
                         const std::string& provider,
                         const std::string& dirstr)
   {
   Algo_Registry<Transform>::Spec spec(specstr, dirstr);
   return Algo_Registry<Transform>::global_registry().make(spec, provider);
   }

}
/*
* Base64 Encoding and Decoding
* (C) 2010,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

static const byte BIN_TO_BASE64[64] = {
   'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
   'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
   'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
   'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
   '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

void do_base64_encode(char out[4], const byte in[3])
   {
   out[0] = BIN_TO_BASE64[((in[0] & 0xFC) >> 2)];
   out[1] = BIN_TO_BASE64[((in[0] & 0x03) << 4) | (in[1] >> 4)];
   out[2] = BIN_TO_BASE64[((in[1] & 0x0F) << 2) | (in[2] >> 6)];
   out[3] = BIN_TO_BASE64[((in[2] & 0x3F)     )];
   }

}

size_t base64_encode(char out[],
                     const byte in[],
                     size_t input_length,
                     size_t& input_consumed,
                     bool final_inputs)
   {
   input_consumed = 0;

   size_t input_remaining = input_length;
   size_t output_produced = 0;

   while(input_remaining >= 3)
      {
      do_base64_encode(out + output_produced, in + input_consumed);

      input_consumed += 3;
      output_produced += 4;
      input_remaining -= 3;
      }

   if(final_inputs && input_remaining)
      {
      byte remainder[3] = { 0 };
      for(size_t i = 0; i != input_remaining; ++i)
         remainder[i] = in[input_consumed + i];

      do_base64_encode(out + output_produced, remainder);

      size_t empty_bits = 8 * (3 - input_remaining);
      size_t index = output_produced + 4 - 1;
      while(empty_bits >= 8)
         {
         out[index--] = '=';
         empty_bits -= 6;
         }

      input_consumed += input_remaining;
      output_produced += 4;
      }

   return output_produced;
   }

std::string base64_encode(const byte input[],
                          size_t input_length)
   {
   const size_t output_length = (round_up(input_length, 3) / 3) * 4;
   std::string output(output_length, 0);

   size_t consumed = 0;
   size_t produced = 0;
   
   if (output_length > 0)
   {
      produced = base64_encode(&output.front(),
                               input, input_length,
                               consumed, true);
   }

   BOTAN_ASSERT_EQUAL(consumed, input_length, "Consumed the entire input");
   BOTAN_ASSERT_EQUAL(produced, output.size(), "Produced expected size");

   return output;
   }

size_t base64_decode(byte output[],
                     const char input[],
                     size_t input_length,
                     size_t& input_consumed,
                     bool final_inputs,
                     bool ignore_ws)
   {
   /*
   * Base64 Decoder Lookup Table
   * Warning: assumes ASCII encodings
   */
   static const byte BASE64_TO_BIN[256] = {
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80,
      0x80, 0xFF, 0xFF, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF, 0xFF, 0x3F, 0x34, 0x35,
      0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0xFF, 0xFF,
      0xFF, 0x81, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04,
      0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
      0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
      0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1A, 0x1B, 0x1C,
      0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
      0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
      0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

   byte* out_ptr = output;
   byte decode_buf[4];
   size_t decode_buf_pos = 0;
   size_t final_truncate = 0;

   clear_mem(output, input_length * 3 / 4);

   for(size_t i = 0; i != input_length; ++i)
      {
      const byte bin = BASE64_TO_BIN[static_cast<byte>(input[i])];

      if(bin <= 0x3F)
         {
         decode_buf[decode_buf_pos] = bin;
         decode_buf_pos += 1;
         }
      else if(!(bin == 0x81 || (bin == 0x80 && ignore_ws)))
         {
         std::string bad_char(1, input[i]);
         if(bad_char == "\t")
           bad_char = "\\t";
         else if(bad_char == "\n")
           bad_char = "\\n";
         else if(bad_char == "\r")
           bad_char = "\\r";

         throw std::invalid_argument(
           std::string("base64_decode: invalid base64 character '") +
           bad_char + "'");
         }

      /*
      * If we're at the end of the input, pad with 0s and truncate
      */
      if(final_inputs && (i == input_length - 1))
         {
         if(decode_buf_pos)
            {
            for(size_t j = decode_buf_pos; j != 4; ++j)
               decode_buf[j] = 0;
            final_truncate = (4 - decode_buf_pos);
            decode_buf_pos = 4;
            }
         }

      if(decode_buf_pos == 4)
         {
         out_ptr[0] = (decode_buf[0] << 2) | (decode_buf[1] >> 4);
         out_ptr[1] = (decode_buf[1] << 4) | (decode_buf[2] >> 2);
         out_ptr[2] = (decode_buf[2] << 6) | decode_buf[3];

         out_ptr += 3;
         decode_buf_pos = 0;
         input_consumed = i+1;
         }
      }

   while(input_consumed < input_length &&
         BASE64_TO_BIN[static_cast<byte>(input[input_consumed])] == 0x80)
      {
      ++input_consumed;
      }

   size_t written = (out_ptr - output) - final_truncate;

   return written;
   }

size_t base64_decode(byte output[],
                     const char input[],
                     size_t input_length,
                     bool ignore_ws)
   {
   size_t consumed = 0;
   size_t written = base64_decode(output, input, input_length,
                                  consumed, true, ignore_ws);

   if(consumed != input_length)
      throw std::invalid_argument("base64_decode: input did not have full bytes");

   return written;
   }

size_t base64_decode(byte output[],
                     const std::string& input,
                     bool ignore_ws)
   {
   return base64_decode(output, input.data(), input.length(), ignore_ws);
   }

secure_vector<byte> base64_decode(const char input[],
                                 size_t input_length,
                                 bool ignore_ws)
   {
   const size_t output_length = (round_up(input_length, 4) * 3) / 4;
   secure_vector<byte> bin(output_length);

   size_t written = base64_decode(bin.data(),
                                  input,
                                  input_length,
                                  ignore_ws);

   bin.resize(written);
   return bin;
   }

secure_vector<byte> base64_decode(const std::string& input,
                                 bool ignore_ws)
   {
   return base64_decode(input.data(), input.size(), ignore_ws);
   }


}
/*
* BigInt Encoding/Decoding
* (C) 1999-2010,2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Encode a BigInt
*/
void BigInt::encode(byte output[], const BigInt& n, Base base)
   {
   if(base == Binary)
      {
      n.binary_encode(output);
      }
   else if(base == Hexadecimal)
      {
      secure_vector<byte> binary(n.encoded_size(Binary));
      n.binary_encode(binary.data());

      hex_encode(reinterpret_cast<char*>(output),
                 binary.data(), binary.size());
      }
   else if(base == Decimal)
      {
      BigInt copy = n;
      BigInt remainder;
      copy.set_sign(Positive);
      const size_t output_size = n.encoded_size(Decimal);
      for(size_t j = 0; j != output_size; ++j)
         {
         divide(copy, 10, copy, remainder);
         output[output_size - 1 - j] =
            Charset::digit2char(static_cast<byte>(remainder.word_at(0)));
         if(copy.is_zero())
            break;
         }
      }
   else
      throw Invalid_Argument("Unknown BigInt encoding method");
   }

/*
* Encode a BigInt
*/
std::vector<byte> BigInt::encode(const BigInt& n, Base base)
   {
   std::vector<byte> output(n.encoded_size(base));
   encode(output.data(), n, base);
   if(base != Binary)
      for(size_t j = 0; j != output.size(); ++j)
         if(output[j] == 0)
            output[j] = '0';
   return output;
   }

/*
* Encode a BigInt
*/
secure_vector<byte> BigInt::encode_locked(const BigInt& n, Base base)
   {
   secure_vector<byte> output(n.encoded_size(base));
   encode(output.data(), n, base);
   if(base != Binary)
      for(size_t j = 0; j != output.size(); ++j)
         if(output[j] == 0)
            output[j] = '0';
   return output;
   }

/*
* Encode a BigInt, with leading 0s if needed
*/
secure_vector<byte> BigInt::encode_1363(const BigInt& n, size_t bytes)
   {
   secure_vector<byte> output(bytes);
   BigInt::encode_1363(output.data(), output.size(), n);
   return output;
   }

//static
void BigInt::encode_1363(byte output[], size_t bytes, const BigInt& n)
   {
   const size_t n_bytes = n.bytes();
   if(n_bytes > bytes)
      throw Encoding_Error("encode_1363: n is too large to encode properly");

   const size_t leading_0s = bytes - n_bytes;
   encode(&output[leading_0s], n, Binary);
   }

/*
* Decode a BigInt
*/
BigInt BigInt::decode(const byte buf[], size_t length, Base base)
   {
   BigInt r;
   if(base == Binary)
      r.binary_decode(buf, length);
   else if(base == Hexadecimal)
      {
      secure_vector<byte> binary;

      if(length % 2)
         {
         // Handle lack of leading 0
         const char buf0_with_leading_0[2] =
            { '0', static_cast<char>(buf[0]) };

         binary = hex_decode_locked(buf0_with_leading_0, 2);

         binary += hex_decode_locked(reinterpret_cast<const char*>(&buf[1]),
                                     length - 1,
                                     false);
         }
      else
         binary = hex_decode_locked(reinterpret_cast<const char*>(buf),
                                    length, false);

      r.binary_decode(binary.data(), binary.size());
      }
   else if(base == Decimal)
      {
      for(size_t i = 0; i != length; ++i)
         {
         if(Charset::is_space(buf[i]))
            continue;

         if(!Charset::is_digit(buf[i]))
            throw Invalid_Argument("BigInt::decode: "
                                   "Invalid character in decimal input");

         const byte x = Charset::char2digit(buf[i]);

         if(x >= 10)
            throw Invalid_Argument("BigInt: Invalid decimal string");

         r *= 10;
         r += x;
         }
      }
   else
      throw Invalid_Argument("Unknown BigInt decoding method");
   return r;
   }

}
/*
* BigInt Input/Output
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <iostream>

namespace Botan {

/*
* Write the BigInt into a stream
*/
std::ostream& operator<<(std::ostream& stream, const BigInt& n)
   {
   BigInt::Base base = BigInt::Decimal;
   if(stream.flags() & std::ios::hex)
      base = BigInt::Hexadecimal;
   else if(stream.flags() & std::ios::oct)
      throw std::runtime_error("Octal output of BigInt not supported");

   if(n == 0)
      stream.write("0", 1);
   else
      {
      if(n < 0)
         stream.write("-", 1);
      const std::vector<byte> buffer = BigInt::encode(n, base);
      size_t skip = 0;
      while(skip < buffer.size() && buffer[skip] == '0')
         ++skip;
      stream.write(reinterpret_cast<const char*>(buffer.data()) + skip,
                   buffer.size() - skip);
      }
   if(!stream.good())
      throw Stream_IO_Error("BigInt output operator has failed");
   return stream;
   }

/*
* Read the BigInt from a stream
*/
std::istream& operator>>(std::istream& stream, BigInt& n)
   {
   std::string str;
   std::getline(stream, str);
   if(stream.bad() || (stream.fail() && !stream.eof()))
      throw Stream_IO_Error("BigInt input operator has failed");
   n = BigInt(str);
   return stream;
   }

}
/*
* BigInt Assignment Operators
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Addition Operator
*/
BigInt& BigInt::operator+=(const BigInt& y)
   {
   const size_t x_sw = sig_words(), y_sw = y.sig_words();

   const size_t reg_size = std::max(x_sw, y_sw) + 1;
   grow_to(reg_size);

   if(sign() == y.sign())
      bigint_add2(mutable_data(), reg_size - 1, y.data(), y_sw);
   else
      {
      s32bit relative_size = bigint_cmp(data(), x_sw, y.data(), y_sw);

      if(relative_size < 0)
         {
         secure_vector<word> z(reg_size - 1);
         bigint_sub3(z.data(), y.data(), reg_size - 1, data(), x_sw);
         std::swap(m_reg, z);
         set_sign(y.sign());
         }
      else if(relative_size == 0)
         {
         zeroise(m_reg);
         set_sign(Positive);
         }
      else if(relative_size > 0)
         bigint_sub2(mutable_data(), x_sw, y.data(), y_sw);
      }

   return (*this);
   }

/*
* Subtraction Operator
*/
BigInt& BigInt::operator-=(const BigInt& y)
   {
   const size_t x_sw = sig_words(), y_sw = y.sig_words();

   s32bit relative_size = bigint_cmp(data(), x_sw, y.data(), y_sw);

   const size_t reg_size = std::max(x_sw, y_sw) + 1;
   grow_to(reg_size);

   if(relative_size < 0)
      {
      if(sign() == y.sign())
         bigint_sub2_rev(mutable_data(), y.data(), y_sw);
      else
         bigint_add2(mutable_data(), reg_size - 1, y.data(), y_sw);

      set_sign(y.reverse_sign());
      }
   else if(relative_size == 0)
      {
      if(sign() == y.sign())
         {
         clear();
         set_sign(Positive);
         }
      else
         bigint_shl1(mutable_data(), x_sw, 0, 1);
      }
   else if(relative_size > 0)
      {
      if(sign() == y.sign())
         bigint_sub2(mutable_data(), x_sw, y.data(), y_sw);
      else
         bigint_add2(mutable_data(), reg_size - 1, y.data(), y_sw);
      }

   return (*this);
   }

/*
* Multiplication Operator
*/
BigInt& BigInt::operator*=(const BigInt& y)
   {
   const size_t x_sw = sig_words(), y_sw = y.sig_words();
   set_sign((sign() == y.sign()) ? Positive : Negative);

   if(x_sw == 0 || y_sw == 0)
      {
      clear();
      set_sign(Positive);
      }
   else if(x_sw == 1 && y_sw)
      {
      grow_to(y_sw + 2);
      bigint_linmul3(mutable_data(), y.data(), y_sw, word_at(0));
      }
   else if(y_sw == 1 && x_sw)
      {
      grow_to(x_sw + 2);
      bigint_linmul2(mutable_data(), x_sw, y.word_at(0));
      }
   else
      {
      grow_to(size() + y.size());

      secure_vector<word> z(data(), data() + x_sw);
      secure_vector<word> workspace(size());

      bigint_mul(mutable_data(), size(), workspace.data(),
                 z.data(), z.size(), x_sw,
                 y.data(), y.size(), y_sw);
      }

   return (*this);
   }

/*
* Division Operator
*/
BigInt& BigInt::operator/=(const BigInt& y)
   {
   if(y.sig_words() == 1 && is_power_of_2(y.word_at(0)))
      (*this) >>= (y.bits() - 1);
   else
      (*this) = (*this) / y;
   return (*this);
   }

/*
* Modulo Operator
*/
BigInt& BigInt::operator%=(const BigInt& mod)
   {
   return (*this = (*this) % mod);
   }

/*
* Modulo Operator
*/
word BigInt::operator%=(word mod)
   {
   if(mod == 0)
      throw BigInt::DivideByZero();

   if(is_power_of_2(mod))
       {
       word result = (word_at(0) & (mod - 1));
       clear();
       grow_to(2);
       m_reg[0] = result;
       return result;
       }

   word remainder = 0;

   for(size_t j = sig_words(); j > 0; --j)
      remainder = bigint_modop(remainder, word_at(j-1), mod);
   clear();
   grow_to(2);

   if(remainder && sign() == BigInt::Negative)
      m_reg[0] = mod - remainder;
   else
      m_reg[0] = remainder;

   set_sign(BigInt::Positive);

   return word_at(0);
   }

/*
* Left Shift Operator
*/
BigInt& BigInt::operator<<=(size_t shift)
   {
   if(shift)
      {
      const size_t shift_words = shift / MP_WORD_BITS,
                   shift_bits  = shift % MP_WORD_BITS,
                   words = sig_words();

      grow_to(words + shift_words + (shift_bits ? 1 : 0));
      bigint_shl1(mutable_data(), words, shift_words, shift_bits);
      }

   return (*this);
   }

/*
* Right Shift Operator
*/
BigInt& BigInt::operator>>=(size_t shift)
   {
   if(shift)
      {
      const size_t shift_words = shift / MP_WORD_BITS,
                   shift_bits  = shift % MP_WORD_BITS;

      bigint_shr1(mutable_data(), sig_words(), shift_words, shift_bits);

      if(is_zero())
         set_sign(Positive);
      }

   return (*this);
   }

}
/*
* BigInt Binary Operators
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Addition Operator
*/
BigInt operator+(const BigInt& x, const BigInt& y)
   {
   const size_t x_sw = x.sig_words(), y_sw = y.sig_words();

   BigInt z(x.sign(), std::max(x_sw, y_sw) + 1);

   if((x.sign() == y.sign()))
      bigint_add3(z.mutable_data(), x.data(), x_sw, y.data(), y_sw);
   else
      {
      s32bit relative_size = bigint_cmp(x.data(), x_sw, y.data(), y_sw);

      if(relative_size < 0)
         {
         bigint_sub3(z.mutable_data(), y.data(), y_sw, x.data(), x_sw);
         z.set_sign(y.sign());
         }
      else if(relative_size == 0)
         z.set_sign(BigInt::Positive);
      else if(relative_size > 0)
         bigint_sub3(z.mutable_data(), x.data(), x_sw, y.data(), y_sw);
      }

   return z;
   }

/*
* Subtraction Operator
*/
BigInt operator-(const BigInt& x, const BigInt& y)
   {
   const size_t x_sw = x.sig_words(), y_sw = y.sig_words();

   s32bit relative_size = bigint_cmp(x.data(), x_sw, y.data(), y_sw);

   BigInt z(BigInt::Positive, std::max(x_sw, y_sw) + 1);

   if(relative_size < 0)
      {
      if(x.sign() == y.sign())
         bigint_sub3(z.mutable_data(), y.data(), y_sw, x.data(), x_sw);
      else
         bigint_add3(z.mutable_data(), x.data(), x_sw, y.data(), y_sw);
      z.set_sign(y.reverse_sign());
      }
   else if(relative_size == 0)
      {
      if(x.sign() != y.sign())
         bigint_shl2(z.mutable_data(), x.data(), x_sw, 0, 1);
      }
   else if(relative_size > 0)
      {
      if(x.sign() == y.sign())
         bigint_sub3(z.mutable_data(), x.data(), x_sw, y.data(), y_sw);
      else
         bigint_add3(z.mutable_data(), x.data(), x_sw, y.data(), y_sw);
      z.set_sign(x.sign());
      }
   return z;
   }

/*
* Multiplication Operator
*/
BigInt operator*(const BigInt& x, const BigInt& y)
   {
   const size_t x_sw = x.sig_words(), y_sw = y.sig_words();

   BigInt z(BigInt::Positive, x.size() + y.size());

   if(x_sw == 1 && y_sw)
      bigint_linmul3(z.mutable_data(), y.data(), y_sw, x.word_at(0));
   else if(y_sw == 1 && x_sw)
      bigint_linmul3(z.mutable_data(), x.data(), x_sw, y.word_at(0));
   else if(x_sw && y_sw)
      {
      secure_vector<word> workspace(z.size());
      bigint_mul(z.mutable_data(), z.size(), workspace.data(),
                 x.data(), x.size(), x_sw,
                 y.data(), y.size(), y_sw);
      }

   if(x_sw && y_sw && x.sign() != y.sign())
      z.flip_sign();
   return z;
   }

/*
* Division Operator
*/
BigInt operator/(const BigInt& x, const BigInt& y)
   {
   BigInt q, r;
   divide(x, y, q, r);
   return q;
   }

/*
* Modulo Operator
*/
BigInt operator%(const BigInt& n, const BigInt& mod)
   {
   if(mod.is_zero())
      throw BigInt::DivideByZero();
   if(mod.is_negative())
      throw Invalid_Argument("BigInt::operator%: modulus must be > 0");
   if(n.is_positive() && mod.is_positive() && n < mod)
      return n;

   BigInt q, r;
   divide(n, mod, q, r);
   return r;
   }

/*
* Modulo Operator
*/
word operator%(const BigInt& n, word mod)
   {
   if(mod == 0)
      throw BigInt::DivideByZero();

   if(is_power_of_2(mod))
      return (n.word_at(0) & (mod - 1));

   word remainder = 0;

   for(size_t j = n.sig_words(); j > 0; --j)
      remainder = bigint_modop(remainder, n.word_at(j-1), mod);

   if(remainder && n.sign() == BigInt::Negative)
      return mod - remainder;
   return remainder;
   }

/*
* Left Shift Operator
*/
BigInt operator<<(const BigInt& x, size_t shift)
   {
   if(shift == 0)
      return x;

   const size_t shift_words = shift / MP_WORD_BITS,
                shift_bits  = shift % MP_WORD_BITS;

   const size_t x_sw = x.sig_words();

   BigInt y(x.sign(), x_sw + shift_words + (shift_bits ? 1 : 0));
   bigint_shl2(y.mutable_data(), x.data(), x_sw, shift_words, shift_bits);
   return y;
   }

/*
* Right Shift Operator
*/
BigInt operator>>(const BigInt& x, size_t shift)
   {
   if(shift == 0)
      return x;
   if(x.bits() <= shift)
      return 0;

   const size_t shift_words = shift / MP_WORD_BITS,
                shift_bits  = shift % MP_WORD_BITS,
                x_sw = x.sig_words();

   BigInt y(x.sign(), x_sw - shift_words);
   bigint_shr2(y.mutable_data(), x.data(), x_sw, shift_words, shift_bits);
   return y;
   }

}
/*
* BigInt Random Generation
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Randomize this number
*/
void BigInt::randomize(RandomNumberGenerator& rng,
                       size_t bitsize, bool set_high_bit)
   {
   set_sign(Positive);

   if(bitsize == 0)
      {
      clear();
      }
   else
      {
      secure_vector<byte> array = rng.random_vec(round_up(bitsize, 8) / 8);

      // Always cut unwanted bits
      if(bitsize % 8)
         array[0] &= 0xFF >> (8 - (bitsize % 8));

      // Set the highest bit if wanted
      if (set_high_bit)
         array[0] |= 0x80 >> ((bitsize % 8) ? (8 - bitsize % 8) : 0);

      binary_decode(array);
      }
   }

/*
* Generate a random integer within given range
*/
BigInt BigInt::random_integer(RandomNumberGenerator& rng,
                              const BigInt& min, const BigInt& max)
   {
   BigInt delta_upper_bound = max - min - 1;

   if(delta_upper_bound < 0)
      throw Invalid_Argument("random_integer: invalid min/max values");

   // Choose x in [0, delta_upper_bound]
   BigInt x;
   do {
      auto bitsize = delta_upper_bound.bits();
      x.randomize(rng, bitsize, false);
   } while(x > delta_upper_bound);

   return min + x;
   }

}
/*
* BigInt Base
* (C) 1999-2011,2012,2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Construct a BigInt from a regular number
*/
BigInt::BigInt(u64bit n)
   {
   if(n == 0)
      return;

   const size_t limbs_needed = sizeof(u64bit) / sizeof(word);

   m_reg.resize(4*limbs_needed);
   for(size_t i = 0; i != limbs_needed; ++i)
      m_reg[i] = ((n >> (i*MP_WORD_BITS)) & MP_WORD_MASK);
   }

/*
* Construct a BigInt of the specified size
*/
BigInt::BigInt(Sign s, size_t size)
   {
   m_reg.resize(round_up(size, 8));
   m_signedness = s;
   }

/*
* Copy constructor
*/
BigInt::BigInt(const BigInt& other)
   {
   m_reg = other.m_reg;
   m_signedness = other.m_signedness;
   }

/*
* Construct a BigInt from a string
*/
BigInt::BigInt(const std::string& str)
   {
   Base base = Decimal;
   size_t markers = 0;
   bool negative = false;

   if(str.length() > 0 && str[0] == '-')
      {
      markers += 1;
      negative = true;
      }

   if(str.length() > markers + 2 && str[markers    ] == '0' &&
                                    str[markers + 1] == 'x')
      {
      markers += 2;
      base = Hexadecimal;
      }

   *this = decode(reinterpret_cast<const byte*>(str.data()) + markers,
                  str.length() - markers, base);

   if(negative) set_sign(Negative);
   else         set_sign(Positive);
   }

/*
* Construct a BigInt from an encoded BigInt
*/
BigInt::BigInt(const byte input[], size_t length, Base base)
   {
   *this = decode(input, length, base);
   }

/*
* Construct a BigInt from an encoded BigInt
*/
BigInt::BigInt(RandomNumberGenerator& rng, size_t bits, bool set_high_bit)
   {
   randomize(rng, bits, set_high_bit);
   }

/*
* Comparison Function
*/
s32bit BigInt::cmp(const BigInt& other, bool check_signs) const
   {
   if(check_signs)
      {
      if(other.is_positive() && this->is_negative())
         return -1;

      if(other.is_negative() && this->is_positive())
         return 1;

      if(other.is_negative() && this->is_negative())
         return (-bigint_cmp(this->data(), this->sig_words(),
                             other.data(), other.sig_words()));
      }

   return bigint_cmp(this->data(), this->sig_words(),
                     other.data(), other.sig_words());
   }

/*
* Return bits {offset...offset+length}
*/
u32bit BigInt::get_substring(size_t offset, size_t length) const
   {
   if(length > 32)
      throw Invalid_Argument("BigInt::get_substring: Substring size too big");

   u64bit piece = 0;
   for(size_t i = 0; i != 8; ++i)
      {
      const byte part = byte_at((offset / 8) + (7-i));
      piece = (piece << 8) | part;
      }

   const u64bit mask = (static_cast<u64bit>(1) << length) - 1;
   const size_t shift = (offset % 8);

   return static_cast<u32bit>((piece >> shift) & mask);
   }

/*
* Convert this number to a u32bit, if possible
*/
u32bit BigInt::to_u32bit() const
   {
   if(is_negative())
      throw Encoding_Error("BigInt::to_u32bit: Number is negative");
   if(bits() > 32)
      throw Encoding_Error("BigInt::to_u32bit: Number is too big to convert");

   u32bit out = 0;
   for(size_t i = 0; i != 4; ++i)
      out = (out << 8) | byte_at(3-i);
   return out;
   }

/*
* Set bit number n
*/
void BigInt::set_bit(size_t n)
   {
   const size_t which = n / MP_WORD_BITS;
   const word mask = static_cast<word>(1) << (n % MP_WORD_BITS);
   if(which >= size()) grow_to(which + 1);
   m_reg[which] |= mask;
   }

/*
* Clear bit number n
*/
void BigInt::clear_bit(size_t n)
   {
   const size_t which = n / MP_WORD_BITS;
   const word mask = static_cast<word>(1) << (n % MP_WORD_BITS);
   if(which < size())
      m_reg[which] &= ~mask;
   }

size_t BigInt::bytes() const
   {
   return round_up(bits(), 8) / 8;
   }

/*
* Count how many bits are being used
*/
size_t BigInt::bits() const
   {
   const size_t words = sig_words();

   if(words == 0)
      return 0;

   const size_t full_words = words - 1;
   return (full_words * MP_WORD_BITS + high_bit(word_at(full_words)));
   }

/*
* Calcluate the size in a certain base
*/
size_t BigInt::encoded_size(Base base) const
   {
   static const double LOG_2_BASE_10 = 0.30102999566;

   if(base == Binary)
      return bytes();
   else if(base == Hexadecimal)
      return 2*bytes();
   else if(base == Decimal)
      return static_cast<size_t>((bits() * LOG_2_BASE_10) + 1);
   else
      throw Invalid_Argument("Unknown base for BigInt encoding");
   }

/*
* Set the sign
*/
void BigInt::set_sign(Sign s)
   {
   if(is_zero())
      m_signedness = Positive;
   else
      m_signedness = s;
   }

/*
* Reverse the value of the sign flag
*/
void BigInt::flip_sign()
   {
   set_sign(reverse_sign());
   }

/*
* Return the opposite value of the current sign
*/
BigInt::Sign BigInt::reverse_sign() const
   {
   if(sign() == Positive)
      return Negative;
   return Positive;
   }

/*
* Return the negation of this number
*/
BigInt BigInt::operator-() const
   {
   BigInt x = (*this);
   x.flip_sign();
   return x;
   }

/*
* Return the absolute value of this number
*/
BigInt BigInt::abs() const
   {
   BigInt x = (*this);
   x.set_sign(Positive);
   return x;
   }

void BigInt::grow_to(size_t n)
   {
   if(n > size())
      m_reg.resize(round_up(n, 8));
   }

/*
* Encode this number into bytes
*/
void BigInt::binary_encode(byte output[]) const
   {
   const size_t sig_bytes = bytes();
   for(size_t i = 0; i != sig_bytes; ++i)
      output[sig_bytes-i-1] = byte_at(i);
   }

/*
* Set this number to the value in buf
*/
void BigInt::binary_decode(const byte buf[], size_t length)
   {
   const size_t WORD_BYTES = sizeof(word);

   clear();
   m_reg.resize(round_up((length / WORD_BYTES) + 1, 8));

   for(size_t i = 0; i != length / WORD_BYTES; ++i)
      {
      const size_t top = length - WORD_BYTES*i;
      for(size_t j = WORD_BYTES; j > 0; --j)
         m_reg[i] = (m_reg[i] << 8) | buf[top - j];
      }

   for(size_t i = 0; i != length % WORD_BYTES; ++i)
      m_reg[length / WORD_BYTES] = (m_reg[length / WORD_BYTES] << 8) | buf[i];
   }

}
/*
* Division Algorithm
* (C) 1999-2007,2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

/*
* Handle signed operands, if necessary
*/
void sign_fixup(const BigInt& x, const BigInt& y, BigInt& q, BigInt& r)
   {
   if(x.sign() == BigInt::Negative)
      {
      q.flip_sign();
      if(r.is_nonzero()) { --q; r = y.abs() - r; }
      }
   if(y.sign() == BigInt::Negative)
      q.flip_sign();
   }

bool division_check(word q, word y2, word y1,
                    word x3, word x2, word x1)
   {
   // Compute (y3,y2,y1) = (y2,y1) * q

   word y3 = 0;
   y1 = word_madd2(q, y1, &y3);
   y2 = word_madd2(q, y2, &y3);

   // Return (y3,y2,y1) >? (x3,x2,x1)

   if(y3 > x3) return true;
   if(y3 < x3) return false;

   if(y2 > x2) return true;
   if(y2 < x2) return false;

   if(y1 > x1) return true;
   if(y1 < x1) return false;

   return false;
   }

}

/*
* Solve x = q * y + r
*/
void divide(const BigInt& x, const BigInt& y_arg, BigInt& q, BigInt& r)
   {
   if(y_arg.is_zero())
      throw BigInt::DivideByZero();

   BigInt y = y_arg;
   const size_t y_words = y.sig_words();

   r = x;
   q = 0;

   r.set_sign(BigInt::Positive);
   y.set_sign(BigInt::Positive);

   s32bit compare = r.cmp(y);

   if(compare == 0)
      {
      q = 1;
      r = 0;
      }
   else if(compare > 0)
      {
      size_t shifts = 0;
      word y_top = y.word_at(y.sig_words()-1);
      while(y_top < MP_WORD_TOP_BIT) { y_top <<= 1; ++shifts; }
      y <<= shifts;
      r <<= shifts;

      const size_t n = r.sig_words() - 1, t = y_words - 1;

      if(n < t)
         throw Internal_Error("BigInt division word sizes");

      q.grow_to(n - t + 1);

      word* q_words = q.mutable_data();

      if(n <= t)
         {
         while(r > y) { r -= y; ++q; }
         r >>= shifts;
         sign_fixup(x, y_arg, q, r);
         return;
         }

      BigInt temp = y << (MP_WORD_BITS * (n-t));

      while(r >= temp) { r -= temp; q_words[n-t] += 1; }

      for(size_t j = n; j != t; --j)
         {
         const word x_j0  = r.word_at(j);
         const word x_j1 = r.word_at(j-1);
         const word y_t  = y.word_at(t);

         if(x_j0 == y_t)
            q_words[j-t-1] = MP_WORD_MAX;
         else
            q_words[j-t-1] = bigint_divop(x_j0, x_j1, y_t);

         while(division_check(q_words[j-t-1],
                              y_t, y.word_at(t-1),
                              x_j0, x_j1, r.word_at(j-2)))
            {
            q_words[j-t-1] -= 1;
            }

         r -= (q_words[j-t-1] * y) << (MP_WORD_BITS * (j-t-1));

         if(r.is_negative())
            {
            r += y << (MP_WORD_BITS * (j-t-1));
            q_words[j-t-1] -= 1;
            }
         }
      r >>= shifts;
      }

   sign_fixup(x, y_arg, q, r);
   }

}
/*
* Block Ciphers
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_AES)
#endif

#if defined(BOTAN_HAS_AES_SSSE3)
#endif

#if defined(BOTAN_HAS_AES_NI)
#endif

#if defined(BOTAN_HAS_BLOWFISH)
#endif

#if defined(BOTAN_HAS_CAMELLIA)
#endif

#if defined(BOTAN_HAS_CAST)
#endif

#if defined(BOTAN_HAS_CASCADE)
#endif

#if defined(BOTAN_HAS_DES)
#endif

#if defined(BOTAN_HAS_GOST_28147_89)
#endif

#if defined(BOTAN_HAS_IDEA)
#endif

#if defined(BOTAN_HAS_IDEA_SSE2)
#endif

#if defined(BOTAN_HAS_KASUMI)
#endif

#if defined(BOTAN_HAS_LION)
#endif

#if defined(BOTAN_HAS_LUBY_RACKOFF)
#endif

#if defined(BOTAN_HAS_MARS)
#endif

#if defined(BOTAN_HAS_MISTY1)
#endif

#if defined(BOTAN_HAS_NOEKEON)
#endif

#if defined(BOTAN_HAS_NOEKEON_SIMD)
#endif

#if defined(BOTAN_HAS_RC2)
#endif

#if defined(BOTAN_HAS_RC5)
#endif

#if defined(BOTAN_HAS_RC6)
#endif

#if defined(BOTAN_HAS_SAFER)
#endif

#if defined(BOTAN_HAS_SEED)
#endif

#if defined(BOTAN_HAS_SERPENT)
#endif

#if defined(BOTAN_HAS_SERPENT_SIMD)
#endif

#if defined(BOTAN_HAS_SKIPJACK)
#endif

#if defined(BOTAN_HAS_SQUARE)
#endif

#if defined(BOTAN_HAS_TEA)
#endif

#if defined(BOTAN_HAS_TWOFISH)
#endif

#if defined(BOTAN_HAS_THREEFISH_512)
#endif

#if defined(BOTAN_HAS_THREEFISH_512_AVX2)
#endif

#if defined(BOTAN_HAS_XTEA)
#endif

#if defined(BOTAN_HAS_XTEA_SIMD)
#endif

namespace Botan {

BlockCipher::~BlockCipher() {}

std::unique_ptr<BlockCipher> BlockCipher::create(const std::string& algo_spec,
                                                 const std::string& provider)
   {
   return std::unique_ptr<BlockCipher>(make_a<BlockCipher>(algo_spec, provider));
   }

std::vector<std::string> BlockCipher::providers(const std::string& algo_spec)
   {
   return providers_of<BlockCipher>(BlockCipher::Spec(algo_spec));
   }

#define BOTAN_REGISTER_BLOCK_CIPHER(name, maker) BOTAN_REGISTER_T(BlockCipher, name, maker)
#define BOTAN_REGISTER_BLOCK_CIPHER_NOARGS(name) BOTAN_REGISTER_T_NOARGS(BlockCipher, name)

#define BOTAN_REGISTER_BLOCK_CIPHER_1LEN(name, def) BOTAN_REGISTER_T_1LEN(BlockCipher, name, def)

#define BOTAN_REGISTER_BLOCK_CIPHER_NAMED_NOARGS(type, name) \
   BOTAN_REGISTER_NAMED_T(BlockCipher, name, type, make_new_T<type>)
#define BOTAN_REGISTER_BLOCK_CIPHER_NAMED_1LEN(type, name, def) \
   BOTAN_REGISTER_NAMED_T(BlockCipher, name, type, (make_new_T_1len<type,def>))
#define BOTAN_REGISTER_BLOCK_CIPHER_NAMED_1STR(type, name, def) \
   BOTAN_REGISTER_NAMED_T(BlockCipher, name, type, std::bind(make_new_T_1str<type>, std::placeholders::_1, def))

#define BOTAN_REGISTER_BLOCK_CIPHER_NOARGS_IF(cond, type, name, provider, pref) \
   BOTAN_COND_REGISTER_NAMED_T_NOARGS(cond, BlockCipher, type, name, provider, pref)

#if defined(BOTAN_HAS_AES)
BOTAN_REGISTER_BLOCK_CIPHER_NAMED_NOARGS(AES_128, "AES-128");
BOTAN_REGISTER_BLOCK_CIPHER_NAMED_NOARGS(AES_192, "AES-192");
BOTAN_REGISTER_BLOCK_CIPHER_NAMED_NOARGS(AES_256, "AES-256");
#endif

#if defined(BOTAN_HAS_AES_NI)
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS_IF(CPUID::has_aes_ni(), AES_128_NI, "AES-128", "aes_ni", 200);
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS_IF(CPUID::has_aes_ni(), AES_192_NI, "AES-192", "aes_ni", 200);
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS_IF(CPUID::has_aes_ni(), AES_256_NI, "AES-256", "aes_ni", 200);
#endif

#if defined(BOTAN_HAS_AES_SSSE3)
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS_IF(CPUID::has_ssse3(), AES_128_SSSE3, "AES-128",
                                      "ssse3", BOTAN_SIMD_ALGORITHM_PRIO);
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS_IF(CPUID::has_ssse3(), AES_192_SSSE3, "AES-192",
                                      "ssse3", BOTAN_SIMD_ALGORITHM_PRIO);
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS_IF(CPUID::has_ssse3(), AES_256_SSSE3, "AES-256",
                                      "ssse3", BOTAN_SIMD_ALGORITHM_PRIO);
#endif

#if defined(BOTAN_HAS_BLOWFISH)
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS(Blowfish);
#endif

#if defined(BOTAN_HAS_CAMELLIA)
BOTAN_REGISTER_BLOCK_CIPHER_NAMED_NOARGS(Camellia_128, "Camellia-128");
BOTAN_REGISTER_BLOCK_CIPHER_NAMED_NOARGS(Camellia_192, "Camellia-192");
BOTAN_REGISTER_BLOCK_CIPHER_NAMED_NOARGS(Camellia_256, "Camellia-256");
#endif

#if defined(BOTAN_HAS_CAST)
BOTAN_REGISTER_BLOCK_CIPHER_NAMED_NOARGS(CAST_128, "CAST-128");
BOTAN_REGISTER_BLOCK_CIPHER_NAMED_NOARGS(CAST_256, "CAST-256");
#endif

#if defined(BOTAN_HAS_DES)
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS(DES);
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS(TripleDES);
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS(DESX);
#endif

#if defined(BOTAN_HAS_GOST_28147_89)
BOTAN_REGISTER_BLOCK_CIPHER_NAMED_1STR(GOST_28147_89, "GOST-28147-89", "R3411_94_TestParam");
#endif

#if defined(BOTAN_HAS_IDEA)
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS(IDEA);
#endif

#if defined(BOTAN_HAS_IDEA_SSE2)
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS_IF(CPUID::has_sse2(), IDEA_SSE2, "IDEA",
                                      "sse2", BOTAN_SIMD_ALGORITHM_PRIO);
#endif

#if defined(BOTAN_HAS_KASUMI)
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS(KASUMI);
#endif

#if defined(BOTAN_HAS_MARS)
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS(MARS);
#endif

#if defined(BOTAN_HAS_MISTY1)
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS(MISTY1);
#endif

#if defined(BOTAN_HAS_NOEKEON)
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS(Noekeon);
#endif

#if defined(BOTAN_HAS_NOEKEON_SIMD)
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS_IF(CPUID::has_simd_32(), Noekeon_SIMD, "Noekeon",
                                      "simd32", BOTAN_SIMD_ALGORITHM_PRIO);
#endif

#if defined(BOTAN_HAS_RC2)
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS(RC2);
#endif

#if defined(BOTAN_HAS_RC5)
BOTAN_REGISTER_BLOCK_CIPHER_1LEN(RC5, 12);
#endif

#if defined(BOTAN_HAS_RC6)
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS(RC6);
#endif

#if defined(BOTAN_HAS_SAFER)
BOTAN_REGISTER_BLOCK_CIPHER_NAMED_1LEN(SAFER_SK, "SAFER-SK", 10);
#endif

#if defined(BOTAN_HAS_SEED)
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS(SEED);
#endif

#if defined(BOTAN_HAS_SERPENT)
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS(Serpent);
#endif

#if defined(BOTAN_HAS_SERPENT_SIMD)
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS_IF(CPUID::has_simd_32(), Serpent_SIMD, "Serpent",
                                      "simd32", BOTAN_SIMD_ALGORITHM_PRIO);
#endif

#if defined(BOTAN_HAS_TEA)
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS(TEA);
#endif

#if defined(BOTAN_HAS_TWOFISH)
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS(Twofish);
#endif

#if defined(BOTAN_HAS_THREEFISH_512)
BOTAN_REGISTER_BLOCK_CIPHER_NAMED_NOARGS(Threefish_512, "Threefish-512");
#endif

#if defined(BOTAN_HAS_THREEFISH_512_AVX2)
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS_IF(CPUID::has_avx2(), Threefish_512_AVX2, "Threefish-512",
                                      "avx2", BOTAN_SIMD_ALGORITHM_PRIO);
#endif

#if defined(BOTAN_HAS_XTEA)
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS(XTEA);
#endif

#if defined(BOTAN_HAS_XTEA_SIMD)
BOTAN_REGISTER_BLOCK_CIPHER_NOARGS_IF(CPUID::has_simd_32(), XTEA_SIMD, "XTEA",
                                      "simd32", BOTAN_SIMD_ALGORITHM_PRIO);
#endif

#if defined(BOTAN_HAS_CASCADE)
BOTAN_REGISTER_NAMED_T(BlockCipher, "Cascade", Cascade_Cipher, Cascade_Cipher::make);
#endif

#if defined(BOTAN_HAS_LION)
BOTAN_REGISTER_NAMED_T(BlockCipher, "Lion", Lion, Lion::make);
#endif

}
/*
* CBC Mode
* (C) 1999-2007,2013 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

CBC_Mode::CBC_Mode(BlockCipher* cipher, BlockCipherModePaddingMethod* padding) :
   m_cipher(cipher),
   m_padding(padding),
   m_state(m_cipher->block_size())
   {
   if(m_padding && !m_padding->valid_blocksize(cipher->block_size()))
      throw std::invalid_argument("Padding " + m_padding->name() +
                                  " cannot be used with " +
                                  cipher->name() + "/CBC");
   }

void CBC_Mode::clear()
   {
   m_cipher->clear();
   m_state.clear();
   }

std::string CBC_Mode::name() const
   {
   if(m_padding)
      return cipher().name() + "/CBC/" + padding().name();
   else
      return cipher().name() + "/CBC/CTS";
   }

size_t CBC_Mode::update_granularity() const
   {
   return cipher().parallel_bytes();
   }

Key_Length_Specification CBC_Mode::key_spec() const
   {
   return cipher().key_spec();
   }

size_t CBC_Mode::default_nonce_length() const
   {
   return cipher().block_size();
   }

bool CBC_Mode::valid_nonce_length(size_t n) const
   {
   return (n == 0 || n == cipher().block_size());
   }

void CBC_Mode::key_schedule(const byte key[], size_t length)
   {
   m_cipher->set_key(key, length);
   }

secure_vector<byte> CBC_Mode::start_raw(const byte nonce[], size_t nonce_len)
   {
   if(!valid_nonce_length(nonce_len))
      throw Invalid_IV_Length(name(), nonce_len);

   /*
   * A nonce of zero length means carry the last ciphertext value over
   * as the new IV, as unfortunately some protocols require this. If
   * this is the first message then we use an IV of all zeros.
   */
   if(nonce_len)
      m_state.assign(nonce, nonce + nonce_len);

   return secure_vector<byte>();
   }

size_t CBC_Encryption::minimum_final_size() const
   {
   return 0;
   }

size_t CBC_Encryption::output_length(size_t input_length) const
   {
   if(input_length == 0)
      return cipher().block_size();
   else
      return round_up(input_length, cipher().block_size());
   }

void CBC_Encryption::update(secure_vector<byte>& buffer, size_t offset)
   {
   BOTAN_ASSERT(buffer.size() >= offset, "Offset is sane");
   const size_t sz = buffer.size() - offset;
   byte* buf = buffer.data() + offset;

   const size_t BS = cipher().block_size();

   BOTAN_ASSERT(sz % BS == 0, "CBC input is full blocks");
   const size_t blocks = sz / BS;

   const byte* prev_block = state_ptr();

   if(blocks)
      {
      for(size_t i = 0; i != blocks; ++i)
         {
         xor_buf(&buf[BS*i], prev_block, BS);
         cipher().encrypt(&buf[BS*i]);
         prev_block = &buf[BS*i];
         }

      state().assign(&buf[BS*(blocks-1)], &buf[BS*blocks]);
      }
   }

void CBC_Encryption::finish(secure_vector<byte>& buffer, size_t offset)
   {
   BOTAN_ASSERT(buffer.size() >= offset, "Offset is sane");

   const size_t BS = cipher().block_size();

   const size_t bytes_in_final_block = (buffer.size()-offset) % BS;

   padding().add_padding(buffer, bytes_in_final_block, BS);

   if((buffer.size()-offset) % BS)
      throw std::runtime_error("Did not pad to full block size in " + name());

   update(buffer, offset);
   }

bool CTS_Encryption::valid_nonce_length(size_t n) const
   {
   return (n == cipher().block_size());
   }

size_t CTS_Encryption::minimum_final_size() const
   {
   return cipher().block_size() + 1;
   }

size_t CTS_Encryption::output_length(size_t input_length) const
   {
   return input_length; // no ciphertext expansion in CTS
   }

void CTS_Encryption::finish(secure_vector<byte>& buffer, size_t offset)
   {
   BOTAN_ASSERT(buffer.size() >= offset, "Offset is sane");
   byte* buf = buffer.data() + offset;
   const size_t sz = buffer.size() - offset;

   const size_t BS = cipher().block_size();

   if(sz < BS + 1)
      throw Encoding_Error(name() + ": insufficient data to encrypt");

   if(sz % BS == 0)
      {
      update(buffer, offset);

      // swap last two blocks
      for(size_t i = 0; i != BS; ++i)
         std::swap(buffer[buffer.size()-BS+i], buffer[buffer.size()-2*BS+i]);
      }
   else
      {
      const size_t full_blocks = ((sz / BS) - 1) * BS;
      const size_t final_bytes = sz - full_blocks;
      BOTAN_ASSERT(final_bytes > BS && final_bytes < 2*BS, "Left over size in expected range");

      secure_vector<byte> last(buf + full_blocks, buf + full_blocks + final_bytes);
      buffer.resize(full_blocks + offset);
      update(buffer, offset);

      xor_buf(last.data(), state_ptr(), BS);
      cipher().encrypt(last.data());

      for(size_t i = 0; i != final_bytes - BS; ++i)
         {
         last[i] ^= last[i + BS];
         last[i + BS] ^= last[i];
         }

      cipher().encrypt(last.data());

      buffer += last;
      }
   }

size_t CBC_Decryption::output_length(size_t input_length) const
   {
   return input_length; // precise for CTS, worst case otherwise
   }

size_t CBC_Decryption::minimum_final_size() const
   {
   return cipher().block_size();
   }

void CBC_Decryption::update(secure_vector<byte>& buffer, size_t offset)
   {
   BOTAN_ASSERT(buffer.size() >= offset, "Offset is sane");
   const size_t sz = buffer.size() - offset;
   byte* buf = buffer.data() + offset;

   const size_t BS = cipher().block_size();

   BOTAN_ASSERT(sz % BS == 0, "Input is full blocks");
   size_t blocks = sz / BS;

   while(blocks)
      {
      const size_t to_proc = std::min(BS * blocks, m_tempbuf.size());

      cipher().decrypt_n(buf, m_tempbuf.data(), to_proc / BS);

      xor_buf(m_tempbuf.data(), state_ptr(), BS);
      xor_buf(&m_tempbuf[BS], buf, to_proc - BS);
      copy_mem(state_ptr(), buf + (to_proc - BS), BS);

      copy_mem(buf, m_tempbuf.data(), to_proc);

      buf += to_proc;
      blocks -= to_proc / BS;
      }
   }

void CBC_Decryption::finish(secure_vector<byte>& buffer, size_t offset)
   {
   BOTAN_ASSERT(buffer.size() >= offset, "Offset is sane");
   const size_t sz = buffer.size() - offset;

   const size_t BS = cipher().block_size();

   if(sz == 0 || sz % BS)
      throw Decoding_Error(name() + ": Ciphertext not a multiple of block size");

   update(buffer, offset);

   const size_t pad_bytes = BS - padding().unpad(&buffer[buffer.size()-BS], BS);
   buffer.resize(buffer.size() - pad_bytes); // remove padding
   }

bool CTS_Decryption::valid_nonce_length(size_t n) const
   {
   return (n == cipher().block_size());
   }

size_t CTS_Decryption::minimum_final_size() const
   {
   return cipher().block_size() + 1;
   }

void CTS_Decryption::finish(secure_vector<byte>& buffer, size_t offset)
   {
   BOTAN_ASSERT(buffer.size() >= offset, "Offset is sane");
   const size_t sz = buffer.size() - offset;
   byte* buf = buffer.data() + offset;

   const size_t BS = cipher().block_size();

   if(sz < BS + 1)
      throw Encoding_Error(name() + ": insufficient data to decrypt");

   if(sz % BS == 0)
      {
      // swap last two blocks

      for(size_t i = 0; i != BS; ++i)
         std::swap(buffer[buffer.size()-BS+i], buffer[buffer.size()-2*BS+i]);

      update(buffer, offset);
      }
   else
      {
      const size_t full_blocks = ((sz / BS) - 1) * BS;
      const size_t final_bytes = sz - full_blocks;
      BOTAN_ASSERT(final_bytes > BS && final_bytes < 2*BS, "Left over size in expected range");

      secure_vector<byte> last(buf + full_blocks, buf + full_blocks + final_bytes);
      buffer.resize(full_blocks + offset);
      update(buffer, offset);

      cipher().decrypt(last.data());

      xor_buf(last.data(), &last[BS], final_bytes - BS);

      for(size_t i = 0; i != final_bytes - BS; ++i)
         std::swap(last[i], last[i + BS]);

      cipher().decrypt(last.data());
      xor_buf(last.data(), state_ptr(), BS);

      buffer += last;
      }
   }

}
/*
* Base64 Encoder/Decoder
* (C) 1999-2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Base64_Encoder Constructor
*/
Base64_Encoder::Base64_Encoder(bool breaks, size_t length, bool t_n) :
   line_length(breaks ? length : 0),
   trailing_newline(t_n && breaks),
   in(48),
   out(64),
   position(0),
   out_position(0)
   {
   }

/*
* Encode and send a block
*/
void Base64_Encoder::encode_and_send(const byte input[], size_t length,
                                     bool final_inputs)
   {
   while(length)
      {
      const size_t proc = std::min(length, in.size());

      size_t consumed = 0;
      size_t produced = base64_encode(reinterpret_cast<char*>(out.data()), input,
                                      proc, consumed, final_inputs);

      do_output(out.data(), produced);

      // FIXME: s/proc/consumed/?
      input += proc;
      length -= proc;
      }
   }

/*
* Handle the output
*/
void Base64_Encoder::do_output(const byte input[], size_t length)
   {
   if(line_length == 0)
      send(input, length);
   else
      {
      size_t remaining = length, offset = 0;
      while(remaining)
         {
         size_t sent = std::min(line_length - out_position, remaining);
         send(input + offset, sent);
         out_position += sent;
         remaining -= sent;
         offset += sent;
         if(out_position == line_length)
            {
            send('\n');
            out_position = 0;
            }
         }
      }
   }

/*
* Convert some data into Base64
*/
void Base64_Encoder::write(const byte input[], size_t length)
   {
   buffer_insert(in, position, input, length);
   if(position + length >= in.size())
      {
      encode_and_send(in.data(), in.size());
      input += (in.size() - position);
      length -= (in.size() - position);
      while(length >= in.size())
         {
         encode_and_send(input, in.size());
         input += in.size();
         length -= in.size();
         }
      copy_mem(in.data(), input, length);
      position = 0;
      }
   position += length;
   }

/*
* Flush buffers
*/
void Base64_Encoder::end_msg()
   {
   encode_and_send(in.data(), position, true);

   if(trailing_newline || (out_position && line_length))
      send('\n');

   out_position = position = 0;
   }

/*
* Base64_Decoder Constructor
*/
Base64_Decoder::Base64_Decoder(Decoder_Checking c) :
   checking(c), in(64), out(48), position(0)
   {
   }

/*
* Convert some data from Base64
*/
void Base64_Decoder::write(const byte input[], size_t length)
   {
   while(length)
      {
      size_t to_copy = std::min<size_t>(length, in.size() - position);
      if(to_copy == 0)
         {
         in.resize(in.size()*2);
         out.resize(out.size()*2);
         }
      copy_mem(&in[position], input, to_copy);
      position += to_copy;

      size_t consumed = 0;
      size_t written = base64_decode(out.data(),
                                     reinterpret_cast<const char*>(in.data()),
                                     position,
                                     consumed,
                                     false,
                                     checking != FULL_CHECK);

      send(out, written);

      if(consumed != position)
         {
         copy_mem(in.data(), in.data() + consumed, position - consumed);
         position = position - consumed;
         }
      else
         position = 0;

      length -= to_copy;
      input += to_copy;
      }
   }

/*
* Flush buffers
*/
void Base64_Decoder::end_msg()
   {
   size_t consumed = 0;
   size_t written = base64_decode(out.data(),
                                  reinterpret_cast<const char*>(in.data()),
                                  position,
                                  consumed,
                                  true,
                                  checking != FULL_CHECK);

   send(out, written);

   const bool not_full_bytes = consumed != position;

   position = 0;

   if(not_full_bytes)
      throw std::invalid_argument("Base64_Decoder: Input not full bytes");
   }

}
/*
* Hex Encoder/Decoder
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/**
* Size used for internal buffer in hex encoder/decoder
*/
const size_t HEX_CODEC_BUFFER_SIZE = 256;

/*
* Hex_Encoder Constructor
*/
Hex_Encoder::Hex_Encoder(bool breaks, size_t length, Case c) :
   casing(c), line_length(breaks ? length : 0)
   {
   in.resize(HEX_CODEC_BUFFER_SIZE);
   out.resize(2*in.size());
   counter = position = 0;
   }

/*
* Hex_Encoder Constructor
*/
Hex_Encoder::Hex_Encoder(Case c) : casing(c), line_length(0)
   {
   in.resize(HEX_CODEC_BUFFER_SIZE);
   out.resize(2*in.size());
   counter = position = 0;
   }

/*
* Encode and send a block
*/
void Hex_Encoder::encode_and_send(const byte block[], size_t length)
   {
   hex_encode(reinterpret_cast<char*>(out.data()),
              block, length,
              casing == Uppercase);

   if(line_length == 0)
      send(out, 2*length);
   else
      {
      size_t remaining = 2*length, offset = 0;
      while(remaining)
         {
         size_t sent = std::min(line_length - counter, remaining);
         send(&out[offset], sent);
         counter += sent;
         remaining -= sent;
         offset += sent;
         if(counter == line_length)
            {
            send('\n');
            counter = 0;
            }
         }
      }
   }

/*
* Convert some data into hex format
*/
void Hex_Encoder::write(const byte input[], size_t length)
   {
   buffer_insert(in, position, input, length);
   if(position + length >= in.size())
      {
      encode_and_send(in.data(), in.size());
      input += (in.size() - position);
      length -= (in.size() - position);
      while(length >= in.size())
         {
         encode_and_send(input, in.size());
         input += in.size();
         length -= in.size();
         }
      copy_mem(in.data(), input, length);
      position = 0;
      }
   position += length;
   }

/*
* Flush buffers
*/
void Hex_Encoder::end_msg()
   {
   encode_and_send(in.data(), position);
   if(counter && line_length)
      send('\n');
   counter = position = 0;
   }

/*
* Hex_Decoder Constructor
*/
Hex_Decoder::Hex_Decoder(Decoder_Checking c) : checking(c)
   {
   in.resize(HEX_CODEC_BUFFER_SIZE);
   out.resize(in.size() / 2);
   position = 0;
   }

/*
* Convert some data from hex format
*/
void Hex_Decoder::write(const byte input[], size_t length)
   {
   while(length)
      {
      size_t to_copy = std::min<size_t>(length, in.size() - position);
      copy_mem(&in[position], input, to_copy);
      position += to_copy;

      size_t consumed = 0;
      size_t written = hex_decode(out.data(),
                                  reinterpret_cast<const char*>(in.data()),
                                  position,
                                  consumed,
                                  checking != FULL_CHECK);

      send(out, written);

      if(consumed != position)
         {
         copy_mem(in.data(), in.data() + consumed, position - consumed);
         position = position - consumed;
         }
      else
         position = 0;

      length -= to_copy;
      input += to_copy;
      }
   }

/*
* Flush buffers
*/
void Hex_Decoder::end_msg()
   {
   size_t consumed = 0;
   size_t written = hex_decode(out.data(),
                               reinterpret_cast<const char*>(in.data()),
                               position,
                               consumed,
                               checking != FULL_CHECK);

   send(out, written);

   const bool not_full_bytes = consumed != position;

   position = 0;

   if(not_full_bytes)
      throw std::invalid_argument("Hex_Decoder: Input not full bytes");
   }

}
/*
* Compression Transform
* (C) 2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <cstdlib>

namespace Botan {

void* Compression_Alloc_Info::do_malloc(size_t n, size_t size)
   {
   const size_t total_sz = n * size;

   void* ptr = std::malloc(total_sz);
   m_current_allocs[ptr] = total_sz;
   return ptr;
   }

void Compression_Alloc_Info::do_free(void* ptr)
   {
   if(ptr)
      {
      auto i = m_current_allocs.find(ptr);

      if(i == m_current_allocs.end())
         throw std::runtime_error("Compression_Alloc_Info::free got pointer not allocated by us");

      zero_mem(ptr, i->second);
      std::free(ptr);
      m_current_allocs.erase(i);
      }
   }

namespace {

Compressor_Transform* do_make_compressor(const std::string& type, const std::string suffix)
   {
   const std::map<std::string, std::string> trans{
      {"zlib", "Zlib"},
      {"deflate", "Deflate"},
      {"gzip", "Gzip"},
      {"gz", "Gzip"},
      {"bzip2", "Bzip2"},
      {"bz2", "Bzip2"},
      {"lzma", "LZMA"},
      {"xz", "LZMA"}};

   auto i = trans.find(type);

   if(i == trans.end())
      return nullptr;

   const std::string t_name = i->second + suffix;

   std::unique_ptr<Transform> t(get_transform(t_name));

   if(!t)
      return nullptr;

   Compressor_Transform* r = dynamic_cast<Compressor_Transform*>(t.get());
   if(!r)
      throw std::runtime_error("Bad cast of compression object " + t_name);

   t.release();
   return r;
   }

}

Compressor_Transform* make_compressor(const std::string& type, size_t level)
   {
   return do_make_compressor(type, "_Compression(" + std::to_string(level) + ")");
   }

Compressor_Transform* make_decompressor(const std::string& type)
   {
   return do_make_compressor(type, "_Decompression");
   }

void Stream_Compression::clear()
   {
   m_stream.reset();
   }

secure_vector<byte> Stream_Compression::start_raw(const byte[], size_t nonce_len)
   {
   if(!valid_nonce_length(nonce_len))
      throw Invalid_IV_Length(name(), nonce_len);

   m_stream.reset(make_stream());
   return secure_vector<byte>();
   }

void Stream_Compression::process(secure_vector<byte>& buf, size_t offset, u32bit flags)
   {
   BOTAN_ASSERT(m_stream, "Initialized");
   BOTAN_ASSERT(buf.size() >= offset, "Offset is sane");

   if(m_buffer.size() < buf.size() + offset)
      m_buffer.resize(buf.size() + offset);

   // If the output buffer has zero length, .data() might return nullptr. This would
   // make some compression algorithms (notably those provided by zlib) fail.
   // Any small positive value works fine, but we choose 32 as it is the smallest power
   // of two that is large enough to hold all the headers and trailers of the common
   // formats, preventing further resizings to make room for output data.
   if(m_buffer.size() == 0)
      m_buffer.resize(32);

   m_stream->next_in(buf.data() + offset, buf.size() - offset);
   m_stream->next_out(m_buffer.data() + offset, m_buffer.size() - offset);

   while(true)
      {
      m_stream->run(flags);

      if(m_stream->avail_out() == 0)
         {
         const size_t added = 8 + m_buffer.size();
         m_buffer.resize(m_buffer.size() + added);
         m_stream->next_out(m_buffer.data() + m_buffer.size() - added, added);
         }
      else if(m_stream->avail_in() == 0)
         {
         m_buffer.resize(m_buffer.size() - m_stream->avail_out());
         break;
         }
      }

   copy_mem(m_buffer.data(), buf.data(), offset);
   buf.swap(m_buffer);
   }

void Stream_Compression::update(secure_vector<byte>& buf, size_t offset)
   {
   BOTAN_ASSERT(m_stream, "Initialized");
   process(buf, offset, m_stream->run_flag());
   }

void Stream_Compression::flush(secure_vector<byte>& buf, size_t offset)
   {
   BOTAN_ASSERT(m_stream, "Initialized");
   process(buf, offset, m_stream->flush_flag());
   }

void Stream_Compression::finish(secure_vector<byte>& buf, size_t offset)
   {
   BOTAN_ASSERT(m_stream, "Initialized");
   process(buf, offset, m_stream->finish_flag());
   clear();
   }

void Stream_Decompression::clear()
   {
   m_stream.reset();
   }

secure_vector<byte> Stream_Decompression::start_raw(const byte[], size_t nonce_len)
   {
   if(!valid_nonce_length(nonce_len))
      throw Invalid_IV_Length(name(), nonce_len);

   m_stream.reset(make_stream());

   return secure_vector<byte>();
   }

void Stream_Decompression::process(secure_vector<byte>& buf, size_t offset, u32bit flags)
   {
   BOTAN_ASSERT(m_stream, "Initialized");
   BOTAN_ASSERT(buf.size() >= offset, "Offset is sane");

   if(m_buffer.size() < buf.size() + offset)
      m_buffer.resize(buf.size() + offset);

   m_stream->next_in(buf.data() + offset, buf.size() - offset);
   m_stream->next_out(m_buffer.data() + offset, m_buffer.size() - offset);

   while(true)
      {
      const bool stream_end = m_stream->run(flags);

      if(stream_end)
         {
         if(m_stream->avail_in() == 0) // all data consumed?
            {
            m_buffer.resize(m_buffer.size() - m_stream->avail_out());
            clear();
            break;
            }

         // More data follows: try to process as a following stream
         const size_t read = (buf.size() - offset) - m_stream->avail_in();
         start();
         m_stream->next_in(buf.data() + offset + read, buf.size() - offset - read);
         }

      if(m_stream->avail_out() == 0)
         {
         const size_t added = 8 + m_buffer.size();
         m_buffer.resize(m_buffer.size() + added);
         m_stream->next_out(m_buffer.data() + m_buffer.size() - added, added);
         }
      else if(m_stream->avail_in() == 0)
         {
         m_buffer.resize(m_buffer.size() - m_stream->avail_out());
         break;
         }
      }

   copy_mem(m_buffer.data(), buf.data(), offset);
   buf.swap(m_buffer);
   }

void Stream_Decompression::update(secure_vector<byte>& buf, size_t offset)
   {
   process(buf, offset, m_stream->run_flag());
   }

void Stream_Decompression::finish(secure_vector<byte>& buf, size_t offset)
   {
   if(buf.size() != offset || m_stream.get())
      process(buf, offset, m_stream->finish_flag());

   if(m_stream.get())
      throw std::runtime_error(name() + " finished but not at stream end");
   }

}
/*
* Counter mode
* (C) 1999-2011,2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

CTR_BE* CTR_BE::make(const Spec& spec)
   {
   if(spec.algo_name() == "CTR-BE" && spec.arg_count() == 1)
      {
      if(auto c = BlockCipher::create(spec.arg(0)))
         return new CTR_BE(c.release());
      }
   return nullptr;
   }

CTR_BE::CTR_BE(BlockCipher* ciph) :
   m_cipher(ciph),
   m_counter(m_cipher->parallel_bytes()),
   m_pad(m_counter.size()),
   m_pad_pos(0)
   {
   }

void CTR_BE::clear()
   {
   m_cipher->clear();
   zeroise(m_pad);
   zeroise(m_counter);
   m_pad_pos = 0;
   }

void CTR_BE::key_schedule(const byte key[], size_t key_len)
   {
   m_cipher->set_key(key, key_len);

   // Set a default all-zeros IV
   set_iv(nullptr, 0);
   }

std::string CTR_BE::name() const
   {
   return ("CTR-BE(" + m_cipher->name() + ")");
   }

void CTR_BE::cipher(const byte in[], byte out[], size_t length)
   {
   while(length >= m_pad.size() - m_pad_pos)
      {
      xor_buf(out, in, &m_pad[m_pad_pos], m_pad.size() - m_pad_pos);
      length -= (m_pad.size() - m_pad_pos);
      in += (m_pad.size() - m_pad_pos);
      out += (m_pad.size() - m_pad_pos);
      increment_counter();
      }
   xor_buf(out, in, &m_pad[m_pad_pos], length);
   m_pad_pos += length;
   }

void CTR_BE::set_iv(const byte iv[], size_t iv_len)
   {
   if(!valid_iv_length(iv_len))
      throw Invalid_IV_Length(name(), iv_len);

   const size_t bs = m_cipher->block_size();

   zeroise(m_counter);

   const size_t n_wide = m_counter.size() / m_cipher->block_size();
   buffer_insert(m_counter, 0, iv, iv_len);

   // Set m_counter blocks to IV, IV + 1, ... IV + n
   for(size_t i = 1; i != n_wide; ++i)
      {
      buffer_insert(m_counter, i*bs, &m_counter[(i-1)*bs], bs);

      for(size_t j = 0; j != bs; ++j)
         if(++m_counter[i*bs + (bs - 1 - j)])
            break;
      }

   m_cipher->encrypt_n(m_counter.data(), m_pad.data(), n_wide);
   m_pad_pos = 0;
   }

/*
* Increment the counter and update the buffer
*/
void CTR_BE::increment_counter()
   {
   const size_t bs = m_cipher->block_size();
   const size_t n_wide = m_counter.size() / bs;

   for(size_t i = 0; i != n_wide; ++i)
      {
      uint16_t carry = n_wide;
      for(size_t j = 0; carry && j != bs; ++j)
         {
         const size_t off = i*bs + (bs-1-j);
         const uint16_t cnt = static_cast<uint16_t>(m_counter[off]) + carry;
         m_counter[off] = static_cast<byte>(cnt);
         carry = (cnt >> 8);
         }
      }

   m_cipher->encrypt_n(m_counter.data(), m_pad.data(), n_wide);
   m_pad_pos = 0;
   }

}
/*
* Reader of /dev/random and company
* (C) 1999-2009,2013 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

namespace Botan {

/**
Device_EntropySource constructor
Open a file descriptor to each (available) device in fsnames
*/
Device_EntropySource::Device_EntropySource(const std::vector<std::string>& fsnames)
   {
#ifndef O_NONBLOCK
  #define O_NONBLOCK 0
#endif

#ifndef O_NOCTTY
  #define O_NOCTTY 0
#endif

   const int flags = O_RDONLY | O_NONBLOCK | O_NOCTTY;

   for(auto fsname : fsnames)
      {
      fd_type fd = ::open(fsname.c_str(), flags);

      if(fd >= 0 && fd < FD_SETSIZE)
         m_devices.push_back(fd);
      else if(fd >= 0)
         ::close(fd);
      }
   }

/**
Device_EntropySource destructor: close all open devices
*/
Device_EntropySource::~Device_EntropySource()
   {
   for(size_t i = 0; i != m_devices.size(); ++i)
      ::close(m_devices[i]);
   }

/**
* Gather entropy from a RNG device
*/
void Device_EntropySource::poll(Entropy_Accumulator& accum)
   {
   if(m_devices.empty())
      return;

   fd_type max_fd = m_devices[0];
   fd_set read_set;
   FD_ZERO(&read_set);
   for(size_t i = 0; i != m_devices.size(); ++i)
      {
      FD_SET(m_devices[i], &read_set);
      max_fd = std::max(m_devices[i], max_fd);
      }

   struct ::timeval timeout;

   timeout.tv_sec = (BOTAN_SYSTEM_RNG_POLL_TIMEOUT_MS / 1000);
   timeout.tv_usec = (BOTAN_SYSTEM_RNG_POLL_TIMEOUT_MS % 1000) * 1000;

   if(::select(max_fd + 1, &read_set, nullptr, nullptr, &timeout) < 0)
      return;

   secure_vector<byte>& buf = accum.get_io_buf(BOTAN_SYSTEM_RNG_POLL_REQUEST);

   for(size_t i = 0; i != m_devices.size(); ++i)
      {
      if(FD_ISSET(m_devices[i], &read_set))
         {
         const ssize_t got = ::read(m_devices[i], buf.data(), buf.size());
         if(got > 0)
            accum.add(buf.data(), got, BOTAN_ENTROPY_ESTIMATE_STRONG_RNG);
         }
      }
   }

}
/*
* Discrete Logarithm Parameters
* (C) 1999-2008,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* DL_Group Constructor
*/
DL_Group::DL_Group()
   {
   initialized = false;
   }

/*
* DL_Group Constructor
*/
DL_Group::DL_Group(const std::string& name)
   {
   const char* pem = PEM_for_named_group(name);

   if(!pem)
      throw Invalid_Argument("DL_Group: Unknown group " + name);

   PEM_decode(pem);
   }

/*
* DL_Group Constructor
*/
DL_Group::DL_Group(RandomNumberGenerator& rng,
                   PrimeType type, size_t pbits, size_t qbits)
   {
   if(pbits < 1024)
      throw Invalid_Argument("DL_Group: prime size " + std::to_string(pbits) +
                             " is too small");

   if(type == Strong)
      {
      p = random_safe_prime(rng, pbits);
      q = (p - 1) / 2;
      g = 2;
      }
   else if(type == Prime_Subgroup)
      {
      if(!qbits)
         qbits = dl_exponent_size(pbits);

      q = random_prime(rng, qbits);
      BigInt X;
      while(p.bits() != pbits || !is_prime(p, rng))
         {
         X.randomize(rng, pbits);
         p = X - (X % (2*q) - 1);
         }

      g = make_dsa_generator(p, q);
      }
   else if(type == DSA_Kosherizer)
      {
      qbits = qbits ? qbits : ((pbits <= 1024) ? 160 : 256);

      generate_dsa_primes(rng, p, q, pbits, qbits);

      g = make_dsa_generator(p, q);
      }

   initialized = true;
   }

/*
* DL_Group Constructor
*/
DL_Group::DL_Group(RandomNumberGenerator& rng,
                   const std::vector<byte>& seed,
                   size_t pbits, size_t qbits)
   {
   if(!generate_dsa_primes(rng, p, q, pbits, qbits, seed))
      throw Invalid_Argument("DL_Group: The seed given does not "
                             "generate a DSA group");

   g = make_dsa_generator(p, q);

   initialized = true;
   }

/*
* DL_Group Constructor
*/
DL_Group::DL_Group(const BigInt& p1, const BigInt& g1)
   {
   initialize(p1, 0, g1);
   }

/*
* DL_Group Constructor
*/
DL_Group::DL_Group(const BigInt& p1, const BigInt& q1, const BigInt& g1)
   {
   initialize(p1, q1, g1);
   }

/*
* DL_Group Initializer
*/
void DL_Group::initialize(const BigInt& p1, const BigInt& q1, const BigInt& g1)
   {
   if(p1 < 3)
      throw Invalid_Argument("DL_Group: Prime invalid");
   if(g1 < 2 || g1 >= p1)
      throw Invalid_Argument("DL_Group: Generator invalid");
   if(q1 < 0 || q1 >= p1)
      throw Invalid_Argument("DL_Group: Subgroup invalid");

   p = p1;
   g = g1;
   q = q1;

   initialized = true;
   }

/*
* Verify that the group has been set
*/
void DL_Group::init_check() const
   {
   if(!initialized)
      throw Invalid_State("DLP group cannot be used uninitialized");
   }

/*
* Verify the parameters
*/
bool DL_Group::verify_group(RandomNumberGenerator& rng,
                            bool strong) const
   {
   init_check();

   if(g < 2 || p < 3 || q < 0)
      return false;
   if((q != 0) && ((p - 1) % q != 0))
      return false;

   const size_t prob = (strong) ? 56 : 10;

   if(!is_prime(p, rng, prob))
      return false;
   if((q > 0) && !is_prime(q, rng, prob))
      return false;
   return true;
   }

/*
* Return the prime
*/
const BigInt& DL_Group::get_p() const
   {
   init_check();
   return p;
   }

/*
* Return the generator
*/
const BigInt& DL_Group::get_g() const
   {
   init_check();
   return g;
   }

/*
* Return the subgroup
*/
const BigInt& DL_Group::get_q() const
   {
   init_check();
   if(q == 0)
      throw Invalid_State("DLP group has no q prime specified");
   return q;
   }

/*
* DER encode the parameters
*/
std::vector<byte> DL_Group::DER_encode(Format format) const
   {
   init_check();

   if((q == 0) && (format != PKCS_3))
      throw Encoding_Error("The ANSI DL parameter formats require a subgroup");

   if(format == ANSI_X9_57)
      {
      return DER_Encoder()
         .start_cons(SEQUENCE)
            .encode(p)
            .encode(q)
            .encode(g)
         .end_cons()
      .get_contents_unlocked();
      }
   else if(format == ANSI_X9_42)
      {
      return DER_Encoder()
         .start_cons(SEQUENCE)
            .encode(p)
            .encode(g)
            .encode(q)
         .end_cons()
      .get_contents_unlocked();
      }
   else if(format == PKCS_3)
      {
      return DER_Encoder()
         .start_cons(SEQUENCE)
            .encode(p)
            .encode(g)
         .end_cons()
      .get_contents_unlocked();
      }

   throw Invalid_Argument("Unknown DL_Group encoding " + std::to_string(format));
   }

/*
* PEM encode the parameters
*/
std::string DL_Group::PEM_encode(Format format) const
   {
   const std::vector<byte> encoding = DER_encode(format);

   if(format == PKCS_3)
      return PEM_Code::encode(encoding, "DH PARAMETERS");
   else if(format == ANSI_X9_57)
      return PEM_Code::encode(encoding, "DSA PARAMETERS");
   else if(format == ANSI_X9_42)
      return PEM_Code::encode(encoding, "X942 DH PARAMETERS");
   else
      throw Invalid_Argument("Unknown DL_Group encoding " + std::to_string(format));
   }

/*
* Decode BER encoded parameters
*/
void DL_Group::BER_decode(const std::vector<byte>& data,
                          Format format)
   {
   BigInt new_p, new_q, new_g;

   BER_Decoder decoder(data);
   BER_Decoder ber = decoder.start_cons(SEQUENCE);

   if(format == ANSI_X9_57)
      {
      ber.decode(new_p)
         .decode(new_q)
         .decode(new_g)
         .verify_end();
      }
   else if(format == ANSI_X9_42)
      {
      ber.decode(new_p)
         .decode(new_g)
         .decode(new_q)
         .discard_remaining();
      }
   else if(format == PKCS_3)
      {
      ber.decode(new_p)
         .decode(new_g)
         .discard_remaining();
      }
   else
      throw Invalid_Argument("Unknown DL_Group encoding " + std::to_string(format));

   initialize(new_p, new_q, new_g);
   }

/*
* Decode PEM encoded parameters
*/
void DL_Group::PEM_decode(const std::string& pem)
   {
   std::string label;

   auto ber = unlock(PEM_Code::decode(pem, label));

   if(label == "DH PARAMETERS")
      BER_decode(ber, PKCS_3);
   else if(label == "DSA PARAMETERS")
      BER_decode(ber, ANSI_X9_57);
   else if(label == "X942 DH PARAMETERS")
      BER_decode(ber, ANSI_X9_42);
   else
      throw Decoding_Error("DL_Group: Invalid PEM label " + label);
   }

/*
* Create generator of the q-sized subgroup (DSA style generator)
*/
BigInt DL_Group::make_dsa_generator(const BigInt& p, const BigInt& q)
   {
   const BigInt e = (p - 1) / q;

   if(e == 0 || (p - 1) % q > 0)
      throw std::invalid_argument("make_dsa_generator q does not divide p-1");

   for(size_t i = 0; i != PRIME_TABLE_SIZE; ++i)
      {
      BigInt g = power_mod(PRIMES[i], e, p);
      if(g > 1)
         return g;
      }

   throw Internal_Error("DL_Group: Couldn't create a suitable generator");
   }

}
/*
* List of discrete log groups
* (C) 2013 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

const char* DL_Group::PEM_for_named_group(const std::string& name)
   {
   if(name == "modp/ietf/1024")
      return
         "-----BEGIN X942 DH PARAMETERS-----"
         "MIIBCgKBgQD//////////8kP2qIhaMI0xMZii4DcHNEpAk4IimfMdAILvqY7E5si"
         "UUoIeY40BN3vlRmzzTpDGzArCm3yXxQ3T+E1bW1RwkXkhbV2Yl5+xvRMQummN+1r"
         "C/9ctvQGt+3uOGv7Womfpa6fJBF8Sx/mSShmUezmU4H//////////wIBAgKBgH//"
         "////////5IftURC0YRpiYzFFwG4OaJSBJwRFM+Y6AQXfUx2JzZEopQQ8xxoCbvfK"
         "jNnmnSGNmBWFNvkvihun8Jq2tqjhIvJC2rsxLz9jeiYhdNMb9rWF/65begNb9vcc"
         "Nf2tRM/S10+SCL4lj/MklDMo9nMpwP//////////"
         "-----END X942 DH PARAMETERS-----";

   if(name == "modp/srp/1024")
      return
         "-----BEGIN X942 DH PARAMETERS-----"
         "MIIBCgKBgQDurwq5rbON1pwz+Ar6j8XoYHJhh3X/PAueojFMnCVldtZ033SW6oHT"
         "ODtIE9aSxuDg1djiULmL5I5JXB1gidrRXcfXtGFU1rbOjvStabFdSYJVmyl7zxiF"
         "xSn1ZmYOV+xo7bw8BXJswC/Uy/SXbqqa/VE4/oN2Q1ufxh0vwOsG4wIBAgKBgHdX"
         "hVzW2cbrThn8BX1H4vQwOTDDuv+eBc9RGKZOErK7azpvukt1QOmcHaQJ60ljcHBq"
         "7HEoXMXyRySuDrBE7Wiu4+vaMKprW2dHela02K6kwSrNlL3njELilPqzMwcr9jR2"
         "3h4CuTZgF+pl+ku3VU1+qJx/Qbshrc/jDpfgdYNx"
         "-----END X942 DH PARAMETERS-----";

   if(name == "modp/ietf/1536")
      return
         "-----BEGIN X942 DH PARAMETERS-----"
         "MIIBigKBwQD//////////8kP2qIhaMI0xMZii4DcHNEpAk4IimfMdAILvqY7E5si"
         "UUoIeY40BN3vlRmzzTpDGzArCm3yXxQ3T+E1bW1RwkXkhbV2Yl5+xvRMQummN+1r"
         "C/9ctvQGt+3uOGv7Womfpa6fJBF8Sx/mSShmUezkWz3CAHy4oWO/BZjaSDYcVdOa"
         "aRY/qP0kz1+DZV0j3KOtlhxi81YghVK7ntUpB3CWlm1nDDVOSryYBPF0bAjKI3Mn"
         "//////////8CAQICgcB//////////+SH7VEQtGEaYmMxRcBuDmiUgScERTPmOgEF"
         "31Mdic2RKKUEPMcaAm73yozZ5p0hjZgVhTb5L4obp/Catrao4SLyQtq7MS8/Y3om"
         "IXTTG/a1hf+uW3oDW/b3HDX9rUTP0tdPkgi+JY/zJJQzKPZyLZ7hAD5cULHfgsxt"
         "JBsOKunNNIsf1H6SZ6/Bsq6R7lHWyw4xeasQQqldz2qUg7hLSzazhhqnJV5MAni6"
         "NgRlEbmT//////////8="
         "-----END X942 DH PARAMETERS-----";

   if(name == "modp/srp/1536")
      return
         "-----BEGIN DH PARAMETERS-----"
         "MIHHAoHBAJ3vPK+5OSd6sfEqhheke7vbpR30maxMgL7uqWFLGcxNX09fVW4ny95R"
         "xqlL5GB6KRVYkDug0PhDgLZVu5oi6NzfAop87Gfw0IE0sci5eYkUm2CeC+O6tj1H"
         "VIOB28Wx/HZOP0tT3Z2hFYv9PiucjPVu3wGVOTSWJ9sv1T0kt8SGZXcuQ31sf4zk"
         "QnNK98y3roN8Jkrjqb64f4ov6bi1KS5aAh//XpFHnoznoowkQsbzFRgPk0maI03P"
         "duP+0TX5uwIBAg=="
         "-----END DH PARAMETERS-----";

   if(name == "modp/ietf/2048")
      return
         "-----BEGIN X942 DH PARAMETERS-----"
         "MIICDAKCAQEA///////////JD9qiIWjCNMTGYouA3BzRKQJOCIpnzHQCC76mOxOb"
         "IlFKCHmONATd75UZs806QxswKwpt8l8UN0/hNW1tUcJF5IW1dmJefsb0TELppjft"
         "awv/XLb0Brft7jhr+1qJn6WunyQRfEsf5kkoZlHs5Fs9wgB8uKFjvwWY2kg2HFXT"
         "mmkWP6j9JM9fg2VdI9yjrZYcYvNWIIVSu57VKQdwlpZtZww1Tkq8mATxdGwIyhgh"
         "fDKQXkYuNs474553LBgOhgObJ4Oi7Aeij7XFXfBvTFLJ3ivL9pVYFxg5lUl86pVq"
         "5RXSJhiY+gUQFXKOWoqsqmj//////////wIBAgKCAQB//////////+SH7VEQtGEa"
         "YmMxRcBuDmiUgScERTPmOgEF31Mdic2RKKUEPMcaAm73yozZ5p0hjZgVhTb5L4ob"
         "p/Catrao4SLyQtq7MS8/Y3omIXTTG/a1hf+uW3oDW/b3HDX9rUTP0tdPkgi+JY/z"
         "JJQzKPZyLZ7hAD5cULHfgsxtJBsOKunNNIsf1H6SZ6/Bsq6R7lHWyw4xeasQQqld"
         "z2qUg7hLSzazhhqnJV5MAni6NgRlDBC+GUgvIxcbZx3xzzuWDAdDAc2TwdF2A9FH"
         "2uKu+DemKWTvFeX7SqwLjBzKpL51SrVyiukTDEx9AogKuUctRVZVNH//////////"
         "-----END X942 DH PARAMETERS-----";

   if(name == "modp/srp/2048")
      return
         "-----BEGIN X942 DH PARAMETERS-----"
         "MIICDAKCAQEArGvbQTJKmpvxZt5eE4lYL69ytmUZh+4H/DGSlD21YFCjcynLtKCZ"
         "7YGT4HV3Z6E91SMSq0sDMQ3Nf0ip2gT9UOgIOWntt2ewz2CVF5oWOrNmGgX71fqq"
         "6CkYqZYvC5O4Vfl5k+yXXuqoDXQK2/T/dHNZ0EHVwz6nHSgeRGsUdzvKl7Q6I/uA"
         "Fna9IHpDbGSB8dK5B4cXRhpbnTLmiPh3SFRFI7UksNV9Xqd6J3XS7PoDLPvb9S+z"
         "eGFgJ5AE5Xrmr4dOcwPOUymczAQce8MI2CpWmPOo0MOCca41+Onb+7aUtcgD2J96"
         "5DXeI21SX1R1m2XjcvzWjvIPpxEfnkr/cwIBAgKCAQBWNe2gmSVNTfizby8JxKwX"
         "17lbMozD9wP+GMlKHtqwKFG5lOXaUEz2wMnwOruz0J7qkYlVpYGYhua/pFTtAn6o"
         "dAQctPbbs9hnsEqLzQsdWbMNAv3q/VV0FIxUyxeFydwq/LzJ9kuvdVQGugVt+n+6"
         "OazoIOrhn1OOlA8iNYo7neVL2h0R/cALO16QPSG2MkD46VyDw4ujDS3OmXNEfDuk"
         "KiKR2pJYar6vU70Tuul2fQGWfe36l9m8MLATyAJyvXNXw6c5gecplM5mAg494YRs"
         "FStMedRoYcE41xr8dO3920pa5AHsT71yGu8RtqkvqjrNsvG5fmtHeQfTiI/PJX+5"
         "-----END X942 DH PARAMETERS-----";

   if(name == "modp/ietf/3072")
      return
         "-----BEGIN X942 DH PARAMETERS-----"
         "MIIDDAKCAYEA///////////JD9qiIWjCNMTGYouA3BzRKQJOCIpnzHQCC76mOxOb"
         "IlFKCHmONATd75UZs806QxswKwpt8l8UN0/hNW1tUcJF5IW1dmJefsb0TELppjft"
         "awv/XLb0Brft7jhr+1qJn6WunyQRfEsf5kkoZlHs5Fs9wgB8uKFjvwWY2kg2HFXT"
         "mmkWP6j9JM9fg2VdI9yjrZYcYvNWIIVSu57VKQdwlpZtZww1Tkq8mATxdGwIyhgh"
         "fDKQXkYuNs474553LBgOhgObJ4Oi7Aeij7XFXfBvTFLJ3ivL9pVYFxg5lUl86pVq"
         "5RXSJhiY+gUQFXKOWoqqxC2tMxcNBFB6M6hVIavfHLpk7PuFBFjb7wqK6nFXXQYM"
         "fbOXD4Wm4eTHq/WujNsJM9cejJTgSiVhnc7j0iYa0u5r8S/6BtmKCGTYdgJzPshq"
         "ZFIfKxgXeyAMu+EXV3phXWx3CYjAutlG4gjiT6B05asxQ9tb/OD9EI5LgtEgqTrS"
         "yv//////////AgECAoIBgH//////////5IftURC0YRpiYzFFwG4OaJSBJwRFM+Y6"
         "AQXfUx2JzZEopQQ8xxoCbvfKjNnmnSGNmBWFNvkvihun8Jq2tqjhIvJC2rsxLz9j"
         "eiYhdNMb9rWF/65begNb9vccNf2tRM/S10+SCL4lj/MklDMo9nItnuEAPlxQsd+C"
         "zG0kGw4q6c00ix/UfpJnr8GyrpHuUdbLDjF5qxBCqV3PapSDuEtLNrOGGqclXkwC"
         "eLo2BGUMEL4ZSC8jFxtnHfHPO5YMB0MBzZPB0XYD0Ufa4q74N6YpZO8V5ftKrAuM"
         "HMqkvnVKtXKK6RMMTH0CiAq5Ry1FVWIW1pmLhoIoPRnUKpDV745dMnZ9woIsbfeF"
         "RXU4q66DBj7Zy4fC03DyY9X610ZthJnrj0ZKcCUSsM7ncekTDWl3NfiX/QNsxQQy"
         "bDsBOZ9kNTIpD5WMC72QBl3wi6u9MK62O4TEYF1so3EEcSfQOnLVmKHtrf5wfohH"
         "JcFokFSdaWV//////////w=="
         "-----END X942 DH PARAMETERS-----";

   if(name == "modp/srp/3072")
      return
         "-----BEGIN DH PARAMETERS-----"
         "MIIBiAKCAYEA///////////JD9qiIWjCNMTGYouA3BzRKQJOCIpnzHQCC76mOxOb"
         "IlFKCHmONATd75UZs806QxswKwpt8l8UN0/hNW1tUcJF5IW1dmJefsb0TELppjft"
         "awv/XLb0Brft7jhr+1qJn6WunyQRfEsf5kkoZlHs5Fs9wgB8uKFjvwWY2kg2HFXT"
         "mmkWP6j9JM9fg2VdI9yjrZYcYvNWIIVSu57VKQdwlpZtZww1Tkq8mATxdGwIyhgh"
         "fDKQXkYuNs474553LBgOhgObJ4Oi7Aeij7XFXfBvTFLJ3ivL9pVYFxg5lUl86pVq"
         "5RXSJhiY+gUQFXKOWoqqxC2tMxcNBFB6M6hVIavfHLpk7PuFBFjb7wqK6nFXXQYM"
         "fbOXD4Wm4eTHq/WujNsJM9cejJTgSiVhnc7j0iYa0u5r8S/6BtmKCGTYdgJzPshq"
         "ZFIfKxgXeyAMu+EXV3phXWx3CYjAutlG4gjiT6B05asxQ9tb/OD9EI5LgtEgqTrS"
         "yv//////////AgEF"
         "-----END DH PARAMETERS-----";

   if(name == "modp/ietf/4096")
      return
         "-----BEGIN X942 DH PARAMETERS-----"
         "MIIEDAKCAgEA///////////JD9qiIWjCNMTGYouA3BzRKQJOCIpnzHQCC76mOxOb"
         "IlFKCHmONATd75UZs806QxswKwpt8l8UN0/hNW1tUcJF5IW1dmJefsb0TELppjft"
         "awv/XLb0Brft7jhr+1qJn6WunyQRfEsf5kkoZlHs5Fs9wgB8uKFjvwWY2kg2HFXT"
         "mmkWP6j9JM9fg2VdI9yjrZYcYvNWIIVSu57VKQdwlpZtZww1Tkq8mATxdGwIyhgh"
         "fDKQXkYuNs474553LBgOhgObJ4Oi7Aeij7XFXfBvTFLJ3ivL9pVYFxg5lUl86pVq"
         "5RXSJhiY+gUQFXKOWoqqxC2tMxcNBFB6M6hVIavfHLpk7PuFBFjb7wqK6nFXXQYM"
         "fbOXD4Wm4eTHq/WujNsJM9cejJTgSiVhnc7j0iYa0u5r8S/6BtmKCGTYdgJzPshq"
         "ZFIfKxgXeyAMu+EXV3phXWx3CYjAutlG4gjiT6B05asxQ9tb/OD9EI5LgtEgqSEI"
         "ARpyPBKnh+bXiHGaEL26WyaZwycYavTiPBqUaDS2FQvaJYPpyirUTOjbu8LbBN6O"
         "+S6O/BQfvsqmKHxZR05rwF2ZspZPoJDDoiM7oYZRW+ftH2EpcM7i16+4G912IXBI"
         "HNAGkSfVsFqpk7TqmI2P3cGG/7fckKbAj030Nck0BjGZ//////////8CAQICggIA"
         "f//////////kh+1RELRhGmJjMUXAbg5olIEnBEUz5joBBd9THYnNkSilBDzHGgJu"
         "98qM2eadIY2YFYU2+S+KG6fwmra2qOEi8kLauzEvP2N6JiF00xv2tYX/rlt6A1v2"
         "9xw1/a1Ez9LXT5IIviWP8ySUMyj2ci2e4QA+XFCx34LMbSQbDirpzTSLH9R+kmev"
         "wbKuke5R1ssOMXmrEEKpXc9qlIO4S0s2s4YapyVeTAJ4ujYEZQwQvhlILyMXG2cd"
         "8c87lgwHQwHNk8HRdgPRR9rirvg3pilk7xXl+0qsC4wcyqS+dUq1corpEwxMfQKI"
         "CrlHLUVVYhbWmYuGgig9GdQqkNXvjl0ydn3Cgixt94VFdTirroMGPtnLh8LTcPJj"
         "1frXRm2EmeuPRkpwJRKwzudx6RMNaXc1+Jf9A2zFBDJsOwE5n2Q1MikPlYwLvZAG"
         "XfCLq70wrrY7hMRgXWyjcQRxJ9A6ctWYoe2t/nB+iEclwWiQVJCEAI05HglTw/Nr"
         "xDjNCF7dLZNM4ZOMNXpxHg1KNBpbCoXtEsH05RVqJnRt3eFtgm9HfJdHfgoP32VT"
         "FD4so6c14C7M2Usn0Ehh0RGd0MMorfP2j7CUuGdxa9fcDe67ELgkDmgDSJPq2C1U"
         "ydp1TEbH7uDDf9vuSFNgR6b6GuSaAxjM//////////8="
         "-----END X942 DH PARAMETERS-----";

   if(name == "modp/srp/4096")
      return
         "-----BEGIN DH PARAMETERS-----"
         "MIICCAKCAgEA///////////JD9qiIWjCNMTGYouA3BzRKQJOCIpnzHQCC76mOxOb"
         "IlFKCHmONATd75UZs806QxswKwpt8l8UN0/hNW1tUcJF5IW1dmJefsb0TELppjft"
         "awv/XLb0Brft7jhr+1qJn6WunyQRfEsf5kkoZlHs5Fs9wgB8uKFjvwWY2kg2HFXT"
         "mmkWP6j9JM9fg2VdI9yjrZYcYvNWIIVSu57VKQdwlpZtZww1Tkq8mATxdGwIyhgh"
         "fDKQXkYuNs474553LBgOhgObJ4Oi7Aeij7XFXfBvTFLJ3ivL9pVYFxg5lUl86pVq"
         "5RXSJhiY+gUQFXKOWoqqxC2tMxcNBFB6M6hVIavfHLpk7PuFBFjb7wqK6nFXXQYM"
         "fbOXD4Wm4eTHq/WujNsJM9cejJTgSiVhnc7j0iYa0u5r8S/6BtmKCGTYdgJzPshq"
         "ZFIfKxgXeyAMu+EXV3phXWx3CYjAutlG4gjiT6B05asxQ9tb/OD9EI5LgtEgqSEI"
         "ARpyPBKnh+bXiHGaEL26WyaZwycYavTiPBqUaDS2FQvaJYPpyirUTOjbu8LbBN6O"
         "+S6O/BQfvsqmKHxZR05rwF2ZspZPoJDDoiM7oYZRW+ftH2EpcM7i16+4G912IXBI"
         "HNAGkSfVsFqpk7TqmI2P3cGG/7fckKbAj030Nck0BjGZ//////////8CAQU="
         "-----END DH PARAMETERS-----";

   if(name == "modp/ietf/6144")
      return
         "-----BEGIN X942 DH PARAMETERS-----"
         "MIIGDAKCAwEA///////////JD9qiIWjCNMTGYouA3BzRKQJOCIpnzHQCC76mOxOb"
         "IlFKCHmONATd75UZs806QxswKwpt8l8UN0/hNW1tUcJF5IW1dmJefsb0TELppjft"
         "awv/XLb0Brft7jhr+1qJn6WunyQRfEsf5kkoZlHs5Fs9wgB8uKFjvwWY2kg2HFXT"
         "mmkWP6j9JM9fg2VdI9yjrZYcYvNWIIVSu57VKQdwlpZtZww1Tkq8mATxdGwIyhgh"
         "fDKQXkYuNs474553LBgOhgObJ4Oi7Aeij7XFXfBvTFLJ3ivL9pVYFxg5lUl86pVq"
         "5RXSJhiY+gUQFXKOWoqqxC2tMxcNBFB6M6hVIavfHLpk7PuFBFjb7wqK6nFXXQYM"
         "fbOXD4Wm4eTHq/WujNsJM9cejJTgSiVhnc7j0iYa0u5r8S/6BtmKCGTYdgJzPshq"
         "ZFIfKxgXeyAMu+EXV3phXWx3CYjAutlG4gjiT6B05asxQ9tb/OD9EI5LgtEgqSEI"
         "ARpyPBKnh+bXiHGaEL26WyaZwycYavTiPBqUaDS2FQvaJYPpyirUTOjbu8LbBN6O"
         "+S6O/BQfvsqmKHxZR05rwF2ZspZPoJDDoiM7oYZRW+ftH2EpcM7i16+4G912IXBI"
         "HNAGkSfVsFqpk7TqmI2P3cGG/7fckKbAj030Nck0AoSSNsP6tNJ8cCbB1NyyYCZG"
         "3sl1HnY9uje9+P+UBq2eUw7l2zgvQTABrrBqU+2QJ9gxF5cnsIZaiRjaPtvrz5sU"
         "7UTObLrO1Lsb238UR+bMJUszIFFRK9evQm+49AE3jNK/WYPKAcZLkuzwMuoV0XId"
         "A/SC185udP721V5wL0aYDIK1qEAxkAscnlnnyX++x+jzI6l6fjbMiL4PHUW3/1ha"
         "xUvUB7IrQVSqzI9tfr9I4dgUzF7SD4A34KeXFe7ym+MoBqHVi7fF2nb1UKo9ih+/"
         "8OsZzLGjE9Vc2lbJ7C7yljI4f+jXbjwEaAQ+j2Y/SGDuEr8tWwt0dNbmlPkebcxA"
         "JP//////////AoIDAH//////////5IftURC0YRpiYzFFwG4OaJSBJwRFM+Y6AQXf"
         "Ux2JzZEopQQ8xxoCbvfKjNnmnSGNmBWFNvkvihun8Jq2tqjhIvJC2rsxLz9jeiYh"
         "dNMb9rWF/65begNb9vccNf2tRM/S10+SCL4lj/MklDMo9nItnuEAPlxQsd+CzG0k"
         "Gw4q6c00ix/UfpJnr8GyrpHuUdbLDjF5qxBCqV3PapSDuEtLNrOGGqclXkwCeLo2"
         "BGUMEL4ZSC8jFxtnHfHPO5YMB0MBzZPB0XYD0Ufa4q74N6YpZO8V5ftKrAuMHMqk"
         "vnVKtXKK6RMMTH0CiAq5Ry1FVWIW1pmLhoIoPRnUKpDV745dMnZ9woIsbfeFRXU4"
         "q66DBj7Zy4fC03DyY9X610ZthJnrj0ZKcCUSsM7ncekTDWl3NfiX/QNsxQQybDsB"
         "OZ9kNTIpD5WMC72QBl3wi6u9MK62O4TEYF1so3EEcSfQOnLVmKHtrf5wfohHJcFo"
         "kFSQhACNOR4JU8Pza8Q4zQhe3S2TTOGTjDV6cR4NSjQaWwqF7RLB9OUVaiZ0bd3h"
         "bYJvR3yXR34KD99lUxQ+LKOnNeAuzNlLJ9BIYdERndDDKK3z9o+wlLhncWvX3A3u"
         "uxC4JA5oA0iT6tgtVMnadUxGx+7gw3/b7khTYEem+hrkmgFCSRth/VppPjgTYOpu"
         "WTATI29kuo87Ht0b3vx/ygNWzymHcu2cF6CYANdYNSn2yBPsGIvLk9hDLUSMbR9t"
         "9efNinaiZzZdZ2pdje2/iiPzZhKlmZAoqJXr16E33HoAm8ZpX6zB5QDjJcl2eBl1"
         "Cui5DoH6QWvnNzp/e2qvOBejTAZBWtQgGMgFjk8s8+S/32P0eZHUvT8bZkRfB46i"
         "2/+sLWKl6gPZFaCqVWZHtr9fpHDsCmYvaQfAG/BTy4r3eU3xlANQ6sXb4u07eqhV"
         "HsUP3/h1jOZY0Ynqrm0rZPYXeUsZHD/0a7ceAjQCH0ezH6Qwdwlflq2Fujprc0p8"
         "jzbmIBJ//////////wIBAg=="
         "-----END X942 DH PARAMETERS-----";

   if(name == "modp/srp/6144")
      return
         "-----BEGIN DH PARAMETERS-----"
         "MIIDCAKCAwEA///////////JD9qiIWjCNMTGYouA3BzRKQJOCIpnzHQCC76mOxOb"
         "IlFKCHmONATd75UZs806QxswKwpt8l8UN0/hNW1tUcJF5IW1dmJefsb0TELppjft"
         "awv/XLb0Brft7jhr+1qJn6WunyQRfEsf5kkoZlHs5Fs9wgB8uKFjvwWY2kg2HFXT"
         "mmkWP6j9JM9fg2VdI9yjrZYcYvNWIIVSu57VKQdwlpZtZww1Tkq8mATxdGwIyhgh"
         "fDKQXkYuNs474553LBgOhgObJ4Oi7Aeij7XFXfBvTFLJ3ivL9pVYFxg5lUl86pVq"
         "5RXSJhiY+gUQFXKOWoqqxC2tMxcNBFB6M6hVIavfHLpk7PuFBFjb7wqK6nFXXQYM"
         "fbOXD4Wm4eTHq/WujNsJM9cejJTgSiVhnc7j0iYa0u5r8S/6BtmKCGTYdgJzPshq"
         "ZFIfKxgXeyAMu+EXV3phXWx3CYjAutlG4gjiT6B05asxQ9tb/OD9EI5LgtEgqSEI"
         "ARpyPBKnh+bXiHGaEL26WyaZwycYavTiPBqUaDS2FQvaJYPpyirUTOjbu8LbBN6O"
         "+S6O/BQfvsqmKHxZR05rwF2ZspZPoJDDoiM7oYZRW+ftH2EpcM7i16+4G912IXBI"
         "HNAGkSfVsFqpk7TqmI2P3cGG/7fckKbAj030Nck0AoSSNsP6tNJ8cCbB1NyyYCZG"
         "3sl1HnY9uje9+P+UBq2eUw7l2zgvQTABrrBqU+2QJ9gxF5cnsIZaiRjaPtvrz5sU"
         "7UTObLrO1Lsb238UR+bMJUszIFFRK9evQm+49AE3jNK/WYPKAcZLkuzwMuoV0XId"
         "A/SC185udP721V5wL0aYDIK1qEAxkAscnlnnyX++x+jzI6l6fjbMiL4PHUW3/1ha"
         "xUvUB7IrQVSqzI9tfr9I4dgUzF7SD4A34KeXFe7ym+MoBqHVi7fF2nb1UKo9ih+/"
         "8OsZzLGjE9Vc2lbJ7C7yljI4f+jXbjwEaAQ+j2Y/SGDuEr8tWwt0dNbmlPkebcxA"
         "JP//////////AgEF"
         "-----END DH PARAMETERS-----";

   if(name == "modp/ietf/8192")
      return
         "-----BEGIN X942 DH PARAMETERS-----"
         "MIIIDAKCBAEA///////////JD9qiIWjCNMTGYouA3BzRKQJOCIpnzHQCC76mOxOb"
         "IlFKCHmONATd75UZs806QxswKwpt8l8UN0/hNW1tUcJF5IW1dmJefsb0TELppjft"
         "awv/XLb0Brft7jhr+1qJn6WunyQRfEsf5kkoZlHs5Fs9wgB8uKFjvwWY2kg2HFXT"
         "mmkWP6j9JM9fg2VdI9yjrZYcYvNWIIVSu57VKQdwlpZtZww1Tkq8mATxdGwIyhgh"
         "fDKQXkYuNs474553LBgOhgObJ4Oi7Aeij7XFXfBvTFLJ3ivL9pVYFxg5lUl86pVq"
         "5RXSJhiY+gUQFXKOWoqqxC2tMxcNBFB6M6hVIavfHLpk7PuFBFjb7wqK6nFXXQYM"
         "fbOXD4Wm4eTHq/WujNsJM9cejJTgSiVhnc7j0iYa0u5r8S/6BtmKCGTYdgJzPshq"
         "ZFIfKxgXeyAMu+EXV3phXWx3CYjAutlG4gjiT6B05asxQ9tb/OD9EI5LgtEgqSEI"
         "ARpyPBKnh+bXiHGaEL26WyaZwycYavTiPBqUaDS2FQvaJYPpyirUTOjbu8LbBN6O"
         "+S6O/BQfvsqmKHxZR05rwF2ZspZPoJDDoiM7oYZRW+ftH2EpcM7i16+4G912IXBI"
         "HNAGkSfVsFqpk7TqmI2P3cGG/7fckKbAj030Nck0AoSSNsP6tNJ8cCbB1NyyYCZG"
         "3sl1HnY9uje9+P+UBq2eUw7l2zgvQTABrrBqU+2QJ9gxF5cnsIZaiRjaPtvrz5sU"
         "7UTObLrO1Lsb238UR+bMJUszIFFRK9evQm+49AE3jNK/WYPKAcZLkuzwMuoV0XId"
         "A/SC185udP721V5wL0aYDIK1qEAxkAscnlnnyX++x+jzI6l6fjbMiL4PHUW3/1ha"
         "xUvUB7IrQVSqzI9tfr9I4dgUzF7SD4A34KeXFe7ym+MoBqHVi7fF2nb1UKo9ih+/"
         "8OsZzLGjE9Vc2lbJ7C7yljI4f+jXbjwEaAQ+j2Y/SGDuEr8tWwt0dNbmlPkebb4R"
         "WXSjkm8S/uXkOHd8tqky34zYvsTQc7kxujvIMraNndMAdB+nv4r8R+0ldvaTa6Qk"
         "ZjqrY5xa5PVoNCO0dCvxyXgjjxbL451lLeP9uL78hIrZIiIuBKQDfAcT61eoGiPw"
         "xzRz/GRs6jBrS8vIhi+Dhd36nUt/osCH6HloMwPtW906Bis89bOieKZtKhP4P0T4"
         "Ld8xDuB0q2o2RZfomaAlXcFk8xzFCEaFHfmrSBld7X6hsdUQvX7nTXP682vDHs+i"
         "aDWQRvTrh5+SQAlDi0gcbNeImgAu1e44K8kZDab8Am5HlVjkR1Z36aqeMFDidlaU"
         "38gfVuiAuW5xYMmA3Zjt09///////////wKCBAB//////////+SH7VEQtGEaYmMx"
         "RcBuDmiUgScERTPmOgEF31Mdic2RKKUEPMcaAm73yozZ5p0hjZgVhTb5L4obp/Ca"
         "trao4SLyQtq7MS8/Y3omIXTTG/a1hf+uW3oDW/b3HDX9rUTP0tdPkgi+JY/zJJQz"
         "KPZyLZ7hAD5cULHfgsxtJBsOKunNNIsf1H6SZ6/Bsq6R7lHWyw4xeasQQqldz2qU"
         "g7hLSzazhhqnJV5MAni6NgRlDBC+GUgvIxcbZx3xzzuWDAdDAc2TwdF2A9FH2uKu"
         "+DemKWTvFeX7SqwLjBzKpL51SrVyiukTDEx9AogKuUctRVViFtaZi4aCKD0Z1CqQ"
         "1e+OXTJ2fcKCLG33hUV1OKuugwY+2cuHwtNw8mPV+tdGbYSZ649GSnAlErDO53Hp"
         "Ew1pdzX4l/0DbMUEMmw7ATmfZDUyKQ+VjAu9kAZd8IurvTCutjuExGBdbKNxBHEn"
         "0Dpy1Zih7a3+cH6IRyXBaJBUkIQAjTkeCVPD82vEOM0IXt0tk0zhk4w1enEeDUo0"
         "GlsKhe0SwfTlFWomdG3d4W2Cb0d8l0d+Cg/fZVMUPiyjpzXgLszZSyfQSGHREZ3Q"
         "wyit8/aPsJS4Z3Fr19wN7rsQuCQOaANIk+rYLVTJ2nVMRsfu4MN/2+5IU2BHpvoa"
         "5JoBQkkbYf1aaT44E2DqblkwEyNvZLqPOx7dG978f8oDVs8ph3LtnBegmADXWDUp"
         "9sgT7BiLy5PYQy1EjG0fbfXnzYp2omc2XWdqXY3tv4oj82YSpZmQKKiV69ehN9x6"
         "AJvGaV+sweUA4yXJdngZdQrouQ6B+kFr5zc6f3tqrzgXo0wGQVrUIBjIBY5PLPPk"
         "v99j9HmR1L0/G2ZEXweOotv/rC1ipeoD2RWgqlVmR7a/X6Rw7ApmL2kHwBvwU8uK"
         "93lN8ZQDUOrF2+LtO3qoVR7FD9/4dYzmWNGJ6q5tK2T2F3lLGRw/9Gu3HgI0Ah9H"
         "sx+kMHcJX5athbo6a3NKfI823wisulHJN4l/cvIcO75bVJlvxmxfYmg53JjdHeQZ"
         "W0bO6YA6D9PfxX4j9pK7e0m10hIzHVWxzi1yerQaEdo6FfjkvBHHi2XxzrKW8f7c"
         "X35CRWyRERcCUgG+A4n1q9QNEfhjmjn+MjZ1GDWl5eRDF8HC7v1Opb/RYEP0PLQZ"
         "gfat7p0DFZ562dE8UzaVCfwfonwW75iHcDpVtRsiy/RM0BKu4LJ5jmKEI0KO/NWk"
         "DK72v1DY6ohev3Omuf15teGPZ9E0GsgjenXDz8kgBKHFpA42a8RNABdq9xwV5IyG"
         "034BNyPKrHIjqzv01U8YKHE7K0pv5A+rdEBctziwZMBuzHbp7///////////AgEC"
         "-----END X942 DH PARAMETERS-----";

   if(name == "modp/srp/8192")
      return
         "-----BEGIN DH PARAMETERS-----"
         "MIIECAKCBAEA///////////JD9qiIWjCNMTGYouA3BzRKQJOCIpnzHQCC76mOxOb"
         "IlFKCHmONATd75UZs806QxswKwpt8l8UN0/hNW1tUcJF5IW1dmJefsb0TELppjft"
         "awv/XLb0Brft7jhr+1qJn6WunyQRfEsf5kkoZlHs5Fs9wgB8uKFjvwWY2kg2HFXT"
         "mmkWP6j9JM9fg2VdI9yjrZYcYvNWIIVSu57VKQdwlpZtZww1Tkq8mATxdGwIyhgh"
         "fDKQXkYuNs474553LBgOhgObJ4Oi7Aeij7XFXfBvTFLJ3ivL9pVYFxg5lUl86pVq"
         "5RXSJhiY+gUQFXKOWoqqxC2tMxcNBFB6M6hVIavfHLpk7PuFBFjb7wqK6nFXXQYM"
         "fbOXD4Wm4eTHq/WujNsJM9cejJTgSiVhnc7j0iYa0u5r8S/6BtmKCGTYdgJzPshq"
         "ZFIfKxgXeyAMu+EXV3phXWx3CYjAutlG4gjiT6B05asxQ9tb/OD9EI5LgtEgqSEI"
         "ARpyPBKnh+bXiHGaEL26WyaZwycYavTiPBqUaDS2FQvaJYPpyirUTOjbu8LbBN6O"
         "+S6O/BQfvsqmKHxZR05rwF2ZspZPoJDDoiM7oYZRW+ftH2EpcM7i16+4G912IXBI"
         "HNAGkSfVsFqpk7TqmI2P3cGG/7fckKbAj030Nck0AoSSNsP6tNJ8cCbB1NyyYCZG"
         "3sl1HnY9uje9+P+UBq2eUw7l2zgvQTABrrBqU+2QJ9gxF5cnsIZaiRjaPtvrz5sU"
         "7UTObLrO1Lsb238UR+bMJUszIFFRK9evQm+49AE3jNK/WYPKAcZLkuzwMuoV0XId"
         "A/SC185udP721V5wL0aYDIK1qEAxkAscnlnnyX++x+jzI6l6fjbMiL4PHUW3/1ha"
         "xUvUB7IrQVSqzI9tfr9I4dgUzF7SD4A34KeXFe7ym+MoBqHVi7fF2nb1UKo9ih+/"
         "8OsZzLGjE9Vc2lbJ7C7yljI4f+jXbjwEaAQ+j2Y/SGDuEr8tWwt0dNbmlPkebb4R"
         "WXSjkm8S/uXkOHd8tqky34zYvsTQc7kxujvIMraNndMAdB+nv4r8R+0ldvaTa6Qk"
         "ZjqrY5xa5PVoNCO0dCvxyXgjjxbL451lLeP9uL78hIrZIiIuBKQDfAcT61eoGiPw"
         "xzRz/GRs6jBrS8vIhi+Dhd36nUt/osCH6HloMwPtW906Bis89bOieKZtKhP4P0T4"
         "Ld8xDuB0q2o2RZfomaAlXcFk8xzFCEaFHfmrSBld7X6hsdUQvX7nTXP682vDHs+i"
         "aDWQRvTrh5+SQAlDi0gcbNeImgAu1e44K8kZDab8Am5HlVjkR1Z36aqeMFDidlaU"
         "38gfVuiAuW5xYMmA3Zjt09///////////wIBEw=="
         "-----END DH PARAMETERS-----";

   if(name == "dsa/jce/1024")
      return
         "-----BEGIN DSA PARAMETERS-----"
         "MIIBHgKBgQD9f1OBHXUSKVLfSpwu7OTn9hG3UjzvRADDHj+AtlEmaUVdQCJR+1k9"
         "jVj6v8X1ujD2y5tVbNeBO4AdNG/yZmC3a5lQpaSfn+gEexAiwk+7qdf+t8Yb+DtX"
         "58aophUPBPuD9tPFHsMCNVQTWhaRMvZ1864rYdcq7/IiAxmd0UgBxwIVAJdgUI8V"
         "IwvMspK5gqLrhAvwWBz1AoGARpYDUS4wJ4zTlHWV2yLuyYJqYyKtyXNE9B10DDJX"
         "JMj577qn1NgD/4xgnc0QDrxb38+tfGpCX66nhuogUOvpg1HqH9of3yTWlHqmuaoj"
         "dmlTgC9NfUqOy6BtGXaKJJH/sW0O+cQ6mbX3FnL/bwoktETQc20E04oaEyLa9s3Y"
         "jJ0="
         "-----END DSA PARAMETERS-----";

   if(name == "dsa/botan/2048")
      return
         "-----BEGIN DSA PARAMETERS-----"
         "MIICLAKCAQEAkcSKT9+898Aq6V59oSYSK13Shk9Vm4fo50oobVL1m9HeaN/WRdDg"
         "DGDAgAMYkZgDdO61lKUyv9Z7mgnqxLhmOgeRDmjzlGX7cEDSXfE5MuusQ0elMOy6"
         "YchU+biA08DDZgCAWHxFVm2t4mvVo5S+CTtMDyS1r/747GxbPlf7iQJam8FnaZMh"
         "MeFtPJTvyrGNDfBhIDzFPmEDvHLVWUv9QMplOA9EqahR3LB1SV/AM6ilgHGhvXj+"
         "BS9mVVZI60txnSr+i0iA+NrW8VgYuhePiSdMhwvpuW6wjEbEAEDMLv4d+xsYaN0x"
         "nePDSjKmOrbrEiQgmkGWgMx5AtFyjU354QIhAIzX1FD4bwrZTu5M5GmodW0evRBY"
         "JBlD6v+ws1RYXpJNAoIBAA2fXgdhtNvRgz1qsalhoJlsXyIwP3LYTBQPZ8Qx2Uq1"
         "cVvqgaDJjTnOS8941rnryJXTT+idlAkdWEhhXvFfXobxHZb2yWniA936WDVkIKSc"
         "tES1lbkBqTPP4HZ7WU8YoHt/kd7NukRriJkPePL/kfL+fNQ/0uRtGOraH3u2YCxh"
         "f27zpLKE8v2boQo2BC3o+oeiyjZZf+yBFXoUheRAQd8CgwERy4gLvm7UlIFIhvll"
         "zcMTX1zPE4Nyi/ZbgG+WksCxDWxMCcdabKO0ATyxarLBBfa+I66pAA6rIXiYX5cs"
         "mAV+HIbkTnIYaI6krg82NtzKdFydzU5q/7Z8y8E9YTE="
         "-----END DSA PARAMETERS-----";

   if(name == "dsa/botan/3072")
      return
         "-----BEGIN DSA PARAMETERS-----"
         "MIIDLAKCAYEA5LUIgHWWY1heFCRgyi2d/xMviuTIQN2jomZoiRJP5WOLhOiim3rz"
         "+hIJvmv8S1By7Tsrc4e68/hX9HioAijvNgC3az3Pth0g00RlslBtLK+H3259wM6R"
         "vS0Wekb2rcwxxTHk+cervbkq3fNbCoBsZikqX14X6WTdCZkDczrEKKs12A6m9oW/"
         "uovkBo5UGK5eytno/wc94rY+Tn6tNciptwtb1Hz7iNNztm83kxk5sKtxvVWVgJCG"
         "2gFVM30YWg5Ps2pRmxtiArhZHmACRJzxzTpmOE9tIHOxzXO+ypO68eGmEX0COPIi"
         "rh7X/tGFqJDn9n+rj+uXU8wTSlGD3+h64llfe1wtn7tCJJ/dWVE+HTOWs+sv2GaE"
         "8oWoRI/nV6ApiBxAdguU75Gb35dAw4OJWZ7FGm6btRmo4GhJHpzgovz+PLYNZs8N"
         "+tIKjsaEBIaEphREV1vRck1zUrRKdgB3s71r04XOWwpyUMwL92jagpI4Buuc+7E4"
         "hDcxthggjHWbAiEAs+vTZOxp74zzuvZDt1c0sWM5suSeXN4bWcHp+0DuDFsCggGA"
         "K+0h7vg5ZKIwrom7px2ffDnFL8gim047x+WUTTKdoQ8BDqyee69sAJ/E6ylgcj4r"
         "Vt9GY+TDrIAOkljeL3ZJ0gZ4KJP4Ze/KSY0u7zAHTqXop6smJxKk2UovOwuaku5A"
         "D7OKPMWaXcfkNtXABLIuNQKDgbUck0B+sy1K4P1Cy0XhLQ7O6KJiOO3iCCp7FSIR"
         "PGbO+NdFxs88uUX4TS9N4W1Epx3hmCcOE/A1U8iLjTI60LlIob8hA6lJl5tu0W+1"
         "88lT2Vt8jojKZ9z1pjb7nKOdkkIV96iE7Wx+48ltjZcVQnl0t8Q1EoLhPTdz99KL"
         "RS8QiSoTx1hzKN6kgntrNpsqjcFyrcWD9R8qZZjFSD5bxGewL5HQWcQC0Y4sJoD3"
         "dqoG9JKAoscsF8xC1bbnQMXEsas8UcLtCSviotiwU65Xc9FCXtKwjwbi3VBZLfGk"
         "eMFVkc39EVZP+I/zi3IdQjkv2kcyEtz9jS2IqXagCv/m//tDCjWeZMorNRyiQSOU"
         "-----END DSA PARAMETERS-----";

   return nullptr;
   }

}
/*
* OAEP
* (C) 1999-2010,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

OAEP* OAEP::make(const Spec& request)
   {
   if(request.algo_name() == "OAEP" && request.arg_count_between(1, 2))
      {
      if(request.arg_count() == 1 ||
         (request.arg_count() == 2 && request.arg(1) == "MGF1"))
         {
         if(auto hash = HashFunction::create(request.arg(0)))
            return new OAEP(hash.release());
         }
      }

   return nullptr;
   }

/*
* OAEP Pad Operation
*/
secure_vector<byte> OAEP::pad(const byte in[], size_t in_length,
                             size_t key_length,
                             RandomNumberGenerator& rng) const
   {
   key_length /= 8;

   if(key_length < in_length + 2*m_Phash.size() + 1)
      throw Invalid_Argument("OAEP: Input is too large");

   secure_vector<byte> out(key_length);

   rng.randomize(out.data(), m_Phash.size());

   buffer_insert(out, m_Phash.size(), m_Phash.data(), m_Phash.size());
   out[out.size() - in_length - 1] = 0x01;
   buffer_insert(out, out.size() - in_length, in, in_length);

   mgf1_mask(*m_hash,
             out.data(), m_Phash.size(),
             &out[m_Phash.size()], out.size() - m_Phash.size());

   mgf1_mask(*m_hash,
             &out[m_Phash.size()], out.size() - m_Phash.size(),
             out.data(), m_Phash.size());

   return out;
   }

/*
* OAEP Unpad Operation
*/
secure_vector<byte> OAEP::unpad(const byte in[], size_t in_length,
                                size_t key_length) const
   {
   /*
   Must be careful about error messages here; if an attacker can
   distinguish them, it is easy to use the differences as an oracle to
   find the secret key, as described in "A Chosen Ciphertext Attack on
   RSA Optimal Asymmetric Encryption Padding (OAEP) as Standardized in
   PKCS #1 v2.0", James Manger, Crypto 2001

   Also have to be careful about timing attacks! Pointed out by Falko
   Strenzke.
   */

   key_length /= 8;

   // Invalid input: truncate to zero length input, causing later
   // checks to fail
   if(in_length > key_length)
      in_length = 0;

   secure_vector<byte> input(key_length);
   buffer_insert(input, key_length - in_length, in, in_length);

   CT::poison(input.data(), input.size());

   const size_t hlen = m_Phash.size();

   mgf1_mask(*m_hash,
             &input[hlen], input.size() - hlen,
             input.data(), hlen);

   mgf1_mask(*m_hash,
             input.data(), hlen,
             &input[hlen], input.size() - hlen);

   size_t delim_idx = 2 * hlen;
   byte waiting_for_delim = 0xFF;
   byte bad_input = 0;

   for(size_t i = delim_idx; i < input.size(); ++i)
      {
      const byte zero_m = CT::is_zero<byte>(input[i]);
      const byte one_m = CT::is_equal<byte>(input[i], 1);

      const byte add_m = waiting_for_delim & zero_m;

      bad_input |= waiting_for_delim & ~(zero_m | one_m);

      delim_idx += CT::select<byte>(add_m, 1, 0);

      waiting_for_delim &= zero_m;
      }

   // If we never saw any non-zero byte, then it's not valid input
   bad_input |= waiting_for_delim;
   bad_input |= CT::expand_mask<byte>(!same_mem(&input[hlen], m_Phash.data(), hlen));

   CT::unpoison(input.data(), input.size());
   CT::unpoison(&bad_input, 1);
   CT::unpoison(&delim_idx, 1);

   if(bad_input)
      throw Decoding_Error("Invalid OAEP encoding");

   return secure_vector<byte>(input.begin() + delim_idx + 1, input.end());
   }

/*
* Return the max input size for a given key size
*/
size_t OAEP::maximum_input_size(size_t keybits) const
   {
   if(keybits / 8 > 2*m_Phash.size() + 1)
      return ((keybits / 8) - 2*m_Phash.size() - 1);
   else
      return 0;
   }

/*
* OAEP Constructor
*/
OAEP::OAEP(HashFunction* hash, const std::string& P) : m_hash(hash)
   {
   m_Phash = m_hash->process(P);
   }

}
/*
* PSSR
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

PSSR* PSSR::make(const Spec& request)
   {
   if(request.arg(1, "MGF1") != "MGF1")
      return nullptr;

   if(auto h = HashFunction::create(request.arg(0)))
      {
      const size_t salt_size = request.arg_as_integer(2, h->output_length());
      return new PSSR(h.release(), salt_size);
      }

   return nullptr;
   }

/*
* PSSR Update Operation
*/
void PSSR::update(const byte input[], size_t length)
   {
   hash->update(input, length);
   }

/*
* Return the raw (unencoded) data
*/
secure_vector<byte> PSSR::raw_data()
   {
   return hash->final();
   }

/*
* PSSR Encode Operation
*/
secure_vector<byte> PSSR::encoding_of(const secure_vector<byte>& msg,
                                      size_t output_bits,
                                      RandomNumberGenerator& rng)
   {
   const size_t HASH_SIZE = hash->output_length();

   if(msg.size() != HASH_SIZE)
      throw Encoding_Error("PSSR::encoding_of: Bad input length");
   if(output_bits < 8*HASH_SIZE + 8*SALT_SIZE + 9)
      throw Encoding_Error("PSSR::encoding_of: Output length is too small");

   const size_t output_length = (output_bits + 7) / 8;

   secure_vector<byte> salt = rng.random_vec(SALT_SIZE);

   for(size_t j = 0; j != 8; ++j)
      hash->update(0);
   hash->update(msg);
   hash->update(salt);
   secure_vector<byte> H = hash->final();

   secure_vector<byte> EM(output_length);

   EM[output_length - HASH_SIZE - SALT_SIZE - 2] = 0x01;
   buffer_insert(EM, output_length - 1 - HASH_SIZE - SALT_SIZE, salt);
   mgf1_mask(*hash, H.data(), HASH_SIZE, EM.data(), output_length - HASH_SIZE - 1);
   EM[0] &= 0xFF >> (8 * ((output_bits + 7) / 8) - output_bits);
   buffer_insert(EM, output_length - 1 - HASH_SIZE, H);
   EM[output_length-1] = 0xBC;

   return EM;
   }

/*
* PSSR Decode/Verify Operation
*/
bool PSSR::verify(const secure_vector<byte>& const_coded,
                   const secure_vector<byte>& raw, size_t key_bits)
   {
   const size_t HASH_SIZE = hash->output_length();
   const size_t KEY_BYTES = (key_bits + 7) / 8;

   if(key_bits < 8*HASH_SIZE + 9)
      return false;

   if(raw.size() != HASH_SIZE)
      return false;

   if(const_coded.size() > KEY_BYTES || const_coded.size() <= 1)
      return false;

   if(const_coded[const_coded.size()-1] != 0xBC)
      return false;

   secure_vector<byte> coded = const_coded;
   if(coded.size() < KEY_BYTES)
      {
      secure_vector<byte> temp(KEY_BYTES);
      buffer_insert(temp, KEY_BYTES - coded.size(), coded);
      coded = temp;
      }

   const size_t TOP_BITS = 8 * ((key_bits + 7) / 8) - key_bits;
   if(TOP_BITS > 8 - high_bit(coded[0]))
      return false;

   byte* DB = coded.data();
   const size_t DB_size = coded.size() - HASH_SIZE - 1;

   const byte* H = &coded[DB_size];
   const size_t H_size = HASH_SIZE;

   mgf1_mask(*hash, H, H_size, DB, DB_size);
   DB[0] &= 0xFF >> TOP_BITS;

   size_t salt_offset = 0;
   for(size_t j = 0; j != DB_size; ++j)
      {
      if(DB[j] == 0x01)
         { salt_offset = j + 1; break; }
      if(DB[j])
         return false;
      }
   if(salt_offset == 0)
      return false;

   for(size_t j = 0; j != 8; ++j)
      hash->update(0);
   hash->update(raw);
   hash->update(&DB[salt_offset], DB_size - salt_offset);
   secure_vector<byte> H2 = hash->final();

   return same_mem(H, H2.data(), HASH_SIZE);
   }

PSSR::PSSR(HashFunction* h) :
   SALT_SIZE(h->output_length()), hash(h)
   {
   }

PSSR::PSSR(HashFunction* h, size_t salt_size) :
   SALT_SIZE(salt_size), hash(h)
   {
   }

}
/*
* Entropy Source Polling
* (C) 2008-2010,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_ENTROPY_SRC_HIGH_RESOLUTION_TIMER)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_RDRAND)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_DEV_RANDOM)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_EGD)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_UNIX_PROCESS_RUNNER)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_BEOS)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_CAPI)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_WIN32)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_PROC_WALKER)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_DARWIN_SECRANDOM)
#endif

namespace Botan {

std::unique_ptr<Entropy_Source> Entropy_Source::create(const std::string& name)
   {
   if(name == "timestamp")
      {
#if defined(BOTAN_HAS_ENTROPY_SRC_HIGH_RESOLUTION_TIMER)
   return std::unique_ptr<Entropy_Source>(new High_Resolution_Timestamp);
#endif
      }

   if(name == "rdrand")
      {
#if defined(BOTAN_HAS_ENTROPY_SRC_RDRAND)
      return std::unique_ptr<Entropy_Source>(new Intel_Rdrand);
#endif
      }

   if(name == "proc_info")
      {
#if defined(BOTAN_HAS_ENTROPY_SRC_UNIX_PROCESS_RUNNER)
   return std::unique_ptr<Entropy_Source>(new UnixProcessInfo_EntropySource);
#endif
      }

   if(name == "darwin_secrandom")
      {
#if defined(BOTAN_HAS_ENTROPY_SRC_DARWIN_SECRANDOM)
      return std::unique_ptr<Entropy_Source>(new Darwin_SecRandom);
#endif
      }

   if(name == "dev_random")
      {
#if defined(BOTAN_HAS_ENTROPY_SRC_DEV_RANDOM)
      return std::unique_ptr<Entropy_Source>(new Device_EntropySource(BOTAN_SYSTEM_RNG_POLL_DEVICES));
      }

   if(name == "win32_cryptoapi")
      {
#elif defined(BOTAN_HAS_ENTROPY_SRC_CAPI)
      return std::unique_ptr<Entropy_Source>(new Win32_CAPI_EntropySource);
#endif
      }

   if(name == "proc_walk")
      {
#if defined(BOTAN_HAS_ENTROPY_SRC_PROC_WALKER)
      return std::unique_ptr<Entropy_Source>(new ProcWalking_EntropySource("/proc"));
#endif
      }

   if(name == "system_stats")
      {
#if defined(BOTAN_HAS_ENTROPY_SRC_WIN32)
      return std::unique_ptr<Entropy_Source>(new Win32_EntropySource);
#elif defined(BOTAN_HAS_ENTROPY_SRC_BEOS)
   return std::unique_ptr<Entropy_Source>(new BeOS_EntropySource);
#endif
      }

   if(name == "unix_procs")
      {
#if defined(BOTAN_HAS_ENTROPY_SRC_UNIX_PROCESS_RUNNER)
      return std::unique_ptr<Entropy_Source>(new Unix_EntropySource(BOTAN_ENTROPY_SAFE_PATHS));
#endif
      }

   if(name == "egd")
      {
#if defined(BOTAN_HAS_ENTROPY_SRC_EGD)
      return std::unique_ptr<Entropy_Source>(new EGD_EntropySource(BOTAN_ENTROPY_EGD_PATHS));
#endif
      }

   return std::unique_ptr<Entropy_Source>();
   }

void Entropy_Sources::add_source(std::unique_ptr<Entropy_Source> src)
   {
   if(src.get())
      {
      m_srcs.push_back(src.release());
      }
   }

std::vector<std::string> Entropy_Sources::enabled_sources() const
   {
   std::vector<std::string> sources;
   for(size_t i = 0; i != m_srcs.size(); ++i)
      {
      sources.push_back(m_srcs[i]->name());
      }
   return sources;
   }

void Entropy_Sources::poll(Entropy_Accumulator& accum)
   {
   for(size_t i = 0; i != m_srcs.size(); ++i)
      {
      m_srcs[i]->poll(accum);
      if(accum.polling_goal_achieved())
         break;
      }
   }

bool Entropy_Sources::poll_just(Entropy_Accumulator& accum, const std::string& the_src)
   {
   for(size_t i = 0; i != m_srcs.size(); ++i)
      {
      if(m_srcs[i]->name() == the_src)
         {
         m_srcs[i]->poll(accum);
         return true;
         }
      }

   return false;
   }

Entropy_Sources::Entropy_Sources(const std::vector<std::string>& sources)
   {
   for(auto&& src_name : sources)
      {
      add_source(Entropy_Source::create(src_name));
      }
   }

Entropy_Sources::~Entropy_Sources()
   {
   for(size_t i = 0; i != m_srcs.size(); ++i)
      {
      delete m_srcs[i];
      m_srcs[i] = nullptr;
      }
   m_srcs.clear();
   }

Entropy_Sources& Entropy_Sources::global_sources()
   {
   static Entropy_Sources global_entropy_sources(BOTAN_ENTROPY_DEFAULT_SOURCES);

   return global_entropy_sources;
   }

}

/*
* Filters
* (C) 1999-2007,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

StreamCipher_Filter::StreamCipher_Filter(StreamCipher* cipher) :
   m_buffer(DEFAULT_BUFFERSIZE),
   m_cipher(cipher)
   {
   }

StreamCipher_Filter::StreamCipher_Filter(StreamCipher* cipher, const SymmetricKey& key) :
   m_buffer(DEFAULT_BUFFERSIZE),
   m_cipher(cipher)
   {
   m_cipher->set_key(key);
   }

StreamCipher_Filter::StreamCipher_Filter(const std::string& sc_name) :
   m_buffer(DEFAULT_BUFFERSIZE),
   m_cipher(StreamCipher::create(sc_name))
   {
   if(!m_cipher)
      throw Algorithm_Not_Found(sc_name);
   }

StreamCipher_Filter::StreamCipher_Filter(const std::string& sc_name, const SymmetricKey& key) :
   m_buffer(DEFAULT_BUFFERSIZE),
   m_cipher(StreamCipher::create(sc_name))
   {
   if(!m_cipher)
      throw Algorithm_Not_Found(sc_name);
   m_cipher->set_key(key);
   }

void StreamCipher_Filter::write(const byte input[], size_t length)
   {
   while(length)
      {
      size_t copied = std::min<size_t>(length, m_buffer.size());
      m_cipher->cipher(input, m_buffer.data(), copied);
      send(m_buffer, copied);
      input += copied;
      length -= copied;
      }
   }

Hash_Filter::Hash_Filter(const std::string& hash_name, size_t len) :
   m_hash(HashFunction::create(hash_name)),
   m_out_len(len)
   {
   if(!m_hash)
      throw Algorithm_Not_Found(hash_name);
   }
void Hash_Filter::end_msg()   {
   secure_vector<byte> output = m_hash->final();
   if(m_out_len)
      send(output, std::min<size_t>(m_out_len, output.size()));
   else
      send(output);
   }

MAC_Filter::MAC_Filter(const std::string& mac_name, size_t len) :
   m_mac(MessageAuthenticationCode::create(mac_name)),
   m_out_len(len)
   {
   if(!m_mac)
      throw Algorithm_Not_Found(mac_name);
   }

MAC_Filter::MAC_Filter(const std::string& mac_name, const SymmetricKey& key, size_t len) :
   m_mac(MessageAuthenticationCode::create(mac_name)),
   m_out_len(len)
   {
   if(!m_mac)
      throw Algorithm_Not_Found(mac_name);
   m_mac->set_key(key);
   }

void MAC_Filter::end_msg()
   {
   secure_vector<byte> output = m_mac->final();
   if(m_out_len)
      send(output, std::min<size_t>(m_out_len, output.size()));
   else
      send(output);
   }

}
/*
* Basic Filters
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

void Keyed_Filter::set_iv(const InitializationVector& iv)
   {
   if(iv.length() != 0)
      throw Invalid_IV_Length(name(), iv.length());
   }

/*
* Chain Constructor
*/
Chain::Chain(Filter* f1, Filter* f2, Filter* f3, Filter* f4)
   {
   if(f1) { attach(f1); incr_owns(); }
   if(f2) { attach(f2); incr_owns(); }
   if(f3) { attach(f3); incr_owns(); }
   if(f4) { attach(f4); incr_owns(); }
   }

/*
* Chain Constructor
*/
Chain::Chain(Filter* filters[], size_t count)
   {
   for(size_t j = 0; j != count; ++j)
      if(filters[j])
         {
         attach(filters[j]);
         incr_owns();
         }
   }

std::string Chain::name() const
   {
   return "Chain";
   }

/*
* Fork Constructor
*/
Fork::Fork(Filter* f1, Filter* f2, Filter* f3, Filter* f4)
   {
   Filter* filters[4] = { f1, f2, f3, f4 };
   set_next(filters, 4);
   }

/*
* Fork Constructor
*/
Fork::Fork(Filter* filters[], size_t count)
   {
   set_next(filters, count);
   }

std::string Fork::name() const
   {
   return "Fork";
   }

}
/*
* Buffered Filter
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Buffered_Filter Constructor
*/
Buffered_Filter::Buffered_Filter(size_t b, size_t f) :
   main_block_mod(b), final_minimum(f)
   {
   if(main_block_mod == 0)
      throw std::invalid_argument("main_block_mod == 0");

   if(final_minimum > main_block_mod)
      throw std::invalid_argument("final_minimum > main_block_mod");

   buffer.resize(2 * main_block_mod);
   buffer_pos = 0;
   }

/*
* Buffer input into blocks, trying to minimize copying
*/
void Buffered_Filter::write(const byte input[], size_t input_size)
   {
   if(!input_size)
      return;

   if(buffer_pos + input_size >= main_block_mod + final_minimum)
      {
      size_t to_copy = std::min<size_t>(buffer.size() - buffer_pos, input_size);

      copy_mem(&buffer[buffer_pos], input, to_copy);
      buffer_pos += to_copy;

      input += to_copy;
      input_size -= to_copy;

      size_t total_to_consume =
         round_down(std::min(buffer_pos,
                             buffer_pos + input_size - final_minimum),
                    main_block_mod);

      buffered_block(buffer.data(), total_to_consume);

      buffer_pos -= total_to_consume;

      copy_mem(buffer.data(), buffer.data() + total_to_consume, buffer_pos);
      }

   if(input_size >= final_minimum)
      {
      size_t full_blocks = (input_size - final_minimum) / main_block_mod;
      size_t to_copy = full_blocks * main_block_mod;

      if(to_copy)
         {
         buffered_block(input, to_copy);

         input += to_copy;
         input_size -= to_copy;
         }
      }

   copy_mem(&buffer[buffer_pos], input, input_size);
   buffer_pos += input_size;
   }

/*
* Finish/flush operation
*/
void Buffered_Filter::end_msg()
   {
   if(buffer_pos < final_minimum)
      throw std::runtime_error("Buffered filter end_msg without enough input");

   size_t spare_blocks = (buffer_pos - final_minimum) / main_block_mod;

   if(spare_blocks)
      {
      size_t spare_bytes = main_block_mod * spare_blocks;
      buffered_block(buffer.data(), spare_bytes);
      buffered_final(&buffer[spare_bytes], buffer_pos - spare_bytes);
      }
   else
      {
      buffered_final(buffer.data(), buffer_pos);
      }

   buffer_pos = 0;
   }

}
/*
* Filter interface for compression
* (C) 2014,2015 Jack Lloyd
* (C) 2015 Matej Kenda
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

Compression_Filter::Compression_Filter(const std::string& type, size_t level, size_t bs) :
   Compression_Decompression_Filter(make_compressor(type, level), bs)
   {
   }

Decompression_Filter::Decompression_Filter(const std::string& type, size_t bs) :
   Compression_Decompression_Filter(make_decompressor(type), bs)
   {
   }

Compression_Decompression_Filter::Compression_Decompression_Filter(Transform* transform, size_t bs) :
   m_buffersize(std::max<size_t>(256, bs)), m_buffer(m_buffersize)
   {
   if (!transform)
      {
         throw std::invalid_argument("Transform is null");
      }
   m_transform.reset(dynamic_cast<Compressor_Transform*>(transform));
   if(!m_transform)
      {
      throw std::invalid_argument("Transform " + transform->name() + " is not a compressor");
      }
   }

std::string Compression_Decompression_Filter::name() const
   {
   return m_transform->name();
   }

void Compression_Decompression_Filter::start_msg()
   {
   send(m_transform->start());
   }

void Compression_Decompression_Filter::write(const byte input[], size_t input_length)
   {
   while(input_length)
      {
      const size_t take = std::min(m_buffersize, input_length);
      BOTAN_ASSERT(take > 0, "Consumed something");

      m_buffer.assign(input, input + take);
      m_transform->update(m_buffer);

      send(m_buffer);

      input += take;
      input_length -= take;
      }
   }

void Compression_Decompression_Filter::flush()
   {
   m_buffer.clear();
   m_transform->flush(m_buffer);
   send(m_buffer);
   }

void Compression_Decompression_Filter::end_msg()
   {
   m_buffer.clear();
   m_transform->finish(m_buffer);
   send(m_buffer);
   }

}
/*
* DataSink
* (C) 1999-2007 Jack Lloyd
*     2005 Matthew Gregan
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <fstream>

namespace Botan {

/*
* Write to a stream
*/
void DataSink_Stream::write(const byte out[], size_t length)
   {
   sink.write(reinterpret_cast<const char*>(out), length);
   if(!sink.good())
      throw Stream_IO_Error("DataSink_Stream: Failure writing to " +
                            identifier);
   }

/*
* DataSink_Stream Constructor
*/
DataSink_Stream::DataSink_Stream(std::ostream& out,
                                 const std::string& name) :
   identifier(name),
   sink_p(nullptr),
   sink(out)
   {
   }

/*
* DataSink_Stream Constructor
*/
DataSink_Stream::DataSink_Stream(const std::string& path,
                                 bool use_binary) :
   identifier(path),
   sink_p(new std::ofstream(path,
                            use_binary ? std::ios::binary : std::ios::out)),
   sink(*sink_p)
   {
   if(!sink.good())
      {
      delete sink_p;
      throw Stream_IO_Error("DataSink_Stream: Failure opening " + path);
      }
   }

/*
* DataSink_Stream Destructor
*/
DataSink_Stream::~DataSink_Stream()
   {
   delete sink_p;
   }

}
/*
* Filter
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Filter Constructor
*/
Filter::Filter()
   {
   next.resize(1);
   port_num = 0;
   filter_owns = 0;
   owned = false;
   }

/*
* Send data to all ports
*/
void Filter::send(const byte input[], size_t length)
   {
   if(!length)
      return;

   bool nothing_attached = true;
   for(size_t j = 0; j != total_ports(); ++j)
      if(next[j])
         {
         if(write_queue.size())
            next[j]->write(write_queue.data(), write_queue.size());
         next[j]->write(input, length);
         nothing_attached = false;
         }

   if(nothing_attached)
      write_queue += std::make_pair(input, length);
   else
      write_queue.clear();
   }

/*
* Start a new message
*/
void Filter::new_msg()
   {
   start_msg();
   for(size_t j = 0; j != total_ports(); ++j)
      if(next[j])
         next[j]->new_msg();
   }

/*
* End the current message
*/
void Filter::finish_msg()
   {
   end_msg();
   for(size_t j = 0; j != total_ports(); ++j)
      if(next[j])
         next[j]->finish_msg();
   }

/*
* Attach a filter to the current port
*/
void Filter::attach(Filter* new_filter)
   {
   if(new_filter)
      {
      Filter* last = this;
      while(last->get_next())
         last = last->get_next();
      last->next[last->current_port()] = new_filter;
      }
   }

/*
* Set the active port on a filter
*/
void Filter::set_port(size_t new_port)
   {
   if(new_port >= total_ports())
      throw Invalid_Argument("Filter: Invalid port number");
   port_num = new_port;
   }

/*
* Return the next Filter in the logical chain
*/
Filter* Filter::get_next() const
   {
   if(port_num < next.size())
      return next[port_num];
   return nullptr;
   }

/*
* Set the next Filters
*/
void Filter::set_next(Filter* filters[], size_t size)
   {
   next.clear();

   port_num = 0;
   filter_owns = 0;

   while(size && filters && (filters[size-1] == nullptr))
      --size;

   if(filters && size)
      next.assign(filters, filters + size);
   }

/*
* Return the total number of ports
*/
size_t Filter::total_ports() const
   {
   return next.size();
   }

}
/*
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

Keyed_Filter* get_cipher(const std::string& algo_spec,
                         Cipher_Dir direction)
   {
   std::unique_ptr<Cipher_Mode> c(get_cipher_mode(algo_spec, direction));
   if(c)
      return new Transform_Filter(c.release());
   throw Algorithm_Not_Found(algo_spec);
   }

Keyed_Filter* get_cipher(const std::string& algo_spec,
                         const SymmetricKey& key,
                         const InitializationVector& iv,
                         Cipher_Dir direction)
   {
   Keyed_Filter* cipher = get_cipher(algo_spec, key, direction);
   if(iv.length())
      cipher->set_iv(iv);
   return cipher;
   }

Keyed_Filter* get_cipher(const std::string& algo_spec,
                         const SymmetricKey& key,
                         Cipher_Dir direction)
   {
   Keyed_Filter* cipher = get_cipher(algo_spec, direction);
   cipher->set_key(key);
   return cipher;
   }

}
/*
* Pipe Output Buffer
* (C) 1999-2007,2011 Jack Lloyd
*     2012 Markus Wanner
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Read data from a message
*/
size_t Output_Buffers::read(byte output[], size_t length,
                            Pipe::message_id msg)
   {
   SecureQueue* q = get(msg);
   if(q)
      return q->read(output, length);
   return 0;
   }

/*
* Peek at data in a message
*/
size_t Output_Buffers::peek(byte output[], size_t length,
                            size_t stream_offset,
                            Pipe::message_id msg) const
   {
   SecureQueue* q = get(msg);
   if(q)
      return q->peek(output, length, stream_offset);
   return 0;
   }

/*
* Check available bytes in a message
*/
size_t Output_Buffers::remaining(Pipe::message_id msg) const
   {
   SecureQueue* q = get(msg);
   if(q)
      return q->size();
   return 0;
   }

/*
* Return the total bytes of a message that have already been read.
*/
size_t Output_Buffers::get_bytes_read(Pipe::message_id msg) const
   {
   SecureQueue* q = get(msg);
   if (q)
      return q->get_bytes_read();
   return 0;
   }

/*
* Add a new output queue
*/
void Output_Buffers::add(SecureQueue* queue)
   {
   BOTAN_ASSERT(queue, "queue was provided");

   BOTAN_ASSERT(buffers.size() < buffers.max_size(),
                "Room was available in container");

   buffers.push_back(queue);
   }

/*
* Retire old output queues
*/
void Output_Buffers::retire()
   {
   for(size_t i = 0; i != buffers.size(); ++i)
      if(buffers[i] && buffers[i]->size() == 0)
         {
         delete buffers[i];
         buffers[i] = nullptr;
         }

   while(buffers.size() && !buffers[0])
      {
      buffers.pop_front();
      offset = offset + Pipe::message_id(1);
      }
   }

/*
* Get a particular output queue
*/
SecureQueue* Output_Buffers::get(Pipe::message_id msg) const
   {
   if(msg < offset)
      return nullptr;

   BOTAN_ASSERT(msg < message_count(), "Message number is in range");

   return buffers[msg-offset];
   }

/*
* Return the total number of messages
*/
Pipe::message_id Output_Buffers::message_count() const
   {
   return (offset + buffers.size());
   }

/*
* Output_Buffers Constructor
*/
Output_Buffers::Output_Buffers()
   {
   offset = 0;
   }

/*
* Output_Buffers Destructor
*/
Output_Buffers::~Output_Buffers()
   {
   for(size_t j = 0; j != buffers.size(); ++j)
      delete buffers[j];
   }

}
/*
* Pipe
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

/*
* A Filter that does nothing
*/
class Null_Filter : public Filter
   {
   public:
      void write(const byte input[], size_t length) override
         { send(input, length); }

      std::string name() const override { return "Null"; }
   };

}

/*
* Pipe Constructor
*/
Pipe::Pipe(Filter* f1, Filter* f2, Filter* f3, Filter* f4)
   {
   init();
   append(f1);
   append(f2);
   append(f3);
   append(f4);
   }

/*
* Pipe Constructor
*/
Pipe::Pipe(std::initializer_list<Filter*> args)
   {
   init();

   for(auto i = args.begin(); i != args.end(); ++i)
      append(*i);
   }

/*
* Pipe Destructor
*/
Pipe::~Pipe()
   {
   destruct(pipe);
   delete outputs;
   }

/*
* Initialize the Pipe
*/
void Pipe::init()
   {
   outputs = new Output_Buffers;
   pipe = nullptr;
   default_read = 0;
   inside_msg = false;
   }

/*
* Reset the Pipe
*/
void Pipe::reset()
   {
   destruct(pipe);
   pipe = nullptr;
   inside_msg = false;
   }

/*
* Destroy the Pipe
*/
void Pipe::destruct(Filter* to_kill)
   {
   if(!to_kill || dynamic_cast<SecureQueue*>(to_kill))
      return;
   for(size_t j = 0; j != to_kill->total_ports(); ++j)
      destruct(to_kill->next[j]);
   delete to_kill;
   }

/*
* Test if the Pipe has any data in it
*/
bool Pipe::end_of_data() const
   {
   return (remaining() == 0);
   }

/*
* Set the default read message
*/
void Pipe::set_default_msg(message_id msg)
   {
   if(msg >= message_count())
      throw Invalid_Argument("Pipe::set_default_msg: msg number is too high");
   default_read = msg;
   }

/*
* Process a full message at once
*/
void Pipe::process_msg(const byte input[], size_t length)
   {
   start_msg();
   write(input, length);
   end_msg();
   }

/*
* Process a full message at once
*/
void Pipe::process_msg(const secure_vector<byte>& input)
   {
   process_msg(input.data(), input.size());
   }

void Pipe::process_msg(const std::vector<byte>& input)
   {
   process_msg(input.data(), input.size());
   }

/*
* Process a full message at once
*/
void Pipe::process_msg(const std::string& input)
   {
   process_msg(reinterpret_cast<const byte*>(input.data()), input.length());
   }

/*
* Process a full message at once
*/
void Pipe::process_msg(DataSource& input)
   {
   start_msg();
   write(input);
   end_msg();
   }

/*
* Start a new message
*/
void Pipe::start_msg()
   {
   if(inside_msg)
      throw Invalid_State("Pipe::start_msg: Message was already started");
   if(pipe == nullptr)
      pipe = new Null_Filter;
   find_endpoints(pipe);
   pipe->new_msg();
   inside_msg = true;
   }

/*
* End the current message
*/
void Pipe::end_msg()
   {
   if(!inside_msg)
      throw Invalid_State("Pipe::end_msg: Message was already ended");
   pipe->finish_msg();
   clear_endpoints(pipe);
   if(dynamic_cast<Null_Filter*>(pipe))
      {
      delete pipe;
      pipe = nullptr;
      }
   inside_msg = false;

   outputs->retire();
   }

/*
* Find the endpoints of the Pipe
*/
void Pipe::find_endpoints(Filter* f)
   {
   for(size_t j = 0; j != f->total_ports(); ++j)
      if(f->next[j] && !dynamic_cast<SecureQueue*>(f->next[j]))
         find_endpoints(f->next[j]);
      else
         {
         SecureQueue* q = new SecureQueue;
         f->next[j] = q;
         outputs->add(q);
         }
   }

/*
* Remove the SecureQueues attached to the Filter
*/
void Pipe::clear_endpoints(Filter* f)
   {
   if(!f) return;
   for(size_t j = 0; j != f->total_ports(); ++j)
      {
      if(f->next[j] && dynamic_cast<SecureQueue*>(f->next[j]))
         f->next[j] = nullptr;
      clear_endpoints(f->next[j]);
      }
   }

/*
* Append a Filter to the Pipe
*/
void Pipe::append(Filter* filter)
   {
   if(inside_msg)
      throw Invalid_State("Cannot append to a Pipe while it is processing");
   if(!filter)
      return;
   if(dynamic_cast<SecureQueue*>(filter))
      throw Invalid_Argument("Pipe::append: SecureQueue cannot be used");
   if(filter->owned)
      throw Invalid_Argument("Filters cannot be shared among multiple Pipes");

   filter->owned = true;

   if(!pipe) pipe = filter;
   else      pipe->attach(filter);
   }

/*
* Prepend a Filter to the Pipe
*/
void Pipe::prepend(Filter* filter)
   {
   if(inside_msg)
      throw Invalid_State("Cannot prepend to a Pipe while it is processing");
   if(!filter)
      return;
   if(dynamic_cast<SecureQueue*>(filter))
      throw Invalid_Argument("Pipe::prepend: SecureQueue cannot be used");
   if(filter->owned)
      throw Invalid_Argument("Filters cannot be shared among multiple Pipes");

   filter->owned = true;

   if(pipe) filter->attach(pipe);
   pipe = filter;
   }

/*
* Pop a Filter off the Pipe
*/
void Pipe::pop()
   {
   if(inside_msg)
      throw Invalid_State("Cannot pop off a Pipe while it is processing");

   if(!pipe)
      return;

   if(pipe->total_ports() > 1)
      throw Invalid_State("Cannot pop off a Filter with multiple ports");

   Filter* f = pipe;
   size_t owns = f->owns();
   pipe = pipe->next[0];
   delete f;

   while(owns--)
      {
      f = pipe;
      pipe = pipe->next[0];
      delete f;
      }
   }

/*
* Return the number of messages in this Pipe
*/
Pipe::message_id Pipe::message_count() const
   {
   return outputs->message_count();
   }

/*
* Static Member Variables
*/
const Pipe::message_id Pipe::LAST_MESSAGE =
   static_cast<Pipe::message_id>(-2);

const Pipe::message_id Pipe::DEFAULT_MESSAGE =
   static_cast<Pipe::message_id>(-1);

}
/*
* Pipe I/O
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Write data from a pipe into an ostream
*/
std::ostream& operator<<(std::ostream& stream, Pipe& pipe)
   {
   secure_vector<byte> buffer(DEFAULT_BUFFERSIZE);
   while(stream.good() && pipe.remaining())
      {
      size_t got = pipe.read(buffer.data(), buffer.size());
      stream.write(reinterpret_cast<const char*>(buffer.data()), got);
      }
   if(!stream.good())
      throw Stream_IO_Error("Pipe output operator (iostream) has failed");
   return stream;
   }

/*
* Read data from an istream into a pipe
*/
std::istream& operator>>(std::istream& stream, Pipe& pipe)
   {
   secure_vector<byte> buffer(DEFAULT_BUFFERSIZE);
   while(stream.good())
      {
      stream.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
      pipe.write(buffer.data(), stream.gcount());
      }
   if(stream.bad() || (stream.fail() && !stream.eof()))
      throw Stream_IO_Error("Pipe input operator (iostream) has failed");
   return stream;
   }

}
/*
* Pipe Reading/Writing
* (C) 1999-2007 Jack Lloyd
*     2012 Markus Wanner
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Look up the canonical ID for a queue
*/
Pipe::message_id Pipe::get_message_no(const std::string& func_name,
                                      message_id msg) const
   {
   if(msg == DEFAULT_MESSAGE)
      msg = default_msg();
   else if(msg == LAST_MESSAGE)
      msg = message_count() - 1;

   if(msg >= message_count())
      throw Invalid_Message_Number(func_name, msg);

   return msg;
   }

/*
* Write into a Pipe
*/
void Pipe::write(const byte input[], size_t length)
   {
   if(!inside_msg)
      throw Invalid_State("Cannot write to a Pipe while it is not processing");
   pipe->write(input, length);
   }

/*
* Write a string into a Pipe
*/
void Pipe::write(const std::string& str)
   {
   write(reinterpret_cast<const byte*>(str.data()), str.size());
   }

/*
* Write a single byte into a Pipe
*/
void Pipe::write(byte input)
   {
   write(&input, 1);
   }

/*
* Write the contents of a DataSource into a Pipe
*/
void Pipe::write(DataSource& source)
   {
   secure_vector<byte> buffer(DEFAULT_BUFFERSIZE);
   while(!source.end_of_data())
      {
      size_t got = source.read(buffer.data(), buffer.size());
      write(buffer.data(), got);
      }
   }

/*
* Read some data from the pipe
*/
size_t Pipe::read(byte output[], size_t length, message_id msg)
   {
   return outputs->read(output, length, get_message_no("read", msg));
   }

/*
* Read some data from the pipe
*/
size_t Pipe::read(byte output[], size_t length)
   {
   return read(output, length, DEFAULT_MESSAGE);
   }

/*
* Read a single byte from the pipe
*/
size_t Pipe::read(byte& out, message_id msg)
   {
   return read(&out, 1, msg);
   }

/*
* Return all data in the pipe
*/
secure_vector<byte> Pipe::read_all(message_id msg)
   {
   msg = ((msg != DEFAULT_MESSAGE) ? msg : default_msg());
   secure_vector<byte> buffer(remaining(msg));
   size_t got = read(buffer.data(), buffer.size(), msg);
   buffer.resize(got);
   return buffer;
   }

/*
* Return all data in the pipe as a string
*/
std::string Pipe::read_all_as_string(message_id msg)
   {
   msg = ((msg != DEFAULT_MESSAGE) ? msg : default_msg());
   secure_vector<byte> buffer(DEFAULT_BUFFERSIZE);
   std::string str;
   str.reserve(remaining(msg));

   while(true)
      {
      size_t got = read(buffer.data(), buffer.size(), msg);
      if(got == 0)
         break;
      str.append(reinterpret_cast<const char*>(buffer.data()), got);
      }

   return str;
   }

/*
* Find out how many bytes are ready to read
*/
size_t Pipe::remaining(message_id msg) const
   {
   return outputs->remaining(get_message_no("remaining", msg));
   }

/*
* Peek at some data in the pipe
*/
size_t Pipe::peek(byte output[], size_t length,
                  size_t offset, message_id msg) const
   {
   return outputs->peek(output, length, offset, get_message_no("peek", msg));
   }

/*
* Peek at some data in the pipe
*/
size_t Pipe::peek(byte output[], size_t length, size_t offset) const
   {
   return peek(output, length, offset, DEFAULT_MESSAGE);
   }

/*
* Peek at a byte in the pipe
*/
size_t Pipe::peek(byte& out, size_t offset, message_id msg) const
   {
   return peek(&out, 1, offset, msg);
   }

size_t Pipe::get_bytes_read() const
   {
   return outputs->get_bytes_read(DEFAULT_MESSAGE);
   }

size_t Pipe::get_bytes_read(message_id msg) const
   {
   return outputs->get_bytes_read(msg);
   }

bool Pipe::check_available(size_t n)
   {
   return (n <= remaining(DEFAULT_MESSAGE));
   }

bool Pipe::check_available_msg(size_t n, message_id msg)
   {
   return (n <= remaining(msg));
   }

}
/*
* SecureQueue
* (C) 1999-2007 Jack Lloyd
*     2012 Markus Wanner
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/**
* A node in a SecureQueue
*/
class SecureQueueNode
   {
   public:
      SecureQueueNode() : buffer(DEFAULT_BUFFERSIZE)
         { next = nullptr; start = end = 0; }

      ~SecureQueueNode() { next = nullptr; start = end = 0; }

      size_t write(const byte input[], size_t length)
         {
         size_t copied = std::min<size_t>(length, buffer.size() - end);
         copy_mem(buffer.data() + end, input, copied);
         end += copied;
         return copied;
         }

      size_t read(byte output[], size_t length)
         {
         size_t copied = std::min(length, end - start);
         copy_mem(output, buffer.data() + start, copied);
         start += copied;
         return copied;
         }

      size_t peek(byte output[], size_t length, size_t offset = 0)
         {
         const size_t left = end - start;
         if(offset >= left) return 0;
         size_t copied = std::min(length, left - offset);
         copy_mem(output, buffer.data() + start + offset, copied);
         return copied;
         }

      size_t size() const { return (end - start); }
   private:
      friend class SecureQueue;
      SecureQueueNode* next;
      secure_vector<byte> buffer;
      size_t start, end;
   };

/*
* Create a SecureQueue
*/
SecureQueue::SecureQueue()
   {
   m_bytes_read = 0;
   set_next(nullptr, 0);
   m_head = m_tail = new SecureQueueNode;
   }

/*
* Copy a SecureQueue
*/
SecureQueue::SecureQueue(const SecureQueue& input) :
   Fanout_Filter(), DataSource()
   {
   m_bytes_read = 0;
   set_next(nullptr, 0);

   m_head = m_tail = new SecureQueueNode;
   SecureQueueNode* temp = input.m_head;
   while(temp)
      {
      write(&temp->buffer[temp->start], temp->end - temp->start);
      temp = temp->next;
      }
   }

/*
* Destroy this SecureQueue
*/
void SecureQueue::destroy()
   {
   SecureQueueNode* temp = m_head;
   while(temp)
      {
      SecureQueueNode* holder = temp->next;
      delete temp;
      temp = holder;
      }
   m_head = m_tail = nullptr;
   }

/*
* Copy a SecureQueue
*/
SecureQueue& SecureQueue::operator=(const SecureQueue& input)
   {
   destroy();
   m_head = m_tail = new SecureQueueNode;
   SecureQueueNode* temp = input.m_head;
   while(temp)
      {
      write(&temp->buffer[temp->start], temp->end - temp->start);
      temp = temp->next;
      }
   return (*this);
   }

/*
* Add some bytes to the queue
*/
void SecureQueue::write(const byte input[], size_t length)
   {
   if(!m_head)
      m_head = m_tail = new SecureQueueNode;
   while(length)
      {
      const size_t n = m_tail->write(input, length);
      input += n;
      length -= n;
      if(length)
         {
         m_tail->next = new SecureQueueNode;
         m_tail = m_tail->next;
         }
      }
   }

/*
* Read some bytes from the queue
*/
size_t SecureQueue::read(byte output[], size_t length)
   {
   size_t got = 0;
   while(length && m_head)
      {
      const size_t n = m_head->read(output, length);
      output += n;
      got += n;
      length -= n;
      if(m_head->size() == 0)
         {
         SecureQueueNode* holder = m_head->next;
         delete m_head;
         m_head = holder;
         }
      }
   m_bytes_read += got;
   return got;
   }

/*
* Read data, but do not remove it from queue
*/
size_t SecureQueue::peek(byte output[], size_t length, size_t offset) const
   {
   SecureQueueNode* current = m_head;

   while(offset && current)
      {
      if(offset >= current->size())
         {
         offset -= current->size();
         current = current->next;
         }
      else
         break;
      }

   size_t got = 0;
   while(length && current)
      {
      const size_t n = current->peek(output, length, offset);
      offset = 0;
      output += n;
      got += n;
      length -= n;
      current = current->next;
      }
   return got;
   }

/**
* Return how many bytes have been read so far.
*/
size_t SecureQueue::get_bytes_read() const
   {
   return m_bytes_read;
   }

/*
* Return how many bytes the queue holds
*/
size_t SecureQueue::size() const
   {
   SecureQueueNode* current = m_head;
   size_t count = 0;

   while(current)
      {
      count += current->size();
      current = current->next;
      }
   return count;
   }

/*
* Test if the queue has any data in it
*/
bool SecureQueue::end_of_data() const
   {
   return (size() == 0);
   }

bool SecureQueue::empty() const
   {
   return (size() == 0);
   }

}
/*
* Threaded Fork
* (C) 2013 Joel Low
*     2013 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

struct Threaded_Fork_Data
   {
   /*
   * Semaphore for indicating that there is work to be done (or to
   * quit)
   */
   Semaphore m_input_ready_semaphore;

   /*
   * Ensures that all threads have completed processing data.
   */
   Semaphore m_input_complete_semaphore;

   /*
   * The work that needs to be done. This should be only when the threads
   * are NOT running (i.e. before notifying the work condition, after
   * the input_complete_semaphore is completely reset.)
   */
   const byte* m_input = nullptr;

   /*
   * The length of the work that needs to be done.
   */
   size_t m_input_length = 0;
   };

/*
* Threaded_Fork constructor
*/
Threaded_Fork::Threaded_Fork(Filter* f1, Filter* f2, Filter* f3, Filter* f4) :
   Fork(nullptr, static_cast<size_t>(0)),
   m_thread_data(new Threaded_Fork_Data)
   {
   Filter* filters[4] = { f1, f2, f3, f4 };
   set_next(filters, 4);
   }

/*
* Threaded_Fork constructor
*/
Threaded_Fork::Threaded_Fork(Filter* filters[], size_t count) :
   Fork(nullptr, static_cast<size_t>(0)),
   m_thread_data(new Threaded_Fork_Data)
   {
   set_next(filters, count);
   }

Threaded_Fork::~Threaded_Fork()
   {
   m_thread_data->m_input = nullptr;
   m_thread_data->m_input_length = 0;

   m_thread_data->m_input_ready_semaphore.release(m_threads.size());

   for(auto& thread : m_threads)
     thread->join();
   }

std::string Threaded_Fork::name() const
   {
   return "Threaded Fork";
   }

void Threaded_Fork::set_next(Filter* f[], size_t n)
   {
   Fork::set_next(f, n);
   n = next.size();

   if(n < m_threads.size())
      m_threads.resize(n);
   else
      {
      m_threads.reserve(n);
      for(size_t i = m_threads.size(); i != n; ++i)
         {
         m_threads.push_back(
            std::shared_ptr<std::thread>(
               new std::thread(
                  std::bind(&Threaded_Fork::thread_entry, this, next[i]))));
         }
      }
   }

void Threaded_Fork::send(const byte input[], size_t length)
   {
   if(write_queue.size())
      thread_delegate_work(write_queue.data(), write_queue.size());
   thread_delegate_work(input, length);

   bool nothing_attached = true;
   for(size_t j = 0; j != total_ports(); ++j)
      if(next[j])
         nothing_attached = false;

   if(nothing_attached)
      write_queue += std::make_pair(input, length);
   else
      write_queue.clear();
   }

void Threaded_Fork::thread_delegate_work(const byte input[], size_t length)
   {
   //Set the data to do.
   m_thread_data->m_input = input;
   m_thread_data->m_input_length = length;

   //Let the workers start processing.
   m_thread_data->m_input_ready_semaphore.release(total_ports());

   //Wait for all the filters to finish processing.
   for(size_t i = 0; i != total_ports(); ++i)
      m_thread_data->m_input_complete_semaphore.acquire();

   //Reset the thread data
   m_thread_data->m_input = nullptr;
   m_thread_data->m_input_length = 0;
   }

void Threaded_Fork::thread_entry(Filter* filter)
   {
   while(true)
      {
      m_thread_data->m_input_ready_semaphore.acquire();

      if(!m_thread_data->m_input)
         break;

      filter->write(m_thread_data->m_input, m_thread_data->m_input_length);
      m_thread_data->m_input_complete_semaphore.release();
      }
   }

}
/*
* Filter interface for Transforms
* (C) 2013,2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

size_t choose_update_size(size_t update_granularity)
   {
   const size_t target_size = 1024;

   if(update_granularity >= target_size)
      return update_granularity;

   return round_up(target_size, update_granularity);
   }

}

Transform_Filter::Transform_Filter(Transform* transform) :
   Buffered_Filter(choose_update_size(transform->update_granularity()),
                   transform->minimum_final_size()),
   m_nonce(transform->default_nonce_length() == 0),
   m_transform(transform),
   m_buffer(m_transform->update_granularity())
   {
   }

std::string Transform_Filter::name() const
   {
   return m_transform->name();
   }

void Transform_Filter::Nonce_State::update(const InitializationVector& iv)
   {
   m_nonce = unlock(iv.bits_of());
   m_fresh_nonce = true;
   }

std::vector<byte> Transform_Filter::Nonce_State::get()
   {
   BOTAN_ASSERT(m_fresh_nonce, "The nonce is fresh for this message");

   if(!m_nonce.empty())
      m_fresh_nonce = false;
   return m_nonce;
   }

void Transform_Filter::set_iv(const InitializationVector& iv)
   {
   m_nonce.update(iv);
   }

void Transform_Filter::set_key(const SymmetricKey& key)
   {
   if(Keyed_Transform* keyed = dynamic_cast<Keyed_Transform*>(m_transform.get()))
      keyed->set_key(key);
   else if(key.length() != 0)
      throw std::runtime_error("Transform " + name() + " does not accept keys");
   }

Key_Length_Specification Transform_Filter::key_spec() const
   {
   if(Keyed_Transform* keyed = dynamic_cast<Keyed_Transform*>(m_transform.get()))
      return keyed->key_spec();
   return Key_Length_Specification(0);
   }

bool Transform_Filter::valid_iv_length(size_t length) const
   {
   return m_transform->valid_nonce_length(length);
   }

void Transform_Filter::write(const byte input[], size_t input_length)
   {
   Buffered_Filter::write(input, input_length);
   }

void Transform_Filter::end_msg()
   {
   Buffered_Filter::end_msg();
   }

void Transform_Filter::start_msg()
   {
   send(m_transform->start(m_nonce.get()));
   }

void Transform_Filter::buffered_block(const byte input[], size_t input_length)
   {
   while(input_length)
      {
      const size_t take = std::min(m_transform->update_granularity(), input_length);

      m_buffer.assign(input, input + take);
      m_transform->update(m_buffer);

      send(m_buffer);

      input += take;
      input_length -= take;
      }
   }

void Transform_Filter::buffered_final(const byte input[], size_t input_length)
   {
   secure_vector<byte> buf(input, input + input_length);
   m_transform->finish(buf);
   send(buf);
   }

}
/*
* GCM Mode Encryption
* (C) 2013,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_GCM_CLMUL)
#endif

namespace Botan {

void GHASH::gcm_multiply(secure_vector<byte>& x) const
   {
#if defined(BOTAN_HAS_GCM_CLMUL)
   if(CPUID::has_clmul())
      return gcm_multiply_clmul(x.data(), m_H.data());
#endif

   static const u64bit R = 0xE100000000000000;

   u64bit H[2] = {
      load_be<u64bit>(m_H.data(), 0),
      load_be<u64bit>(m_H.data(), 1)
   };

   u64bit Z[2] = { 0, 0 };

   CT::poison(H, 2);
   CT::poison(Z, 2);
   CT::poison(x.data(), x.size());

   // SSE2 might be useful here

   for(size_t i = 0; i != 2; ++i)
      {
      const u64bit X = load_be<u64bit>(x.data(), i);

      u64bit mask = 0x8000000000000000;
      for(size_t j = 0; j != 64; ++j)
         {
         const u64bit XMASK = CT::expand_mask<u64bit>(X & mask);
         mask >>= 1;
         Z[0] ^= H[0] & XMASK;
         Z[1] ^= H[1] & XMASK;

         // GCM's bit ops are reversed so we carry out of the bottom
         const u64bit carry = R & CT::expand_mask<u64bit>(H[1] & 1);

         H[1] = (H[1] >> 1) | (H[0] << 63);
         H[0] = (H[0] >> 1) ^ carry;
         }
      }

   store_be<u64bit>(x.data(), Z[0], Z[1]);
   CT::unpoison(x.data(), x.size());
   }

void GHASH::ghash_update(secure_vector<byte>& ghash,
                         const byte input[], size_t length)
   {
   const size_t BS = 16;

   /*
   This assumes if less than block size input then we're just on the
   final block and should pad with zeros
   */
   while(length)
      {
      const size_t to_proc = std::min(length, BS);

      xor_buf(ghash.data(), input, to_proc);

      gcm_multiply(ghash);

      input += to_proc;
      length -= to_proc;
      }
   }

void GHASH::key_schedule(const byte key[], size_t length)
   {
   m_H.assign(key, key+length);
   m_H_ad.resize(16);
   m_ad_len = 0;
   m_text_len = 0;
   }

void GHASH::start(const byte nonce[], size_t len)
   {
   m_nonce.assign(nonce, nonce + len);
   m_ghash = m_H_ad;
   }

void GHASH::set_associated_data(const byte input[], size_t length)
   {
   zeroise(m_H_ad);

   ghash_update(m_H_ad, input, length);
   m_ad_len = length;
   }

void GHASH::update(const byte input[], size_t length)
   {
   BOTAN_ASSERT(m_ghash.size() == 16, "Key was set");

   m_text_len += length;

   ghash_update(m_ghash, input, length);
   }

void GHASH::add_final_block(secure_vector<byte>& hash,
                            size_t ad_len, size_t text_len)
   {
   secure_vector<byte> final_block(16);
   store_be<u64bit>(final_block.data(), 8*ad_len, 8*text_len);
   ghash_update(hash, final_block.data(), final_block.size());
   }

secure_vector<byte> GHASH::final()
   {
   add_final_block(m_ghash, m_ad_len, m_text_len);

   secure_vector<byte> mac;
   mac.swap(m_ghash);

   mac ^= m_nonce;
   m_text_len = 0;
   return mac;
   }

secure_vector<byte> GHASH::nonce_hash(const byte nonce[], size_t nonce_len)
   {
   BOTAN_ASSERT(m_ghash.size() == 0, "nonce_hash called during wrong time");
   secure_vector<byte> y0(16);

   ghash_update(y0, nonce, nonce_len);
   add_final_block(y0, 0, nonce_len);

   return y0;
   }

void GHASH::clear()
   {
   zeroise(m_H);
   zeroise(m_H_ad);
   m_ghash.clear();
   m_text_len = m_ad_len = 0;
   }

/*
* GCM_Mode Constructor
*/
GCM_Mode::GCM_Mode(BlockCipher* cipher, size_t tag_size) :
   m_tag_size(tag_size),
   m_cipher_name(cipher->name())
   {
   if(cipher->block_size() != BS)
      throw std::invalid_argument("GCM requires a 128 bit cipher so cannot be used with " +
                                  cipher->name());

   m_ghash.reset(new GHASH);

   m_ctr.reset(new CTR_BE(cipher)); // CTR_BE takes ownership of cipher

   if(m_tag_size != 8 && m_tag_size != 16)
      throw Invalid_Argument(name() + ": Bad tag size " + std::to_string(m_tag_size));
   }

void GCM_Mode::clear()
   {
   m_ctr->clear();
   m_ghash->clear();
   }

std::string GCM_Mode::name() const
   {
   return (m_cipher_name + "/GCM");
   }

size_t GCM_Mode::update_granularity() const
   {
   return BS;
   }

Key_Length_Specification GCM_Mode::key_spec() const
   {
   return m_ctr->key_spec();
   }

void GCM_Mode::key_schedule(const byte key[], size_t keylen)
   {
   m_ctr->set_key(key, keylen);

   const std::vector<byte> zeros(BS);
   m_ctr->set_iv(zeros.data(), zeros.size());

   secure_vector<byte> H(BS);
   m_ctr->encipher(H);
   m_ghash->set_key(H);
   }

void GCM_Mode::set_associated_data(const byte ad[], size_t ad_len)
   {
   m_ghash->set_associated_data(ad, ad_len);
   }

secure_vector<byte> GCM_Mode::start_raw(const byte nonce[], size_t nonce_len)
   {
   if(!valid_nonce_length(nonce_len))
      throw Invalid_IV_Length(name(), nonce_len);

   secure_vector<byte> y0(BS);

   if(nonce_len == 12)
      {
      copy_mem(y0.data(), nonce, nonce_len);
      y0[15] = 1;
      }
   else
      {
      y0 = m_ghash->nonce_hash(nonce, nonce_len);
      }

   m_ctr->set_iv(y0.data(), y0.size());

   secure_vector<byte> m_enc_y0(BS);
   m_ctr->encipher(m_enc_y0);

   m_ghash->start(m_enc_y0.data(), m_enc_y0.size());

   return secure_vector<byte>();
   }

void GCM_Encryption::update(secure_vector<byte>& buffer, size_t offset)
   {
   BOTAN_ASSERT(buffer.size() >= offset, "Offset is sane");
   const size_t sz = buffer.size() - offset;
   byte* buf = buffer.data() + offset;

   m_ctr->cipher(buf, buf, sz);
   m_ghash->update(buf, sz);
   }

void GCM_Encryption::finish(secure_vector<byte>& buffer, size_t offset)
   {
   update(buffer, offset);
   auto mac = m_ghash->final();
   buffer += std::make_pair(mac.data(), tag_size());
   }

void GCM_Decryption::update(secure_vector<byte>& buffer, size_t offset)
   {
   BOTAN_ASSERT(buffer.size() >= offset, "Offset is sane");
   const size_t sz = buffer.size() - offset;
   byte* buf = buffer.data() + offset;

   m_ghash->update(buf, sz);
   m_ctr->cipher(buf, buf, sz);
   }

void GCM_Decryption::finish(secure_vector<byte>& buffer, size_t offset)
   {
   BOTAN_ASSERT(buffer.size() >= offset, "Offset is sane");
   const size_t sz = buffer.size() - offset;
   byte* buf = buffer.data() + offset;

   BOTAN_ASSERT(sz >= tag_size(), "Have the tag as part of final input");

   const size_t remaining = sz - tag_size();

   // handle any final input before the tag
   if(remaining)
      {
      m_ghash->update(buf, remaining);
      m_ctr->cipher(buf, buf, remaining);
      }

   auto mac = m_ghash->final();

   const byte* included_tag = &buffer[remaining];

   if(!same_mem(mac.data(), included_tag, tag_size()))
      throw Integrity_Failure("GCM tag check failed");

   buffer.resize(offset + remaining);
   }

}
/*
* Hash Functions
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_ADLER32)
#endif

#if defined(BOTAN_HAS_CRC24)
#endif

#if defined(BOTAN_HAS_CRC32)
#endif

#if defined(BOTAN_HAS_GOST_34_11)
#endif

#if defined(BOTAN_HAS_HAS_160)
#endif

#if defined(BOTAN_HAS_KECCAK)
#endif

#if defined(BOTAN_HAS_MD2)
#endif

#if defined(BOTAN_HAS_MD4)
#endif

#if defined(BOTAN_HAS_MD5)
#endif

#if defined(BOTAN_HAS_RIPEMD_128)
#endif

#if defined(BOTAN_HAS_RIPEMD_160)
#endif

#if defined(BOTAN_HAS_SHA1)
#endif

#if defined(BOTAN_HAS_SHA1_SSE2)
#endif

#if defined(BOTAN_HAS_SHA2_32)
#endif

#if defined(BOTAN_HAS_SHA2_64)
#endif

#if defined(BOTAN_HAS_SKEIN_512)
#endif

#if defined(BOTAN_HAS_TIGER)
#endif

#if defined(BOTAN_HAS_WHIRLPOOL)
#endif

#if defined(BOTAN_HAS_PARALLEL_HASH)
#endif

#if defined(BOTAN_HAS_COMB4P)
#endif

namespace Botan {

std::unique_ptr<HashFunction> HashFunction::create(const std::string& algo_spec,
                                                   const std::string& provider)
   {
   return std::unique_ptr<HashFunction>(make_a<HashFunction>(algo_spec, provider));
   }

std::vector<std::string> HashFunction::providers(const std::string& algo_spec)
   {
   return providers_of<HashFunction>(HashFunction::Spec(algo_spec));
   }

HashFunction::HashFunction() {}

HashFunction::~HashFunction() {}

#define BOTAN_REGISTER_HASH(name, maker) BOTAN_REGISTER_T(HashFunction, name, maker)
#define BOTAN_REGISTER_HASH_NOARGS(name) BOTAN_REGISTER_T_NOARGS(HashFunction, name)

#define BOTAN_REGISTER_HASH_1LEN(name, def) BOTAN_REGISTER_T_1LEN(HashFunction, name, def)

#define BOTAN_REGISTER_HASH_NAMED_NOARGS(type, name) \
   BOTAN_REGISTER_NAMED_T(HashFunction, name, type, make_new_T<type>)
#define BOTAN_REGISTER_HASH_NAMED_1LEN(type, name, def) \
   BOTAN_REGISTER_NAMED_T(HashFunction, name, type, (make_new_T_1len<type,def>))

#define BOTAN_REGISTER_HASH_NOARGS_IF(cond, type, name, provider, pref)      \
   BOTAN_COND_REGISTER_NAMED_T_NOARGS(cond, HashFunction, type, name, provider, pref)

#if defined(BOTAN_HAS_ADLER32)
BOTAN_REGISTER_HASH_NOARGS(Adler32);
#endif

#if defined(BOTAN_HAS_CRC24)
BOTAN_REGISTER_HASH_NOARGS(CRC24);
#endif

#if defined(BOTAN_HAS_CRC32)
BOTAN_REGISTER_HASH_NOARGS(CRC32);
#endif

#if defined(BOTAN_HAS_COMB4P)
BOTAN_REGISTER_NAMED_T(HashFunction, "Comb4P", Comb4P, Comb4P::make);
#endif

#if defined(BOTAN_HAS_PARALLEL_HASH)
BOTAN_REGISTER_NAMED_T(HashFunction, "Parallel", Parallel, Parallel::make);
#endif

#if defined(BOTAN_HAS_GOST_34_11)
BOTAN_REGISTER_HASH_NAMED_NOARGS(GOST_34_11, "GOST-R-34.11-94");
#endif

#if defined(BOTAN_HAS_HAS_160)
BOTAN_REGISTER_HASH_NAMED_NOARGS(HAS_160, "HAS-160");
#endif

#if defined(BOTAN_HAS_KECCAK)
BOTAN_REGISTER_HASH_NAMED_1LEN(Keccak_1600, "Keccak-1600", 512);
#endif

#if defined(BOTAN_HAS_MD2)
BOTAN_REGISTER_HASH_NOARGS(MD2);
#endif

#if defined(BOTAN_HAS_MD4)
BOTAN_REGISTER_HASH_NOARGS(MD4);
#endif

#if defined(BOTAN_HAS_MD5)
BOTAN_REGISTER_HASH_NOARGS(MD5);
#endif

#if defined(BOTAN_HAS_RIPEMD_128)
BOTAN_REGISTER_HASH_NAMED_NOARGS(RIPEMD_128, "RIPEMD-128");
#endif

#if defined(BOTAN_HAS_RIPEMD_160)
BOTAN_REGISTER_HASH_NAMED_NOARGS(RIPEMD_160, "RIPEMD-160");
#endif

#if defined(BOTAN_HAS_SHA1)
BOTAN_REGISTER_HASH_NAMED_NOARGS(SHA_160, "SHA-160");
#endif

#if defined(BOTAN_HAS_SHA1_SSE2)
BOTAN_REGISTER_HASH_NOARGS_IF(CPUID::has_sse2(), SHA_160_SSE2, "SHA-160",
                              "sse2", BOTAN_SIMD_ALGORITHM_PRIO);
#endif

#if defined(BOTAN_HAS_SHA2_32)
BOTAN_REGISTER_HASH_NAMED_NOARGS(SHA_224, "SHA-224");
BOTAN_REGISTER_HASH_NAMED_NOARGS(SHA_256, "SHA-256");
#endif

#if defined(BOTAN_HAS_SHA2_64)
BOTAN_REGISTER_HASH_NAMED_NOARGS(SHA_384, "SHA-384");
BOTAN_REGISTER_HASH_NAMED_NOARGS(SHA_512, "SHA-512");
BOTAN_REGISTER_HASH_NAMED_NOARGS(SHA_512_256, "SHA-512-256");
#endif

#if defined(BOTAN_HAS_TIGER)
BOTAN_REGISTER_NAMED_T_2LEN(HashFunction, Tiger, "Tiger", "base", 24, 3);
#endif

#if defined(BOTAN_HAS_SKEIN_512)
BOTAN_REGISTER_NAMED_T(HashFunction, "Skein-512", Skein_512, Skein_512::make);
#endif

#if defined(BOTAN_HAS_WHIRLPOOL)
BOTAN_REGISTER_HASH_NOARGS(Whirlpool);
#endif

}
/*
* Hex Encoding and Decoding
* (C) 2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

void hex_encode(char output[],
                const byte input[],
                size_t input_length,
                bool uppercase)
   {
   static const byte BIN_TO_HEX_UPPER[16] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
      'A', 'B', 'C', 'D', 'E', 'F' };

   static const byte BIN_TO_HEX_LOWER[16] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
      'a', 'b', 'c', 'd', 'e', 'f' };

   const byte* tbl = uppercase ? BIN_TO_HEX_UPPER : BIN_TO_HEX_LOWER;

   for(size_t i = 0; i != input_length; ++i)
      {
      byte x = input[i];
      output[2*i  ] = tbl[(x >> 4) & 0x0F];
      output[2*i+1] = tbl[(x     ) & 0x0F];
      }
   }

std::string hex_encode(const byte input[],
                       size_t input_length,
                       bool uppercase)
   {
   std::string output(2 * input_length, 0);

   if(input_length)
      hex_encode(&output.front(), input, input_length, uppercase);

   return output;
   }

size_t hex_decode(byte output[],
                  const char input[],
                  size_t input_length,
                  size_t& input_consumed,
                  bool ignore_ws)
   {
   /*
   * Mapping of hex characters to either their binary equivalent
   * or to an error code.
   *  If valid hex (0-9 A-F a-f), the value.
   *  If whitespace, then 0x80
   *  Otherwise 0xFF
   * Warning: this table assumes ASCII character encodings
   */

   static const byte HEX_TO_BIN[256] = {
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80,
      0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01,
      0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
      0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0A, 0x0B, 0x0C,
      0x0D, 0x0E, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

   byte* out_ptr = output;
   bool top_nibble = true;

   clear_mem(output, input_length / 2);

   for(size_t i = 0; i != input_length; ++i)
      {
      const byte bin = HEX_TO_BIN[static_cast<byte>(input[i])];

      if(bin >= 0x10)
         {
         if(bin == 0x80 && ignore_ws)
            continue;

         std::string bad_char(1, input[i]);
         if(bad_char == "\t")
           bad_char = "\\t";
         else if(bad_char == "\n")
           bad_char = "\\n";

         throw std::invalid_argument(
           std::string("hex_decode: invalid hex character '") +
           bad_char + "'");
         }

      *out_ptr |= bin << (top_nibble*4);

      top_nibble = !top_nibble;
      if(top_nibble)
         ++out_ptr;
      }

   input_consumed = input_length;
   size_t written = (out_ptr - output);

   /*
   * We only got half of a byte at the end; zap the half-written
   * output and mark it as unread
   */
   if(!top_nibble)
      {
      *out_ptr = 0;
      input_consumed -= 1;
      }

   return written;
   }

size_t hex_decode(byte output[],
                  const char input[],
                  size_t input_length,
                  bool ignore_ws)
   {
   size_t consumed = 0;
   size_t written = hex_decode(output, input, input_length,
                               consumed, ignore_ws);

   if(consumed != input_length)
      throw std::invalid_argument("hex_decode: input did not have full bytes");

   return written;
   }

size_t hex_decode(byte output[],
                  const std::string& input,
                  bool ignore_ws)
   {
   return hex_decode(output, input.data(), input.length(), ignore_ws);
   }

secure_vector<byte> hex_decode_locked(const char input[],
                                      size_t input_length,
                                      bool ignore_ws)
   {
   secure_vector<byte> bin(1 + input_length / 2);

   size_t written = hex_decode(bin.data(),
                               input,
                               input_length,
                               ignore_ws);

   bin.resize(written);
   return bin;
   }

secure_vector<byte> hex_decode_locked(const std::string& input,
                                      bool ignore_ws)
   {
   return hex_decode_locked(input.data(), input.size(), ignore_ws);
   }

std::vector<byte> hex_decode(const char input[],
                             size_t input_length,
                             bool ignore_ws)
   {
   std::vector<byte> bin(1 + input_length / 2);

   size_t written = hex_decode(bin.data(),
                               input,
                               input_length,
                               ignore_ws);

   bin.resize(written);
   return bin;
   }

std::vector<byte> hex_decode(const std::string& input,
                             bool ignore_ws)
   {
   return hex_decode(input.data(), input.size(), ignore_ws);
   }

}
/*
* HMAC
* (C) 1999-2007,2014 Jack Lloyd
*     2007 Yves Jerschow
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

HMAC* HMAC::make(const Spec& spec)
   {
   if(spec.arg_count() == 1)
      {
      if(auto h = HashFunction::create(spec.arg(0)))
         return new HMAC(h.release());
      }
   return nullptr;
   }

/*
* Update a HMAC Calculation
*/
void HMAC::add_data(const byte input[], size_t length)
   {
   m_hash->update(input, length);
   }

/*
* Finalize a HMAC Calculation
*/
void HMAC::final_result(byte mac[])
   {
   m_hash->final(mac);
   m_hash->update(m_okey);
   m_hash->update(mac, output_length());
   m_hash->final(mac);
   m_hash->update(m_ikey);
   }

/*
* HMAC Key Schedule
*/
void HMAC::key_schedule(const byte key[], size_t length)
   {
   m_hash->clear();

   m_ikey.resize(m_hash->hash_block_size());
   m_okey.resize(m_hash->hash_block_size());

   std::fill(m_ikey.begin(), m_ikey.end(), 0x36);
   std::fill(m_okey.begin(), m_okey.end(), 0x5C);

   if(length > m_hash->hash_block_size())
      {
      secure_vector<byte> hmac_key = m_hash->process(key, length);
      xor_buf(m_ikey, hmac_key, hmac_key.size());
      xor_buf(m_okey, hmac_key, hmac_key.size());
      }
   else
      {
      xor_buf(m_ikey, key, length);
      xor_buf(m_okey, key, length);
      }

   m_hash->update(m_ikey);
   }

/*
* Clear memory of sensitive data
*/
void HMAC::clear()
   {
   m_hash->clear();
   zap(m_ikey);
   zap(m_okey);
   }

/*
* Return the name of this type
*/
std::string HMAC::name() const
   {
   return "HMAC(" + m_hash->name() + ")";
   }

/*
* Return a clone of this object
*/
MessageAuthenticationCode* HMAC::clone() const
   {
   return new HMAC(m_hash->clone());
   }

/*
* HMAC Constructor
*/
HMAC::HMAC(HashFunction* hash) : m_hash(hash)
   {
   if(m_hash->hash_block_size() == 0)
      throw Invalid_Argument("HMAC cannot be used with " + m_hash->name());
   }

}
/*
* HMAC_RNG
* (C) 2008,2009,2013,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* HMAC_RNG Constructor
*/
HMAC_RNG::HMAC_RNG(MessageAuthenticationCode* extractor,
                   MessageAuthenticationCode* prf) :
   m_extractor(extractor), m_prf(prf)
   {
   if(!m_prf->valid_keylength(m_extractor->output_length()) ||
      !m_extractor->valid_keylength(m_prf->output_length()))
      {
      throw Invalid_Argument("HMAC_RNG: Bad algo combination " +
                             m_extractor->name() + " and " +
                             m_prf->name());
      }

   this->clear();
   }

void HMAC_RNG::clear()
   {
   m_collected_entropy_estimate = 0;
   m_counter = 0;

   // First PRF inputs are all zero, as specified in section 2
   m_K.resize(m_prf->output_length());
   zeroise(m_K);

   /*
   Normally we want to feedback PRF outputs to the extractor function
   to ensure a single bad poll does not reduce entropy. Thus in reseed
   we'll want to invoke the PRF before we reset the PRF key, but until
   the first reseed the PRF is unkeyed. Rather than trying to keep
   track of this, just set the initial PRF key to constant zero.
   Since all PRF inputs in the first reseed are constants, this
   amounts to suffixing the seed in the first poll with a fixed
   constant string.

   The PRF key will not be used to generate outputs until after reseed
   sets m_seeded to true.
   */
   std::vector<byte> prf_zero_key(m_extractor->output_length());
   m_prf->set_key(prf_zero_key.data(), prf_zero_key.size());

   /*
   Use PRF("Botan HMAC_RNG XTS") as the intitial XTS key.

   This will be used during the first extraction sequence; XTS values
   after this one are generated using the PRF.

   If I understand the E-t-E paper correctly (specifically Section 4),
   using this fixed initial extractor key is safe to do.
   */
   m_extractor->set_key(m_prf->process("Botan HMAC_RNG XTS"));
   }

void HMAC_RNG::new_K_value(byte label)
   {
   typedef std::chrono::high_resolution_clock clock;

   m_prf->update(m_K);
   m_prf->update_be(clock::now().time_since_epoch().count());
   m_prf->update_be(m_counter++);
   m_prf->update(label);
   m_prf->final(m_K.data());
   }

/*
* Generate a buffer of random bytes
*/
void HMAC_RNG::randomize(byte out[], size_t length)
   {
   if(!is_seeded())
      {
      reseed(256);
      if(!is_seeded())
         throw PRNG_Unseeded(name());
      }

   const size_t max_per_prf_iter = m_prf->output_length() / 2;

   m_output_since_reseed += length;

   if(m_output_since_reseed >= BOTAN_RNG_MAX_OUTPUT_BEFORE_RESEED)
      {
      reseed_with_sources(Entropy_Sources::global_sources(),
                          BOTAN_RNG_RESEED_POLL_BITS,
                          BOTAN_RNG_AUTO_RESEED_TIMEOUT);
      }

   /*
    HMAC KDF as described in E-t-E, using a CTXinfo of "rng"
   */
   while(length)
      {
      new_K_value(Running);

      const size_t copied = std::min<size_t>(length, max_per_prf_iter);

      copy_mem(out, m_K.data(), copied);
      out += copied;
      length -= copied;
      }
   }

size_t HMAC_RNG::reseed_with_sources(Entropy_Sources& srcs,
                                     size_t poll_bits,
                                     std::chrono::milliseconds timeout)
   {
   /*
   Using the terminology of E-t-E, XTR is the MAC function (normally
   HMAC) seeded with XTS (below) and we form SKM, the key material, by
   polling as many sources as we think needed to reach our polling
   goal. We then also include feedback of the current PRK so that
   a bad poll doesn't wipe us out.
   */

   typedef std::chrono::system_clock clock;
   auto deadline = clock::now() + timeout;

   double bits_collected = 0;

   Entropy_Accumulator accum([&](const byte in[], size_t in_len, double entropy_estimate) {
      m_extractor->update(in, in_len);
      bits_collected += entropy_estimate;
      return (bits_collected >= poll_bits || clock::now() > deadline);
      });

   srcs.poll(accum);

   /*
   * It is necessary to feed forward poll data. Otherwise, a good poll
   * (collecting a large amount of conditional entropy) followed by a
   * bad one (collecting little) would be unsafe. Do this by
   * generating new PRF outputs using the previous key and feeding
   * them into the extractor function.
   */
   new_K_value(Reseed);
   m_extractor->update(m_K); // K is the CTXinfo=reseed PRF output

   /* Now derive the new PRK using everything that has been fed into
      the extractor, and set the PRF key to that */
   m_prf->set_key(m_extractor->final());

   // Now generate a new PRF output to use as the XTS extractor salt
   new_K_value(ExtractorSeed);
   m_extractor->set_key(m_K);

   // Reset state
   zeroise(m_K);
   m_counter = 0;

   m_collected_entropy_estimate =
      std::min<size_t>(m_collected_entropy_estimate + bits_collected,
                       m_extractor->output_length() * 8);

   m_output_since_reseed = 0;

   return static_cast<size_t>(bits_collected);
   }

bool HMAC_RNG::is_seeded() const
   {
   return (m_collected_entropy_estimate >= 256);
   }

/*
* Add user-supplied entropy to the extractor input then reseed
* to incorporate it into the state
*/
void HMAC_RNG::add_entropy(const byte input[], size_t length)
   {
   m_extractor->update(input, length);

   reseed_with_sources(Entropy_Sources::global_sources(),
                       BOTAN_RNG_RESEED_POLL_BITS,
                       BOTAN_RNG_RESEED_DEFAULT_TIMEOUT);
   }

/*
* Return the name of this type
*/
std::string HMAC_RNG::name() const
   {
   return "HMAC_RNG(" + m_extractor->name() + "," + m_prf->name() + ")";
   }

}
/*
* High Resolution Timestamp Entropy Source
* (C) 1999-2009,2011,2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_TARGET_OS_HAS_QUERY_PERF_COUNTER)
  #include <windows.h>
  #undef min
  #undef max
#endif

#if defined(BOTAN_TARGET_OS_HAS_CLOCK_GETTIME)
  #include <time.h>
#endif

namespace Botan {

/*
* Get the timestamp
*/
void High_Resolution_Timestamp::poll(Entropy_Accumulator& accum)
   {
#if defined(BOTAN_TARGET_OS_HAS_CLOCK_GETTIME)

#define CLOCK_GETTIME_POLL(src)                                     \
   do {                                                             \
     struct timespec ts;                                            \
     ::clock_gettime(src, &ts);                                     \
     accum.add(&ts, sizeof(ts), BOTAN_ENTROPY_ESTIMATE_TIMESTAMPS); \
   } while(0)

#if defined(CLOCK_REALTIME)
   CLOCK_GETTIME_POLL(CLOCK_REALTIME);
#endif

#if defined(CLOCK_MONOTONIC)
   CLOCK_GETTIME_POLL(CLOCK_MONOTONIC);
#endif

#if defined(CLOCK_MONOTONIC_RAW)
   CLOCK_GETTIME_POLL(CLOCK_MONOTONIC_RAW);
#endif

#if defined(CLOCK_PROCESS_CPUTIME_ID)
   CLOCK_GETTIME_POLL(CLOCK_PROCESS_CPUTIME_ID);
#endif

#if defined(CLOCK_THREAD_CPUTIME_ID)
   CLOCK_GETTIME_POLL(CLOCK_THREAD_CPUTIME_ID);
#endif

#undef CLOCK_GETTIME_POLL

#else

#define STD_CHRONO_POLL(clock)                                  \
   do {                                                         \
      auto timestamp = clock::now().time_since_epoch().count(); \
      accum.add(timestamp, BOTAN_ENTROPY_ESTIMATE_TIMESTAMPS);         \
   } while(0)

  STD_CHRONO_POLL(std::chrono::high_resolution_clock);
  STD_CHRONO_POLL(std::chrono::system_clock);

#undef STD_CHRONO_POLL

#endif

#if defined(BOTAN_USE_GCC_INLINE_ASM)

   u64bit rtc = 0;

#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)
   if(CPUID::has_rdtsc()) // not availble on all x86 CPUs
      {
      u32bit rtc_low = 0, rtc_high = 0;
      asm volatile("rdtsc" : "=d" (rtc_high), "=a" (rtc_low));
      rtc = (static_cast<u64bit>(rtc_high) << 32) | rtc_low;
      }

#elif defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY)
   u32bit rtc_low = 0, rtc_high = 0;
   asm volatile("mftbu %0; mftb %1" : "=r" (rtc_high), "=r" (rtc_low));
   rtc = (static_cast<u64bit>(rtc_high) << 32) | rtc_low;

#elif defined(BOTAN_TARGET_ARCH_IS_ALPHA)
   asm volatile("rpcc %0" : "=r" (rtc));

#elif defined(BOTAN_TARGET_ARCH_IS_SPARC64) && !defined(BOTAN_TARGET_OS_IS_OPENBSD)
   asm volatile("rd %%tick, %0" : "=r" (rtc));

#elif defined(BOTAN_TARGET_ARCH_IS_IA64)
   asm volatile("mov %0=ar.itc" : "=r" (rtc));

#elif defined(BOTAN_TARGET_ARCH_IS_S390X)
   asm volatile("stck 0(%0)" : : "a" (&rtc) : "memory", "cc");

#elif defined(BOTAN_TARGET_ARCH_IS_HPPA)
   asm volatile("mfctl 16,%0" : "=r" (rtc)); // 64-bit only?

#endif

   accum.add(rtc, BOTAN_ENTROPY_ESTIMATE_TIMESTAMPS);

#endif

#if defined(BOTAN_TARGET_OS_HAS_QUERY_PERF_COUNTER)
   {
   LARGE_INTEGER tv;
   ::QueryPerformanceCounter(&tv);
   accum.add(tv.QuadPart, BOTAN_ENTROPY_ESTIMATE_TIMESTAMPS);
   }
#endif
   }

}
/*
* IF Scheme
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

size_t IF_Scheme_PublicKey::estimated_strength() const
   {
   return if_work_factor(n.bits());
   }

AlgorithmIdentifier IF_Scheme_PublicKey::algorithm_identifier() const
   {
   return AlgorithmIdentifier(get_oid(),
                              AlgorithmIdentifier::USE_NULL_PARAM);
   }

std::vector<byte> IF_Scheme_PublicKey::x509_subject_public_key() const
   {
   return DER_Encoder()
      .start_cons(SEQUENCE)
         .encode(n)
         .encode(e)
      .end_cons()
      .get_contents_unlocked();
   }

IF_Scheme_PublicKey::IF_Scheme_PublicKey(const AlgorithmIdentifier&,
                                         const secure_vector<byte>& key_bits)
   {
   BER_Decoder(key_bits)
      .start_cons(SEQUENCE)
        .decode(n)
        .decode(e)
      .verify_end()
      .end_cons();
   }

/*
* Check IF Scheme Public Parameters
*/
bool IF_Scheme_PublicKey::check_key(RandomNumberGenerator&, bool) const
   {
   if(n < 35 || n.is_even() || e < 2)
      return false;
   return true;
   }

secure_vector<byte> IF_Scheme_PrivateKey::pkcs8_private_key() const
   {
   return DER_Encoder()
      .start_cons(SEQUENCE)
         .encode(static_cast<size_t>(0))
         .encode(n)
         .encode(e)
         .encode(d)
         .encode(p)
         .encode(q)
         .encode(d1)
         .encode(d2)
         .encode(c)
      .end_cons()
   .get_contents();
   }

IF_Scheme_PrivateKey::IF_Scheme_PrivateKey(RandomNumberGenerator& rng,
                                           const AlgorithmIdentifier&,
                                           const secure_vector<byte>& key_bits)
   {
   BER_Decoder(key_bits)
      .start_cons(SEQUENCE)
         .decode_and_check<size_t>(0, "Unknown PKCS #1 key format version")
         .decode(n)
         .decode(e)
         .decode(d)
         .decode(p)
         .decode(q)
         .decode(d1)
         .decode(d2)
         .decode(c)
      .end_cons();

   load_check(rng);
   }

IF_Scheme_PrivateKey::IF_Scheme_PrivateKey(RandomNumberGenerator& rng,
                                           const BigInt& prime1,
                                           const BigInt& prime2,
                                           const BigInt& exp,
                                           const BigInt& d_exp,
                                           const BigInt& mod)
   {
   p = prime1;
   q = prime2;
   e = exp;
   d = d_exp;
   n = mod.is_nonzero() ? mod : p * q;

   if(d == 0)
      {
      BigInt inv_for_d = lcm(p - 1, q - 1);
      if(e.is_even())
         inv_for_d >>= 1;

      d = inverse_mod(e, inv_for_d);
      }

   d1 = d % (p - 1);
   d2 = d % (q - 1);
   c = inverse_mod(q, p);

   load_check(rng);
   }

/*
* Check IF Scheme Private Parameters
*/
bool IF_Scheme_PrivateKey::check_key(RandomNumberGenerator& rng,
                                     bool strong) const
   {
   if(n < 35 || n.is_even() || e < 2 || d < 2 || p < 3 || q < 3 || p*q != n)
      return false;

   if(d1 != d % (p - 1) || d2 != d % (q - 1) || c != inverse_mod(q, p))
      return false;

   const size_t prob = (strong) ? 56 : 12;

   if(!is_prime(p, rng, prob) || !is_prime(q, rng, prob))
      return false;
   return true;
   }

}
/*
* KDF Retrieval
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_HKDF)
#endif

#if defined(BOTAN_HAS_KDF1)
#endif

#if defined(BOTAN_HAS_KDF2)
#endif

#if defined(BOTAN_HAS_TLS_V10_PRF)
#endif

#if defined(BOTAN_HAS_TLS_V12_PRF)
#endif

#if defined(BOTAN_HAS_X942_PRF)
#endif

#define BOTAN_REGISTER_KDF_NOARGS(type, name)                    \
   BOTAN_REGISTER_NAMED_T(KDF, name, type, (make_new_T<type>))
#define BOTAN_REGISTER_KDF_1HASH(type, name)                    \
   BOTAN_REGISTER_NAMED_T(KDF, name, type, (make_new_T_1X<type, HashFunction>))

#define BOTAN_REGISTER_KDF_NAMED_1STR(type, name) \
   BOTAN_REGISTER_NAMED_T(KDF, name, type, (make_new_T_1str_req<type>))

namespace Botan {

KDF::~KDF() {}

std::unique_ptr<KDF> KDF::create(const std::string& algo_spec,
                                                 const std::string& provider)
   {
   return std::unique_ptr<KDF>(make_a<KDF>(algo_spec, provider));
   }

std::vector<std::string> KDF::providers(const std::string& algo_spec)
   {
   return providers_of<KDF>(KDF::Spec(algo_spec));
   }

KDF* get_kdf(const std::string& algo_spec)
   {
   SCAN_Name request(algo_spec);

   if(request.algo_name() == "Raw")
      return nullptr; // No KDF

   auto kdf = KDF::create(algo_spec);
   if(!kdf)
      throw Algorithm_Not_Found(algo_spec);
   return kdf.release();
   }

#if defined(BOTAN_HAS_HKDF)
BOTAN_REGISTER_NAMED_T(KDF, "HKDF", HKDF, HKDF::make);
#endif

#if defined(BOTAN_HAS_KDF1)
BOTAN_REGISTER_KDF_1HASH(KDF1, "KDF1");
#endif

#if defined(BOTAN_HAS_KDF2)
BOTAN_REGISTER_KDF_1HASH(KDF2, "KDF2");
#endif

#if defined(BOTAN_HAS_TLS_V10_PRF)
BOTAN_REGISTER_KDF_NOARGS(TLS_PRF, "TLS-PRF");
#endif

#if defined(BOTAN_HAS_TLS_V12_PRF)
BOTAN_REGISTER_NAMED_T(KDF, "TLS-12-PRF", TLS_12_PRF, TLS_12_PRF::make);
#endif

#if defined(BOTAN_HAS_X942_PRF)
BOTAN_REGISTER_KDF_NAMED_1STR(X942_PRF, "X9.42-PRF");
#endif

}
/*
* Keypair Checks
* (C) 1999-2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace KeyPair {

/*
* Check an encryption key pair for consistency
*/
bool encryption_consistency_check(RandomNumberGenerator& rng,
                                  const Private_Key& key,
                                  const std::string& padding)
   {
   PK_Encryptor_EME encryptor(key, padding);
   PK_Decryptor_EME decryptor(key, padding);

   /*
   Weird corner case, if the key is too small to encrypt anything at
   all. This can happen with very small RSA keys with PSS
   */
   if(encryptor.maximum_input_size() == 0)
      return true;

   std::vector<byte> plaintext =
      unlock(rng.random_vec(encryptor.maximum_input_size() - 1));

   std::vector<byte> ciphertext = encryptor.encrypt(plaintext, rng);
   if(ciphertext == plaintext)
      return false;

   std::vector<byte> decrypted = unlock(decryptor.decrypt(ciphertext));

   return (plaintext == decrypted);
   }

/*
* Check a signature key pair for consistency
*/
bool signature_consistency_check(RandomNumberGenerator& rng,
                                 const Private_Key& key,
                                 const std::string& padding)
   {
   PK_Signer signer(key, padding);
   PK_Verifier verifier(key, padding);

   std::vector<byte> message = unlock(rng.random_vec(16));

   std::vector<byte> signature;

   try
      {
      signature = signer.sign_message(message, rng);
      }
   catch(Encoding_Error)
      {
      return false;
      }

   if(!verifier.verify_message(message, signature))
      return false;

   // Now try to check a corrupt signature, ensure it does not succeed
   ++message[0];

   if(verifier.verify_message(message, signature))
      return false;

   return true;
   }

}

}
/*
* Message Authentication Code base class
* (C) 1999-2008 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_CBC_MAC)
#endif

#if defined(BOTAN_HAS_CMAC)
#endif

#if defined(BOTAN_HAS_HMAC)
#endif

#if defined(BOTAN_HAS_POLY1305)
#endif

#if defined(BOTAN_HAS_SIPHASH)
#endif

#if defined(BOTAN_HAS_ANSI_X919_MAC)
#endif

namespace Botan {

std::unique_ptr<MessageAuthenticationCode> MessageAuthenticationCode::create(const std::string& algo_spec,
                                                                             const std::string& provider)
   {
   return std::unique_ptr<MessageAuthenticationCode>(make_a<MessageAuthenticationCode>(algo_spec, provider));
   }

std::vector<std::string> MessageAuthenticationCode::providers(const std::string& algo_spec)
   {
   return providers_of<MessageAuthenticationCode>(MessageAuthenticationCode::Spec(algo_spec));
   }

MessageAuthenticationCode::~MessageAuthenticationCode() {}

/*
* Default (deterministic) MAC verification operation
*/
bool MessageAuthenticationCode::verify_mac(const byte mac[], size_t length)
   {
   secure_vector<byte> our_mac = final();

   if(our_mac.size() != length)
      return false;

   return same_mem(our_mac.data(), mac, length);
   }

#if defined(BOTAN_HAS_CBC_MAC)
BOTAN_REGISTER_NAMED_T(MessageAuthenticationCode, "CBC-MAC", CBC_MAC, CBC_MAC::make);
#endif

#if defined(BOTAN_HAS_CMAC)
BOTAN_REGISTER_NAMED_T(MessageAuthenticationCode, "CMAC", CMAC, CMAC::make);
#endif

#if defined(BOTAN_HAS_HMAC)
BOTAN_REGISTER_NAMED_T(MessageAuthenticationCode, "HMAC", HMAC, HMAC::make);
#endif

#if defined(BOTAN_HAS_POLY1305)
BOTAN_REGISTER_T_NOARGS(MessageAuthenticationCode, Poly1305);
#endif

#if defined(BOTAN_HAS_SIPHASH)
BOTAN_REGISTER_NAMED_T_2LEN(MessageAuthenticationCode, SipHash, "SipHash", "base", 2, 4);
#endif

#if defined(BOTAN_HAS_ANSI_X919_MAC)
BOTAN_REGISTER_NAMED_T(MessageAuthenticationCode, "X9.19-MAC", ANSI_X919_MAC, make_new_T<ANSI_X919_MAC>);
#endif

}
/*
* Merkle-Damgard Hash Function
* (C) 1999-2008 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* MDx_HashFunction Constructor
*/
MDx_HashFunction::MDx_HashFunction(size_t block_len,
                                   bool byte_end,
                                   bool bit_end,
                                   size_t cnt_size) :
   buffer(block_len),
   BIG_BYTE_ENDIAN(byte_end),
   BIG_BIT_ENDIAN(bit_end),
   COUNT_SIZE(cnt_size)
   {
   count = position = 0;
   }

/*
* Clear memory of sensitive data
*/
void MDx_HashFunction::clear()
   {
   zeroise(buffer);
   count = position = 0;
   }

/*
* Update the hash
*/
void MDx_HashFunction::add_data(const byte input[], size_t length)
   {
   count += length;

   if(position)
      {
      buffer_insert(buffer, position, input, length);

      if(position + length >= buffer.size())
         {
         compress_n(buffer.data(), 1);
         input += (buffer.size() - position);
         length -= (buffer.size() - position);
         position = 0;
         }
      }

   const size_t full_blocks = length / buffer.size();
   const size_t remaining   = length % buffer.size();

   if(full_blocks)
      compress_n(input, full_blocks);

   buffer_insert(buffer, position, input + full_blocks * buffer.size(), remaining);
   position += remaining;
   }

/*
* Finalize a hash
*/
void MDx_HashFunction::final_result(byte output[])
   {
   buffer[position] = (BIG_BIT_ENDIAN ? 0x80 : 0x01);
   for(size_t i = position+1; i != buffer.size(); ++i)
      buffer[i] = 0;

   if(position >= buffer.size() - COUNT_SIZE)
      {
      compress_n(buffer.data(), 1);
      zeroise(buffer);
      }

   write_count(&buffer[buffer.size() - COUNT_SIZE]);

   compress_n(buffer.data(), 1);
   copy_out(output);
   clear();
   }

/*
* Write the count bits to the buffer
*/
void MDx_HashFunction::write_count(byte out[])
   {
   if(COUNT_SIZE < 8)
      throw Invalid_State("MDx_HashFunction::write_count: COUNT_SIZE < 8");
   if(COUNT_SIZE >= output_length() || COUNT_SIZE >= hash_block_size())
      throw Invalid_Argument("MDx_HashFunction: COUNT_SIZE is too big");

   const u64bit bit_count = count * 8;

   if(BIG_BYTE_ENDIAN)
      store_be(bit_count, out + COUNT_SIZE - 8);
   else
      store_le(bit_count, out + COUNT_SIZE - 8);
   }

}
/*
* MGF1
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

void mgf1_mask(HashFunction& hash,
               const byte in[], size_t in_len,
               byte out[], size_t out_len)
   {
   u32bit counter = 0;

   while(out_len)
      {
      hash.update(in, in_len);
      hash.update_be(counter);
      secure_vector<byte> buffer = hash.final();

      size_t xored = std::min<size_t>(buffer.size(), out_len);
      xor_buf(out, buffer.data(), xored);
      out += xored;
      out_len -= xored;

      ++counter;
      }
   }

}
/*
* CBC Padding Methods
* (C) 1999-2007,2013 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/**
* Get a block cipher padding method by name
*/
BlockCipherModePaddingMethod* get_bc_pad(const std::string& algo_spec)
   {
   if(algo_spec == "NoPadding")
      return new Null_Padding;

   if(algo_spec == "PKCS7")
      return new PKCS7_Padding;

   if(algo_spec == "OneAndZeros")
      return new OneAndZeros_Padding;

   if(algo_spec == "X9.23")
      return new ANSI_X923_Padding;

   return nullptr;
   }

/*
* Pad with PKCS #7 Method
*/
void PKCS7_Padding::add_padding(secure_vector<byte>& buffer,
                                size_t last_byte_pos,
                                size_t block_size) const
   {
   const byte pad_value = block_size - last_byte_pos;

   for(size_t i = 0; i != pad_value; ++i)
      buffer.push_back(pad_value);
   }

/*
* Unpad with PKCS #7 Method
*/
size_t PKCS7_Padding::unpad(const byte block[], size_t size) const
   {
   size_t position = block[size-1];

   if(position > size)
      throw Decoding_Error("Bad padding in " + name());

   for(size_t j = size-position; j != size-1; ++j)
      if(block[j] != position)
         throw Decoding_Error("Bad padding in " + name());

   return (size-position);
   }

/*
* Pad with ANSI X9.23 Method
*/
void ANSI_X923_Padding::add_padding(secure_vector<byte>& buffer,
                                    size_t last_byte_pos,
                                    size_t block_size) const
   {
   const byte pad_value = block_size - last_byte_pos;

   for(size_t i = last_byte_pos; i < block_size; ++i)
      buffer.push_back(0);
   buffer.push_back(pad_value);
   }

/*
* Unpad with ANSI X9.23 Method
*/
size_t ANSI_X923_Padding::unpad(const byte block[], size_t size) const
   {
   size_t position = block[size-1];
   if(position > size)
      throw Decoding_Error(name());
   for(size_t j = size-position; j != size-1; ++j)
      if(block[j] != 0)
         throw Decoding_Error(name());
   return (size-position);
   }

/*
* Pad with One and Zeros Method
*/
void OneAndZeros_Padding::add_padding(secure_vector<byte>& buffer,
                                      size_t last_byte_pos,
                                      size_t block_size) const
   {
   buffer.push_back(0x80);

   for(size_t i = last_byte_pos + 1; i % block_size; ++i)
      buffer.push_back(0x00);
   }

/*
* Unpad with One and Zeros Method
*/
size_t OneAndZeros_Padding::unpad(const byte block[], size_t size) const
   {
   while(size)
      {
      if(block[size-1] == 0x80)
         break;
      if(block[size-1] != 0x00)
         throw Decoding_Error(name());
      size--;
      }
   if(!size)
      throw Decoding_Error(name());
   return (size-1);
   }


}
/*
* Cipher Modes
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_MODE_ECB)
#endif

#if defined(BOTAN_HAS_MODE_CBC)
#endif

#if defined(BOTAN_HAS_MODE_CFB)
#endif

#if defined(BOTAN_HAS_MODE_XTS)
#endif

namespace Botan {

#if defined(BOTAN_HAS_MODE_ECB)

template<typename T>
Transform* make_ecb_mode(const Transform::Spec& spec)
   {
   std::unique_ptr<BlockCipher> bc(BlockCipher::create(spec.arg(0)));
   std::unique_ptr<BlockCipherModePaddingMethod> pad(get_bc_pad(spec.arg(1, "NoPadding")));
   if(bc && pad)
      return new T(bc.release(), pad.release());
   return nullptr;
   }

BOTAN_REGISTER_TRANSFORM(ECB_Encryption, make_ecb_mode<ECB_Encryption>);
BOTAN_REGISTER_TRANSFORM(ECB_Decryption, make_ecb_mode<ECB_Decryption>);
#endif

#if defined(BOTAN_HAS_MODE_CBC)

template<typename CBC_T, typename CTS_T>
Transform* make_cbc_mode(const Transform::Spec& spec)
   {
   std::unique_ptr<BlockCipher> bc(BlockCipher::create(spec.arg(0)));

   if(bc)
      {
      const std::string padding = spec.arg(1, "PKCS7");

      if(padding == "CTS")
         return new CTS_T(bc.release());
      else
         return new CBC_T(bc.release(), get_bc_pad(padding));
      }

   return nullptr;
   }

BOTAN_REGISTER_TRANSFORM(CBC_Encryption, (make_cbc_mode<CBC_Encryption,CTS_Encryption>));
BOTAN_REGISTER_TRANSFORM(CBC_Decryption, (make_cbc_mode<CBC_Decryption,CTS_Decryption>));
#endif

#if defined(BOTAN_HAS_MODE_CFB)
BOTAN_REGISTER_BLOCK_CIPHER_MODE_LEN(CFB_Encryption, CFB_Decryption, 0);
#endif

#if defined(BOTAN_HAS_MODE_XTS)
BOTAN_REGISTER_BLOCK_CIPHER_MODE(XTS_Encryption, XTS_Decryption);
#endif

Cipher_Mode* get_cipher_mode(const std::string& algo_spec, Cipher_Dir direction)
   {
   const std::string provider = "";

   const char* dir_string = (direction == ENCRYPTION) ? "_Encryption" : "_Decryption";

   std::unique_ptr<Transform> t;

   t.reset(get_transform(algo_spec, provider, dir_string));

   if(Cipher_Mode* cipher = dynamic_cast<Cipher_Mode*>(t.get()))
      {
      t.release();
      return cipher;
      }

   const std::vector<std::string> algo_parts = split_on(algo_spec, '/');
   if(algo_parts.size() < 2)
      return nullptr;

   const std::string cipher_name = algo_parts[0];
   const std::vector<std::string> mode_info = parse_algorithm_name(algo_parts[1]);

   if(mode_info.empty())
      return nullptr;

   std::ostringstream alg_args;

   alg_args << '(' << cipher_name;
   for(size_t i = 1; i < mode_info.size(); ++i)
      alg_args << ',' << mode_info[i];
   for(size_t i = 2; i < algo_parts.size(); ++i)
      alg_args << ',' << algo_parts[i];
   alg_args << ')';

   const std::string mode_name = mode_info[0] + alg_args.str();
   const std::string mode_name_directional = mode_info[0] + dir_string + alg_args.str();

   t.reset(get_transform(mode_name_directional, provider));

   if(Cipher_Mode* cipher = dynamic_cast<Cipher_Mode*>(t.get()))
      {
      t.release();
      return cipher;
      }

   t.reset(get_transform(mode_name, provider));

   if(Cipher_Mode* cipher = dynamic_cast<Cipher_Mode*>(t.get()))
      {
      t.release();
      return cipher;
      }

   if(auto sc = StreamCipher::create(mode_name, provider))
      return new Stream_Cipher_Mode(sc.release());

   return nullptr;
   }

}
/*
* Lowest Level MPI Algorithms
* (C) 1999-2010 Jack Lloyd
*     2006 Luca Piccarreta
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Two Operand Addition, No Carry
*/
word bigint_add2_nc(word x[], size_t x_size, const word y[], size_t y_size)
   {
   word carry = 0;

   BOTAN_ASSERT(x_size >= y_size, "Expected sizes");

   const size_t blocks = y_size - (y_size % 8);

   for(size_t i = 0; i != blocks; i += 8)
      carry = word8_add2(x + i, y + i, carry);

   for(size_t i = blocks; i != y_size; ++i)
      x[i] = word_add(x[i], y[i], &carry);

   for(size_t i = y_size; i != x_size; ++i)
      x[i] = word_add(x[i], 0, &carry);

   return carry;
   }

/*
* Three Operand Addition, No Carry
*/
word bigint_add3_nc(word z[], const word x[], size_t x_size,
                              const word y[], size_t y_size)
   {
   if(x_size < y_size)
      { return bigint_add3_nc(z, y, y_size, x, x_size); }

   word carry = 0;

   const size_t blocks = y_size - (y_size % 8);

   for(size_t i = 0; i != blocks; i += 8)
      carry = word8_add3(z + i, x + i, y + i, carry);

   for(size_t i = blocks; i != y_size; ++i)
      z[i] = word_add(x[i], y[i], &carry);

   for(size_t i = y_size; i != x_size; ++i)
      z[i] = word_add(x[i], 0, &carry);

   return carry;
   }

/*
* Two Operand Addition
*/
void bigint_add2(word x[], size_t x_size, const word y[], size_t y_size)
   {
   if(bigint_add2_nc(x, x_size, y, y_size))
      x[x_size] += 1;
   }

/*
* Three Operand Addition
*/
void bigint_add3(word z[], const word x[], size_t x_size,
                           const word y[], size_t y_size)
   {
   z[(x_size > y_size ? x_size : y_size)] +=
      bigint_add3_nc(z, x, x_size, y, y_size);
   }

/*
* Two Operand Subtraction
*/
word bigint_sub2(word x[], size_t x_size, const word y[], size_t y_size)
   {
   word borrow = 0;

   BOTAN_ASSERT(x_size >= y_size, "Expected sizes");

   const size_t blocks = y_size - (y_size % 8);

   for(size_t i = 0; i != blocks; i += 8)
      borrow = word8_sub2(x + i, y + i, borrow);

   for(size_t i = blocks; i != y_size; ++i)
      x[i] = word_sub(x[i], y[i], &borrow);

   for(size_t i = y_size; i != x_size; ++i)
      x[i] = word_sub(x[i], 0, &borrow);

   return borrow;
   }

/*
* Two Operand Subtraction x = y - x
*/
void bigint_sub2_rev(word x[],  const word y[], size_t y_size)
   {
   word borrow = 0;

   const size_t blocks = y_size - (y_size % 8);

   for(size_t i = 0; i != blocks; i += 8)
      borrow = word8_sub2_rev(x + i, y + i, borrow);

   for(size_t i = blocks; i != y_size; ++i)
      x[i] = word_sub(y[i], x[i], &borrow);

   BOTAN_ASSERT(!borrow, "y must be greater than x");
   }

/*
* Three Operand Subtraction
*/
word bigint_sub3(word z[], const word x[], size_t x_size,
                           const word y[], size_t y_size)
   {
   word borrow = 0;

   BOTAN_ASSERT(x_size >= y_size, "Expected sizes");

   const size_t blocks = y_size - (y_size % 8);

   for(size_t i = 0; i != blocks; i += 8)
      borrow = word8_sub3(z + i, x + i, y + i, borrow);

   for(size_t i = blocks; i != y_size; ++i)
      z[i] = word_sub(x[i], y[i], &borrow);

   for(size_t i = y_size; i != x_size; ++i)
      z[i] = word_sub(x[i], 0, &borrow);

   return borrow;
   }

/*
* Two Operand Linear Multiply
*/
void bigint_linmul2(word x[], size_t x_size, word y)
   {
   const size_t blocks = x_size - (x_size % 8);

   word carry = 0;

   for(size_t i = 0; i != blocks; i += 8)
      carry = word8_linmul2(x + i, y, carry);

   for(size_t i = blocks; i != x_size; ++i)
      x[i] = word_madd2(x[i], y, &carry);

   x[x_size] = carry;
   }

/*
* Three Operand Linear Multiply
*/
void bigint_linmul3(word z[], const word x[], size_t x_size, word y)
   {
   const size_t blocks = x_size - (x_size % 8);

   word carry = 0;

   for(size_t i = 0; i != blocks; i += 8)
      carry = word8_linmul3(z + i, x + i, y, carry);

   for(size_t i = blocks; i != x_size; ++i)
      z[i] = word_madd2(x[i], y, &carry);

   z[x_size] = carry;
   }

}
/*
* Comba Multiplication and Squaring
* (C) 1999-2007,2011,2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Comba 4x4 Squaring
*/
void bigint_comba_sqr4(word z[8], const word x[4])
   {
   word w2 = 0, w1 = 0, w0 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], x[ 0]);
   z[ 0] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[ 1]);
   z[ 1] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 1], x[ 1]);
   z[ 2] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 0], x[ 3]);
   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[ 2]);
   z[ 3] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 1], x[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 2], x[ 2]);
   z[ 4] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 2], x[ 3]);
   z[ 5] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 3], x[ 3]);
   z[ 6] = w0;
   z[ 7] = w1;
   }

/*
* Comba 4x4 Multiplication
*/
void bigint_comba_mul4(word z[8], const word x[4], const word y[4])
   {
   word w2 = 0, w1 = 0, w0 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 0]);
   z[ 0] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 0]);
   z[ 1] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 0]);
   z[ 2] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[ 1], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[ 1]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 0]);
   z[ 3] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 2], y[ 2]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[ 1]);
   z[ 4] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 3]);
   word3_muladd(&w1, &w0, &w2, x[ 3], y[ 2]);
   z[ 5] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 3]);
   z[ 6] = w0;
   z[ 7] = w1;
   }

/*
* Comba 6x6 Squaring
*/
void bigint_comba_sqr6(word z[12], const word x[6])
   {
   word w2 = 0, w1 = 0, w0 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], x[ 0]);
   z[ 0] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[ 1]);
   z[ 1] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 1], x[ 1]);
   z[ 2] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 0], x[ 3]);
   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[ 2]);
   z[ 3] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[ 4]);
   word3_muladd_2(&w0, &w2, &w1, x[ 1], x[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 2], x[ 2]);
   z[ 4] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[ 5]);
   word3_muladd_2(&w1, &w0, &w2, x[ 1], x[ 4]);
   word3_muladd_2(&w1, &w0, &w2, x[ 2], x[ 3]);
   z[ 5] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[ 5]);
   word3_muladd_2(&w2, &w1, &w0, x[ 2], x[ 4]);
   word3_muladd(&w2, &w1, &w0, x[ 3], x[ 3]);
   z[ 6] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 2], x[ 5]);
   word3_muladd_2(&w0, &w2, &w1, x[ 3], x[ 4]);
   z[ 7] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 3], x[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 4], x[ 4]);
   z[ 8] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 4], x[ 5]);
   z[ 9] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 5], x[ 5]);
   z[10] = w1;
   z[11] = w2;
   }

/*
* Comba 6x6 Multiplication
*/
void bigint_comba_mul6(word z[12], const word x[6], const word y[6])
   {
   word w2 = 0, w1 = 0, w0 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 0]);
   z[ 0] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 0]);
   z[ 1] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 0]);
   z[ 2] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[ 1], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[ 1]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 0]);
   z[ 3] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 2], y[ 2]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[ 0]);
   z[ 4] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[ 4]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 3]);
   word3_muladd(&w1, &w0, &w2, x[ 3], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 4], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[ 0]);
   z[ 5] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 1], y[ 5]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[ 4]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[ 4], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[ 5], y[ 1]);
   z[ 6] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 2], y[ 5]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 5], y[ 2]);
   z[ 7] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 3], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 4], y[ 4]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[ 3]);
   z[ 8] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 4], y[ 5]);
   word3_muladd(&w2, &w1, &w0, x[ 5], y[ 4]);
   z[ 9] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 5], y[ 5]);
   z[10] = w1;
   z[11] = w2;
   }

/*
* Comba 8x8 Squaring
*/
void bigint_comba_sqr8(word z[16], const word x[8])
   {
   word w2 = 0, w1 = 0, w0 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], x[ 0]);
   z[ 0] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[ 1]);
   z[ 1] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 1], x[ 1]);
   z[ 2] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 0], x[ 3]);
   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[ 2]);
   z[ 3] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[ 4]);
   word3_muladd_2(&w0, &w2, &w1, x[ 1], x[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 2], x[ 2]);
   z[ 4] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[ 5]);
   word3_muladd_2(&w1, &w0, &w2, x[ 1], x[ 4]);
   word3_muladd_2(&w1, &w0, &w2, x[ 2], x[ 3]);
   z[ 5] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 0], x[ 6]);
   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[ 5]);
   word3_muladd_2(&w2, &w1, &w0, x[ 2], x[ 4]);
   word3_muladd(&w2, &w1, &w0, x[ 3], x[ 3]);
   z[ 6] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[ 7]);
   word3_muladd_2(&w0, &w2, &w1, x[ 1], x[ 6]);
   word3_muladd_2(&w0, &w2, &w1, x[ 2], x[ 5]);
   word3_muladd_2(&w0, &w2, &w1, x[ 3], x[ 4]);
   z[ 7] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 1], x[ 7]);
   word3_muladd_2(&w1, &w0, &w2, x[ 2], x[ 6]);
   word3_muladd_2(&w1, &w0, &w2, x[ 3], x[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 4], x[ 4]);
   z[ 8] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 2], x[ 7]);
   word3_muladd_2(&w2, &w1, &w0, x[ 3], x[ 6]);
   word3_muladd_2(&w2, &w1, &w0, x[ 4], x[ 5]);
   z[ 9] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 3], x[ 7]);
   word3_muladd_2(&w0, &w2, &w1, x[ 4], x[ 6]);
   word3_muladd(&w0, &w2, &w1, x[ 5], x[ 5]);
   z[10] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 4], x[ 7]);
   word3_muladd_2(&w1, &w0, &w2, x[ 5], x[ 6]);
   z[11] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 5], x[ 7]);
   word3_muladd(&w2, &w1, &w0, x[ 6], x[ 6]);
   z[12] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 6], x[ 7]);
   z[13] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 7], x[ 7]);
   z[14] = w2;
   z[15] = w0;
   }

/*
* Comba 8x8 Multiplication
*/
void bigint_comba_mul8(word z[16], const word x[8], const word y[8])
   {
   word w2 = 0, w1 = 0, w0 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 0]);
   z[ 0] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 0]);
   z[ 1] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 0]);
   z[ 2] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[ 1], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[ 1]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 0]);
   z[ 3] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 2], y[ 2]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[ 0]);
   z[ 4] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[ 4]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 3]);
   word3_muladd(&w1, &w0, &w2, x[ 3], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 4], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[ 0]);
   z[ 5] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 6]);
   word3_muladd(&w2, &w1, &w0, x[ 1], y[ 5]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[ 4]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[ 4], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[ 5], y[ 1]);
   word3_muladd(&w2, &w1, &w0, x[ 6], y[ 0]);
   z[ 6] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[ 7]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 6]);
   word3_muladd(&w0, &w2, &w1, x[ 2], y[ 5]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 5], y[ 2]);
   word3_muladd(&w0, &w2, &w1, x[ 6], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[ 7], y[ 0]);
   z[ 7] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 1], y[ 7]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 6]);
   word3_muladd(&w1, &w0, &w2, x[ 3], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 4], y[ 4]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[ 3]);
   word3_muladd(&w1, &w0, &w2, x[ 6], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 7], y[ 1]);
   z[ 8] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 2], y[ 7]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 6]);
   word3_muladd(&w2, &w1, &w0, x[ 4], y[ 5]);
   word3_muladd(&w2, &w1, &w0, x[ 5], y[ 4]);
   word3_muladd(&w2, &w1, &w0, x[ 6], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[ 7], y[ 2]);
   z[ 9] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 3], y[ 7]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[ 6]);
   word3_muladd(&w0, &w2, &w1, x[ 5], y[ 5]);
   word3_muladd(&w0, &w2, &w1, x[ 6], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[ 7], y[ 3]);
   z[10] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 4], y[ 7]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[ 6]);
   word3_muladd(&w1, &w0, &w2, x[ 6], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 7], y[ 4]);
   z[11] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 5], y[ 7]);
   word3_muladd(&w2, &w1, &w0, x[ 6], y[ 6]);
   word3_muladd(&w2, &w1, &w0, x[ 7], y[ 5]);
   z[12] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 6], y[ 7]);
   word3_muladd(&w0, &w2, &w1, x[ 7], y[ 6]);
   z[13] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 7], y[ 7]);
   z[14] = w2;
   z[15] = w0;
   }

/*
* Comba 9x9 Squaring
*/
void bigint_comba_sqr9(word z[18], const word x[9])
   {
   word w2 = 0, w1 = 0, w0 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], x[ 0]);
   z[ 0] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[ 1]);
   z[ 1] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 1], x[ 1]);
   z[ 2] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 0], x[ 3]);
   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[ 2]);
   z[ 3] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[ 4]);
   word3_muladd_2(&w0, &w2, &w1, x[ 1], x[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 2], x[ 2]);
   z[ 4] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[ 5]);
   word3_muladd_2(&w1, &w0, &w2, x[ 1], x[ 4]);
   word3_muladd_2(&w1, &w0, &w2, x[ 2], x[ 3]);
   z[ 5] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 0], x[ 6]);
   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[ 5]);
   word3_muladd_2(&w2, &w1, &w0, x[ 2], x[ 4]);
   word3_muladd(&w2, &w1, &w0, x[ 3], x[ 3]);
   z[ 6] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[ 7]);
   word3_muladd_2(&w0, &w2, &w1, x[ 1], x[ 6]);
   word3_muladd_2(&w0, &w2, &w1, x[ 2], x[ 5]);
   word3_muladd_2(&w0, &w2, &w1, x[ 3], x[ 4]);
   z[ 7] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[ 8]);
   word3_muladd_2(&w1, &w0, &w2, x[ 1], x[ 7]);
   word3_muladd_2(&w1, &w0, &w2, x[ 2], x[ 6]);
   word3_muladd_2(&w1, &w0, &w2, x[ 3], x[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 4], x[ 4]);
   z[ 8] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[ 8]);
   word3_muladd_2(&w2, &w1, &w0, x[ 2], x[ 7]);
   word3_muladd_2(&w2, &w1, &w0, x[ 3], x[ 6]);
   word3_muladd_2(&w2, &w1, &w0, x[ 4], x[ 5]);
   z[ 9] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 2], x[ 8]);
   word3_muladd_2(&w0, &w2, &w1, x[ 3], x[ 7]);
   word3_muladd_2(&w0, &w2, &w1, x[ 4], x[ 6]);
   word3_muladd(&w0, &w2, &w1, x[ 5], x[ 5]);
   z[10] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 3], x[ 8]);
   word3_muladd_2(&w1, &w0, &w2, x[ 4], x[ 7]);
   word3_muladd_2(&w1, &w0, &w2, x[ 5], x[ 6]);
   z[11] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 4], x[ 8]);
   word3_muladd_2(&w2, &w1, &w0, x[ 5], x[ 7]);
   word3_muladd(&w2, &w1, &w0, x[ 6], x[ 6]);
   z[12] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 5], x[ 8]);
   word3_muladd_2(&w0, &w2, &w1, x[ 6], x[ 7]);
   z[13] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 6], x[ 8]);
   word3_muladd(&w1, &w0, &w2, x[ 7], x[ 7]);
   z[14] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 7], x[ 8]);
   z[15] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 8], x[ 8]);
   z[16] = w1;
   z[17] = w2;
   }

/*
* Comba 9x9 Multiplication
*/
void bigint_comba_mul9(word z[18], const word x[9], const word y[9])
   {
   word w2 = 0, w1 = 0, w0 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 0]);
   z[ 0] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 0]);
   z[ 1] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 0]);
   z[ 2] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[ 1], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[ 1]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 0]);
   z[ 3] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 2], y[ 2]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[ 0]);
   z[ 4] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[ 4]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 3]);
   word3_muladd(&w1, &w0, &w2, x[ 3], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 4], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[ 0]);
   z[ 5] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 6]);
   word3_muladd(&w2, &w1, &w0, x[ 1], y[ 5]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[ 4]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[ 4], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[ 5], y[ 1]);
   word3_muladd(&w2, &w1, &w0, x[ 6], y[ 0]);
   z[ 6] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[ 7]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 6]);
   word3_muladd(&w0, &w2, &w1, x[ 2], y[ 5]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 5], y[ 2]);
   word3_muladd(&w0, &w2, &w1, x[ 6], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[ 7], y[ 0]);
   z[ 7] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[ 8]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[ 7]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 6]);
   word3_muladd(&w1, &w0, &w2, x[ 3], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 4], y[ 4]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[ 3]);
   word3_muladd(&w1, &w0, &w2, x[ 6], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 7], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[ 8], y[ 0]);
   z[ 8] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 1], y[ 8]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[ 7]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 6]);
   word3_muladd(&w2, &w1, &w0, x[ 4], y[ 5]);
   word3_muladd(&w2, &w1, &w0, x[ 5], y[ 4]);
   word3_muladd(&w2, &w1, &w0, x[ 6], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[ 7], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[ 8], y[ 1]);
   z[ 9] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 2], y[ 8]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[ 7]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[ 6]);
   word3_muladd(&w0, &w2, &w1, x[ 5], y[ 5]);
   word3_muladd(&w0, &w2, &w1, x[ 6], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[ 7], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 8], y[ 2]);
   z[10] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 3], y[ 8]);
   word3_muladd(&w1, &w0, &w2, x[ 4], y[ 7]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[ 6]);
   word3_muladd(&w1, &w0, &w2, x[ 6], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 7], y[ 4]);
   word3_muladd(&w1, &w0, &w2, x[ 8], y[ 3]);
   z[11] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 4], y[ 8]);
   word3_muladd(&w2, &w1, &w0, x[ 5], y[ 7]);
   word3_muladd(&w2, &w1, &w0, x[ 6], y[ 6]);
   word3_muladd(&w2, &w1, &w0, x[ 7], y[ 5]);
   word3_muladd(&w2, &w1, &w0, x[ 8], y[ 4]);
   z[12] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 5], y[ 8]);
   word3_muladd(&w0, &w2, &w1, x[ 6], y[ 7]);
   word3_muladd(&w0, &w2, &w1, x[ 7], y[ 6]);
   word3_muladd(&w0, &w2, &w1, x[ 8], y[ 5]);
   z[13] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 6], y[ 8]);
   word3_muladd(&w1, &w0, &w2, x[ 7], y[ 7]);
   word3_muladd(&w1, &w0, &w2, x[ 8], y[ 6]);
   z[14] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 7], y[ 8]);
   word3_muladd(&w2, &w1, &w0, x[ 8], y[ 7]);
   z[15] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 8], y[ 8]);
   z[16] = w1;
   z[17] = w2;
   }

/*
* Comba 16x16 Squaring
*/
void bigint_comba_sqr16(word z[32], const word x[16])
   {
   word w2 = 0, w1 = 0, w0 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], x[ 0]);
   z[ 0] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[ 1]);
   z[ 1] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 1], x[ 1]);
   z[ 2] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 0], x[ 3]);
   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[ 2]);
   z[ 3] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[ 4]);
   word3_muladd_2(&w0, &w2, &w1, x[ 1], x[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 2], x[ 2]);
   z[ 4] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[ 5]);
   word3_muladd_2(&w1, &w0, &w2, x[ 1], x[ 4]);
   word3_muladd_2(&w1, &w0, &w2, x[ 2], x[ 3]);
   z[ 5] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 0], x[ 6]);
   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[ 5]);
   word3_muladd_2(&w2, &w1, &w0, x[ 2], x[ 4]);
   word3_muladd(&w2, &w1, &w0, x[ 3], x[ 3]);
   z[ 6] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[ 7]);
   word3_muladd_2(&w0, &w2, &w1, x[ 1], x[ 6]);
   word3_muladd_2(&w0, &w2, &w1, x[ 2], x[ 5]);
   word3_muladd_2(&w0, &w2, &w1, x[ 3], x[ 4]);
   z[ 7] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[ 8]);
   word3_muladd_2(&w1, &w0, &w2, x[ 1], x[ 7]);
   word3_muladd_2(&w1, &w0, &w2, x[ 2], x[ 6]);
   word3_muladd_2(&w1, &w0, &w2, x[ 3], x[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 4], x[ 4]);
   z[ 8] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 0], x[ 9]);
   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[ 8]);
   word3_muladd_2(&w2, &w1, &w0, x[ 2], x[ 7]);
   word3_muladd_2(&w2, &w1, &w0, x[ 3], x[ 6]);
   word3_muladd_2(&w2, &w1, &w0, x[ 4], x[ 5]);
   z[ 9] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[10]);
   word3_muladd_2(&w0, &w2, &w1, x[ 1], x[ 9]);
   word3_muladd_2(&w0, &w2, &w1, x[ 2], x[ 8]);
   word3_muladd_2(&w0, &w2, &w1, x[ 3], x[ 7]);
   word3_muladd_2(&w0, &w2, &w1, x[ 4], x[ 6]);
   word3_muladd(&w0, &w2, &w1, x[ 5], x[ 5]);
   z[10] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[11]);
   word3_muladd_2(&w1, &w0, &w2, x[ 1], x[10]);
   word3_muladd_2(&w1, &w0, &w2, x[ 2], x[ 9]);
   word3_muladd_2(&w1, &w0, &w2, x[ 3], x[ 8]);
   word3_muladd_2(&w1, &w0, &w2, x[ 4], x[ 7]);
   word3_muladd_2(&w1, &w0, &w2, x[ 5], x[ 6]);
   z[11] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 0], x[12]);
   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[11]);
   word3_muladd_2(&w2, &w1, &w0, x[ 2], x[10]);
   word3_muladd_2(&w2, &w1, &w0, x[ 3], x[ 9]);
   word3_muladd_2(&w2, &w1, &w0, x[ 4], x[ 8]);
   word3_muladd_2(&w2, &w1, &w0, x[ 5], x[ 7]);
   word3_muladd(&w2, &w1, &w0, x[ 6], x[ 6]);
   z[12] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 0], x[13]);
   word3_muladd_2(&w0, &w2, &w1, x[ 1], x[12]);
   word3_muladd_2(&w0, &w2, &w1, x[ 2], x[11]);
   word3_muladd_2(&w0, &w2, &w1, x[ 3], x[10]);
   word3_muladd_2(&w0, &w2, &w1, x[ 4], x[ 9]);
   word3_muladd_2(&w0, &w2, &w1, x[ 5], x[ 8]);
   word3_muladd_2(&w0, &w2, &w1, x[ 6], x[ 7]);
   z[13] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 0], x[14]);
   word3_muladd_2(&w1, &w0, &w2, x[ 1], x[13]);
   word3_muladd_2(&w1, &w0, &w2, x[ 2], x[12]);
   word3_muladd_2(&w1, &w0, &w2, x[ 3], x[11]);
   word3_muladd_2(&w1, &w0, &w2, x[ 4], x[10]);
   word3_muladd_2(&w1, &w0, &w2, x[ 5], x[ 9]);
   word3_muladd_2(&w1, &w0, &w2, x[ 6], x[ 8]);
   word3_muladd(&w1, &w0, &w2, x[ 7], x[ 7]);
   z[14] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 0], x[15]);
   word3_muladd_2(&w2, &w1, &w0, x[ 1], x[14]);
   word3_muladd_2(&w2, &w1, &w0, x[ 2], x[13]);
   word3_muladd_2(&w2, &w1, &w0, x[ 3], x[12]);
   word3_muladd_2(&w2, &w1, &w0, x[ 4], x[11]);
   word3_muladd_2(&w2, &w1, &w0, x[ 5], x[10]);
   word3_muladd_2(&w2, &w1, &w0, x[ 6], x[ 9]);
   word3_muladd_2(&w2, &w1, &w0, x[ 7], x[ 8]);
   z[15] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 1], x[15]);
   word3_muladd_2(&w0, &w2, &w1, x[ 2], x[14]);
   word3_muladd_2(&w0, &w2, &w1, x[ 3], x[13]);
   word3_muladd_2(&w0, &w2, &w1, x[ 4], x[12]);
   word3_muladd_2(&w0, &w2, &w1, x[ 5], x[11]);
   word3_muladd_2(&w0, &w2, &w1, x[ 6], x[10]);
   word3_muladd_2(&w0, &w2, &w1, x[ 7], x[ 9]);
   word3_muladd(&w0, &w2, &w1, x[ 8], x[ 8]);
   z[16] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 2], x[15]);
   word3_muladd_2(&w1, &w0, &w2, x[ 3], x[14]);
   word3_muladd_2(&w1, &w0, &w2, x[ 4], x[13]);
   word3_muladd_2(&w1, &w0, &w2, x[ 5], x[12]);
   word3_muladd_2(&w1, &w0, &w2, x[ 6], x[11]);
   word3_muladd_2(&w1, &w0, &w2, x[ 7], x[10]);
   word3_muladd_2(&w1, &w0, &w2, x[ 8], x[ 9]);
   z[17] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 3], x[15]);
   word3_muladd_2(&w2, &w1, &w0, x[ 4], x[14]);
   word3_muladd_2(&w2, &w1, &w0, x[ 5], x[13]);
   word3_muladd_2(&w2, &w1, &w0, x[ 6], x[12]);
   word3_muladd_2(&w2, &w1, &w0, x[ 7], x[11]);
   word3_muladd_2(&w2, &w1, &w0, x[ 8], x[10]);
   word3_muladd(&w2, &w1, &w0, x[ 9], x[ 9]);
   z[18] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 4], x[15]);
   word3_muladd_2(&w0, &w2, &w1, x[ 5], x[14]);
   word3_muladd_2(&w0, &w2, &w1, x[ 6], x[13]);
   word3_muladd_2(&w0, &w2, &w1, x[ 7], x[12]);
   word3_muladd_2(&w0, &w2, &w1, x[ 8], x[11]);
   word3_muladd_2(&w0, &w2, &w1, x[ 9], x[10]);
   z[19] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 5], x[15]);
   word3_muladd_2(&w1, &w0, &w2, x[ 6], x[14]);
   word3_muladd_2(&w1, &w0, &w2, x[ 7], x[13]);
   word3_muladd_2(&w1, &w0, &w2, x[ 8], x[12]);
   word3_muladd_2(&w1, &w0, &w2, x[ 9], x[11]);
   word3_muladd(&w1, &w0, &w2, x[10], x[10]);
   z[20] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 6], x[15]);
   word3_muladd_2(&w2, &w1, &w0, x[ 7], x[14]);
   word3_muladd_2(&w2, &w1, &w0, x[ 8], x[13]);
   word3_muladd_2(&w2, &w1, &w0, x[ 9], x[12]);
   word3_muladd_2(&w2, &w1, &w0, x[10], x[11]);
   z[21] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[ 7], x[15]);
   word3_muladd_2(&w0, &w2, &w1, x[ 8], x[14]);
   word3_muladd_2(&w0, &w2, &w1, x[ 9], x[13]);
   word3_muladd_2(&w0, &w2, &w1, x[10], x[12]);
   word3_muladd(&w0, &w2, &w1, x[11], x[11]);
   z[22] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[ 8], x[15]);
   word3_muladd_2(&w1, &w0, &w2, x[ 9], x[14]);
   word3_muladd_2(&w1, &w0, &w2, x[10], x[13]);
   word3_muladd_2(&w1, &w0, &w2, x[11], x[12]);
   z[23] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[ 9], x[15]);
   word3_muladd_2(&w2, &w1, &w0, x[10], x[14]);
   word3_muladd_2(&w2, &w1, &w0, x[11], x[13]);
   word3_muladd(&w2, &w1, &w0, x[12], x[12]);
   z[24] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[10], x[15]);
   word3_muladd_2(&w0, &w2, &w1, x[11], x[14]);
   word3_muladd_2(&w0, &w2, &w1, x[12], x[13]);
   z[25] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[11], x[15]);
   word3_muladd_2(&w1, &w0, &w2, x[12], x[14]);
   word3_muladd(&w1, &w0, &w2, x[13], x[13]);
   z[26] = w2; w2 = 0;

   word3_muladd_2(&w2, &w1, &w0, x[12], x[15]);
   word3_muladd_2(&w2, &w1, &w0, x[13], x[14]);
   z[27] = w0; w0 = 0;

   word3_muladd_2(&w0, &w2, &w1, x[13], x[15]);
   word3_muladd(&w0, &w2, &w1, x[14], x[14]);
   z[28] = w1; w1 = 0;

   word3_muladd_2(&w1, &w0, &w2, x[14], x[15]);
   z[29] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[15], x[15]);
   z[30] = w0;
   z[31] = w1;
   }

/*
* Comba 16x16 Multiplication
*/
void bigint_comba_mul16(word z[32], const word x[16], const word y[16])
   {
   word w2 = 0, w1 = 0, w0 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 0]);
   z[ 0] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 0]);
   z[ 1] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 0]);
   z[ 2] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[ 1], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[ 1]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 0]);
   z[ 3] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 2], y[ 2]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[ 0]);
   z[ 4] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[ 4]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 3]);
   word3_muladd(&w1, &w0, &w2, x[ 3], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 4], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[ 0]);
   z[ 5] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 6]);
   word3_muladd(&w2, &w1, &w0, x[ 1], y[ 5]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[ 4]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[ 4], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[ 5], y[ 1]);
   word3_muladd(&w2, &w1, &w0, x[ 6], y[ 0]);
   z[ 6] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[ 7]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 6]);
   word3_muladd(&w0, &w2, &w1, x[ 2], y[ 5]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 5], y[ 2]);
   word3_muladd(&w0, &w2, &w1, x[ 6], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[ 7], y[ 0]);
   z[ 7] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[ 8]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[ 7]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 6]);
   word3_muladd(&w1, &w0, &w2, x[ 3], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 4], y[ 4]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[ 3]);
   word3_muladd(&w1, &w0, &w2, x[ 6], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[ 7], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[ 8], y[ 0]);
   z[ 8] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[ 9]);
   word3_muladd(&w2, &w1, &w0, x[ 1], y[ 8]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[ 7]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 6]);
   word3_muladd(&w2, &w1, &w0, x[ 4], y[ 5]);
   word3_muladd(&w2, &w1, &w0, x[ 5], y[ 4]);
   word3_muladd(&w2, &w1, &w0, x[ 6], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[ 7], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[ 8], y[ 1]);
   word3_muladd(&w2, &w1, &w0, x[ 9], y[ 0]);
   z[ 9] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[10]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[ 9]);
   word3_muladd(&w0, &w2, &w1, x[ 2], y[ 8]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[ 7]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[ 6]);
   word3_muladd(&w0, &w2, &w1, x[ 5], y[ 5]);
   word3_muladd(&w0, &w2, &w1, x[ 6], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[ 7], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[ 8], y[ 2]);
   word3_muladd(&w0, &w2, &w1, x[ 9], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[10], y[ 0]);
   z[10] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[11]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[10]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[ 9]);
   word3_muladd(&w1, &w0, &w2, x[ 3], y[ 8]);
   word3_muladd(&w1, &w0, &w2, x[ 4], y[ 7]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[ 6]);
   word3_muladd(&w1, &w0, &w2, x[ 6], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[ 7], y[ 4]);
   word3_muladd(&w1, &w0, &w2, x[ 8], y[ 3]);
   word3_muladd(&w1, &w0, &w2, x[ 9], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[10], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[11], y[ 0]);
   z[11] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[12]);
   word3_muladd(&w2, &w1, &w0, x[ 1], y[11]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[10]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[ 9]);
   word3_muladd(&w2, &w1, &w0, x[ 4], y[ 8]);
   word3_muladd(&w2, &w1, &w0, x[ 5], y[ 7]);
   word3_muladd(&w2, &w1, &w0, x[ 6], y[ 6]);
   word3_muladd(&w2, &w1, &w0, x[ 7], y[ 5]);
   word3_muladd(&w2, &w1, &w0, x[ 8], y[ 4]);
   word3_muladd(&w2, &w1, &w0, x[ 9], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[10], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[11], y[ 1]);
   word3_muladd(&w2, &w1, &w0, x[12], y[ 0]);
   z[12] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 0], y[13]);
   word3_muladd(&w0, &w2, &w1, x[ 1], y[12]);
   word3_muladd(&w0, &w2, &w1, x[ 2], y[11]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[10]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[ 9]);
   word3_muladd(&w0, &w2, &w1, x[ 5], y[ 8]);
   word3_muladd(&w0, &w2, &w1, x[ 6], y[ 7]);
   word3_muladd(&w0, &w2, &w1, x[ 7], y[ 6]);
   word3_muladd(&w0, &w2, &w1, x[ 8], y[ 5]);
   word3_muladd(&w0, &w2, &w1, x[ 9], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[10], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[11], y[ 2]);
   word3_muladd(&w0, &w2, &w1, x[12], y[ 1]);
   word3_muladd(&w0, &w2, &w1, x[13], y[ 0]);
   z[13] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 0], y[14]);
   word3_muladd(&w1, &w0, &w2, x[ 1], y[13]);
   word3_muladd(&w1, &w0, &w2, x[ 2], y[12]);
   word3_muladd(&w1, &w0, &w2, x[ 3], y[11]);
   word3_muladd(&w1, &w0, &w2, x[ 4], y[10]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[ 9]);
   word3_muladd(&w1, &w0, &w2, x[ 6], y[ 8]);
   word3_muladd(&w1, &w0, &w2, x[ 7], y[ 7]);
   word3_muladd(&w1, &w0, &w2, x[ 8], y[ 6]);
   word3_muladd(&w1, &w0, &w2, x[ 9], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[10], y[ 4]);
   word3_muladd(&w1, &w0, &w2, x[11], y[ 3]);
   word3_muladd(&w1, &w0, &w2, x[12], y[ 2]);
   word3_muladd(&w1, &w0, &w2, x[13], y[ 1]);
   word3_muladd(&w1, &w0, &w2, x[14], y[ 0]);
   z[14] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 0], y[15]);
   word3_muladd(&w2, &w1, &w0, x[ 1], y[14]);
   word3_muladd(&w2, &w1, &w0, x[ 2], y[13]);
   word3_muladd(&w2, &w1, &w0, x[ 3], y[12]);
   word3_muladd(&w2, &w1, &w0, x[ 4], y[11]);
   word3_muladd(&w2, &w1, &w0, x[ 5], y[10]);
   word3_muladd(&w2, &w1, &w0, x[ 6], y[ 9]);
   word3_muladd(&w2, &w1, &w0, x[ 7], y[ 8]);
   word3_muladd(&w2, &w1, &w0, x[ 8], y[ 7]);
   word3_muladd(&w2, &w1, &w0, x[ 9], y[ 6]);
   word3_muladd(&w2, &w1, &w0, x[10], y[ 5]);
   word3_muladd(&w2, &w1, &w0, x[11], y[ 4]);
   word3_muladd(&w2, &w1, &w0, x[12], y[ 3]);
   word3_muladd(&w2, &w1, &w0, x[13], y[ 2]);
   word3_muladd(&w2, &w1, &w0, x[14], y[ 1]);
   word3_muladd(&w2, &w1, &w0, x[15], y[ 0]);
   z[15] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 1], y[15]);
   word3_muladd(&w0, &w2, &w1, x[ 2], y[14]);
   word3_muladd(&w0, &w2, &w1, x[ 3], y[13]);
   word3_muladd(&w0, &w2, &w1, x[ 4], y[12]);
   word3_muladd(&w0, &w2, &w1, x[ 5], y[11]);
   word3_muladd(&w0, &w2, &w1, x[ 6], y[10]);
   word3_muladd(&w0, &w2, &w1, x[ 7], y[ 9]);
   word3_muladd(&w0, &w2, &w1, x[ 8], y[ 8]);
   word3_muladd(&w0, &w2, &w1, x[ 9], y[ 7]);
   word3_muladd(&w0, &w2, &w1, x[10], y[ 6]);
   word3_muladd(&w0, &w2, &w1, x[11], y[ 5]);
   word3_muladd(&w0, &w2, &w1, x[12], y[ 4]);
   word3_muladd(&w0, &w2, &w1, x[13], y[ 3]);
   word3_muladd(&w0, &w2, &w1, x[14], y[ 2]);
   word3_muladd(&w0, &w2, &w1, x[15], y[ 1]);
   z[16] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 2], y[15]);
   word3_muladd(&w1, &w0, &w2, x[ 3], y[14]);
   word3_muladd(&w1, &w0, &w2, x[ 4], y[13]);
   word3_muladd(&w1, &w0, &w2, x[ 5], y[12]);
   word3_muladd(&w1, &w0, &w2, x[ 6], y[11]);
   word3_muladd(&w1, &w0, &w2, x[ 7], y[10]);
   word3_muladd(&w1, &w0, &w2, x[ 8], y[ 9]);
   word3_muladd(&w1, &w0, &w2, x[ 9], y[ 8]);
   word3_muladd(&w1, &w0, &w2, x[10], y[ 7]);
   word3_muladd(&w1, &w0, &w2, x[11], y[ 6]);
   word3_muladd(&w1, &w0, &w2, x[12], y[ 5]);
   word3_muladd(&w1, &w0, &w2, x[13], y[ 4]);
   word3_muladd(&w1, &w0, &w2, x[14], y[ 3]);
   word3_muladd(&w1, &w0, &w2, x[15], y[ 2]);
   z[17] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 3], y[15]);
   word3_muladd(&w2, &w1, &w0, x[ 4], y[14]);
   word3_muladd(&w2, &w1, &w0, x[ 5], y[13]);
   word3_muladd(&w2, &w1, &w0, x[ 6], y[12]);
   word3_muladd(&w2, &w1, &w0, x[ 7], y[11]);
   word3_muladd(&w2, &w1, &w0, x[ 8], y[10]);
   word3_muladd(&w2, &w1, &w0, x[ 9], y[ 9]);
   word3_muladd(&w2, &w1, &w0, x[10], y[ 8]);
   word3_muladd(&w2, &w1, &w0, x[11], y[ 7]);
   word3_muladd(&w2, &w1, &w0, x[12], y[ 6]);
   word3_muladd(&w2, &w1, &w0, x[13], y[ 5]);
   word3_muladd(&w2, &w1, &w0, x[14], y[ 4]);
   word3_muladd(&w2, &w1, &w0, x[15], y[ 3]);
   z[18] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 4], y[15]);
   word3_muladd(&w0, &w2, &w1, x[ 5], y[14]);
   word3_muladd(&w0, &w2, &w1, x[ 6], y[13]);
   word3_muladd(&w0, &w2, &w1, x[ 7], y[12]);
   word3_muladd(&w0, &w2, &w1, x[ 8], y[11]);
   word3_muladd(&w0, &w2, &w1, x[ 9], y[10]);
   word3_muladd(&w0, &w2, &w1, x[10], y[ 9]);
   word3_muladd(&w0, &w2, &w1, x[11], y[ 8]);
   word3_muladd(&w0, &w2, &w1, x[12], y[ 7]);
   word3_muladd(&w0, &w2, &w1, x[13], y[ 6]);
   word3_muladd(&w0, &w2, &w1, x[14], y[ 5]);
   word3_muladd(&w0, &w2, &w1, x[15], y[ 4]);
   z[19] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 5], y[15]);
   word3_muladd(&w1, &w0, &w2, x[ 6], y[14]);
   word3_muladd(&w1, &w0, &w2, x[ 7], y[13]);
   word3_muladd(&w1, &w0, &w2, x[ 8], y[12]);
   word3_muladd(&w1, &w0, &w2, x[ 9], y[11]);
   word3_muladd(&w1, &w0, &w2, x[10], y[10]);
   word3_muladd(&w1, &w0, &w2, x[11], y[ 9]);
   word3_muladd(&w1, &w0, &w2, x[12], y[ 8]);
   word3_muladd(&w1, &w0, &w2, x[13], y[ 7]);
   word3_muladd(&w1, &w0, &w2, x[14], y[ 6]);
   word3_muladd(&w1, &w0, &w2, x[15], y[ 5]);
   z[20] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 6], y[15]);
   word3_muladd(&w2, &w1, &w0, x[ 7], y[14]);
   word3_muladd(&w2, &w1, &w0, x[ 8], y[13]);
   word3_muladd(&w2, &w1, &w0, x[ 9], y[12]);
   word3_muladd(&w2, &w1, &w0, x[10], y[11]);
   word3_muladd(&w2, &w1, &w0, x[11], y[10]);
   word3_muladd(&w2, &w1, &w0, x[12], y[ 9]);
   word3_muladd(&w2, &w1, &w0, x[13], y[ 8]);
   word3_muladd(&w2, &w1, &w0, x[14], y[ 7]);
   word3_muladd(&w2, &w1, &w0, x[15], y[ 6]);
   z[21] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[ 7], y[15]);
   word3_muladd(&w0, &w2, &w1, x[ 8], y[14]);
   word3_muladd(&w0, &w2, &w1, x[ 9], y[13]);
   word3_muladd(&w0, &w2, &w1, x[10], y[12]);
   word3_muladd(&w0, &w2, &w1, x[11], y[11]);
   word3_muladd(&w0, &w2, &w1, x[12], y[10]);
   word3_muladd(&w0, &w2, &w1, x[13], y[ 9]);
   word3_muladd(&w0, &w2, &w1, x[14], y[ 8]);
   word3_muladd(&w0, &w2, &w1, x[15], y[ 7]);
   z[22] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[ 8], y[15]);
   word3_muladd(&w1, &w0, &w2, x[ 9], y[14]);
   word3_muladd(&w1, &w0, &w2, x[10], y[13]);
   word3_muladd(&w1, &w0, &w2, x[11], y[12]);
   word3_muladd(&w1, &w0, &w2, x[12], y[11]);
   word3_muladd(&w1, &w0, &w2, x[13], y[10]);
   word3_muladd(&w1, &w0, &w2, x[14], y[ 9]);
   word3_muladd(&w1, &w0, &w2, x[15], y[ 8]);
   z[23] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[ 9], y[15]);
   word3_muladd(&w2, &w1, &w0, x[10], y[14]);
   word3_muladd(&w2, &w1, &w0, x[11], y[13]);
   word3_muladd(&w2, &w1, &w0, x[12], y[12]);
   word3_muladd(&w2, &w1, &w0, x[13], y[11]);
   word3_muladd(&w2, &w1, &w0, x[14], y[10]);
   word3_muladd(&w2, &w1, &w0, x[15], y[ 9]);
   z[24] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[10], y[15]);
   word3_muladd(&w0, &w2, &w1, x[11], y[14]);
   word3_muladd(&w0, &w2, &w1, x[12], y[13]);
   word3_muladd(&w0, &w2, &w1, x[13], y[12]);
   word3_muladd(&w0, &w2, &w1, x[14], y[11]);
   word3_muladd(&w0, &w2, &w1, x[15], y[10]);
   z[25] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[11], y[15]);
   word3_muladd(&w1, &w0, &w2, x[12], y[14]);
   word3_muladd(&w1, &w0, &w2, x[13], y[13]);
   word3_muladd(&w1, &w0, &w2, x[14], y[12]);
   word3_muladd(&w1, &w0, &w2, x[15], y[11]);
   z[26] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[12], y[15]);
   word3_muladd(&w2, &w1, &w0, x[13], y[14]);
   word3_muladd(&w2, &w1, &w0, x[14], y[13]);
   word3_muladd(&w2, &w1, &w0, x[15], y[12]);
   z[27] = w0; w0 = 0;

   word3_muladd(&w0, &w2, &w1, x[13], y[15]);
   word3_muladd(&w0, &w2, &w1, x[14], y[14]);
   word3_muladd(&w0, &w2, &w1, x[15], y[13]);
   z[28] = w1; w1 = 0;

   word3_muladd(&w1, &w0, &w2, x[14], y[15]);
   word3_muladd(&w1, &w0, &w2, x[15], y[14]);
   z[29] = w2; w2 = 0;

   word3_muladd(&w2, &w1, &w0, x[15], y[15]);
   z[30] = w0;
   z[31] = w1;
   }

}
/*
* Karatsuba Multiplication/Squaring
* (C) 1999-2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

const size_t KARATSUBA_MULTIPLY_THRESHOLD = 32;
const size_t KARATSUBA_SQUARE_THRESHOLD = 32;

/*
* Karatsuba Multiplication Operation
*/
void karatsuba_mul(word z[], const word x[], const word y[], size_t N,
                   word workspace[])
   {
   if(N < KARATSUBA_MULTIPLY_THRESHOLD || N % 2)
      {
      if(N == 6)
         return bigint_comba_mul6(z, x, y);
      else if(N == 8)
         return bigint_comba_mul8(z, x, y);
      else if(N == 16)
         return bigint_comba_mul16(z, x, y);
      else
         return bigint_simple_mul(z, x, N, y, N);
      }

   const size_t N2 = N / 2;

   const word* x0 = x;
   const word* x1 = x + N2;
   const word* y0 = y;
   const word* y1 = y + N2;
   word* z0 = z;
   word* z1 = z + N;

   const s32bit cmp0 = bigint_cmp(x0, N2, x1, N2);
   const s32bit cmp1 = bigint_cmp(y1, N2, y0, N2);

   clear_mem(workspace, 2*N);

   /*
   * If either of cmp0 or cmp1 is zero then z0 or z1 resp is zero here,
   * resulting in a no-op - z0*z1 will be equal to zero so we don't need to do
   * anything, clear_mem above already set the correct result.
   *
   * However we ignore the result of the comparisons and always perform the
   * subtractions and recursively multiply to avoid the timing channel.
   */

   //if(cmp0 && cmp1)
      {
      if(cmp0 > 0)
         bigint_sub3(z0, x0, N2, x1, N2);
      else
         bigint_sub3(z0, x1, N2, x0, N2);

      if(cmp1 > 0)
         bigint_sub3(z1, y1, N2, y0, N2);
      else
         bigint_sub3(z1, y0, N2, y1, N2);

      karatsuba_mul(workspace, z0, z1, N2, workspace+N);
      }

   karatsuba_mul(z0, x0, y0, N2, workspace+N);
   karatsuba_mul(z1, x1, y1, N2, workspace+N);

   const word ws_carry = bigint_add3_nc(workspace + N, z0, N, z1, N);
   word z_carry = bigint_add2_nc(z + N2, N, workspace + N, N);

   z_carry += bigint_add2_nc(z + N + N2, N2, &ws_carry, 1);
   bigint_add2_nc(z + N + N2, N2, &z_carry, 1);

   if((cmp0 == cmp1) || (cmp0 == 0) || (cmp1 == 0))
      bigint_add2(z + N2, 2*N-N2, workspace, N);
   else
      bigint_sub2(z + N2, 2*N-N2, workspace, N);
   }

/*
* Karatsuba Squaring Operation
*/
void karatsuba_sqr(word z[], const word x[], size_t N, word workspace[])
   {
   if(N < KARATSUBA_SQUARE_THRESHOLD || N % 2)
      {
      if(N == 6)
         return bigint_comba_sqr6(z, x);
      else if(N == 8)
         return bigint_comba_sqr8(z, x);
      else if(N == 16)
         return bigint_comba_sqr16(z, x);
      else
         return bigint_simple_sqr(z, x, N);
      }

   const size_t N2 = N / 2;

   const word* x0 = x;
   const word* x1 = x + N2;
   word* z0 = z;
   word* z1 = z + N;

   const s32bit cmp = bigint_cmp(x0, N2, x1, N2);

   clear_mem(workspace, 2*N);

   // See comment in karatsuba_mul

   //if(cmp)
      {
      if(cmp > 0)
         bigint_sub3(z0, x0, N2, x1, N2);
      else
         bigint_sub3(z0, x1, N2, x0, N2);

      karatsuba_sqr(workspace, z0, N2, workspace+N);
      }

   karatsuba_sqr(z0, x0, N2, workspace+N);
   karatsuba_sqr(z1, x1, N2, workspace+N);

   const word ws_carry = bigint_add3_nc(workspace + N, z0, N, z1, N);
   word z_carry = bigint_add2_nc(z + N2, N, workspace + N, N);

   z_carry += bigint_add2_nc(z + N + N2, N2, &ws_carry, 1);
   bigint_add2_nc(z + N + N2, N2, &z_carry, 1);

   /*
   * This is only actually required if cmp is != 0, however
   * if cmp==0 then workspace[0:N] == 0 and avoiding the jump
   * hides a timing channel.
   */
   bigint_sub2(z + N2, 2*N-N2, workspace, N);
   }

/*
* Pick a good size for the Karatsuba multiply
*/
size_t karatsuba_size(size_t z_size,
                      size_t x_size, size_t x_sw,
                      size_t y_size, size_t y_sw)
   {
   if(x_sw > x_size || x_sw > y_size || y_sw > x_size || y_sw > y_size)
      return 0;

   if(((x_size == x_sw) && (x_size % 2)) ||
      ((y_size == y_sw) && (y_size % 2)))
      return 0;

   const size_t start = (x_sw > y_sw) ? x_sw : y_sw;
   const size_t end = (x_size < y_size) ? x_size : y_size;

   if(start == end)
      {
      if(start % 2)
         return 0;
      return start;
      }

   for(size_t j = start; j <= end; ++j)
      {
      if(j % 2)
         continue;

      if(2*j > z_size)
         return 0;

      if(x_sw <= j && j <= x_size && y_sw <= j && j <= y_size)
         {
         if(j % 4 == 2 &&
            (j+2) <= x_size && (j+2) <= y_size && 2*(j+2) <= z_size)
            return j+2;
         return j;
         }
      }

   return 0;
   }

/*
* Pick a good size for the Karatsuba squaring
*/
size_t karatsuba_size(size_t z_size, size_t x_size, size_t x_sw)
   {
   if(x_sw == x_size)
      {
      if(x_sw % 2)
         return 0;
      return x_sw;
      }

   for(size_t j = x_sw; j <= x_size; ++j)
      {
      if(j % 2)
         continue;

      if(2*j > z_size)
         return 0;

      if(j % 4 == 2 && (j+2) <= x_size && 2*(j+2) <= z_size)
         return j+2;
      return j;
      }

   return 0;
   }

}

/*
* Multiplication Algorithm Dispatcher
*/
void bigint_mul(word z[], size_t z_size, word workspace[],
                const word x[], size_t x_size, size_t x_sw,
                const word y[], size_t y_size, size_t y_sw)
   {
   if(x_sw == 1)
      {
      bigint_linmul3(z, y, y_sw, x[0]);
      }
   else if(y_sw == 1)
      {
      bigint_linmul3(z, x, x_sw, y[0]);
      }
   else if(x_sw <= 4 && x_size >= 4 &&
           y_sw <= 4 && y_size >= 4 && z_size >= 8)
      {
      bigint_comba_mul4(z, x, y);
      }
   else if(x_sw <= 6 && x_size >= 6 &&
           y_sw <= 6 && y_size >= 6 && z_size >= 12)
      {
      bigint_comba_mul6(z, x, y);
      }
   else if(x_sw <= 8 && x_size >= 8 &&
           y_sw <= 8 && y_size >= 8 && z_size >= 16)
      {
      bigint_comba_mul8(z, x, y);
      }
   else if(x_sw <= 9 && x_size >= 9 &&
           y_sw <= 9 && y_size >= 9 && z_size >= 18)
      {
      bigint_comba_mul9(z, x, y);
      }
   else if(x_sw <= 16 && x_size >= 16 &&
           y_sw <= 16 && y_size >= 16 && z_size >= 32)
      {
      bigint_comba_mul16(z, x, y);
      }
   else if(x_sw < KARATSUBA_MULTIPLY_THRESHOLD ||
           y_sw < KARATSUBA_MULTIPLY_THRESHOLD ||
           !workspace)
      {
      bigint_simple_mul(z, x, x_sw, y, y_sw);
      }
   else
      {
      const size_t N = karatsuba_size(z_size, x_size, x_sw, y_size, y_sw);

      if(N)
         karatsuba_mul(z, x, y, N, workspace);
      else
         bigint_simple_mul(z, x, x_sw, y, y_sw);
      }
   }

/*
* Squaring Algorithm Dispatcher
*/
void bigint_sqr(word z[], size_t z_size, word workspace[],
                const word x[], size_t x_size, size_t x_sw)
   {
   if(x_sw == 1)
      {
      bigint_linmul3(z, x, x_sw, x[0]);
      }
   else if(x_sw <= 4 && x_size >= 4 && z_size >= 8)
      {
      bigint_comba_sqr4(z, x);
      }
   else if(x_sw <= 6 && x_size >= 6 && z_size >= 12)
      {
      bigint_comba_sqr6(z, x);
      }
   else if(x_sw <= 8 && x_size >= 8 && z_size >= 16)
      {
      bigint_comba_sqr8(z, x);
      }
   else if(x_sw == 9 && x_size >= 9 && z_size >= 18)
      {
      bigint_comba_sqr9(z, x);
      }
   else if(x_sw <= 16 && x_size >= 16 && z_size >= 32)
      {
      bigint_comba_sqr16(z, x);
      }
   else if(x_size < KARATSUBA_SQUARE_THRESHOLD || !workspace)
      {
      bigint_simple_sqr(z, x, x_sw);
      }
   else
      {
      const size_t N = karatsuba_size(z_size, x_size, x_sw);

      if(N)
         karatsuba_sqr(z, x, N, workspace);
      else
         bigint_simple_sqr(z, x, x_sw);
      }
   }

}
/*
* MP Misc Functions
* (C) 1999-2008 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Compare two MP integers
*/
s32bit bigint_cmp(const word x[], size_t x_size,
                  const word y[], size_t y_size)
   {
   if(x_size < y_size) { return (-bigint_cmp(y, y_size, x, x_size)); }

   while(x_size > y_size)
      {
      if(x[x_size-1])
         return 1;
      x_size--;
      }

   for(size_t i = x_size; i > 0; --i)
      {
      if(x[i-1] > y[i-1])
         return 1;
      if(x[i-1] < y[i-1])
         return -1;
      }

   return 0;
   }

/*
* Do a 2-word/1-word Division
*/
word bigint_divop(word n1, word n0, word d)
   {
   if(d == 0)
      throw std::runtime_error("bigint_divop divide by zero");

   word high = n1 % d, quotient = 0;

   for(size_t i = 0; i != MP_WORD_BITS; ++i)
      {
      word high_top_bit = (high & MP_WORD_TOP_BIT);

      high <<= 1;
      high |= (n0 >> (MP_WORD_BITS-1-i)) & 1;
      quotient <<= 1;

      if(high_top_bit || high >= d)
         {
         high -= d;
         quotient |= 1;
         }
      }

   return quotient;
   }

/*
* Do a 2-word/1-word Modulo
*/
word bigint_modop(word n1, word n0, word d)
   {
   word z = bigint_divop(n1, n0, d);
   word dummy = 0;
   z = word_madd2(z, d, &dummy);
   return (n0-z);
   }

}
/*
* Montgomery Reduction
* (C) 1999-2011 Jack Lloyd
*     2006 Luca Piccarreta
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Montgomery Reduction Algorithm
*/
void bigint_monty_redc(word z[],
                       const word p[], size_t p_size,
                       word p_dash, word ws[])
   {
   const size_t z_size = 2*(p_size+1);

   CT::poison(z, z_size);
   CT::poison(p, p_size);
   CT::poison(ws, 2*(p_size+1));

   const size_t blocks_of_8 = p_size - (p_size % 8);

   for(size_t i = 0; i != p_size; ++i)
      {
      word* z_i = z + i;

      const word y = z_i[0] * p_dash;

      /*
      bigint_linmul3(ws, p, p_size, y);
      bigint_add2(z_i, z_size - i, ws, p_size+1);
      */

      word carry = 0;

      for(size_t j = 0; j != blocks_of_8; j += 8)
         carry = word8_madd3(z_i + j, p + j, y, carry);

      for(size_t j = blocks_of_8; j != p_size; ++j)
         z_i[j] = word_madd3(p[j], y, z_i[j], &carry);

      word z_sum = z_i[p_size] + carry;
      carry = (z_sum < z_i[p_size]);
      z_i[p_size] = z_sum;

      for(size_t j = p_size + 1; j < z_size - i; ++j)
         {
         z_i[j] += carry;
         carry = carry & !z_i[j];
         }
      }

   /*
   * The result might need to be reduced mod p. To avoid a timing
   * channel, always perform the subtraction. If in the compution
   * of x - p a borrow is required then x was already < p.
   *
   * x - p starts at ws[0] and is p_size+1 bytes long
   * x starts at ws[p_size+1] and is also p_size+1 bytes log
   * (that's the copy_mem)
   *
   * Select which address to copy from indexing off of the final
   * borrow.
   */

   word borrow = 0;
   for(size_t i = 0; i != p_size; ++i)
      ws[i] = word_sub(z[p_size + i], p[i], &borrow);

   ws[p_size] = word_sub(z[p_size+p_size], 0, &borrow);

   copy_mem(ws + p_size + 1, z + p_size, p_size + 1);

   CT::conditional_copy_mem(borrow, z, ws + (p_size + 1), ws, (p_size + 1));
   clear_mem(z + p_size + 1, z_size - p_size - 1);

   CT::unpoison(z, z_size);
   CT::unpoison(p, p_size);
   CT::unpoison(ws, 2*(p_size+1));

   // This check comes after we've used it but that's ok here
   CT::unpoison(&borrow, 1);
   BOTAN_ASSERT(borrow == 0 || borrow == 1, "Expected borrow");
   }

void bigint_monty_mul(word z[], size_t z_size,
                      const word x[], size_t x_size, size_t x_sw,
                      const word y[], size_t y_size, size_t y_sw,
                      const word p[], size_t p_size, word p_dash,
                      word ws[])
   {
   bigint_mul(&z[0], z_size, &ws[0],
              &x[0], x_size, x_sw,
              &y[0], y_size, y_sw);

   bigint_monty_redc(&z[0],
                     &p[0], p_size, p_dash,
                     &ws[0]);
   }

void bigint_monty_sqr(word z[], size_t z_size,
                      const word x[], size_t x_size, size_t x_sw,
                      const word p[], size_t p_size, word p_dash,
                      word ws[])
   {
   bigint_sqr(&z[0], z_size, &ws[0],
              &x[0], x_size, x_sw);

   bigint_monty_redc(&z[0],
                     &p[0], p_size, p_dash,
                     &ws[0]);
   }

}
/*
* Simple O(N^2) Multiplication and Squaring
* (C) 1999-2008 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Simple O(N^2) Multiplication
*/
void bigint_simple_mul(word z[], const word x[], size_t x_size,
                                 const word y[], size_t y_size)
   {
   const size_t x_size_8 = x_size - (x_size % 8);

   clear_mem(z, x_size + y_size);

   for(size_t i = 0; i != y_size; ++i)
      {
      const word y_i = y[i];

      word carry = 0;

      for(size_t j = 0; j != x_size_8; j += 8)
         carry = word8_madd3(z + i + j, x + j, y_i, carry);

      for(size_t j = x_size_8; j != x_size; ++j)
         z[i+j] = word_madd3(x[j], y_i, z[i+j], &carry);

      z[x_size+i] = carry;
      }
   }

/*
* Simple O(N^2) Squaring
*
* This is exactly the same algorithm as bigint_simple_mul, however
* because C/C++ compilers suck at alias analysis it is good to have
* the version where the compiler knows that x == y
*
* There is an O(n^1.5) squaring algorithm specified in Handbook of
* Applied Cryptography, chapter 14
*
*/
void bigint_simple_sqr(word z[], const word x[], size_t x_size)
   {
   const size_t x_size_8 = x_size - (x_size % 8);

   clear_mem(z, 2*x_size);

   for(size_t i = 0; i != x_size; ++i)
      {
      const word x_i = x[i];
      word carry = 0;

      for(size_t j = 0; j != x_size_8; j += 8)
         carry = word8_madd3(z + i + j, x + j, x_i, carry);

      for(size_t j = x_size_8; j != x_size; ++j)
         z[i+j] = word_madd3(x[j], x_i, z[i+j], &carry);

      z[x_size+i] = carry;
      }
   }

}
/*
* MP Shift Algorithms
* (C) 1999-2007,2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Single Operand Left Shift
*/
void bigint_shl1(word x[], size_t x_size, size_t word_shift, size_t bit_shift)
   {
   if(word_shift)
      {
      copy_mem(x + word_shift, x, x_size);
      clear_mem(x, word_shift);
      }

   if(bit_shift)
      {
      word carry = 0;
      for(size_t j = word_shift; j != x_size + word_shift + 1; ++j)
         {
         word temp = x[j];
         x[j] = (temp << bit_shift) | carry;
         carry = (temp >> (MP_WORD_BITS - bit_shift));
         }
      }
   }

/*
* Single Operand Right Shift
*/
void bigint_shr1(word x[], size_t x_size, size_t word_shift, size_t bit_shift)
   {
   if(x_size < word_shift)
      {
      clear_mem(x, x_size);
      return;
      }

   if(word_shift)
      {
      copy_mem(x, x + word_shift, x_size - word_shift);
      clear_mem(x + x_size - word_shift, word_shift);
      }

   if(bit_shift)
      {
      word carry = 0;

      size_t top = x_size - word_shift;

      while(top >= 4)
         {
         word w = x[top-1];
         x[top-1] = (w >> bit_shift) | carry;
         carry = (w << (MP_WORD_BITS - bit_shift));

         w = x[top-2];
         x[top-2] = (w >> bit_shift) | carry;
         carry = (w << (MP_WORD_BITS - bit_shift));

         w = x[top-3];
         x[top-3] = (w >> bit_shift) | carry;
         carry = (w << (MP_WORD_BITS - bit_shift));

         w = x[top-4];
         x[top-4] = (w >> bit_shift) | carry;
         carry = (w << (MP_WORD_BITS - bit_shift));

         top -= 4;
         }

      while(top)
         {
         word w = x[top-1];
         x[top-1] = (w >> bit_shift) | carry;
         carry = (w << (MP_WORD_BITS - bit_shift));

         top--;
         }
      }
   }

/*
* Two Operand Left Shift
*/
void bigint_shl2(word y[], const word x[], size_t x_size,
                 size_t word_shift, size_t bit_shift)
   {
   for(size_t j = 0; j != x_size; ++j)
      y[j + word_shift] = x[j];
   if(bit_shift)
      {
      word carry = 0;
      for(size_t j = word_shift; j != x_size + word_shift + 1; ++j)
         {
         word w = y[j];
         y[j] = (w << bit_shift) | carry;
         carry = (w >> (MP_WORD_BITS - bit_shift));
         }
      }
   }

/*
* Two Operand Right Shift
*/
void bigint_shr2(word y[], const word x[], size_t x_size,
                 size_t word_shift, size_t bit_shift)
   {
   if(x_size < word_shift) return;

   for(size_t j = 0; j != x_size - word_shift; ++j)
      y[j] = x[j + word_shift];
   if(bit_shift)
      {
      word carry = 0;
      for(size_t j = x_size - word_shift; j > 0; --j)
         {
         word w = y[j-1];
         y[j-1] = (w >> bit_shift) | carry;
         carry = (w << (MP_WORD_BITS - bit_shift));
         }
      }
   }

}
/*
* DSA Parameter Generation
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

/*
* Check if this size is allowed by FIPS 186-3
*/
bool fips186_3_valid_size(size_t pbits, size_t qbits)
   {
   if(qbits == 160)
      return (pbits == 512 || pbits == 768 || pbits == 1024);

   if(qbits == 224)
      return (pbits == 2048);

   if(qbits == 256)
      return (pbits == 2048 || pbits == 3072);

   return false;
   }

}

/*
* Attempt DSA prime generation with given seed
*/
bool generate_dsa_primes(RandomNumberGenerator& rng,
                         BigInt& p, BigInt& q,
                         size_t pbits, size_t qbits,
                         const std::vector<byte>& seed_c)
   {
   if(!fips186_3_valid_size(pbits, qbits))
      throw Invalid_Argument(
         "FIPS 186-3 does not allow DSA domain parameters of " +
         std::to_string(pbits) + "/" + std::to_string(qbits) + " bits long");

   if(seed_c.size() * 8 < qbits)
      throw Invalid_Argument(
         "Generating a DSA parameter set with a " + std::to_string(qbits) +
         "long q requires a seed at least as many bits long");

   const std::string hash_name = "SHA-" + std::to_string(qbits);
   std::unique_ptr<HashFunction> hash(HashFunction::create(hash_name));
   if(!hash)
      throw Algorithm_Not_Found(hash_name);

   const size_t HASH_SIZE = hash->output_length();

   class Seed
      {
      public:
         Seed(const std::vector<byte>& s) : seed(s) {}

         operator std::vector<byte>& () { return seed; }

         Seed& operator++()
            {
            for(size_t j = seed.size(); j > 0; --j)
               if(++seed[j-1])
                  break;
            return (*this);
            }
      private:
         std::vector<byte> seed;
      };

   Seed seed(seed_c);

   q.binary_decode(hash->process(seed));
   q.set_bit(qbits-1);
   q.set_bit(0);

   if(!is_prime(q, rng))
      return false;

   const size_t n = (pbits-1) / (HASH_SIZE * 8),
                b = (pbits-1) % (HASH_SIZE * 8);

   BigInt X;
   std::vector<byte> V(HASH_SIZE * (n+1));

   for(size_t j = 0; j != 4096; ++j)
      {
      for(size_t k = 0; k <= n; ++k)
         {
         ++seed;
         hash->update(seed);
         hash->final(&V[HASH_SIZE * (n-k)]);
         }

      X.binary_decode(&V[HASH_SIZE - 1 - b/8],
                      V.size() - (HASH_SIZE - 1 - b/8));
      X.set_bit(pbits-1);

      p = X - (X % (2*q) - 1);

      if(p.bits() == pbits && is_prime(p, rng))
         return true;
      }
   return false;
   }

/*
* Generate DSA Primes
*/
std::vector<byte> generate_dsa_primes(RandomNumberGenerator& rng,
                                      BigInt& p, BigInt& q,
                                      size_t pbits, size_t qbits)
   {
   while(true)
      {
      std::vector<byte> seed(qbits / 8);
      rng.randomize(seed.data(), seed.size());

      if(generate_dsa_primes(rng, p, q, pbits, qbits, seed))
         return seed;
      }
   }

}
/*
* Jacobi Function
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Calculate the Jacobi symbol
*/
s32bit jacobi(const BigInt& a, const BigInt& n)
   {
   if(a.is_negative())
      throw Invalid_Argument("jacobi: first argument must be non-negative");
   if(n.is_even() || n < 2)
      throw Invalid_Argument("jacobi: second argument must be odd and > 1");

   BigInt x = a, y = n;
   s32bit J = 1;

   while(y > 1)
      {
      x %= y;
      if(x > y / 2)
         {
         x = y - x;
         if(y % 4 == 3)
            J = -J;
         }
      if(x.is_zero())
         return 0;

      size_t shifts = low_zero_bits(x);
      x >>= shifts;
      if(shifts % 2)
         {
         word y_mod_8 = y % 8;
         if(y_mod_8 == 3 || y_mod_8 == 5)
            J = -J;
         }

      if(x % 4 == 3 && y % 4 == 3)
         J = -J;
      std::swap(x, y);
      }
   return J;
   }

}
/*
* Prime Generation
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Generate a random prime
*/
BigInt random_prime(RandomNumberGenerator& rng,
                    size_t bits, const BigInt& coprime,
                    size_t equiv, size_t modulo)
   {
   if(coprime <= 0)
      {
      throw Invalid_Argument("random_prime: coprime must be > 0");
      }
   if(modulo % 2 == 1 || modulo == 0)
      {
      throw Invalid_Argument("random_prime: Invalid modulo value");
      }
   if(equiv >= modulo || equiv % 2 == 0)
      {
      throw Invalid_Argument("random_prime: equiv must be < modulo, and odd");
      }

   // Handle small values:
   if(bits <= 1)
      {
      throw Invalid_Argument("random_prime: Can't make a prime of " +
                             std::to_string(bits) + " bits");
      }
   else if(bits == 2)
      {
      return ((rng.next_byte() % 2) ? 2 : 3);
      }
   else if(bits == 3)
      {
      return ((rng.next_byte() % 2) ? 5 : 7);
      }
   else if(bits == 4)
      {
      return ((rng.next_byte() % 2) ? 11 : 13);
      }

   while(true)
      {
      BigInt p(rng, bits);

      // Force lowest and two top bits on
      p.set_bit(bits - 1);
      p.set_bit(bits - 2);
      p.set_bit(0);

      if(p % modulo != equiv)
         p += (modulo - p % modulo) + equiv;

      const size_t sieve_size = std::min(bits / 2, PRIME_TABLE_SIZE);
      secure_vector<u16bit> sieve(sieve_size);

      for(size_t j = 0; j != sieve.size(); ++j)
         sieve[j] = p % PRIMES[j];

      size_t counter = 0;
      while(true)
         {
         ++counter;

         if(counter >= 4096)
            {
            break; // don't try forever, choose a new starting point
            }

         p += modulo;

         if(p.bits() > bits)
            break;

         bool passes_sieve = true;
         for(size_t j = 0; j != sieve.size(); ++j)
            {
            sieve[j] = (sieve[j] + modulo) % PRIMES[j];
            if(sieve[j] == 0)
               {
               passes_sieve = false;
               break;
               }
            }

         if(!passes_sieve)
            continue;

         if(gcd(p - 1, coprime) != 1)
            continue;

         if(is_prime(p, rng, 128, true))
            {
            return p;
            }
         }
      }
   }

/*
* Generate a random safe prime
*/
BigInt random_safe_prime(RandomNumberGenerator& rng, size_t bits)
   {
   if(bits <= 64)
      throw Invalid_Argument("random_safe_prime: Can't make a prime of " +
                             std::to_string(bits) + " bits");

   BigInt p;
   do
      p = (random_prime(rng, bits - 1) << 1) + 1;
   while(!is_prime(p, rng, 128, true));

   return p;
   }

}
/*
* Fused and Important MP Algorithms
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Square a BigInt
*/
BigInt square(const BigInt& x)
   {
   const size_t x_sw = x.sig_words();

   BigInt z(BigInt::Positive, round_up(2*x_sw, 16));
   secure_vector<word> workspace(z.size());

   bigint_sqr(z.mutable_data(), z.size(),
              workspace.data(),
              x.data(), x.size(), x_sw);
   return z;
   }

/*
* Multiply-Add Operation
*/
BigInt mul_add(const BigInt& a, const BigInt& b, const BigInt& c)
   {
   if(c.is_negative() || c.is_zero())
      throw Invalid_Argument("mul_add: Third argument must be > 0");

   BigInt::Sign sign = BigInt::Positive;
   if(a.sign() != b.sign())
      sign = BigInt::Negative;

   const size_t a_sw = a.sig_words();
   const size_t b_sw = b.sig_words();
   const size_t c_sw = c.sig_words();

   BigInt r(sign, std::max(a.size() + b.size(), c_sw) + 1);
   secure_vector<word> workspace(r.size());

   bigint_mul(r.mutable_data(), r.size(),
              workspace.data(),
              a.data(), a.size(), a_sw,
              b.data(), b.size(), b_sw);

   const size_t r_size = std::max(r.sig_words(), c_sw);
   bigint_add2(r.mutable_data(), r_size, c.data(), c_sw);
   return r;
   }

/*
* Subtract-Multiply Operation
*/
BigInt sub_mul(const BigInt& a, const BigInt& b, const BigInt& c)
   {
   if(a.is_negative() || b.is_negative())
      throw Invalid_Argument("sub_mul: First two arguments must be >= 0");

   BigInt r = a;
   r -= b;
   r *= c;
   return r;
   }

}
/*
* Number Theory Functions
* (C) 1999-2011 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Return the number of 0 bits at the end of n
*/
size_t low_zero_bits(const BigInt& n)
   {
   size_t low_zero = 0;

   if(n.is_positive() && n.is_nonzero())
      {
      for(size_t i = 0; i != n.size(); ++i)
         {
         const word x = n.word_at(i);

         if(x)
            {
            low_zero += ctz(x);
            break;
            }
         else
            low_zero += BOTAN_MP_WORD_BITS;
         }
      }

   return low_zero;
   }

/*
* Calculate the GCD
*/
BigInt gcd(const BigInt& a, const BigInt& b)
   {
   if(a.is_zero() || b.is_zero()) return 0;
   if(a == 1 || b == 1)           return 1;

   BigInt x = a, y = b;
   x.set_sign(BigInt::Positive);
   y.set_sign(BigInt::Positive);
   size_t shift = std::min(low_zero_bits(x), low_zero_bits(y));

   x >>= shift;
   y >>= shift;

   while(x.is_nonzero())
      {
      x >>= low_zero_bits(x);
      y >>= low_zero_bits(y);
      if(x >= y) { x -= y; x >>= 1; }
      else       { y -= x; y >>= 1; }
      }

   return (y << shift);
   }

/*
* Calculate the LCM
*/
BigInt lcm(const BigInt& a, const BigInt& b)
   {
   return ((a * b) / gcd(a, b));
   }

namespace {

/*
* If the modulus is odd, then we can avoid computing A and C. This is
* a critical path algorithm in some instances and an odd modulus is
* the common case for crypto, so worth special casing. See note 14.64
* in Handbook of Applied Cryptography for more details.
*/
BigInt inverse_mod_odd_modulus(const BigInt& n, const BigInt& mod)
   {
   BigInt u = mod, v = n;
   BigInt B = 0, D = 1;

   while(u.is_nonzero())
      {
      const size_t u_zero_bits = low_zero_bits(u);
      u >>= u_zero_bits;
      for(size_t i = 0; i != u_zero_bits; ++i)
         {
         if(B.is_odd())
            { B -= mod; }
         B >>= 1;
         }

      const size_t v_zero_bits = low_zero_bits(v);
      v >>= v_zero_bits;
      for(size_t i = 0; i != v_zero_bits; ++i)
         {
         if(D.is_odd())
            { D -= mod; }
         D >>= 1;
         }

      if(u >= v) { u -= v; B -= D; }
      else       { v -= u; D -= B; }
      }

   if(v != 1)
      return 0; // no modular inverse

   while(D.is_negative()) D += mod;
   while(D >= mod) D -= mod;

   return D;
   }

}

/*
* Find the Modular Inverse
*/
BigInt inverse_mod(const BigInt& n, const BigInt& mod)
   {
   if(mod.is_zero())
      throw BigInt::DivideByZero();
   if(mod.is_negative() || n.is_negative())
      throw Invalid_Argument("inverse_mod: arguments must be non-negative");

   if(n.is_zero() || (n.is_even() && mod.is_even()))
      return 0; // fast fail checks

   if(mod.is_odd())
      return inverse_mod_odd_modulus(n, mod);

   BigInt u = mod, v = n;
   BigInt A = 1, B = 0, C = 0, D = 1;

   while(u.is_nonzero())
      {
      const size_t u_zero_bits = low_zero_bits(u);
      u >>= u_zero_bits;
      for(size_t i = 0; i != u_zero_bits; ++i)
         {
         if(A.is_odd() || B.is_odd())
            { A += n; B -= mod; }
         A >>= 1; B >>= 1;
         }

      const size_t v_zero_bits = low_zero_bits(v);
      v >>= v_zero_bits;
      for(size_t i = 0; i != v_zero_bits; ++i)
         {
         if(C.is_odd() || D.is_odd())
            { C += n; D -= mod; }
         C >>= 1; D >>= 1;
         }

      if(u >= v) { u -= v; A -= C; B -= D; }
      else       { v -= u; C -= A; D -= B; }
      }

   if(v != 1)
      return 0; // no modular inverse

   while(D.is_negative()) D += mod;
   while(D >= mod) D -= mod;

   return D;
   }

word monty_inverse(word input)
   {
   if(input == 0)
      throw std::runtime_error("monty_inverse: divide by zero");

   word b = input;
   word x2 = 1, x1 = 0, y2 = 0, y1 = 1;

   // First iteration, a = n+1
   word q = bigint_divop(1, 0, b);
   word r = (MP_WORD_MAX - q*b) + 1;
   word x = x2 - q*x1;
   word y = y2 - q*y1;

   word a = b;
   b = r;
   x2 = x1;
   x1 = x;
   y2 = y1;
   y1 = y;

   while(b > 0)
      {
      q = a / b;
      r = a - q*b;
      x = x2 - q*x1;
      y = y2 - q*y1;

      a = b;
      b = r;
      x2 = x1;
      x1 = x;
      y2 = y1;
      y1 = y;
      }

   // Now invert in addition space
   y2 = (MP_WORD_MAX - y2) + 1;

   return y2;
   }

/*
* Modular Exponentiation
*/
BigInt power_mod(const BigInt& base, const BigInt& exp, const BigInt& mod)
   {
   Power_Mod pow_mod(mod);

   /*
   * Calling set_base before set_exponent means we end up using a
   * minimal window. This makes sense given that here we know that any
   * precomputation is wasted.
   */
   pow_mod.set_base(base);
   pow_mod.set_exponent(exp);
   return pow_mod.execute();
   }

namespace {

bool mr_witness(BigInt&& y,
                const Modular_Reducer& reducer_n,
                const BigInt& n_minus_1, size_t s)
   {
   if(y == 1 || y == n_minus_1)
      return false;

   for(size_t i = 1; i != s; ++i)
      {
      y = reducer_n.square(y);

      if(y == 1) // found a non-trivial square root
         return true;

      if(y == n_minus_1) // -1, trivial square root, so give up
         return false;
      }

   return true; // fails Fermat test
   }

size_t mr_test_iterations(size_t n_bits, size_t prob, bool random)
   {
   const size_t base = (prob + 2) / 2; // worst case 4^-t error rate

   /*
   * For randomly chosen numbers we can use the estimates from
   * http://www.math.dartmouth.edu/~carlp/PDF/paper88.pdf
   *
   * These values are derived from the inequality for p(k,t) given on
   * the second page.
   */
   if(random && prob <= 80)
      {
      if(n_bits >= 1536)
         return 2; // < 2^-89
      if(n_bits >= 1024)
         return 4; // < 2^-89
      if(n_bits >= 512)
         return 5; // < 2^-80
      if(n_bits >= 256)
         return 11; // < 2^-80
      }

   return base;
   }

}

/*
* Test for primaility using Miller-Rabin
*/
bool is_prime(const BigInt& n, RandomNumberGenerator& rng,
              size_t prob, bool is_random)
   {
   if(n == 2)
      return true;
   if(n <= 1 || n.is_even())
      return false;

   // Fast path testing for small numbers (<= 65521)
   if(n <= PRIMES[PRIME_TABLE_SIZE-1])
      {
      const u16bit num = n.word_at(0);

      return std::binary_search(PRIMES, PRIMES + PRIME_TABLE_SIZE, num);
      }

   const size_t test_iterations = mr_test_iterations(n.bits(), prob, is_random);

   const BigInt n_minus_1 = n - 1;
   const size_t s = low_zero_bits(n_minus_1);

   Fixed_Exponent_Power_Mod pow_mod(n_minus_1 >> s, n);
   Modular_Reducer reducer(n);

   for(size_t i = 0; i != test_iterations; ++i)
      {
      const BigInt a = BigInt::random_integer(rng, 2, n_minus_1);
      BigInt y = pow_mod(a);

      if(mr_witness(std::move(y), reducer, n_minus_1, s))
         return false;
      }

   return true;
   }

}
/*
* Modular Exponentiation Proxy
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Power_Mod Constructor
*/
Power_Mod::Power_Mod(const BigInt& n, Usage_Hints hints)
   {
   m_core = nullptr;
   set_modulus(n, hints);
   }

/*
* Power_Mod Copy Constructor
*/
Power_Mod::Power_Mod(const Power_Mod& other)
   {
   m_core = nullptr;
   if(other.m_core)
      m_core = other.m_core->copy();
   }

/*
* Power_Mod Assignment Operator
*/
Power_Mod& Power_Mod::operator=(const Power_Mod& other)
   {
   delete m_core;
   m_core = nullptr;
   if(other.m_core)
      m_core = other.m_core->copy();
   return (*this);
   }

/*
* Power_Mod Destructor
*/
Power_Mod::~Power_Mod()
   {
   delete m_core;
   m_core = nullptr;
   }

/*
* Set the modulus
*/
void Power_Mod::set_modulus(const BigInt& n, Usage_Hints hints) const
   {
   delete m_core;

   if(n.is_odd())
      m_core = new Montgomery_Exponentiator(n, hints);
   else if(n != 0)
      m_core = new Fixed_Window_Exponentiator(n, hints);
   }

/*
* Set the base
*/
void Power_Mod::set_base(const BigInt& b) const
   {
   if(b.is_zero() || b.is_negative())
      throw Invalid_Argument("Power_Mod::set_base: arg must be > 0");

   if(!m_core)
      throw Internal_Error("Power_Mod::set_base: m_core was NULL");
   m_core->set_base(b);
   }

/*
* Set the exponent
*/
void Power_Mod::set_exponent(const BigInt& e) const
   {
   if(e.is_negative())
      throw Invalid_Argument("Power_Mod::set_exponent: arg must be > 0");

   if(!m_core)
      throw Internal_Error("Power_Mod::set_exponent: m_core was NULL");
   m_core->set_exponent(e);
   }

/*
* Compute the result
*/
BigInt Power_Mod::execute() const
   {
   if(!m_core)
      throw Internal_Error("Power_Mod::execute: m_core was NULL");
   return m_core->execute();
   }

/*
* Try to choose a good window size
*/
size_t Power_Mod::window_bits(size_t exp_bits, size_t,
                              Power_Mod::Usage_Hints hints)
   {
   static const size_t wsize[][2] = {
      { 1434, 7 },
      {  539, 6 },
      {  197, 4 },
      {   70, 3 },
      {   25, 2 },
      {    0, 0 }
   };

   size_t window_bits = 1;

   if(exp_bits)
      {
      for(size_t j = 0; wsize[j][0]; ++j)
         {
         if(exp_bits >= wsize[j][0])
            {
            window_bits += wsize[j][1];
            break;
            }
         }
      }

   if(hints & Power_Mod::BASE_IS_FIXED)
      window_bits += 2;
   if(hints & Power_Mod::EXP_IS_LARGE)
      ++window_bits;

   return window_bits;
   }

namespace {

/*
* Choose potentially useful hints
*/
Power_Mod::Usage_Hints choose_base_hints(const BigInt& b, const BigInt& n)
   {
   if(b == 2)
      return Power_Mod::Usage_Hints(Power_Mod::BASE_IS_2 |
                                    Power_Mod::BASE_IS_SMALL);

   const size_t b_bits = b.bits();
   const size_t n_bits = n.bits();

   if(b_bits < n_bits / 32)
      return Power_Mod::BASE_IS_SMALL;
   if(b_bits > n_bits / 4)
      return Power_Mod::BASE_IS_LARGE;

   return Power_Mod::NO_HINTS;
   }

/*
* Choose potentially useful hints
*/
Power_Mod::Usage_Hints choose_exp_hints(const BigInt& e, const BigInt& n)
   {
   const size_t e_bits = e.bits();
   const size_t n_bits = n.bits();

   if(e_bits < n_bits / 32)
      return Power_Mod::BASE_IS_SMALL;
   if(e_bits > n_bits / 4)
      return Power_Mod::BASE_IS_LARGE;
   return Power_Mod::NO_HINTS;
   }

}

/*
* Fixed_Exponent_Power_Mod Constructor
*/
Fixed_Exponent_Power_Mod::Fixed_Exponent_Power_Mod(const BigInt& e,
                                                   const BigInt& n,
                                                   Usage_Hints hints) :
   Power_Mod(n, Usage_Hints(hints | EXP_IS_FIXED | choose_exp_hints(e, n)))
   {
   set_exponent(e);
   }

/*
* Fixed_Base_Power_Mod Constructor
*/
Fixed_Base_Power_Mod::Fixed_Base_Power_Mod(const BigInt& b, const BigInt& n,
                                           Usage_Hints hints) :
   Power_Mod(n, Usage_Hints(hints | BASE_IS_FIXED | choose_base_hints(b, n)))
   {
   set_base(b);
   }

}
/*
* Fixed Window Exponentiation
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Set the exponent
*/
void Fixed_Window_Exponentiator::set_exponent(const BigInt& e)
   {
   exp = e;
   }

/*
* Set the base
*/
void Fixed_Window_Exponentiator::set_base(const BigInt& base)
   {
   window_bits = Power_Mod::window_bits(exp.bits(), base.bits(), hints);

   g.resize((1 << window_bits));
   g[0] = 1;
   g[1] = base;

   for(size_t i = 2; i != g.size(); ++i)
      g[i] = reducer.multiply(g[i-1], g[0]);
   }

/*
* Compute the result
*/
BigInt Fixed_Window_Exponentiator::execute() const
   {
   const size_t exp_nibbles = (exp.bits() + window_bits - 1) / window_bits;

   BigInt x = 1;

   for(size_t i = exp_nibbles; i > 0; --i)
      {
      for(size_t j = 0; j != window_bits; ++j)
         x = reducer.square(x);

      const u32bit nibble = exp.get_substring(window_bits*(i-1), window_bits);

      x = reducer.multiply(x, g[nibble]);
      }
   return x;
   }

/*
* Fixed_Window_Exponentiator Constructor
*/
Fixed_Window_Exponentiator::Fixed_Window_Exponentiator(const BigInt& n,
                                                       Power_Mod::Usage_Hints hints)
   {
   reducer = Modular_Reducer(n);
   this->hints = hints;
   window_bits = 0;
   }

}
/*
* Montgomery Exponentiation
* (C) 1999-2010,2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Set the exponent
*/
void Montgomery_Exponentiator::set_exponent(const BigInt& exp)
   {
   m_exp = exp;
   m_exp_bits = exp.bits();
   }

/*
* Set the base
*/
void Montgomery_Exponentiator::set_base(const BigInt& base)
   {
   m_window_bits = Power_Mod::window_bits(m_exp.bits(), base.bits(), m_hints);

   m_g.resize((1 << m_window_bits));

   BigInt z(BigInt::Positive, 2 * (m_mod_words + 1));
   secure_vector<word> workspace(z.size());

   m_g[0] = 1;

   bigint_monty_mul(z.mutable_data(), z.size(),
                    m_g[0].data(), m_g[0].size(), m_g[0].sig_words(),
                    m_R2_mod.data(), m_R2_mod.size(), m_R2_mod.sig_words(),
                    m_modulus.data(), m_mod_words, m_mod_prime,
                    workspace.data());

   m_g[0] = z;

   m_g[1] = (base >= m_modulus) ? (base % m_modulus) : base;

   bigint_monty_mul(z.mutable_data(), z.size(),
                    m_g[1].data(), m_g[1].size(), m_g[1].sig_words(),
                    m_R2_mod.data(), m_R2_mod.size(), m_R2_mod.sig_words(),
                    m_modulus.data(), m_mod_words, m_mod_prime,
                    workspace.data());

   m_g[1] = z;

   const BigInt& x = m_g[1];
   const size_t x_sig = x.sig_words();

   for(size_t i = 2; i != m_g.size(); ++i)
      {
      const BigInt& y = m_g[i-1];
      const size_t y_sig = y.sig_words();

      bigint_monty_mul(z.mutable_data(), z.size(),
                       x.data(), x.size(), x_sig,
                       y.data(), y.size(), y_sig,
                       m_modulus.data(), m_mod_words, m_mod_prime,
                       workspace.data());

      m_g[i] = z;
      }
   }

/*
* Compute the result
*/
BigInt Montgomery_Exponentiator::execute() const
   {
   const size_t exp_nibbles = (m_exp_bits + m_window_bits - 1) / m_window_bits;

   BigInt x = m_R_mod;

   const size_t z_size = 2*(m_mod_words + 1);

   BigInt z(BigInt::Positive, z_size);
   secure_vector<word> workspace(z_size);

   for(size_t i = exp_nibbles; i > 0; --i)
      {
      for(size_t k = 0; k != m_window_bits; ++k)
         {
         bigint_monty_sqr(z.mutable_data(), z_size,
                          x.data(), x.size(), x.sig_words(),
                          m_modulus.data(), m_mod_words, m_mod_prime,
                          workspace.data());

         x = z;
         }

      const u32bit nibble = m_exp.get_substring(m_window_bits*(i-1), m_window_bits);

      const BigInt& y = m_g[nibble];

      bigint_monty_mul(z.mutable_data(), z_size,
                       x.data(), x.size(), x.sig_words(),
                       y.data(), y.size(), y.sig_words(),
                       m_modulus.data(), m_mod_words, m_mod_prime,
                       workspace.data());

      x = z;
      }

   x.grow_to(2*m_mod_words + 1);

   bigint_monty_redc(x.mutable_data(),
                     m_modulus.data(), m_mod_words, m_mod_prime,
                     workspace.data());

   return x;
   }

/*
* Montgomery_Exponentiator Constructor
*/
Montgomery_Exponentiator::Montgomery_Exponentiator(const BigInt& mod,
                                                   Power_Mod::Usage_Hints hints) :
   m_modulus(mod),
   m_mod_words(m_modulus.sig_words()),
   m_window_bits(1),
   m_hints(hints)
   {
   // Montgomery reduction only works for positive odd moduli
   if(!m_modulus.is_positive() || m_modulus.is_even())
      throw Invalid_Argument("Montgomery_Exponentiator: invalid modulus");

   m_mod_prime = monty_inverse(mod.word_at(0));

   const BigInt r = BigInt::power_of_2(m_mod_words * BOTAN_MP_WORD_BITS);
   m_R_mod = r % m_modulus;
   m_R2_mod = (m_R_mod * m_R_mod) % m_modulus;
   m_exp_bits = 0;
   }

}
/*
* Small Primes Table
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

const u16bit PRIMES[PRIME_TABLE_SIZE+1] = {
    3,     5,     7,    11,    13,    17,    19,    23,    29,    31,    37,
   41,    43,    47,    53,    59,    61,    67,    71,    73,    79,    83,
   89,    97,   101,   103,   107,   109,   113,   127,   131,   137,   139,
  149,   151,   157,   163,   167,   173,   179,   181,   191,   193,   197,
  199,   211,   223,   227,   229,   233,   239,   241,   251,   257,   263,
  269,   271,   277,   281,   283,   293,   307,   311,   313,   317,   331,
  337,   347,   349,   353,   359,   367,   373,   379,   383,   389,   397,
  401,   409,   419,   421,   431,   433,   439,   443,   449,   457,   461,
  463,   467,   479,   487,   491,   499,   503,   509,   521,   523,   541,
  547,   557,   563,   569,   571,   577,   587,   593,   599,   601,   607,
  613,   617,   619,   631,   641,   643,   647,   653,   659,   661,   673,
  677,   683,   691,   701,   709,   719,   727,   733,   739,   743,   751,
  757,   761,   769,   773,   787,   797,   809,   811,   821,   823,   827,
  829,   839,   853,   857,   859,   863,   877,   881,   883,   887,   907,
  911,   919,   929,   937,   941,   947,   953,   967,   971,   977,   983,
  991,   997,  1009,  1013,  1019,  1021,  1031,  1033,  1039,  1049,  1051,
 1061,  1063,  1069,  1087,  1091,  1093,  1097,  1103,  1109,  1117,  1123,
 1129,  1151,  1153,  1163,  1171,  1181,  1187,  1193,  1201,  1213,  1217,
 1223,  1229,  1231,  1237,  1249,  1259,  1277,  1279,  1283,  1289,  1291,
 1297,  1301,  1303,  1307,  1319,  1321,  1327,  1361,  1367,  1373,  1381,
 1399,  1409,  1423,  1427,  1429,  1433,  1439,  1447,  1451,  1453,  1459,
 1471,  1481,  1483,  1487,  1489,  1493,  1499,  1511,  1523,  1531,  1543,
 1549,  1553,  1559,  1567,  1571,  1579,  1583,  1597,  1601,  1607,  1609,
 1613,  1619,  1621,  1627,  1637,  1657,  1663,  1667,  1669,  1693,  1697,
 1699,  1709,  1721,  1723,  1733,  1741,  1747,  1753,  1759,  1777,  1783,
 1787,  1789,  1801,  1811,  1823,  1831,  1847,  1861,  1867,  1871,  1873,
 1877,  1879,  1889,  1901,  1907,  1913,  1931,  1933,  1949,  1951,  1973,
 1979,  1987,  1993,  1997,  1999,  2003,  2011,  2017,  2027,  2029,  2039,
 2053,  2063,  2069,  2081,  2083,  2087,  2089,  2099,  2111,  2113,  2129,
 2131,  2137,  2141,  2143,  2153,  2161,  2179,  2203,  2207,  2213,  2221,
 2237,  2239,  2243,  2251,  2267,  2269,  2273,  2281,  2287,  2293,  2297,
 2309,  2311,  2333,  2339,  2341,  2347,  2351,  2357,  2371,  2377,  2381,
 2383,  2389,  2393,  2399,  2411,  2417,  2423,  2437,  2441,  2447,  2459,
 2467,  2473,  2477,  2503,  2521,  2531,  2539,  2543,  2549,  2551,  2557,
 2579,  2591,  2593,  2609,  2617,  2621,  2633,  2647,  2657,  2659,  2663,
 2671,  2677,  2683,  2687,  2689,  2693,  2699,  2707,  2711,  2713,  2719,
 2729,  2731,  2741,  2749,  2753,  2767,  2777,  2789,  2791,  2797,  2801,
 2803,  2819,  2833,  2837,  2843,  2851,  2857,  2861,  2879,  2887,  2897,
 2903,  2909,  2917,  2927,  2939,  2953,  2957,  2963,  2969,  2971,  2999,
 3001,  3011,  3019,  3023,  3037,  3041,  3049,  3061,  3067,  3079,  3083,
 3089,  3109,  3119,  3121,  3137,  3163,  3167,  3169,  3181,  3187,  3191,
 3203,  3209,  3217,  3221,  3229,  3251,  3253,  3257,  3259,  3271,  3299,
 3301,  3307,  3313,  3319,  3323,  3329,  3331,  3343,  3347,  3359,  3361,
 3371,  3373,  3389,  3391,  3407,  3413,  3433,  3449,  3457,  3461,  3463,
 3467,  3469,  3491,  3499,  3511,  3517,  3527,  3529,  3533,  3539,  3541,
 3547,  3557,  3559,  3571,  3581,  3583,  3593,  3607,  3613,  3617,  3623,
 3631,  3637,  3643,  3659,  3671,  3673,  3677,  3691,  3697,  3701,  3709,
 3719,  3727,  3733,  3739,  3761,  3767,  3769,  3779,  3793,  3797,  3803,
 3821,  3823,  3833,  3847,  3851,  3853,  3863,  3877,  3881,  3889,  3907,
 3911,  3917,  3919,  3923,  3929,  3931,  3943,  3947,  3967,  3989,  4001,
 4003,  4007,  4013,  4019,  4021,  4027,  4049,  4051,  4057,  4073,  4079,
 4091,  4093,  4099,  4111,  4127,  4129,  4133,  4139,  4153,  4157,  4159,
 4177,  4201,  4211,  4217,  4219,  4229,  4231,  4241,  4243,  4253,  4259,
 4261,  4271,  4273,  4283,  4289,  4297,  4327,  4337,  4339,  4349,  4357,
 4363,  4373,  4391,  4397,  4409,  4421,  4423,  4441,  4447,  4451,  4457,
 4463,  4481,  4483,  4493,  4507,  4513,  4517,  4519,  4523,  4547,  4549,
 4561,  4567,  4583,  4591,  4597,  4603,  4621,  4637,  4639,  4643,  4649,
 4651,  4657,  4663,  4673,  4679,  4691,  4703,  4721,  4723,  4729,  4733,
 4751,  4759,  4783,  4787,  4789,  4793,  4799,  4801,  4813,  4817,  4831,
 4861,  4871,  4877,  4889,  4903,  4909,  4919,  4931,  4933,  4937,  4943,
 4951,  4957,  4967,  4969,  4973,  4987,  4993,  4999,  5003,  5009,  5011,
 5021,  5023,  5039,  5051,  5059,  5077,  5081,  5087,  5099,  5101,  5107,
 5113,  5119,  5147,  5153,  5167,  5171,  5179,  5189,  5197,  5209,  5227,
 5231,  5233,  5237,  5261,  5273,  5279,  5281,  5297,  5303,  5309,  5323,
 5333,  5347,  5351,  5381,  5387,  5393,  5399,  5407,  5413,  5417,  5419,
 5431,  5437,  5441,  5443,  5449,  5471,  5477,  5479,  5483,  5501,  5503,
 5507,  5519,  5521,  5527,  5531,  5557,  5563,  5569,  5573,  5581,  5591,
 5623,  5639,  5641,  5647,  5651,  5653,  5657,  5659,  5669,  5683,  5689,
 5693,  5701,  5711,  5717,  5737,  5741,  5743,  5749,  5779,  5783,  5791,
 5801,  5807,  5813,  5821,  5827,  5839,  5843,  5849,  5851,  5857,  5861,
 5867,  5869,  5879,  5881,  5897,  5903,  5923,  5927,  5939,  5953,  5981,
 5987,  6007,  6011,  6029,  6037,  6043,  6047,  6053,  6067,  6073,  6079,
 6089,  6091,  6101,  6113,  6121,  6131,  6133,  6143,  6151,  6163,  6173,
 6197,  6199,  6203,  6211,  6217,  6221,  6229,  6247,  6257,  6263,  6269,
 6271,  6277,  6287,  6299,  6301,  6311,  6317,  6323,  6329,  6337,  6343,
 6353,  6359,  6361,  6367,  6373,  6379,  6389,  6397,  6421,  6427,  6449,
 6451,  6469,  6473,  6481,  6491,  6521,  6529,  6547,  6551,  6553,  6563,
 6569,  6571,  6577,  6581,  6599,  6607,  6619,  6637,  6653,  6659,  6661,
 6673,  6679,  6689,  6691,  6701,  6703,  6709,  6719,  6733,  6737,  6761,
 6763,  6779,  6781,  6791,  6793,  6803,  6823,  6827,  6829,  6833,  6841,
 6857,  6863,  6869,  6871,  6883,  6899,  6907,  6911,  6917,  6947,  6949,
 6959,  6961,  6967,  6971,  6977,  6983,  6991,  6997,  7001,  7013,  7019,
 7027,  7039,  7043,  7057,  7069,  7079,  7103,  7109,  7121,  7127,  7129,
 7151,  7159,  7177,  7187,  7193,  7207,  7211,  7213,  7219,  7229,  7237,
 7243,  7247,  7253,  7283,  7297,  7307,  7309,  7321,  7331,  7333,  7349,
 7351,  7369,  7393,  7411,  7417,  7433,  7451,  7457,  7459,  7477,  7481,
 7487,  7489,  7499,  7507,  7517,  7523,  7529,  7537,  7541,  7547,  7549,
 7559,  7561,  7573,  7577,  7583,  7589,  7591,  7603,  7607,  7621,  7639,
 7643,  7649,  7669,  7673,  7681,  7687,  7691,  7699,  7703,  7717,  7723,
 7727,  7741,  7753,  7757,  7759,  7789,  7793,  7817,  7823,  7829,  7841,
 7853,  7867,  7873,  7877,  7879,  7883,  7901,  7907,  7919,  7927,  7933,
 7937,  7949,  7951,  7963,  7993,  8009,  8011,  8017,  8039,  8053,  8059,
 8069,  8081,  8087,  8089,  8093,  8101,  8111,  8117,  8123,  8147,  8161,
 8167,  8171,  8179,  8191,  8209,  8219,  8221,  8231,  8233,  8237,  8243,
 8263,  8269,  8273,  8287,  8291,  8293,  8297,  8311,  8317,  8329,  8353,
 8363,  8369,  8377,  8387,  8389,  8419,  8423,  8429,  8431,  8443,  8447,
 8461,  8467,  8501,  8513,  8521,  8527,  8537,  8539,  8543,  8563,  8573,
 8581,  8597,  8599,  8609,  8623,  8627,  8629,  8641,  8647,  8663,  8669,
 8677,  8681,  8689,  8693,  8699,  8707,  8713,  8719,  8731,  8737,  8741,
 8747,  8753,  8761,  8779,  8783,  8803,  8807,  8819,  8821,  8831,  8837,
 8839,  8849,  8861,  8863,  8867,  8887,  8893,  8923,  8929,  8933,  8941,
 8951,  8963,  8969,  8971,  8999,  9001,  9007,  9011,  9013,  9029,  9041,
 9043,  9049,  9059,  9067,  9091,  9103,  9109,  9127,  9133,  9137,  9151,
 9157,  9161,  9173,  9181,  9187,  9199,  9203,  9209,  9221,  9227,  9239,
 9241,  9257,  9277,  9281,  9283,  9293,  9311,  9319,  9323,  9337,  9341,
 9343,  9349,  9371,  9377,  9391,  9397,  9403,  9413,  9419,  9421,  9431,
 9433,  9437,  9439,  9461,  9463,  9467,  9473,  9479,  9491,  9497,  9511,
 9521,  9533,  9539,  9547,  9551,  9587,  9601,  9613,  9619,  9623,  9629,
 9631,  9643,  9649,  9661,  9677,  9679,  9689,  9697,  9719,  9721,  9733,
 9739,  9743,  9749,  9767,  9769,  9781,  9787,  9791,  9803,  9811,  9817,
 9829,  9833,  9839,  9851,  9857,  9859,  9871,  9883,  9887,  9901,  9907,
 9923,  9929,  9931,  9941,  9949,  9967,  9973, 10007, 10009, 10037, 10039,
10061, 10067, 10069, 10079, 10091, 10093, 10099, 10103, 10111, 10133, 10139,
10141, 10151, 10159, 10163, 10169, 10177, 10181, 10193, 10211, 10223, 10243,
10247, 10253, 10259, 10267, 10271, 10273, 10289, 10301, 10303, 10313, 10321,
10331, 10333, 10337, 10343, 10357, 10369, 10391, 10399, 10427, 10429, 10433,
10453, 10457, 10459, 10463, 10477, 10487, 10499, 10501, 10513, 10529, 10531,
10559, 10567, 10589, 10597, 10601, 10607, 10613, 10627, 10631, 10639, 10651,
10657, 10663, 10667, 10687, 10691, 10709, 10711, 10723, 10729, 10733, 10739,
10753, 10771, 10781, 10789, 10799, 10831, 10837, 10847, 10853, 10859, 10861,
10867, 10883, 10889, 10891, 10903, 10909, 10937, 10939, 10949, 10957, 10973,
10979, 10987, 10993, 11003, 11027, 11047, 11057, 11059, 11069, 11071, 11083,
11087, 11093, 11113, 11117, 11119, 11131, 11149, 11159, 11161, 11171, 11173,
11177, 11197, 11213, 11239, 11243, 11251, 11257, 11261, 11273, 11279, 11287,
11299, 11311, 11317, 11321, 11329, 11351, 11353, 11369, 11383, 11393, 11399,
11411, 11423, 11437, 11443, 11447, 11467, 11471, 11483, 11489, 11491, 11497,
11503, 11519, 11527, 11549, 11551, 11579, 11587, 11593, 11597, 11617, 11621,
11633, 11657, 11677, 11681, 11689, 11699, 11701, 11717, 11719, 11731, 11743,
11777, 11779, 11783, 11789, 11801, 11807, 11813, 11821, 11827, 11831, 11833,
11839, 11863, 11867, 11887, 11897, 11903, 11909, 11923, 11927, 11933, 11939,
11941, 11953, 11959, 11969, 11971, 11981, 11987, 12007, 12011, 12037, 12041,
12043, 12049, 12071, 12073, 12097, 12101, 12107, 12109, 12113, 12119, 12143,
12149, 12157, 12161, 12163, 12197, 12203, 12211, 12227, 12239, 12241, 12251,
12253, 12263, 12269, 12277, 12281, 12289, 12301, 12323, 12329, 12343, 12347,
12373, 12377, 12379, 12391, 12401, 12409, 12413, 12421, 12433, 12437, 12451,
12457, 12473, 12479, 12487, 12491, 12497, 12503, 12511, 12517, 12527, 12539,
12541, 12547, 12553, 12569, 12577, 12583, 12589, 12601, 12611, 12613, 12619,
12637, 12641, 12647, 12653, 12659, 12671, 12689, 12697, 12703, 12713, 12721,
12739, 12743, 12757, 12763, 12781, 12791, 12799, 12809, 12821, 12823, 12829,
12841, 12853, 12889, 12893, 12899, 12907, 12911, 12917, 12919, 12923, 12941,
12953, 12959, 12967, 12973, 12979, 12983, 13001, 13003, 13007, 13009, 13033,
13037, 13043, 13049, 13063, 13093, 13099, 13103, 13109, 13121, 13127, 13147,
13151, 13159, 13163, 13171, 13177, 13183, 13187, 13217, 13219, 13229, 13241,
13249, 13259, 13267, 13291, 13297, 13309, 13313, 13327, 13331, 13337, 13339,
13367, 13381, 13397, 13399, 13411, 13417, 13421, 13441, 13451, 13457, 13463,
13469, 13477, 13487, 13499, 13513, 13523, 13537, 13553, 13567, 13577, 13591,
13597, 13613, 13619, 13627, 13633, 13649, 13669, 13679, 13681, 13687, 13691,
13693, 13697, 13709, 13711, 13721, 13723, 13729, 13751, 13757, 13759, 13763,
13781, 13789, 13799, 13807, 13829, 13831, 13841, 13859, 13873, 13877, 13879,
13883, 13901, 13903, 13907, 13913, 13921, 13931, 13933, 13963, 13967, 13997,
13999, 14009, 14011, 14029, 14033, 14051, 14057, 14071, 14081, 14083, 14087,
14107, 14143, 14149, 14153, 14159, 14173, 14177, 14197, 14207, 14221, 14243,
14249, 14251, 14281, 14293, 14303, 14321, 14323, 14327, 14341, 14347, 14369,
14387, 14389, 14401, 14407, 14411, 14419, 14423, 14431, 14437, 14447, 14449,
14461, 14479, 14489, 14503, 14519, 14533, 14537, 14543, 14549, 14551, 14557,
14561, 14563, 14591, 14593, 14621, 14627, 14629, 14633, 14639, 14653, 14657,
14669, 14683, 14699, 14713, 14717, 14723, 14731, 14737, 14741, 14747, 14753,
14759, 14767, 14771, 14779, 14783, 14797, 14813, 14821, 14827, 14831, 14843,
14851, 14867, 14869, 14879, 14887, 14891, 14897, 14923, 14929, 14939, 14947,
14951, 14957, 14969, 14983, 15013, 15017, 15031, 15053, 15061, 15073, 15077,
15083, 15091, 15101, 15107, 15121, 15131, 15137, 15139, 15149, 15161, 15173,
15187, 15193, 15199, 15217, 15227, 15233, 15241, 15259, 15263, 15269, 15271,
15277, 15287, 15289, 15299, 15307, 15313, 15319, 15329, 15331, 15349, 15359,
15361, 15373, 15377, 15383, 15391, 15401, 15413, 15427, 15439, 15443, 15451,
15461, 15467, 15473, 15493, 15497, 15511, 15527, 15541, 15551, 15559, 15569,
15581, 15583, 15601, 15607, 15619, 15629, 15641, 15643, 15647, 15649, 15661,
15667, 15671, 15679, 15683, 15727, 15731, 15733, 15737, 15739, 15749, 15761,
15767, 15773, 15787, 15791, 15797, 15803, 15809, 15817, 15823, 15859, 15877,
15881, 15887, 15889, 15901, 15907, 15913, 15919, 15923, 15937, 15959, 15971,
15973, 15991, 16001, 16007, 16033, 16057, 16061, 16063, 16067, 16069, 16073,
16087, 16091, 16097, 16103, 16111, 16127, 16139, 16141, 16183, 16187, 16189,
16193, 16217, 16223, 16229, 16231, 16249, 16253, 16267, 16273, 16301, 16319,
16333, 16339, 16349, 16361, 16363, 16369, 16381, 16411, 16417, 16421, 16427,
16433, 16447, 16451, 16453, 16477, 16481, 16487, 16493, 16519, 16529, 16547,
16553, 16561, 16567, 16573, 16603, 16607, 16619, 16631, 16633, 16649, 16651,
16657, 16661, 16673, 16691, 16693, 16699, 16703, 16729, 16741, 16747, 16759,
16763, 16787, 16811, 16823, 16829, 16831, 16843, 16871, 16879, 16883, 16889,
16901, 16903, 16921, 16927, 16931, 16937, 16943, 16963, 16979, 16981, 16987,
16993, 17011, 17021, 17027, 17029, 17033, 17041, 17047, 17053, 17077, 17093,
17099, 17107, 17117, 17123, 17137, 17159, 17167, 17183, 17189, 17191, 17203,
17207, 17209, 17231, 17239, 17257, 17291, 17293, 17299, 17317, 17321, 17327,
17333, 17341, 17351, 17359, 17377, 17383, 17387, 17389, 17393, 17401, 17417,
17419, 17431, 17443, 17449, 17467, 17471, 17477, 17483, 17489, 17491, 17497,
17509, 17519, 17539, 17551, 17569, 17573, 17579, 17581, 17597, 17599, 17609,
17623, 17627, 17657, 17659, 17669, 17681, 17683, 17707, 17713, 17729, 17737,
17747, 17749, 17761, 17783, 17789, 17791, 17807, 17827, 17837, 17839, 17851,
17863, 17881, 17891, 17903, 17909, 17911, 17921, 17923, 17929, 17939, 17957,
17959, 17971, 17977, 17981, 17987, 17989, 18013, 18041, 18043, 18047, 18049,
18059, 18061, 18077, 18089, 18097, 18119, 18121, 18127, 18131, 18133, 18143,
18149, 18169, 18181, 18191, 18199, 18211, 18217, 18223, 18229, 18233, 18251,
18253, 18257, 18269, 18287, 18289, 18301, 18307, 18311, 18313, 18329, 18341,
18353, 18367, 18371, 18379, 18397, 18401, 18413, 18427, 18433, 18439, 18443,
18451, 18457, 18461, 18481, 18493, 18503, 18517, 18521, 18523, 18539, 18541,
18553, 18583, 18587, 18593, 18617, 18637, 18661, 18671, 18679, 18691, 18701,
18713, 18719, 18731, 18743, 18749, 18757, 18773, 18787, 18793, 18797, 18803,
18839, 18859, 18869, 18899, 18911, 18913, 18917, 18919, 18947, 18959, 18973,
18979, 19001, 19009, 19013, 19031, 19037, 19051, 19069, 19073, 19079, 19081,
19087, 19121, 19139, 19141, 19157, 19163, 19181, 19183, 19207, 19211, 19213,
19219, 19231, 19237, 19249, 19259, 19267, 19273, 19289, 19301, 19309, 19319,
19333, 19373, 19379, 19381, 19387, 19391, 19403, 19417, 19421, 19423, 19427,
19429, 19433, 19441, 19447, 19457, 19463, 19469, 19471, 19477, 19483, 19489,
19501, 19507, 19531, 19541, 19543, 19553, 19559, 19571, 19577, 19583, 19597,
19603, 19609, 19661, 19681, 19687, 19697, 19699, 19709, 19717, 19727, 19739,
19751, 19753, 19759, 19763, 19777, 19793, 19801, 19813, 19819, 19841, 19843,
19853, 19861, 19867, 19889, 19891, 19913, 19919, 19927, 19937, 19949, 19961,
19963, 19973, 19979, 19991, 19993, 19997, 20011, 20021, 20023, 20029, 20047,
20051, 20063, 20071, 20089, 20101, 20107, 20113, 20117, 20123, 20129, 20143,
20147, 20149, 20161, 20173, 20177, 20183, 20201, 20219, 20231, 20233, 20249,
20261, 20269, 20287, 20297, 20323, 20327, 20333, 20341, 20347, 20353, 20357,
20359, 20369, 20389, 20393, 20399, 20407, 20411, 20431, 20441, 20443, 20477,
20479, 20483, 20507, 20509, 20521, 20533, 20543, 20549, 20551, 20563, 20593,
20599, 20611, 20627, 20639, 20641, 20663, 20681, 20693, 20707, 20717, 20719,
20731, 20743, 20747, 20749, 20753, 20759, 20771, 20773, 20789, 20807, 20809,
20849, 20857, 20873, 20879, 20887, 20897, 20899, 20903, 20921, 20929, 20939,
20947, 20959, 20963, 20981, 20983, 21001, 21011, 21013, 21017, 21019, 21023,
21031, 21059, 21061, 21067, 21089, 21101, 21107, 21121, 21139, 21143, 21149,
21157, 21163, 21169, 21179, 21187, 21191, 21193, 21211, 21221, 21227, 21247,
21269, 21277, 21283, 21313, 21317, 21319, 21323, 21341, 21347, 21377, 21379,
21383, 21391, 21397, 21401, 21407, 21419, 21433, 21467, 21481, 21487, 21491,
21493, 21499, 21503, 21517, 21521, 21523, 21529, 21557, 21559, 21563, 21569,
21577, 21587, 21589, 21599, 21601, 21611, 21613, 21617, 21647, 21649, 21661,
21673, 21683, 21701, 21713, 21727, 21737, 21739, 21751, 21757, 21767, 21773,
21787, 21799, 21803, 21817, 21821, 21839, 21841, 21851, 21859, 21863, 21871,
21881, 21893, 21911, 21929, 21937, 21943, 21961, 21977, 21991, 21997, 22003,
22013, 22027, 22031, 22037, 22039, 22051, 22063, 22067, 22073, 22079, 22091,
22093, 22109, 22111, 22123, 22129, 22133, 22147, 22153, 22157, 22159, 22171,
22189, 22193, 22229, 22247, 22259, 22271, 22273, 22277, 22279, 22283, 22291,
22303, 22307, 22343, 22349, 22367, 22369, 22381, 22391, 22397, 22409, 22433,
22441, 22447, 22453, 22469, 22481, 22483, 22501, 22511, 22531, 22541, 22543,
22549, 22567, 22571, 22573, 22613, 22619, 22621, 22637, 22639, 22643, 22651,
22669, 22679, 22691, 22697, 22699, 22709, 22717, 22721, 22727, 22739, 22741,
22751, 22769, 22777, 22783, 22787, 22807, 22811, 22817, 22853, 22859, 22861,
22871, 22877, 22901, 22907, 22921, 22937, 22943, 22961, 22963, 22973, 22993,
23003, 23011, 23017, 23021, 23027, 23029, 23039, 23041, 23053, 23057, 23059,
23063, 23071, 23081, 23087, 23099, 23117, 23131, 23143, 23159, 23167, 23173,
23189, 23197, 23201, 23203, 23209, 23227, 23251, 23269, 23279, 23291, 23293,
23297, 23311, 23321, 23327, 23333, 23339, 23357, 23369, 23371, 23399, 23417,
23431, 23447, 23459, 23473, 23497, 23509, 23531, 23537, 23539, 23549, 23557,
23561, 23563, 23567, 23581, 23593, 23599, 23603, 23609, 23623, 23627, 23629,
23633, 23663, 23669, 23671, 23677, 23687, 23689, 23719, 23741, 23743, 23747,
23753, 23761, 23767, 23773, 23789, 23801, 23813, 23819, 23827, 23831, 23833,
23857, 23869, 23873, 23879, 23887, 23893, 23899, 23909, 23911, 23917, 23929,
23957, 23971, 23977, 23981, 23993, 24001, 24007, 24019, 24023, 24029, 24043,
24049, 24061, 24071, 24077, 24083, 24091, 24097, 24103, 24107, 24109, 24113,
24121, 24133, 24137, 24151, 24169, 24179, 24181, 24197, 24203, 24223, 24229,
24239, 24247, 24251, 24281, 24317, 24329, 24337, 24359, 24371, 24373, 24379,
24391, 24407, 24413, 24419, 24421, 24439, 24443, 24469, 24473, 24481, 24499,
24509, 24517, 24527, 24533, 24547, 24551, 24571, 24593, 24611, 24623, 24631,
24659, 24671, 24677, 24683, 24691, 24697, 24709, 24733, 24749, 24763, 24767,
24781, 24793, 24799, 24809, 24821, 24841, 24847, 24851, 24859, 24877, 24889,
24907, 24917, 24919, 24923, 24943, 24953, 24967, 24971, 24977, 24979, 24989,
25013, 25031, 25033, 25037, 25057, 25073, 25087, 25097, 25111, 25117, 25121,
25127, 25147, 25153, 25163, 25169, 25171, 25183, 25189, 25219, 25229, 25237,
25243, 25247, 25253, 25261, 25301, 25303, 25307, 25309, 25321, 25339, 25343,
25349, 25357, 25367, 25373, 25391, 25409, 25411, 25423, 25439, 25447, 25453,
25457, 25463, 25469, 25471, 25523, 25537, 25541, 25561, 25577, 25579, 25583,
25589, 25601, 25603, 25609, 25621, 25633, 25639, 25643, 25657, 25667, 25673,
25679, 25693, 25703, 25717, 25733, 25741, 25747, 25759, 25763, 25771, 25793,
25799, 25801, 25819, 25841, 25847, 25849, 25867, 25873, 25889, 25903, 25913,
25919, 25931, 25933, 25939, 25943, 25951, 25969, 25981, 25997, 25999, 26003,
26017, 26021, 26029, 26041, 26053, 26083, 26099, 26107, 26111, 26113, 26119,
26141, 26153, 26161, 26171, 26177, 26183, 26189, 26203, 26209, 26227, 26237,
26249, 26251, 26261, 26263, 26267, 26293, 26297, 26309, 26317, 26321, 26339,
26347, 26357, 26371, 26387, 26393, 26399, 26407, 26417, 26423, 26431, 26437,
26449, 26459, 26479, 26489, 26497, 26501, 26513, 26539, 26557, 26561, 26573,
26591, 26597, 26627, 26633, 26641, 26647, 26669, 26681, 26683, 26687, 26693,
26699, 26701, 26711, 26713, 26717, 26723, 26729, 26731, 26737, 26759, 26777,
26783, 26801, 26813, 26821, 26833, 26839, 26849, 26861, 26863, 26879, 26881,
26891, 26893, 26903, 26921, 26927, 26947, 26951, 26953, 26959, 26981, 26987,
26993, 27011, 27017, 27031, 27043, 27059, 27061, 27067, 27073, 27077, 27091,
27103, 27107, 27109, 27127, 27143, 27179, 27191, 27197, 27211, 27239, 27241,
27253, 27259, 27271, 27277, 27281, 27283, 27299, 27329, 27337, 27361, 27367,
27397, 27407, 27409, 27427, 27431, 27437, 27449, 27457, 27479, 27481, 27487,
27509, 27527, 27529, 27539, 27541, 27551, 27581, 27583, 27611, 27617, 27631,
27647, 27653, 27673, 27689, 27691, 27697, 27701, 27733, 27737, 27739, 27743,
27749, 27751, 27763, 27767, 27773, 27779, 27791, 27793, 27799, 27803, 27809,
27817, 27823, 27827, 27847, 27851, 27883, 27893, 27901, 27917, 27919, 27941,
27943, 27947, 27953, 27961, 27967, 27983, 27997, 28001, 28019, 28027, 28031,
28051, 28057, 28069, 28081, 28087, 28097, 28099, 28109, 28111, 28123, 28151,
28163, 28181, 28183, 28201, 28211, 28219, 28229, 28277, 28279, 28283, 28289,
28297, 28307, 28309, 28319, 28349, 28351, 28387, 28393, 28403, 28409, 28411,
28429, 28433, 28439, 28447, 28463, 28477, 28493, 28499, 28513, 28517, 28537,
28541, 28547, 28549, 28559, 28571, 28573, 28579, 28591, 28597, 28603, 28607,
28619, 28621, 28627, 28631, 28643, 28649, 28657, 28661, 28663, 28669, 28687,
28697, 28703, 28711, 28723, 28729, 28751, 28753, 28759, 28771, 28789, 28793,
28807, 28813, 28817, 28837, 28843, 28859, 28867, 28871, 28879, 28901, 28909,
28921, 28927, 28933, 28949, 28961, 28979, 29009, 29017, 29021, 29023, 29027,
29033, 29059, 29063, 29077, 29101, 29123, 29129, 29131, 29137, 29147, 29153,
29167, 29173, 29179, 29191, 29201, 29207, 29209, 29221, 29231, 29243, 29251,
29269, 29287, 29297, 29303, 29311, 29327, 29333, 29339, 29347, 29363, 29383,
29387, 29389, 29399, 29401, 29411, 29423, 29429, 29437, 29443, 29453, 29473,
29483, 29501, 29527, 29531, 29537, 29567, 29569, 29573, 29581, 29587, 29599,
29611, 29629, 29633, 29641, 29663, 29669, 29671, 29683, 29717, 29723, 29741,
29753, 29759, 29761, 29789, 29803, 29819, 29833, 29837, 29851, 29863, 29867,
29873, 29879, 29881, 29917, 29921, 29927, 29947, 29959, 29983, 29989, 30011,
30013, 30029, 30047, 30059, 30071, 30089, 30091, 30097, 30103, 30109, 30113,
30119, 30133, 30137, 30139, 30161, 30169, 30181, 30187, 30197, 30203, 30211,
30223, 30241, 30253, 30259, 30269, 30271, 30293, 30307, 30313, 30319, 30323,
30341, 30347, 30367, 30389, 30391, 30403, 30427, 30431, 30449, 30467, 30469,
30491, 30493, 30497, 30509, 30517, 30529, 30539, 30553, 30557, 30559, 30577,
30593, 30631, 30637, 30643, 30649, 30661, 30671, 30677, 30689, 30697, 30703,
30707, 30713, 30727, 30757, 30763, 30773, 30781, 30803, 30809, 30817, 30829,
30839, 30841, 30851, 30853, 30859, 30869, 30871, 30881, 30893, 30911, 30931,
30937, 30941, 30949, 30971, 30977, 30983, 31013, 31019, 31033, 31039, 31051,
31063, 31069, 31079, 31081, 31091, 31121, 31123, 31139, 31147, 31151, 31153,
31159, 31177, 31181, 31183, 31189, 31193, 31219, 31223, 31231, 31237, 31247,
31249, 31253, 31259, 31267, 31271, 31277, 31307, 31319, 31321, 31327, 31333,
31337, 31357, 31379, 31387, 31391, 31393, 31397, 31469, 31477, 31481, 31489,
31511, 31513, 31517, 31531, 31541, 31543, 31547, 31567, 31573, 31583, 31601,
31607, 31627, 31643, 31649, 31657, 31663, 31667, 31687, 31699, 31721, 31723,
31727, 31729, 31741, 31751, 31769, 31771, 31793, 31799, 31817, 31847, 31849,
31859, 31873, 31883, 31891, 31907, 31957, 31963, 31973, 31981, 31991, 32003,
32009, 32027, 32029, 32051, 32057, 32059, 32063, 32069, 32077, 32083, 32089,
32099, 32117, 32119, 32141, 32143, 32159, 32173, 32183, 32189, 32191, 32203,
32213, 32233, 32237, 32251, 32257, 32261, 32297, 32299, 32303, 32309, 32321,
32323, 32327, 32341, 32353, 32359, 32363, 32369, 32371, 32377, 32381, 32401,
32411, 32413, 32423, 32429, 32441, 32443, 32467, 32479, 32491, 32497, 32503,
32507, 32531, 32533, 32537, 32561, 32563, 32569, 32573, 32579, 32587, 32603,
32609, 32611, 32621, 32633, 32647, 32653, 32687, 32693, 32707, 32713, 32717,
32719, 32749, 32771, 32779, 32783, 32789, 32797, 32801, 32803, 32831, 32833,
32839, 32843, 32869, 32887, 32909, 32911, 32917, 32933, 32939, 32941, 32957,
32969, 32971, 32983, 32987, 32993, 32999, 33013, 33023, 33029, 33037, 33049,
33053, 33071, 33073, 33083, 33091, 33107, 33113, 33119, 33149, 33151, 33161,
33179, 33181, 33191, 33199, 33203, 33211, 33223, 33247, 33287, 33289, 33301,
33311, 33317, 33329, 33331, 33343, 33347, 33349, 33353, 33359, 33377, 33391,
33403, 33409, 33413, 33427, 33457, 33461, 33469, 33479, 33487, 33493, 33503,
33521, 33529, 33533, 33547, 33563, 33569, 33577, 33581, 33587, 33589, 33599,
33601, 33613, 33617, 33619, 33623, 33629, 33637, 33641, 33647, 33679, 33703,
33713, 33721, 33739, 33749, 33751, 33757, 33767, 33769, 33773, 33791, 33797,
33809, 33811, 33827, 33829, 33851, 33857, 33863, 33871, 33889, 33893, 33911,
33923, 33931, 33937, 33941, 33961, 33967, 33997, 34019, 34031, 34033, 34039,
34057, 34061, 34123, 34127, 34129, 34141, 34147, 34157, 34159, 34171, 34183,
34211, 34213, 34217, 34231, 34253, 34259, 34261, 34267, 34273, 34283, 34297,
34301, 34303, 34313, 34319, 34327, 34337, 34351, 34361, 34367, 34369, 34381,
34403, 34421, 34429, 34439, 34457, 34469, 34471, 34483, 34487, 34499, 34501,
34511, 34513, 34519, 34537, 34543, 34549, 34583, 34589, 34591, 34603, 34607,
34613, 34631, 34649, 34651, 34667, 34673, 34679, 34687, 34693, 34703, 34721,
34729, 34739, 34747, 34757, 34759, 34763, 34781, 34807, 34819, 34841, 34843,
34847, 34849, 34871, 34877, 34883, 34897, 34913, 34919, 34939, 34949, 34961,
34963, 34981, 35023, 35027, 35051, 35053, 35059, 35069, 35081, 35083, 35089,
35099, 35107, 35111, 35117, 35129, 35141, 35149, 35153, 35159, 35171, 35201,
35221, 35227, 35251, 35257, 35267, 35279, 35281, 35291, 35311, 35317, 35323,
35327, 35339, 35353, 35363, 35381, 35393, 35401, 35407, 35419, 35423, 35437,
35447, 35449, 35461, 35491, 35507, 35509, 35521, 35527, 35531, 35533, 35537,
35543, 35569, 35573, 35591, 35593, 35597, 35603, 35617, 35671, 35677, 35729,
35731, 35747, 35753, 35759, 35771, 35797, 35801, 35803, 35809, 35831, 35837,
35839, 35851, 35863, 35869, 35879, 35897, 35899, 35911, 35923, 35933, 35951,
35963, 35969, 35977, 35983, 35993, 35999, 36007, 36011, 36013, 36017, 36037,
36061, 36067, 36073, 36083, 36097, 36107, 36109, 36131, 36137, 36151, 36161,
36187, 36191, 36209, 36217, 36229, 36241, 36251, 36263, 36269, 36277, 36293,
36299, 36307, 36313, 36319, 36341, 36343, 36353, 36373, 36383, 36389, 36433,
36451, 36457, 36467, 36469, 36473, 36479, 36493, 36497, 36523, 36527, 36529,
36541, 36551, 36559, 36563, 36571, 36583, 36587, 36599, 36607, 36629, 36637,
36643, 36653, 36671, 36677, 36683, 36691, 36697, 36709, 36713, 36721, 36739,
36749, 36761, 36767, 36779, 36781, 36787, 36791, 36793, 36809, 36821, 36833,
36847, 36857, 36871, 36877, 36887, 36899, 36901, 36913, 36919, 36923, 36929,
36931, 36943, 36947, 36973, 36979, 36997, 37003, 37013, 37019, 37021, 37039,
37049, 37057, 37061, 37087, 37097, 37117, 37123, 37139, 37159, 37171, 37181,
37189, 37199, 37201, 37217, 37223, 37243, 37253, 37273, 37277, 37307, 37309,
37313, 37321, 37337, 37339, 37357, 37361, 37363, 37369, 37379, 37397, 37409,
37423, 37441, 37447, 37463, 37483, 37489, 37493, 37501, 37507, 37511, 37517,
37529, 37537, 37547, 37549, 37561, 37567, 37571, 37573, 37579, 37589, 37591,
37607, 37619, 37633, 37643, 37649, 37657, 37663, 37691, 37693, 37699, 37717,
37747, 37781, 37783, 37799, 37811, 37813, 37831, 37847, 37853, 37861, 37871,
37879, 37889, 37897, 37907, 37951, 37957, 37963, 37967, 37987, 37991, 37993,
37997, 38011, 38039, 38047, 38053, 38069, 38083, 38113, 38119, 38149, 38153,
38167, 38177, 38183, 38189, 38197, 38201, 38219, 38231, 38237, 38239, 38261,
38273, 38281, 38287, 38299, 38303, 38317, 38321, 38327, 38329, 38333, 38351,
38371, 38377, 38393, 38431, 38447, 38449, 38453, 38459, 38461, 38501, 38543,
38557, 38561, 38567, 38569, 38593, 38603, 38609, 38611, 38629, 38639, 38651,
38653, 38669, 38671, 38677, 38693, 38699, 38707, 38711, 38713, 38723, 38729,
38737, 38747, 38749, 38767, 38783, 38791, 38803, 38821, 38833, 38839, 38851,
38861, 38867, 38873, 38891, 38903, 38917, 38921, 38923, 38933, 38953, 38959,
38971, 38977, 38993, 39019, 39023, 39041, 39043, 39047, 39079, 39089, 39097,
39103, 39107, 39113, 39119, 39133, 39139, 39157, 39161, 39163, 39181, 39191,
39199, 39209, 39217, 39227, 39229, 39233, 39239, 39241, 39251, 39293, 39301,
39313, 39317, 39323, 39341, 39343, 39359, 39367, 39371, 39373, 39383, 39397,
39409, 39419, 39439, 39443, 39451, 39461, 39499, 39503, 39509, 39511, 39521,
39541, 39551, 39563, 39569, 39581, 39607, 39619, 39623, 39631, 39659, 39667,
39671, 39679, 39703, 39709, 39719, 39727, 39733, 39749, 39761, 39769, 39779,
39791, 39799, 39821, 39827, 39829, 39839, 39841, 39847, 39857, 39863, 39869,
39877, 39883, 39887, 39901, 39929, 39937, 39953, 39971, 39979, 39983, 39989,
40009, 40013, 40031, 40037, 40039, 40063, 40087, 40093, 40099, 40111, 40123,
40127, 40129, 40151, 40153, 40163, 40169, 40177, 40189, 40193, 40213, 40231,
40237, 40241, 40253, 40277, 40283, 40289, 40343, 40351, 40357, 40361, 40387,
40423, 40427, 40429, 40433, 40459, 40471, 40483, 40487, 40493, 40499, 40507,
40519, 40529, 40531, 40543, 40559, 40577, 40583, 40591, 40597, 40609, 40627,
40637, 40639, 40693, 40697, 40699, 40709, 40739, 40751, 40759, 40763, 40771,
40787, 40801, 40813, 40819, 40823, 40829, 40841, 40847, 40849, 40853, 40867,
40879, 40883, 40897, 40903, 40927, 40933, 40939, 40949, 40961, 40973, 40993,
41011, 41017, 41023, 41039, 41047, 41051, 41057, 41077, 41081, 41113, 41117,
41131, 41141, 41143, 41149, 41161, 41177, 41179, 41183, 41189, 41201, 41203,
41213, 41221, 41227, 41231, 41233, 41243, 41257, 41263, 41269, 41281, 41299,
41333, 41341, 41351, 41357, 41381, 41387, 41389, 41399, 41411, 41413, 41443,
41453, 41467, 41479, 41491, 41507, 41513, 41519, 41521, 41539, 41543, 41549,
41579, 41593, 41597, 41603, 41609, 41611, 41617, 41621, 41627, 41641, 41647,
41651, 41659, 41669, 41681, 41687, 41719, 41729, 41737, 41759, 41761, 41771,
41777, 41801, 41809, 41813, 41843, 41849, 41851, 41863, 41879, 41887, 41893,
41897, 41903, 41911, 41927, 41941, 41947, 41953, 41957, 41959, 41969, 41981,
41983, 41999, 42013, 42017, 42019, 42023, 42043, 42061, 42071, 42073, 42083,
42089, 42101, 42131, 42139, 42157, 42169, 42179, 42181, 42187, 42193, 42197,
42209, 42221, 42223, 42227, 42239, 42257, 42281, 42283, 42293, 42299, 42307,
42323, 42331, 42337, 42349, 42359, 42373, 42379, 42391, 42397, 42403, 42407,
42409, 42433, 42437, 42443, 42451, 42457, 42461, 42463, 42467, 42473, 42487,
42491, 42499, 42509, 42533, 42557, 42569, 42571, 42577, 42589, 42611, 42641,
42643, 42649, 42667, 42677, 42683, 42689, 42697, 42701, 42703, 42709, 42719,
42727, 42737, 42743, 42751, 42767, 42773, 42787, 42793, 42797, 42821, 42829,
42839, 42841, 42853, 42859, 42863, 42899, 42901, 42923, 42929, 42937, 42943,
42953, 42961, 42967, 42979, 42989, 43003, 43013, 43019, 43037, 43049, 43051,
43063, 43067, 43093, 43103, 43117, 43133, 43151, 43159, 43177, 43189, 43201,
43207, 43223, 43237, 43261, 43271, 43283, 43291, 43313, 43319, 43321, 43331,
43391, 43397, 43399, 43403, 43411, 43427, 43441, 43451, 43457, 43481, 43487,
43499, 43517, 43541, 43543, 43573, 43577, 43579, 43591, 43597, 43607, 43609,
43613, 43627, 43633, 43649, 43651, 43661, 43669, 43691, 43711, 43717, 43721,
43753, 43759, 43777, 43781, 43783, 43787, 43789, 43793, 43801, 43853, 43867,
43889, 43891, 43913, 43933, 43943, 43951, 43961, 43963, 43969, 43973, 43987,
43991, 43997, 44017, 44021, 44027, 44029, 44041, 44053, 44059, 44071, 44087,
44089, 44101, 44111, 44119, 44123, 44129, 44131, 44159, 44171, 44179, 44189,
44201, 44203, 44207, 44221, 44249, 44257, 44263, 44267, 44269, 44273, 44279,
44281, 44293, 44351, 44357, 44371, 44381, 44383, 44389, 44417, 44449, 44453,
44483, 44491, 44497, 44501, 44507, 44519, 44531, 44533, 44537, 44543, 44549,
44563, 44579, 44587, 44617, 44621, 44623, 44633, 44641, 44647, 44651, 44657,
44683, 44687, 44699, 44701, 44711, 44729, 44741, 44753, 44771, 44773, 44777,
44789, 44797, 44809, 44819, 44839, 44843, 44851, 44867, 44879, 44887, 44893,
44909, 44917, 44927, 44939, 44953, 44959, 44963, 44971, 44983, 44987, 45007,
45013, 45053, 45061, 45077, 45083, 45119, 45121, 45127, 45131, 45137, 45139,
45161, 45179, 45181, 45191, 45197, 45233, 45247, 45259, 45263, 45281, 45289,
45293, 45307, 45317, 45319, 45329, 45337, 45341, 45343, 45361, 45377, 45389,
45403, 45413, 45427, 45433, 45439, 45481, 45491, 45497, 45503, 45523, 45533,
45541, 45553, 45557, 45569, 45587, 45589, 45599, 45613, 45631, 45641, 45659,
45667, 45673, 45677, 45691, 45697, 45707, 45737, 45751, 45757, 45763, 45767,
45779, 45817, 45821, 45823, 45827, 45833, 45841, 45853, 45863, 45869, 45887,
45893, 45943, 45949, 45953, 45959, 45971, 45979, 45989, 46021, 46027, 46049,
46051, 46061, 46073, 46091, 46093, 46099, 46103, 46133, 46141, 46147, 46153,
46171, 46181, 46183, 46187, 46199, 46219, 46229, 46237, 46261, 46271, 46273,
46279, 46301, 46307, 46309, 46327, 46337, 46349, 46351, 46381, 46399, 46411,
46439, 46441, 46447, 46451, 46457, 46471, 46477, 46489, 46499, 46507, 46511,
46523, 46549, 46559, 46567, 46573, 46589, 46591, 46601, 46619, 46633, 46639,
46643, 46649, 46663, 46679, 46681, 46687, 46691, 46703, 46723, 46727, 46747,
46751, 46757, 46769, 46771, 46807, 46811, 46817, 46819, 46829, 46831, 46853,
46861, 46867, 46877, 46889, 46901, 46919, 46933, 46957, 46993, 46997, 47017,
47041, 47051, 47057, 47059, 47087, 47093, 47111, 47119, 47123, 47129, 47137,
47143, 47147, 47149, 47161, 47189, 47207, 47221, 47237, 47251, 47269, 47279,
47287, 47293, 47297, 47303, 47309, 47317, 47339, 47351, 47353, 47363, 47381,
47387, 47389, 47407, 47417, 47419, 47431, 47441, 47459, 47491, 47497, 47501,
47507, 47513, 47521, 47527, 47533, 47543, 47563, 47569, 47581, 47591, 47599,
47609, 47623, 47629, 47639, 47653, 47657, 47659, 47681, 47699, 47701, 47711,
47713, 47717, 47737, 47741, 47743, 47777, 47779, 47791, 47797, 47807, 47809,
47819, 47837, 47843, 47857, 47869, 47881, 47903, 47911, 47917, 47933, 47939,
47947, 47951, 47963, 47969, 47977, 47981, 48017, 48023, 48029, 48049, 48073,
48079, 48091, 48109, 48119, 48121, 48131, 48157, 48163, 48179, 48187, 48193,
48197, 48221, 48239, 48247, 48259, 48271, 48281, 48299, 48311, 48313, 48337,
48341, 48353, 48371, 48383, 48397, 48407, 48409, 48413, 48437, 48449, 48463,
48473, 48479, 48481, 48487, 48491, 48497, 48523, 48527, 48533, 48539, 48541,
48563, 48571, 48589, 48593, 48611, 48619, 48623, 48647, 48649, 48661, 48673,
48677, 48679, 48731, 48733, 48751, 48757, 48761, 48767, 48779, 48781, 48787,
48799, 48809, 48817, 48821, 48823, 48847, 48857, 48859, 48869, 48871, 48883,
48889, 48907, 48947, 48953, 48973, 48989, 48991, 49003, 49009, 49019, 49031,
49033, 49037, 49043, 49057, 49069, 49081, 49103, 49109, 49117, 49121, 49123,
49139, 49157, 49169, 49171, 49177, 49193, 49199, 49201, 49207, 49211, 49223,
49253, 49261, 49277, 49279, 49297, 49307, 49331, 49333, 49339, 49363, 49367,
49369, 49391, 49393, 49409, 49411, 49417, 49429, 49433, 49451, 49459, 49463,
49477, 49481, 49499, 49523, 49529, 49531, 49537, 49547, 49549, 49559, 49597,
49603, 49613, 49627, 49633, 49639, 49663, 49667, 49669, 49681, 49697, 49711,
49727, 49739, 49741, 49747, 49757, 49783, 49787, 49789, 49801, 49807, 49811,
49823, 49831, 49843, 49853, 49871, 49877, 49891, 49919, 49921, 49927, 49937,
49939, 49943, 49957, 49991, 49993, 49999, 50021, 50023, 50033, 50047, 50051,
50053, 50069, 50077, 50087, 50093, 50101, 50111, 50119, 50123, 50129, 50131,
50147, 50153, 50159, 50177, 50207, 50221, 50227, 50231, 50261, 50263, 50273,
50287, 50291, 50311, 50321, 50329, 50333, 50341, 50359, 50363, 50377, 50383,
50387, 50411, 50417, 50423, 50441, 50459, 50461, 50497, 50503, 50513, 50527,
50539, 50543, 50549, 50551, 50581, 50587, 50591, 50593, 50599, 50627, 50647,
50651, 50671, 50683, 50707, 50723, 50741, 50753, 50767, 50773, 50777, 50789,
50821, 50833, 50839, 50849, 50857, 50867, 50873, 50891, 50893, 50909, 50923,
50929, 50951, 50957, 50969, 50971, 50989, 50993, 51001, 51031, 51043, 51047,
51059, 51061, 51071, 51109, 51131, 51133, 51137, 51151, 51157, 51169, 51193,
51197, 51199, 51203, 51217, 51229, 51239, 51241, 51257, 51263, 51283, 51287,
51307, 51329, 51341, 51343, 51347, 51349, 51361, 51383, 51407, 51413, 51419,
51421, 51427, 51431, 51437, 51439, 51449, 51461, 51473, 51479, 51481, 51487,
51503, 51511, 51517, 51521, 51539, 51551, 51563, 51577, 51581, 51593, 51599,
51607, 51613, 51631, 51637, 51647, 51659, 51673, 51679, 51683, 51691, 51713,
51719, 51721, 51749, 51767, 51769, 51787, 51797, 51803, 51817, 51827, 51829,
51839, 51853, 51859, 51869, 51871, 51893, 51899, 51907, 51913, 51929, 51941,
51949, 51971, 51973, 51977, 51991, 52009, 52021, 52027, 52051, 52057, 52067,
52069, 52081, 52103, 52121, 52127, 52147, 52153, 52163, 52177, 52181, 52183,
52189, 52201, 52223, 52237, 52249, 52253, 52259, 52267, 52289, 52291, 52301,
52313, 52321, 52361, 52363, 52369, 52379, 52387, 52391, 52433, 52453, 52457,
52489, 52501, 52511, 52517, 52529, 52541, 52543, 52553, 52561, 52567, 52571,
52579, 52583, 52609, 52627, 52631, 52639, 52667, 52673, 52691, 52697, 52709,
52711, 52721, 52727, 52733, 52747, 52757, 52769, 52783, 52807, 52813, 52817,
52837, 52859, 52861, 52879, 52883, 52889, 52901, 52903, 52919, 52937, 52951,
52957, 52963, 52967, 52973, 52981, 52999, 53003, 53017, 53047, 53051, 53069,
53077, 53087, 53089, 53093, 53101, 53113, 53117, 53129, 53147, 53149, 53161,
53171, 53173, 53189, 53197, 53201, 53231, 53233, 53239, 53267, 53269, 53279,
53281, 53299, 53309, 53323, 53327, 53353, 53359, 53377, 53381, 53401, 53407,
53411, 53419, 53437, 53441, 53453, 53479, 53503, 53507, 53527, 53549, 53551,
53569, 53591, 53593, 53597, 53609, 53611, 53617, 53623, 53629, 53633, 53639,
53653, 53657, 53681, 53693, 53699, 53717, 53719, 53731, 53759, 53773, 53777,
53783, 53791, 53813, 53819, 53831, 53849, 53857, 53861, 53881, 53887, 53891,
53897, 53899, 53917, 53923, 53927, 53939, 53951, 53959, 53987, 53993, 54001,
54011, 54013, 54037, 54049, 54059, 54083, 54091, 54101, 54121, 54133, 54139,
54151, 54163, 54167, 54181, 54193, 54217, 54251, 54269, 54277, 54287, 54293,
54311, 54319, 54323, 54331, 54347, 54361, 54367, 54371, 54377, 54401, 54403,
54409, 54413, 54419, 54421, 54437, 54443, 54449, 54469, 54493, 54497, 54499,
54503, 54517, 54521, 54539, 54541, 54547, 54559, 54563, 54577, 54581, 54583,
54601, 54617, 54623, 54629, 54631, 54647, 54667, 54673, 54679, 54709, 54713,
54721, 54727, 54751, 54767, 54773, 54779, 54787, 54799, 54829, 54833, 54851,
54869, 54877, 54881, 54907, 54917, 54919, 54941, 54949, 54959, 54973, 54979,
54983, 55001, 55009, 55021, 55049, 55051, 55057, 55061, 55073, 55079, 55103,
55109, 55117, 55127, 55147, 55163, 55171, 55201, 55207, 55213, 55217, 55219,
55229, 55243, 55249, 55259, 55291, 55313, 55331, 55333, 55337, 55339, 55343,
55351, 55373, 55381, 55399, 55411, 55439, 55441, 55457, 55469, 55487, 55501,
55511, 55529, 55541, 55547, 55579, 55589, 55603, 55609, 55619, 55621, 55631,
55633, 55639, 55661, 55663, 55667, 55673, 55681, 55691, 55697, 55711, 55717,
55721, 55733, 55763, 55787, 55793, 55799, 55807, 55813, 55817, 55819, 55823,
55829, 55837, 55843, 55849, 55871, 55889, 55897, 55901, 55903, 55921, 55927,
55931, 55933, 55949, 55967, 55987, 55997, 56003, 56009, 56039, 56041, 56053,
56081, 56087, 56093, 56099, 56101, 56113, 56123, 56131, 56149, 56167, 56171,
56179, 56197, 56207, 56209, 56237, 56239, 56249, 56263, 56267, 56269, 56299,
56311, 56333, 56359, 56369, 56377, 56383, 56393, 56401, 56417, 56431, 56437,
56443, 56453, 56467, 56473, 56477, 56479, 56489, 56501, 56503, 56509, 56519,
56527, 56531, 56533, 56543, 56569, 56591, 56597, 56599, 56611, 56629, 56633,
56659, 56663, 56671, 56681, 56687, 56701, 56711, 56713, 56731, 56737, 56747,
56767, 56773, 56779, 56783, 56807, 56809, 56813, 56821, 56827, 56843, 56857,
56873, 56891, 56893, 56897, 56909, 56911, 56921, 56923, 56929, 56941, 56951,
56957, 56963, 56983, 56989, 56993, 56999, 57037, 57041, 57047, 57059, 57073,
57077, 57089, 57097, 57107, 57119, 57131, 57139, 57143, 57149, 57163, 57173,
57179, 57191, 57193, 57203, 57221, 57223, 57241, 57251, 57259, 57269, 57271,
57283, 57287, 57301, 57329, 57331, 57347, 57349, 57367, 57373, 57383, 57389,
57397, 57413, 57427, 57457, 57467, 57487, 57493, 57503, 57527, 57529, 57557,
57559, 57571, 57587, 57593, 57601, 57637, 57641, 57649, 57653, 57667, 57679,
57689, 57697, 57709, 57713, 57719, 57727, 57731, 57737, 57751, 57773, 57781,
57787, 57791, 57793, 57803, 57809, 57829, 57839, 57847, 57853, 57859, 57881,
57899, 57901, 57917, 57923, 57943, 57947, 57973, 57977, 57991, 58013, 58027,
58031, 58043, 58049, 58057, 58061, 58067, 58073, 58099, 58109, 58111, 58129,
58147, 58151, 58153, 58169, 58171, 58189, 58193, 58199, 58207, 58211, 58217,
58229, 58231, 58237, 58243, 58271, 58309, 58313, 58321, 58337, 58363, 58367,
58369, 58379, 58391, 58393, 58403, 58411, 58417, 58427, 58439, 58441, 58451,
58453, 58477, 58481, 58511, 58537, 58543, 58549, 58567, 58573, 58579, 58601,
58603, 58613, 58631, 58657, 58661, 58679, 58687, 58693, 58699, 58711, 58727,
58733, 58741, 58757, 58763, 58771, 58787, 58789, 58831, 58889, 58897, 58901,
58907, 58909, 58913, 58921, 58937, 58943, 58963, 58967, 58979, 58991, 58997,
59009, 59011, 59021, 59023, 59029, 59051, 59053, 59063, 59069, 59077, 59083,
59093, 59107, 59113, 59119, 59123, 59141, 59149, 59159, 59167, 59183, 59197,
59207, 59209, 59219, 59221, 59233, 59239, 59243, 59263, 59273, 59281, 59333,
59341, 59351, 59357, 59359, 59369, 59377, 59387, 59393, 59399, 59407, 59417,
59419, 59441, 59443, 59447, 59453, 59467, 59471, 59473, 59497, 59509, 59513,
59539, 59557, 59561, 59567, 59581, 59611, 59617, 59621, 59627, 59629, 59651,
59659, 59663, 59669, 59671, 59693, 59699, 59707, 59723, 59729, 59743, 59747,
59753, 59771, 59779, 59791, 59797, 59809, 59833, 59863, 59879, 59887, 59921,
59929, 59951, 59957, 59971, 59981, 59999, 60013, 60017, 60029, 60037, 60041,
60077, 60083, 60089, 60091, 60101, 60103, 60107, 60127, 60133, 60139, 60149,
60161, 60167, 60169, 60209, 60217, 60223, 60251, 60257, 60259, 60271, 60289,
60293, 60317, 60331, 60337, 60343, 60353, 60373, 60383, 60397, 60413, 60427,
60443, 60449, 60457, 60493, 60497, 60509, 60521, 60527, 60539, 60589, 60601,
60607, 60611, 60617, 60623, 60631, 60637, 60647, 60649, 60659, 60661, 60679,
60689, 60703, 60719, 60727, 60733, 60737, 60757, 60761, 60763, 60773, 60779,
60793, 60811, 60821, 60859, 60869, 60887, 60889, 60899, 60901, 60913, 60917,
60919, 60923, 60937, 60943, 60953, 60961, 61001, 61007, 61027, 61031, 61043,
61051, 61057, 61091, 61099, 61121, 61129, 61141, 61151, 61153, 61169, 61211,
61223, 61231, 61253, 61261, 61283, 61291, 61297, 61331, 61333, 61339, 61343,
61357, 61363, 61379, 61381, 61403, 61409, 61417, 61441, 61463, 61469, 61471,
61483, 61487, 61493, 61507, 61511, 61519, 61543, 61547, 61553, 61559, 61561,
61583, 61603, 61609, 61613, 61627, 61631, 61637, 61643, 61651, 61657, 61667,
61673, 61681, 61687, 61703, 61717, 61723, 61729, 61751, 61757, 61781, 61813,
61819, 61837, 61843, 61861, 61871, 61879, 61909, 61927, 61933, 61949, 61961,
61967, 61979, 61981, 61987, 61991, 62003, 62011, 62017, 62039, 62047, 62053,
62057, 62071, 62081, 62099, 62119, 62129, 62131, 62137, 62141, 62143, 62171,
62189, 62191, 62201, 62207, 62213, 62219, 62233, 62273, 62297, 62299, 62303,
62311, 62323, 62327, 62347, 62351, 62383, 62401, 62417, 62423, 62459, 62467,
62473, 62477, 62483, 62497, 62501, 62507, 62533, 62539, 62549, 62563, 62581,
62591, 62597, 62603, 62617, 62627, 62633, 62639, 62653, 62659, 62683, 62687,
62701, 62723, 62731, 62743, 62753, 62761, 62773, 62791, 62801, 62819, 62827,
62851, 62861, 62869, 62873, 62897, 62903, 62921, 62927, 62929, 62939, 62969,
62971, 62981, 62983, 62987, 62989, 63029, 63031, 63059, 63067, 63073, 63079,
63097, 63103, 63113, 63127, 63131, 63149, 63179, 63197, 63199, 63211, 63241,
63247, 63277, 63281, 63299, 63311, 63313, 63317, 63331, 63337, 63347, 63353,
63361, 63367, 63377, 63389, 63391, 63397, 63409, 63419, 63421, 63439, 63443,
63463, 63467, 63473, 63487, 63493, 63499, 63521, 63527, 63533, 63541, 63559,
63577, 63587, 63589, 63599, 63601, 63607, 63611, 63617, 63629, 63647, 63649,
63659, 63667, 63671, 63689, 63691, 63697, 63703, 63709, 63719, 63727, 63737,
63743, 63761, 63773, 63781, 63793, 63799, 63803, 63809, 63823, 63839, 63841,
63853, 63857, 63863, 63901, 63907, 63913, 63929, 63949, 63977, 63997, 64007,
64013, 64019, 64033, 64037, 64063, 64067, 64081, 64091, 64109, 64123, 64151,
64153, 64157, 64171, 64187, 64189, 64217, 64223, 64231, 64237, 64271, 64279,
64283, 64301, 64303, 64319, 64327, 64333, 64373, 64381, 64399, 64403, 64433,
64439, 64451, 64453, 64483, 64489, 64499, 64513, 64553, 64567, 64577, 64579,
64591, 64601, 64609, 64613, 64621, 64627, 64633, 64661, 64663, 64667, 64679,
64693, 64709, 64717, 64747, 64763, 64781, 64783, 64793, 64811, 64817, 64849,
64853, 64871, 64877, 64879, 64891, 64901, 64919, 64921, 64927, 64937, 64951,
64969, 64997, 65003, 65011, 65027, 65029, 65033, 65053, 65063, 65071, 65089,
65099, 65101, 65111, 65119, 65123, 65129, 65141, 65147, 65167, 65171, 65173,
65179, 65183, 65203, 65213, 65239, 65257, 65267, 65269, 65287, 65293, 65309,
65323, 65327, 65353, 65357, 65371, 65381, 65393, 65407, 65413, 65419, 65423,
65437, 65447, 65449, 65479, 65497, 65519, 65521, 0 };

}
/*
* Modular Reducer
* (C) 1999-2011 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Modular_Reducer Constructor
*/
Modular_Reducer::Modular_Reducer(const BigInt& mod)
   {
   if(mod <= 0)
      throw Invalid_Argument("Modular_Reducer: modulus must be positive");

   modulus = mod;
   mod_words = modulus.sig_words();

   modulus_2 = Botan::square(modulus);

   mu = BigInt::power_of_2(2 * MP_WORD_BITS * mod_words) / modulus;
   }

/*
* Barrett Reduction
*/
BigInt Modular_Reducer::reduce(const BigInt& x) const
   {
   if(mod_words == 0)
      throw Invalid_State("Modular_Reducer: Never initalized");

   if(x.cmp(modulus, false) < 0)
      {
      if(x.is_negative())
         return x + modulus; // make positive
      return x;
      }
   else if(x.cmp(modulus_2, false) < 0)
      {
      BigInt t1 = x;
      t1.set_sign(BigInt::Positive);
      t1 >>= (MP_WORD_BITS * (mod_words - 1));
      t1 *= mu;

      t1 >>= (MP_WORD_BITS * (mod_words + 1));
      t1 *= modulus;

      t1.mask_bits(MP_WORD_BITS * (mod_words + 1));

      BigInt t2 = x;
      t2.set_sign(BigInt::Positive);
      t2.mask_bits(MP_WORD_BITS * (mod_words + 1));

      t2 -= t1;

      if(t2.is_negative())
         {
         t2 += BigInt::power_of_2(MP_WORD_BITS * (mod_words + 1));
         }

      while(t2 >= modulus)
         t2 -= modulus;

      if(x.is_positive())
         return t2;
      else
         return (modulus - t2);
      }
   else
      {
      // too big, fall back to normal division
      return (x % modulus);
      }
   }

}
/*
* Shanks-Tonnelli (RESSOL)
* (C) 2007-2008 Falko Strenzke, FlexSecure GmbH
* (C) 2008 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Shanks-Tonnelli algorithm
*/
BigInt ressol(const BigInt& a, const BigInt& p)
   {
   if(a < 0)
      throw Invalid_Argument("ressol(): a to solve for must be positive");
   if(p <= 1)
      throw Invalid_Argument("ressol(): prime must be > 1");

   if(a == 0)
      return 0;
   if(p == 2)
      return a;

   if(jacobi(a, p) != 1) // not a quadratic residue
      return -BigInt(1);

   if(p % 4 == 3)
      return power_mod(a, ((p+1) >> 2), p);

   size_t s = low_zero_bits(p - 1);
   BigInt q = p >> s;

   q -= 1;
   q >>= 1;

   Modular_Reducer mod_p(p);

   BigInt r = power_mod(a, q, p);
   BigInt n = mod_p.multiply(a, mod_p.square(r));
   r = mod_p.multiply(r, a);

   if(n == 1)
      return r;

   // find random non quadratic residue z
   BigInt z = 2;
   while(jacobi(z, p) == 1) // while z quadratic residue
      ++z;

   BigInt c = power_mod(z, (q << 1) + 1, p);

   while(n > 1)
      {
      q = n;

      size_t i = 0;
      while(q != 1)
         {
         q = mod_p.square(q);
         ++i;
         }

      if(s <= i)
         return -BigInt(1);

      c = power_mod(c, BigInt::power_of_2(s-i-1), p);
      r = mod_p.multiply(r, c);
      c = mod_p.square(c);
      n = mod_p.multiply(n, c);
      s = i;
      }

   return r;
   }

}
/*
* OID Registry
* (C) 1999-2010,2013,2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace OIDS {

const char* default_oid_list()
   {
   return

      // Public key types
      "1.2.840.113549.1.1.1 = RSA" "\n"
      "2.5.8.1.1 = RSA" "\n"
      "1.2.840.10040.4.1 = DSA" "\n"
      "1.2.840.10046.2.1 = DH" "\n"
      "1.3.6.1.4.1.3029.1.2.1 = ElGamal" "\n"
      "1.3.6.1.4.1.25258.1.1 = RW" "\n"
      "1.3.6.1.4.1.25258.1.2 = NR" "\n"
      "1.3.6.1.4.1.25258.1.3 = McEliece" "\n"
      "1.3.6.1.4.1.25258.1.4 = Curve25519" "\n"

      // X9.62 ecPublicKey, valid for ECDSA and ECDH (RFC 3279 sec 2.3.5)
      "1.2.840.10045.2.1 = ECDSA" "\n"
      "1.3.132.1.12 = ECDH" "\n"

      "1.2.643.2.2.19 = GOST-34.10" "\n"

      // Block ciphers
      "1.3.14.3.2.7 = DES/CBC" "\n"
      "1.2.840.113549.3.7 = TripleDES/CBC" "\n"
      "1.2.840.113549.3.2 = RC2/CBC" "\n"
      "1.2.840.113533.7.66.10 = CAST-128/CBC" "\n"
      "2.16.840.1.101.3.4.1.2 = AES-128/CBC" "\n"
      "2.16.840.1.101.3.4.1.22 = AES-192/CBC" "\n"
      "2.16.840.1.101.3.4.1.42 = AES-256/CBC" "\n"
      "1.2.410.200004.1.4 = SEED/CBC" "\n"
      "1.3.6.1.4.1.25258.3.1 = Serpent/CBC" "\n"
      "1.3.6.1.4.1.25258.3.2 = Threefish-512/CBC" "\n"
      "1.3.6.1.4.1.25258.3.3 = Twofish/CBC" "\n"

      "2.16.840.1.101.3.4.1.6 = AES-128/GCM" "\n"
      "2.16.840.1.101.3.4.1.26 = AES-192/GCM" "\n"
      "2.16.840.1.101.3.4.1.46 = AES-256/GCM" "\n"

      "1.3.6.1.4.1.25258.3.101 = Serpent/GCM" "\n"
      "1.3.6.1.4.1.25258.3.102 = Twofish/GCM" "\n"

      "1.3.6.1.4.1.25258.3.2.1 = AES-128/OCB" "\n"
      "1.3.6.1.4.1.25258.3.2.2 = AES-192/OCB" "\n"
      "1.3.6.1.4.1.25258.3.2.3 = AES-256/OCB" "\n"
      "1.3.6.1.4.1.25258.3.2.4 = Serpent/OCB" "\n"
      "1.3.6.1.4.1.25258.3.2.5 = Twofish/OCB" "\n"

      // Hashes
      "1.2.840.113549.2.5 = MD5" "\n"
      "1.3.6.1.4.1.11591.12.2 = Tiger(24,3)" "\n"

      "1.3.14.3.2.26 = SHA-160" "\n"
      "2.16.840.1.101.3.4.2.4 = SHA-224" "\n"
      "2.16.840.1.101.3.4.2.1 = SHA-256" "\n"
      "2.16.840.1.101.3.4.2.2 = SHA-384" "\n"
      "2.16.840.1.101.3.4.2.3 = SHA-512" "\n"
      "2.16.840.1.101.3.4.2.6 = SHA-512-256" "\n"

      // MACs
      "1.2.840.113549.2.7 = HMAC(SHA-160)" "\n"
      "1.2.840.113549.2.8 = HMAC(SHA-224)" "\n"
      "1.2.840.113549.2.9 = HMAC(SHA-256)" "\n"
      "1.2.840.113549.2.10 = HMAC(SHA-384)" "\n"
      "1.2.840.113549.2.11 = HMAC(SHA-512)" "\n"

      // Keywrap
      "1.2.840.113549.1.9.16.3.6 = KeyWrap.TripleDES" "\n"
      "1.2.840.113549.1.9.16.3.7 = KeyWrap.RC2" "\n"
      "1.2.840.113533.7.66.15 = KeyWrap.CAST-128" "\n"
      "2.16.840.1.101.3.4.1.5 = KeyWrap.AES-128" "\n"
      "2.16.840.1.101.3.4.1.25 = KeyWrap.AES-192" "\n"
      "2.16.840.1.101.3.4.1.45 = KeyWrap.AES-256" "\n"

      "1.2.840.113549.1.9.16.3.8 = Compression.Zlib" "\n"

      "1.2.840.113549.1.1.1 = RSA/EME-PKCS1-v1_5" "\n"
      "1.2.840.113549.1.1.2 = RSA/EMSA3(MD2)" "\n"
      "1.2.840.113549.1.1.4 = RSA/EMSA3(MD5)" "\n"
      "1.2.840.113549.1.1.5 = RSA/EMSA3(SHA-160)" "\n"
      "1.2.840.113549.1.1.11 = RSA/EMSA3(SHA-256)" "\n"
      "1.2.840.113549.1.1.12 = RSA/EMSA3(SHA-384)" "\n"
      "1.2.840.113549.1.1.13 = RSA/EMSA3(SHA-512)" "\n"
      "1.3.36.3.3.1.2 = RSA/EMSA3(RIPEMD-160)" "\n"

      "1.2.840.10040.4.3 = DSA/EMSA1(SHA-160)" "\n"
      "2.16.840.1.101.3.4.3.1 = DSA/EMSA1(SHA-224)" "\n"
      "2.16.840.1.101.3.4.3.2 = DSA/EMSA1(SHA-256)" "\n"

      "0.4.0.127.0.7.1.1.4.1.1 = ECDSA/EMSA1_BSI(SHA-160)" "\n"
      "0.4.0.127.0.7.1.1.4.1.2 = ECDSA/EMSA1_BSI(SHA-224)" "\n"
      "0.4.0.127.0.7.1.1.4.1.3 = ECDSA/EMSA1_BSI(SHA-256)" "\n"
      "0.4.0.127.0.7.1.1.4.1.4 = ECDSA/EMSA1_BSI(SHA-384)" "\n"
      "0.4.0.127.0.7.1.1.4.1.5 = ECDSA/EMSA1_BSI(SHA-512)" "\n"
      "0.4.0.127.0.7.1.1.4.1.6 = ECDSA/EMSA1_BSI(RIPEMD-160)" "\n"

      "1.2.840.10045.4.1 = ECDSA/EMSA1(SHA-160)" "\n"
      "1.2.840.10045.4.3.1 = ECDSA/EMSA1(SHA-224)" "\n"
      "1.2.840.10045.4.3.2 = ECDSA/EMSA1(SHA-256)" "\n"
      "1.2.840.10045.4.3.3 = ECDSA/EMSA1(SHA-384)" "\n"
      "1.2.840.10045.4.3.4 = ECDSA/EMSA1(SHA-512)" "\n"

      "1.2.643.2.2.3 = GOST-34.10/EMSA1(GOST-R-34.11-94)" "\n"

      "1.3.6.1.4.1.25258.2.1.1.1 = RW/EMSA2(RIPEMD-160)" "\n"
      "1.3.6.1.4.1.25258.2.1.1.2 = RW/EMSA2(SHA-160)" "\n"
      "1.3.6.1.4.1.25258.2.1.1.3 = RW/EMSA2(SHA-224)" "\n"
      "1.3.6.1.4.1.25258.2.1.1.4 = RW/EMSA2(SHA-256)" "\n"
      "1.3.6.1.4.1.25258.2.1.1.5 = RW/EMSA2(SHA-384)" "\n"
      "1.3.6.1.4.1.25258.2.1.1.6 = RW/EMSA2(SHA-512)" "\n"

      "1.3.6.1.4.1.25258.2.1.2.1 = RW/EMSA4(RIPEMD-160)" "\n"
      "1.3.6.1.4.1.25258.2.1.2.2 = RW/EMSA4(SHA-160)" "\n"
      "1.3.6.1.4.1.25258.2.1.2.3 = RW/EMSA4(SHA-224)" "\n"
      "1.3.6.1.4.1.25258.2.1.2.4 = RW/EMSA4(SHA-256)" "\n"
      "1.3.6.1.4.1.25258.2.1.2.5 = RW/EMSA4(SHA-384)" "\n"
      "1.3.6.1.4.1.25258.2.1.2.6 = RW/EMSA4(SHA-512)" "\n"

      "1.3.6.1.4.1.25258.2.2.1.1 = NR/EMSA2(RIPEMD-160)" "\n"
      "1.3.6.1.4.1.25258.2.2.1.2 = NR/EMSA2(SHA-160)" "\n"
      "1.3.6.1.4.1.25258.2.2.1.3 = NR/EMSA2(SHA-224)" "\n"
      "1.3.6.1.4.1.25258.2.2.1.4 = NR/EMSA2(SHA-256)" "\n"
      "1.3.6.1.4.1.25258.2.2.1.5 = NR/EMSA2(SHA-384)" "\n"
      "1.3.6.1.4.1.25258.2.2.1.6 = NR/EMSA2(SHA-512)" "\n"

      "2.5.4.3 = X520.CommonName" "\n"
      "2.5.4.4 = X520.Surname" "\n"
      "2.5.4.5 = X520.SerialNumber" "\n"
      "2.5.4.6 = X520.Country" "\n"
      "2.5.4.7 = X520.Locality" "\n"
      "2.5.4.8 = X520.State" "\n"
      "2.5.4.10 = X520.Organization" "\n"
      "2.5.4.11 = X520.OrganizationalUnit" "\n"
      "2.5.4.12 = X520.Title" "\n"
      "2.5.4.42 = X520.GivenName" "\n"
      "2.5.4.43 = X520.Initials" "\n"
      "2.5.4.44 = X520.GenerationalQualifier" "\n"
      "2.5.4.46 = X520.DNQualifier" "\n"
      "2.5.4.65 = X520.Pseudonym" "\n"

      "1.2.840.113549.1.5.12 = PKCS5.PBKDF2" "\n"
      "1.2.840.113549.1.5.13 = PBE-PKCS5v20" "\n"

      "1.2.840.113549.1.9.1 = PKCS9.EmailAddress" "\n"
      "1.2.840.113549.1.9.2 = PKCS9.UnstructuredName" "\n"
      "1.2.840.113549.1.9.3 = PKCS9.ContentType" "\n"
      "1.2.840.113549.1.9.4 = PKCS9.MessageDigest" "\n"
      "1.2.840.113549.1.9.7 = PKCS9.ChallengePassword" "\n"
      "1.2.840.113549.1.9.14 = PKCS9.ExtensionRequest" "\n"

      "1.2.840.113549.1.7.1 = CMS.DataContent" "\n"
      "1.2.840.113549.1.7.2 = CMS.SignedData" "\n"
      "1.2.840.113549.1.7.3 = CMS.EnvelopedData" "\n"
      "1.2.840.113549.1.7.5 = CMS.DigestedData" "\n"
      "1.2.840.113549.1.7.6 = CMS.EncryptedData" "\n"
      "1.2.840.113549.1.9.16.1.2 = CMS.AuthenticatedData" "\n"
      "1.2.840.113549.1.9.16.1.9 = CMS.CompressedData" "\n"

      "2.5.29.14 = X509v3.SubjectKeyIdentifier" "\n"
      "2.5.29.15 = X509v3.KeyUsage" "\n"
      "2.5.29.17 = X509v3.SubjectAlternativeName" "\n"
      "2.5.29.18 = X509v3.IssuerAlternativeName" "\n"
      "2.5.29.19 = X509v3.BasicConstraints" "\n"
      "2.5.29.20 = X509v3.CRLNumber" "\n"
      "2.5.29.21 = X509v3.ReasonCode" "\n"
      "2.5.29.23 = X509v3.HoldInstructionCode" "\n"
      "2.5.29.24 = X509v3.InvalidityDate" "\n"
      "2.5.29.31 = X509v3.CRLDistributionPoints" "\n"
      "2.5.29.32 = X509v3.CertificatePolicies" "\n"
      "2.5.29.35 = X509v3.AuthorityKeyIdentifier" "\n"
      "2.5.29.36 = X509v3.PolicyConstraints" "\n"
      "2.5.29.37 = X509v3.ExtendedKeyUsage" "\n"
      "1.3.6.1.5.5.7.1.1 = PKIX.AuthorityInformationAccess" "\n"

      "2.5.29.32.0 = X509v3.AnyPolicy" "\n"

      "1.3.6.1.5.5.7.3.1 = PKIX.ServerAuth" "\n"
      "1.3.6.1.5.5.7.3.2 = PKIX.ClientAuth" "\n"
      "1.3.6.1.5.5.7.3.3 = PKIX.CodeSigning" "\n"
      "1.3.6.1.5.5.7.3.4 = PKIX.EmailProtection" "\n"
      "1.3.6.1.5.5.7.3.5 = PKIX.IPsecEndSystem" "\n"
      "1.3.6.1.5.5.7.3.6 = PKIX.IPsecTunnel" "\n"
      "1.3.6.1.5.5.7.3.7 = PKIX.IPsecUser" "\n"
      "1.3.6.1.5.5.7.3.8 = PKIX.TimeStamping" "\n"
      "1.3.6.1.5.5.7.3.9 = PKIX.OCSPSigning" "\n"

      "1.3.6.1.5.5.7.8.5 = PKIX.XMPPAddr" "\n"

      "1.3.6.1.5.5.7.48.1 = PKIX.OCSP" "\n"
      "1.3.6.1.5.5.7.48.1.1 = PKIX.OCSP.BasicResponse" "\n"

      // ECC param sets
      "1.3.132.0.8 = secp160r1" "\n"
      "1.3.132.0.9 = secp160k1" "\n"
      "1.3.132.0.10 = secp256k1" "\n"
      "1.3.132.0.30 = secp160r2" "\n"
      "1.3.132.0.31 = secp192k1" "\n"
      "1.3.132.0.32 = secp224k1" "\n"
      "1.3.132.0.33 = secp224r1" "\n"
      "1.3.132.0.34 = secp384r1" "\n"
      "1.3.132.0.35 = secp521r1" "\n"

      "1.2.840.10045.3.1.1 = secp192r1" "\n"
      "1.2.840.10045.3.1.2 = x962_p192v2" "\n"
      "1.2.840.10045.3.1.3 = x962_p192v3" "\n"
      "1.2.840.10045.3.1.4 = x962_p239v1" "\n"
      "1.2.840.10045.3.1.5 = x962_p239v2" "\n"
      "1.2.840.10045.3.1.6 = x962_p239v3" "\n"
      "1.2.840.10045.3.1.7 = secp256r1" "\n"

      "1.3.36.3.3.2.8.1.1.1 = brainpool160r1" "\n"
      "1.3.36.3.3.2.8.1.1.3 = brainpool192r1" "\n"
      "1.3.36.3.3.2.8.1.1.5 = brainpool224r1" "\n"
      "1.3.36.3.3.2.8.1.1.7 = brainpool256r1" "\n"
      "1.3.36.3.3.2.8.1.1.9 = brainpool320r1" "\n"
      "1.3.36.3.3.2.8.1.1.11 = brainpool384r1" "\n"
      "1.3.36.3.3.2.8.1.1.13 = brainpool512r1" "\n"

      "1.3.6.1.4.1.8301.3.1.2.9.0.38 = secp521r1" "\n"

      "1.2.643.2.2.35.1 = gost_256A" "\n"
      "1.2.643.2.2.36.0 = gost_256A" "\n"

      "0.4.0.127.0.7.3.1.2.1 = CertificateHolderAuthorizationTemplate" "\n"
      ;
   }

}

}
/*
* OID Registry
* (C) 1999-2008,2013 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace OIDS {

namespace {

class OID_Map
   {
   public:
      void add_oid(const OID& oid, const std::string& str)
         {
         add_str2oid(oid, str);
         add_oid2str(oid, str);
         }

      void add_str2oid(const OID& oid, const std::string& str)
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         auto i = m_str2oid.find(str);
         if(i == m_str2oid.end())
            m_str2oid.insert(std::make_pair(str, oid));
         }

      void add_oid2str(const OID& oid, const std::string& str)
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         auto i = m_oid2str.find(oid);
         if(i == m_oid2str.end())
            m_oid2str.insert(std::make_pair(oid, str));
         }

      std::string lookup(const OID& oid)
         {
         std::lock_guard<std::mutex> lock(m_mutex);

         auto i = m_oid2str.find(oid);
         if(i != m_oid2str.end())
            return i->second;

         return "";
         }

      OID lookup(const std::string& str)
         {
         std::lock_guard<std::mutex> lock(m_mutex);

         auto i = m_str2oid.find(str);
         if(i != m_str2oid.end())
            return i->second;

         // Try to parse as plain OID
         try
            {
            return OID(str);
            }
         catch(...) {}

         throw Lookup_Error("No object identifier found for " + str);
         }

      bool have_oid(const std::string& str)
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         return m_str2oid.find(str) != m_str2oid.end();
         }

      static OID_Map& global_registry()
         {
         static OID_Map g_map;
         return g_map;
         }

      void read_cfg(std::istream& cfg, const std::string& source);

   private:

      OID_Map()
         {
         std::istringstream cfg(default_oid_list());
         read_cfg(cfg, "builtin");
         }

      std::mutex m_mutex;
      std::map<std::string, OID> m_str2oid;
      std::map<OID, std::string> m_oid2str;
   };

void OID_Map::read_cfg(std::istream& cfg, const std::string& source)
   {
   std::lock_guard<std::mutex> lock(m_mutex);

   size_t line = 0;

   while(cfg.good())
      {
      std::string s;
      std::getline(cfg, s);
      ++line;

      if(s == "" || s[0] == '#')
         continue;

      s = clean_ws(s.substr(0, s.find('#')));

      if(s == "")
         continue;

      auto eq = s.find("=");

      if(eq == std::string::npos || eq == 0 || eq == s.size() - 1)
         throw std::runtime_error("Bad config line '" + s + "' in " + source + " line " + std::to_string(line));

      const std::string oid = clean_ws(s.substr(0, eq));
      const std::string name = clean_ws(s.substr(eq + 1, std::string::npos));

      m_str2oid.insert(std::make_pair(name, oid));
      m_oid2str.insert(std::make_pair(oid, name));
      }
   }

}

void add_oid(const OID& oid, const std::string& name)
   {
   OID_Map::global_registry().add_oid(oid, name);
   }

void add_oidstr(const char* oidstr, const char* name)
   {
   add_oid(OID(oidstr), name);
   }

void add_oid2str(const OID& oid, const std::string& name)
   {
   OID_Map::global_registry().add_oid2str(oid, name);
   }

void add_str2oid(const OID& oid, const std::string& name)
   {
   OID_Map::global_registry().add_str2oid(oid, name);
   }

std::string lookup(const OID& oid)
   {
   return OID_Map::global_registry().lookup(oid);
   }

OID lookup(const std::string& name)
   {
   return OID_Map::global_registry().lookup(name);
   }

bool have_oid(const std::string& name)
   {
   return OID_Map::global_registry().have_oid(name);
   }

bool name_of(const OID& oid, const std::string& name)
   {
   return (oid == lookup(name));
   }

}

}
/*
* PKCS #5 PBES2
* (C) 1999-2008,2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

/*
* Encode PKCS#5 PBES2 parameters
*/
std::vector<byte> encode_pbes2_params(const std::string& cipher,
                                      const std::string& prf,
                                      const secure_vector<byte>& salt,
                                      const secure_vector<byte>& iv,
                                      size_t iterations,
                                      size_t key_length)
   {
   return DER_Encoder()
      .start_cons(SEQUENCE)
      .encode(
         AlgorithmIdentifier("PKCS5.PBKDF2",
            DER_Encoder()
               .start_cons(SEQUENCE)
                  .encode(salt, OCTET_STRING)
                  .encode(iterations)
                  .encode(key_length)
                  .encode_if(
                     prf != "HMAC(SHA-160)",
                     AlgorithmIdentifier(prf, AlgorithmIdentifier::USE_NULL_PARAM))
               .end_cons()
            .get_contents_unlocked()
            )
         )
      .encode(
         AlgorithmIdentifier(cipher,
            DER_Encoder().encode(iv, OCTET_STRING).get_contents_unlocked()
            )
         )
      .end_cons()
      .get_contents_unlocked();
   }

}

/*
* PKCS#5 v2.0 PBE Constructor
*/
std::pair<AlgorithmIdentifier, std::vector<byte>>
pbes2_encrypt(const secure_vector<byte>& key_bits,
              const std::string& passphrase,
              std::chrono::milliseconds msec,
              const std::string& cipher,
              const std::string& digest,
              RandomNumberGenerator& rng)
   {
   const std::string prf = "HMAC(" + digest + ")";

   const std::vector<std::string> cipher_spec = split_on(cipher, '/');
   if(cipher_spec.size() != 2)
      throw Decoding_Error("PBE-PKCS5 v2.0: Invalid cipher spec " + cipher);

   const secure_vector<byte> salt = rng.random_vec(12);

   if(cipher_spec[1] != "CBC" && cipher_spec[1] != "GCM")
      throw Decoding_Error("PBE-PKCS5 v2.0: Don't know param format for " + cipher);

   std::unique_ptr<Cipher_Mode> enc(get_cipher_mode(cipher, ENCRYPTION));

   if(!enc)
      throw Decoding_Error("PBE-PKCS5 cannot encrypt no cipher " + cipher);

   std::unique_ptr<PBKDF> pbkdf(get_pbkdf("PBKDF2(" + prf + ")"));

   const size_t key_length = enc->key_spec().maximum_keylength();
   size_t iterations = 0;

   secure_vector<byte> iv = rng.random_vec(enc->default_nonce_length());

   enc->set_key(pbkdf->derive_key(key_length, passphrase, salt.data(), salt.size(),
                                  msec, iterations).bits_of());

   enc->start(iv);
   secure_vector<byte> buf = key_bits;
   enc->finish(buf);

   AlgorithmIdentifier id(
      OIDS::lookup("PBE-PKCS5v20"),
      encode_pbes2_params(cipher, prf, salt, iv, iterations, key_length));

   return std::make_pair(id, unlock(buf));
   }

secure_vector<byte>
pbes2_decrypt(const secure_vector<byte>& key_bits,
              const std::string& passphrase,
              const std::vector<byte>& params)
   {
   AlgorithmIdentifier kdf_algo, enc_algo;

   BER_Decoder(params)
      .start_cons(SEQUENCE)
         .decode(kdf_algo)
         .decode(enc_algo)
         .verify_end()
      .end_cons();

   AlgorithmIdentifier prf_algo;

   if(kdf_algo.oid != OIDS::lookup("PKCS5.PBKDF2"))
      throw Decoding_Error("PBE-PKCS5 v2.0: Unknown KDF algorithm " +
                           kdf_algo.oid.as_string());

   secure_vector<byte> salt;
   size_t iterations = 0, key_length = 0;

   BER_Decoder(kdf_algo.parameters)
      .start_cons(SEQUENCE)
         .decode(salt, OCTET_STRING)
         .decode(iterations)
         .decode_optional(key_length, INTEGER, UNIVERSAL)
         .decode_optional(prf_algo, SEQUENCE, CONSTRUCTED,
                          AlgorithmIdentifier("HMAC(SHA-160)",
                                              AlgorithmIdentifier::USE_NULL_PARAM))
      .verify_end()
      .end_cons();

   const std::string cipher = OIDS::lookup(enc_algo.oid);
   const std::vector<std::string> cipher_spec = split_on(cipher, '/');
   if(cipher_spec.size() != 2)
      throw Decoding_Error("PBE-PKCS5 v2.0: Invalid cipher spec " + cipher);
   if(cipher_spec[1] != "CBC" && cipher_spec[1] != "GCM")
      throw Decoding_Error("PBE-PKCS5 v2.0: Don't know param format for " + cipher);

   if(salt.size() < 8)
      throw Decoding_Error("PBE-PKCS5 v2.0: Encoded salt is too small");

   secure_vector<byte> iv;
   BER_Decoder(enc_algo.parameters).decode(iv, OCTET_STRING).verify_end();

   const std::string prf = OIDS::lookup(prf_algo.oid);

   std::unique_ptr<PBKDF> pbkdf(get_pbkdf("PBKDF2(" + prf + ")"));

   std::unique_ptr<Cipher_Mode> dec(get_cipher_mode(cipher, DECRYPTION));
   if(!dec)
      throw Decoding_Error("PBE-PKCS5 cannot decrypt no cipher " + cipher);

   if(key_length == 0)
      key_length = dec->key_spec().maximum_keylength();

   dec->set_key(pbkdf->pbkdf_iterations(key_length, passphrase, salt.data(), salt.size(), iterations));

   dec->start(iv);

   secure_vector<byte> buf = key_bits;
   dec->finish(buf);

   return buf;
   }

}
/*
* PBKDF
* (C) 2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_PBKDF1)
#endif

#if defined(BOTAN_HAS_PBKDF2)
#endif

namespace Botan {

#define BOTAN_REGISTER_PBKDF_1HASH(type, name)                          \
   BOTAN_REGISTER_NAMED_T(PBKDF, name, type, (make_new_T_1X<type, HashFunction>))

#if defined(BOTAN_HAS_PBKDF1)
BOTAN_REGISTER_PBKDF_1HASH(PKCS5_PBKDF1, "PBKDF1");
#endif

#if defined(BOTAN_HAS_PBKDF2)
BOTAN_REGISTER_NAMED_T(PBKDF, "PBKDF2", PKCS5_PBKDF2, PKCS5_PBKDF2::make);
#endif

PBKDF::~PBKDF() {}

std::unique_ptr<PBKDF> PBKDF::create(const std::string& algo_spec,
                                     const std::string& provider)
   {
   return std::unique_ptr<PBKDF>(make_a<PBKDF>(algo_spec, provider));
   }

std::vector<std::string> PBKDF::providers(const std::string& algo_spec)
   {
   return providers_of<PBKDF>(PBKDF::Spec(algo_spec));
   }

void PBKDF::pbkdf_timed(byte out[], size_t out_len,
                        const std::string& passphrase,
                        const byte salt[], size_t salt_len,
                        std::chrono::milliseconds msec,
                        size_t& iterations) const
   {
   iterations = pbkdf(out, out_len, passphrase, salt, salt_len, 0, msec);
   }

void PBKDF::pbkdf_iterations(byte out[], size_t out_len,
                             const std::string& passphrase,
                             const byte salt[], size_t salt_len,
                             size_t iterations) const
   {
   if(iterations == 0)
      throw std::invalid_argument(name() + ": Invalid iteration count");

   const size_t iterations_run = pbkdf(out, out_len, passphrase,
                                       salt, salt_len, iterations,
                                       std::chrono::milliseconds(0));
   BOTAN_ASSERT_EQUAL(iterations, iterations_run, "Expected PBKDF iterations");
   }

secure_vector<byte> PBKDF::pbkdf_iterations(size_t out_len,
                                            const std::string& passphrase,
                                            const byte salt[], size_t salt_len,
                                            size_t iterations) const
   {
   secure_vector<byte> out(out_len);
   pbkdf_iterations(out.data(), out_len, passphrase, salt, salt_len, iterations);
   return out;
   }

secure_vector<byte> PBKDF::pbkdf_timed(size_t out_len,
                                       const std::string& passphrase,
                                       const byte salt[], size_t salt_len,
                                       std::chrono::milliseconds msec,
                                       size_t& iterations) const
   {
   secure_vector<byte> out(out_len);
   pbkdf_timed(out.data(), out_len, passphrase, salt, salt_len, msec, iterations);
   return out;
   }

}
/*
* PBKDF2
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

PKCS5_PBKDF2* PKCS5_PBKDF2::make(const Spec& spec)
   {
   if(auto mac = MessageAuthenticationCode::create(spec.arg(0)))
      return new PKCS5_PBKDF2(mac.release());

   if(auto mac = MessageAuthenticationCode::create("HMAC(" + spec.arg(0) + ")"))
      return new PKCS5_PBKDF2(mac.release());

   return nullptr;
   }

size_t
pbkdf2(MessageAuthenticationCode& prf,
       byte out[],
       size_t out_len,
       const std::string& passphrase,
       const byte salt[], size_t salt_len,
       size_t iterations,
       std::chrono::milliseconds msec)
   {
   clear_mem(out, out_len);

   if(out_len == 0)
      return 0;

   try
      {
      prf.set_key(reinterpret_cast<const byte*>(passphrase.data()), passphrase.size());
      }
   catch(Invalid_Key_Length)
      {
      throw std::runtime_error("PBKDF2 with " + prf.name() +
                               " cannot accept passphrases of length " +
                               std::to_string(passphrase.size()));
      }

   const size_t prf_sz = prf.output_length();
   secure_vector<byte> U(prf_sz);

   const size_t blocks_needed = round_up(out_len, prf_sz) / prf_sz;

   std::chrono::microseconds usec_per_block =
      std::chrono::duration_cast<std::chrono::microseconds>(msec) / blocks_needed;

   u32bit counter = 1;
   while(out_len)
      {
      const size_t prf_output = std::min<size_t>(prf_sz, out_len);

      prf.update(salt, salt_len);
      prf.update_be(counter++);
      prf.final(U.data());

      xor_buf(out, U.data(), prf_output);

      if(iterations == 0)
         {
         /*
         If no iterations set, run the first block to calibrate based
         on how long hashing takes on whatever machine we're running on.
         */

         const auto start = std::chrono::high_resolution_clock::now();

         iterations = 1; // the first iteration we did above

         while(true)
            {
            prf.update(U);
            prf.final(U.data());
            xor_buf(out, U.data(), prf_output);
            iterations++;

            /*
            Only break on relatively 'even' iterations. For one it
            avoids confusion, and likely some broken implementations
            break on getting completely randomly distributed values
            */
            if(iterations % 10000 == 0)
               {
               auto time_taken = std::chrono::high_resolution_clock::now() - start;
               auto usec_taken = std::chrono::duration_cast<std::chrono::microseconds>(time_taken);
               if(usec_taken > usec_per_block)
                  break;
               }
            }
         }
      else
         {
         for(size_t i = 1; i != iterations; ++i)
            {
            prf.update(U);
            prf.final(U.data());
            xor_buf(out, U.data(), prf_output);
            }
         }

      out_len -= prf_output;
      out += prf_output;
      }

   return iterations;
   }

size_t
PKCS5_PBKDF2::pbkdf(byte key[], size_t key_len,
                    const std::string& passphrase,
                    const byte salt[], size_t salt_len,
                    size_t iterations,
                    std::chrono::milliseconds msec) const
   {
   return pbkdf2(*mac.get(), key, key_len, passphrase, salt, salt_len, iterations, msec);
   }


}
/*
* PEM Encoding/Decoding
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace PEM_Code {

namespace {

std::string linewrap(size_t width, const std::string& in)
   {
   std::string out;
   for(size_t i = 0; i != in.size(); ++i)
      {
      if(i > 0 && i % width == 0)
         {
         out.push_back('\n');
         }
      out.push_back(in[i]);
      }
   if(out.size() > 0 && out[out.size()-1] != '\n')
      {
      out.push_back('\n');
      }

   return out;
   }

}

/*
* PEM encode BER/DER-encoded objects
*/
std::string encode(const byte der[], size_t length, const std::string& label, size_t width)
   {
   const std::string PEM_HEADER = "-----BEGIN " + label + "-----\n";
   const std::string PEM_TRAILER = "-----END " + label + "-----\n";

   return (PEM_HEADER + linewrap(width, base64_encode(der, length)) + PEM_TRAILER);
   }

/*
* Decode PEM down to raw BER/DER
*/
secure_vector<byte> decode_check_label(DataSource& source,
                                      const std::string& label_want)
   {
   std::string label_got;
   secure_vector<byte> ber = decode(source, label_got);
   if(label_got != label_want)
      throw Decoding_Error("PEM: Label mismatch, wanted " + label_want +
                           ", got " + label_got);
   return ber;
   }

/*
* Decode PEM down to raw BER/DER
*/
secure_vector<byte> decode(DataSource& source, std::string& label)
   {
   const size_t RANDOM_CHAR_LIMIT = 8;

   const std::string PEM_HEADER1 = "-----BEGIN ";
   const std::string PEM_HEADER2 = "-----";
   size_t position = 0;

   while(position != PEM_HEADER1.length())
      {
      byte b;
      if(!source.read_byte(b))
         throw Decoding_Error("PEM: No PEM header found");
      if(b == PEM_HEADER1[position])
         ++position;
      else if(position >= RANDOM_CHAR_LIMIT)
         throw Decoding_Error("PEM: Malformed PEM header");
      else
         position = 0;
      }
   position = 0;
   while(position != PEM_HEADER2.length())
      {
      byte b;
      if(!source.read_byte(b))
         throw Decoding_Error("PEM: No PEM header found");
      if(b == PEM_HEADER2[position])
         ++position;
      else if(position)
         throw Decoding_Error("PEM: Malformed PEM header");

      if(position == 0)
         label += static_cast<char>(b);
      }

   std::vector<char> b64;

   const std::string PEM_TRAILER = "-----END " + label + "-----";
   position = 0;
   while(position != PEM_TRAILER.length())
      {
      byte b;
      if(!source.read_byte(b))
         throw Decoding_Error("PEM: No PEM trailer found");
      if(b == PEM_TRAILER[position])
         ++position;
      else if(position)
         throw Decoding_Error("PEM: Malformed PEM trailer");

      if(position == 0)
         b64.push_back(b);
      }

   return base64_decode(b64.data(), b64.size());
   }

secure_vector<byte> decode_check_label(const std::string& pem,
                                      const std::string& label_want)
   {
   DataSource_Memory src(pem);
   return decode_check_label(src, label_want);
   }

secure_vector<byte> decode(const std::string& pem, std::string& label)
   {
   DataSource_Memory src(pem);
   return decode(src, label);
   }

/*
* Search for a PEM signature
*/
bool matches(DataSource& source, const std::string& extra,
             size_t search_range)
   {
   const std::string PEM_HEADER = "-----BEGIN " + extra;

   secure_vector<byte> search_buf(search_range);
   size_t got = source.peek(search_buf.data(), search_buf.size(), 0);

   if(got < PEM_HEADER.length())
      return false;

   size_t index = 0;

   for(size_t j = 0; j != got; ++j)
      {
      if(search_buf[j] == PEM_HEADER[index])
         ++index;
      else
         index = 0;
      if(index == PEM_HEADER.size())
         return true;
      }
   return false;
   }

}

}
/*
* EME Base Class
* (C) 1999-2008 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_EME_OAEP)
#endif

#if defined(BOTAN_HAS_EME_PKCS1v15)
#endif

#if defined(BOTAN_HAS_EME_RAW)
#endif

namespace Botan {

#define BOTAN_REGISTER_EME(name, maker) BOTAN_REGISTER_T(EME, name, maker)
#define BOTAN_REGISTER_EME_NOARGS(name) BOTAN_REGISTER_T_NOARGS(EME, name)

#define BOTAN_REGISTER_EME_NAMED_NOARGS(type, name) \
   BOTAN_REGISTER_NAMED_T(EME, name, type, make_new_T<type>)

#if defined(BOTAN_HAS_EME_OAEP)
BOTAN_REGISTER_NAMED_T(EME, "OAEP", OAEP, OAEP::make);
#endif

#if defined(BOTAN_HAS_EME_PKCS1v15)
BOTAN_REGISTER_EME_NAMED_NOARGS(EME_PKCS1v15, "PKCS1v15");
#endif

#if defined(BOTAN_HAS_EME_RAW)
BOTAN_REGISTER_EME_NAMED_NOARGS(EME_Raw, "Raw");
#endif

EME* get_eme(const std::string& algo_spec)
   {
   SCAN_Name request(algo_spec);

   if(EME* eme = make_a<EME>(algo_spec))
      return eme;

   if(request.algo_name() == "Raw")
      return nullptr; // No padding

   throw Algorithm_Not_Found(algo_spec);
   }

/*
* Encode a message
*/
secure_vector<byte> EME::encode(const byte msg[], size_t msg_len,
                               size_t key_bits,
                               RandomNumberGenerator& rng) const
   {
   return pad(msg, msg_len, key_bits, rng);
   }

/*
* Encode a message
*/
secure_vector<byte> EME::encode(const secure_vector<byte>& msg,
                               size_t key_bits,
                               RandomNumberGenerator& rng) const
   {
   return pad(msg.data(), msg.size(), key_bits, rng);
   }

/*
* Decode a message
*/
secure_vector<byte> EME::decode(const byte msg[], size_t msg_len,
                               size_t key_bits) const
   {
   return unpad(msg, msg_len, key_bits);
   }

/*
* Decode a message
*/
secure_vector<byte> EME::decode(const secure_vector<byte>& msg,
                               size_t key_bits) const
   {
   return unpad(msg.data(), msg.size(), key_bits);
   }

}
/*
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_EMSA1)
#endif

#if defined(BOTAN_HAS_EMSA1_BSI)
#endif

#if defined(BOTAN_HAS_EMSA_X931)
#endif

#if defined(BOTAN_HAS_EMSA_PKCS1)
#endif

#if defined(BOTAN_HAS_EMSA_PSSR)
#endif

#if defined(BOTAN_HAS_EMSA_RAW)
#endif

namespace Botan {

EMSA::~EMSA() {}

EMSA* get_emsa(const std::string& algo_spec)
   {
   SCAN_Name request(algo_spec);

   if(EMSA* emsa = make_a<EMSA>(algo_spec))
      return emsa;

   throw Algorithm_Not_Found(algo_spec);
   }

#define BOTAN_REGISTER_EMSA_NAMED_NOARGS(type, name) \
   BOTAN_REGISTER_NAMED_T(EMSA, name, type, make_new_T<type>)

#define BOTAN_REGISTER_EMSA(name, maker) BOTAN_REGISTER_T(EMSA, name, maker)
#define BOTAN_REGISTER_EMSA_NOARGS(name) BOTAN_REGISTER_T_NOARGS(EMSA, name)

#define BOTAN_REGISTER_EMSA_1HASH(type, name)                    \
   BOTAN_REGISTER_NAMED_T(EMSA, name, type, (make_new_T_1X<type, HashFunction>))

#if defined(BOTAN_HAS_EMSA1)
BOTAN_REGISTER_EMSA_1HASH(EMSA1, "EMSA1");
#endif

#if defined(BOTAN_HAS_EMSA1_BSI)
BOTAN_REGISTER_EMSA_1HASH(EMSA1_BSI, "EMSA1_BSI");
#endif

#if defined(BOTAN_HAS_EMSA_PKCS1)
BOTAN_REGISTER_NAMED_T(EMSA, "EMSA_PKCS1", EMSA_PCS1v15, EMSA_PKCS1v15::make);
#endif

#if defined(BOTAN_HAS_EMSA_PSSR)
BOTAN_REGISTER_NAMED_T(EMSA, "PSSR", PSSR, PSSR::make);
#endif

#if defined(BOTAN_HAS_EMSA_X931)
BOTAN_REGISTER_EMSA_1HASH(EMSA_X931, "EMSA_X931");
#endif

#if defined(BOTAN_HAS_EMSA_RAW)
BOTAN_REGISTER_EMSA_NAMED_NOARGS(EMSA_Raw, "Raw");
#endif

}


/*
* Entropy source based on reading files in /proc on the assumption
* that a remote attacker will have difficulty guessing some of them.
*
* (C) 1999-2008,2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#ifndef _POSIX_C_SOURCE
  #define _POSIX_C_SOURCE 199309
#endif

#include <dirent.h>

namespace Botan {

namespace {

class Directory_Walker : public File_Descriptor_Source
   {
   public:
      Directory_Walker(const std::string& root) :
         m_cur_dir(std::make_pair<DIR*, std::string>(nullptr, ""))
         {
         if(DIR* root_dir = ::opendir(root.c_str()))
            m_cur_dir = std::make_pair(root_dir, root);
         }

      ~Directory_Walker()
         {
         if(m_cur_dir.first)
            ::closedir(m_cur_dir.first);
         }

      int next_fd() override;
   private:
      std::pair<struct dirent*, std::string> get_next_dirent();

      std::pair<DIR*, std::string> m_cur_dir;
      std::deque<std::string> m_dirlist;
   };

std::pair<struct dirent*, std::string> Directory_Walker::get_next_dirent()
   {
   while(m_cur_dir.first)
      {
      if(struct dirent* dir = ::readdir(m_cur_dir.first))
         return std::make_pair(dir, m_cur_dir.second);

      ::closedir(m_cur_dir.first);
      m_cur_dir = std::make_pair<DIR*, std::string>(nullptr, "");

      while(!m_dirlist.empty() && !m_cur_dir.first)
         {
         const std::string next_dir_name = m_dirlist[0];
         m_dirlist.pop_front();

         if(DIR* next_dir = ::opendir(next_dir_name.c_str()))
            m_cur_dir = std::make_pair(next_dir, next_dir_name);
         }
      }

   return std::make_pair<struct dirent*, std::string>(nullptr, ""); // nothing left
   }

int Directory_Walker::next_fd()
   {
   while(true)
      {
      std::pair<struct dirent*, std::string> entry = get_next_dirent();

      if(!entry.first)
         break; // no more dirs

      const std::string filename = entry.first->d_name;

      if(filename == "." || filename == "..")
         continue;

      const std::string full_path = entry.second + "/" + filename;

      struct stat stat_buf;
      if(::lstat(full_path.c_str(), &stat_buf) == -1)
         continue;

      if(S_ISDIR(stat_buf.st_mode))
         {
         m_dirlist.push_back(full_path);
         }
      else if(S_ISREG(stat_buf.st_mode) && (stat_buf.st_mode & S_IROTH))
         {
         int fd = ::open(full_path.c_str(), O_RDONLY | O_NOCTTY);

         if(fd >= 0)
            return fd;
         }
      }

   return -1;
   }

}

void ProcWalking_EntropySource::poll(Entropy_Accumulator& accum)
   {
   const size_t MAX_FILES_READ_PER_POLL = 2048;

   std::lock_guard<std::mutex> lock(m_mutex);

   if(!m_dir)
      m_dir.reset(new Directory_Walker(m_path));

   m_buf.resize(4096);

   for(size_t i = 0; i != MAX_FILES_READ_PER_POLL; ++i)
      {
      int fd = m_dir->next_fd();

      // If we've exhaused this walk of the directory, halt the poll
      if(fd == -1)
         {
         m_dir.reset();
         break;
         }

      ssize_t got = ::read(fd, m_buf.data(), m_buf.size());
      ::close(fd);

      if(got > 0)
         accum.add(m_buf.data(), got, BOTAN_ENTROPY_ESTIMATE_SYSTEM_TEXT);

      if(accum.polling_finished())
         break;
      }
   }

}
/*
* Blinding for public key operations
* (C) 1999-2010,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_SYSTEM_RNG)
#else
#endif

namespace Botan {

Blinder::Blinder(const BigInt& modulus,
                 std::function<BigInt (const BigInt&)> fwd,
                 std::function<BigInt (const BigInt&)> inv) :
   m_fwd_fn(fwd), m_inv_fn(inv)
   {
   m_reducer = Modular_Reducer(modulus);
   m_modulus_bits = modulus.bits();

#if defined(BOTAN_HAS_SYSTEM_RNG)
   m_rng.reset(new System_RNG);
#else
   m_rng.reset(new AutoSeeded_RNG);
#endif

   const BigInt k = blinding_nonce();
   m_e = m_fwd_fn(k);
   m_d = m_inv_fn(k);
   }

BigInt Blinder::blinding_nonce() const
   {
   return BigInt(*m_rng, m_modulus_bits - 1);
   }

BigInt Blinder::blind(const BigInt& i) const
   {
   if(!m_reducer.initialized())
      throw std::runtime_error("Blinder not initialized, cannot blind");

   ++m_counter;

   if(BOTAN_BLINDING_REINIT_INTERVAL > 0 && (m_counter % BOTAN_BLINDING_REINIT_INTERVAL == 0))
      {
      const BigInt k = blinding_nonce();
      m_e = m_fwd_fn(k);
      m_d = m_inv_fn(k);
      }
   else
      {
      m_e = m_reducer.square(m_e);
      m_d = m_reducer.square(m_d);
      }

   return m_reducer.multiply(i, m_e);
   }

BigInt Blinder::unblind(const BigInt& i) const
   {
   if(!m_reducer.initialized())
      throw std::runtime_error("Blinder not initialized, cannot unblind");

   return m_reducer.multiply(i, m_d);
   }

}
/*
* PK Key
* (C) 1999-2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_RSA)
#endif

#if defined(BOTAN_HAS_DSA)
#endif

#if defined(BOTAN_HAS_DIFFIE_HELLMAN)
#endif

#if defined(BOTAN_HAS_ECDSA)
#endif

#if defined(BOTAN_HAS_GOST_34_10_2001)
#endif

#if defined(BOTAN_HAS_NYBERG_RUEPPEL)
#endif

#if defined(BOTAN_HAS_RW)
#endif

#if defined(BOTAN_HAS_ELGAMAL)
#endif

#if defined(BOTAN_HAS_ECDH)
#endif

#if defined(BOTAN_HAS_CURVE_25519)
#endif

#if defined(BOTAN_HAS_MCELIECE)
#endif

namespace Botan {

Public_Key* make_public_key(const AlgorithmIdentifier& alg_id,
                            const secure_vector<byte>& key_bits)
   {
   const std::string alg_name = OIDS::lookup(alg_id.oid);
   if(alg_name == "")
      throw Decoding_Error("Unknown algorithm OID: " + alg_id.oid.as_string());

#if defined(BOTAN_HAS_RSA)
   if(alg_name == "RSA")
      return new RSA_PublicKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_RW)
   if(alg_name == "RW")
      return new RW_PublicKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_DSA)
   if(alg_name == "DSA")
      return new DSA_PublicKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_DIFFIE_HELLMAN)
   if(alg_name == "DH")
      return new DH_PublicKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_NYBERG_RUEPPEL)
   if(alg_name == "NR")
      return new NR_PublicKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_ELGAMAL)
   if(alg_name == "ElGamal")
      return new ElGamal_PublicKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_ECDSA)
   if(alg_name == "ECDSA")
      return new ECDSA_PublicKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_GOST_34_10_2001)
   if(alg_name == "GOST-34.10")
      return new GOST_3410_PublicKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_ECDH)
   if(alg_name == "ECDH")
      return new ECDH_PublicKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_CURVE_25519)
   if(alg_name == "Curve25519")
      return new Curve25519_PublicKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_MCELIECE)
   if(alg_name == "McEliece")
      return new McEliece_PublicKey(unlock(key_bits));
#endif

   throw Decoding_Error("Unhandled PK algorithm " + alg_name);
   }

Private_Key* make_private_key(const AlgorithmIdentifier& alg_id,
                              const secure_vector<byte>& key_bits,
                              RandomNumberGenerator& rng)
   {
   const std::string alg_name = OIDS::lookup(alg_id.oid);
   if(alg_name == "")
      throw Decoding_Error("Unknown algorithm OID: " + alg_id.oid.as_string());

#if defined(BOTAN_HAS_RSA)
   if(alg_name == "RSA")
      return new RSA_PrivateKey(alg_id, key_bits, rng);
#endif

#if defined(BOTAN_HAS_RW)
   if(alg_name == "RW")
      return new RW_PrivateKey(alg_id, key_bits, rng);
#endif

#if defined(BOTAN_HAS_DSA)
   if(alg_name == "DSA")
      return new DSA_PrivateKey(alg_id, key_bits, rng);
#endif

#if defined(BOTAN_HAS_DIFFIE_HELLMAN)
   if(alg_name == "DH")
      return new DH_PrivateKey(alg_id, key_bits, rng);
#endif

#if defined(BOTAN_HAS_NYBERG_RUEPPEL)
   if(alg_name == "NR")
      return new NR_PrivateKey(alg_id, key_bits, rng);
#endif

#if defined(BOTAN_HAS_ELGAMAL)
   if(alg_name == "ElGamal")
      return new ElGamal_PrivateKey(alg_id, key_bits, rng);
#endif

#if defined(BOTAN_HAS_ECDSA)
   if(alg_name == "ECDSA")
      return new ECDSA_PrivateKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_GOST_34_10_2001)
   if(alg_name == "GOST-34.10")
      return new GOST_3410_PrivateKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_ECDH)
   if(alg_name == "ECDH")
      return new ECDH_PrivateKey(alg_id, key_bits);
#endif

#if defined(BOTAN_HAS_CURVE_25519)
   if(alg_name == "Curve25519")
      return new Curve25519_PrivateKey(alg_id, key_bits, rng);
#endif

#if defined(BOTAN_HAS_MCELIECE)
   if(alg_name == "McEliece")
      return new McEliece_PrivateKey(key_bits);
#endif

   throw Decoding_Error("Unhandled PK algorithm " + alg_name);
   }

}
/*
* PK Key Types
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Default OID access
*/
OID Public_Key::get_oid() const
   {
   try {
      return OIDS::lookup(algo_name());
      }
   catch(Lookup_Error)
      {
      throw Lookup_Error("PK algo " + algo_name() + " has no defined OIDs");
      }
   }

/*
* Run checks on a loaded public key
*/
void Public_Key::load_check(RandomNumberGenerator& rng) const
   {
   if(!check_key(rng, BOTAN_PUBLIC_KEY_STRONG_CHECKS_ON_LOAD))
      throw Invalid_Argument("Invalid public key");
   }

/*
* Run checks on a loaded private key
*/
void Private_Key::load_check(RandomNumberGenerator& rng) const
   {
   if(!check_key(rng, BOTAN_PRIVATE_KEY_STRONG_CHECKS_ON_LOAD))
      throw Invalid_Argument("Invalid private key");
   }

/*
* Run checks on a generated private key
*/
void Private_Key::gen_check(RandomNumberGenerator& rng) const
   {
   if(!check_key(rng, BOTAN_PRIVATE_KEY_STRONG_CHECKS_ON_GENERATE))
      throw Self_Test_Failure("Private key generation failed");
   }

}
/*
* PK Operation Types
* (C) 2010,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

PK_Ops::Encryption_with_EME::Encryption_with_EME(const std::string& eme)
   {
   m_eme.reset(get_eme(eme));
   if(!m_eme.get())
      throw Algorithm_Not_Found(eme);
   }

PK_Ops::Encryption_with_EME::~Encryption_with_EME() {}

size_t PK_Ops::Encryption_with_EME::max_input_bits() const
   {
   return m_eme->maximum_input_size(max_raw_input_bits());
   }

secure_vector<byte> PK_Ops::Encryption_with_EME::encrypt(const byte msg[], size_t msg_len,
                                                         RandomNumberGenerator& rng)
   {
   const size_t max_raw = max_raw_input_bits();

   const std::vector<byte> encoded = unlock(m_eme->encode(msg, msg_len, max_raw, rng));

   if(8*(encoded.size() - 1) + high_bit(encoded[0]) > max_raw)
      throw std::runtime_error("Input is too large to encrypt with this key");

   return raw_encrypt(encoded.data(), encoded.size(), rng);
   }

PK_Ops::Decryption_with_EME::Decryption_with_EME(const std::string& eme)
   {
   m_eme.reset(get_eme(eme));
   if(!m_eme.get())
      throw Algorithm_Not_Found(eme);
   }

PK_Ops::Decryption_with_EME::~Decryption_with_EME() {}

size_t PK_Ops::Decryption_with_EME::max_input_bits() const
   {
   return m_eme->maximum_input_size(max_raw_input_bits());
   }

secure_vector<byte> PK_Ops::Decryption_with_EME::decrypt(const byte msg[], size_t length)
   {
   return m_eme->decode(raw_decrypt(msg, length), max_raw_input_bits());
   }

PK_Ops::Key_Agreement_with_KDF::Key_Agreement_with_KDF(const std::string& kdf)
   {
   if(kdf != "Raw")
      m_kdf.reset(get_kdf(kdf));
   }

PK_Ops::Key_Agreement_with_KDF::~Key_Agreement_with_KDF() {}

secure_vector<byte> PK_Ops::Key_Agreement_with_KDF::agree(size_t key_len,
                                                          const byte w[], size_t w_len,
                                                          const byte salt[], size_t salt_len)
   {
   secure_vector<byte> z = raw_agree(w, w_len);
   if(m_kdf)
      return m_kdf->derive_key(key_len, z, salt, salt_len);
   return z;
  }

PK_Ops::Signature_with_EMSA::Signature_with_EMSA(const std::string& emsa)
   {
   m_emsa.reset(get_emsa(emsa));
   if(!m_emsa)
      throw Algorithm_Not_Found(emsa);
   }

PK_Ops::Signature_with_EMSA::~Signature_with_EMSA() {}

void PK_Ops::Signature_with_EMSA::update(const byte msg[], size_t msg_len)
   {
   m_emsa->update(msg, msg_len);
   }

secure_vector<byte> PK_Ops::Signature_with_EMSA::sign(RandomNumberGenerator& rng)
   {
   const secure_vector<byte> msg = m_emsa->raw_data();
   const auto padded = m_emsa->encoding_of(msg, this->max_input_bits(), rng);
   return raw_sign(padded.data(), padded.size(), rng);
   }

PK_Ops::Verification_with_EMSA::Verification_with_EMSA(const std::string& emsa)
   {
   m_emsa.reset(get_emsa(emsa));
   if(!m_emsa)
      throw Algorithm_Not_Found(emsa);
   }

PK_Ops::Verification_with_EMSA::~Verification_with_EMSA() {}

void PK_Ops::Verification_with_EMSA::update(const byte msg[], size_t msg_len)
   {
   m_emsa->update(msg, msg_len);
   }

bool PK_Ops::Verification_with_EMSA::is_valid_signature(const byte sig[], size_t sig_len)
   {
   const secure_vector<byte> msg = m_emsa->raw_data();

   if(with_recovery())
      {
      secure_vector<byte> output_of_key = verify_mr(sig, sig_len);
      return m_emsa->verify(output_of_key, msg, max_input_bits());
      }
   else
      {
      Null_RNG rng;
      secure_vector<byte> encoded = m_emsa->encoding_of(msg, max_input_bits(), rng);
      return verify(encoded.data(), encoded.size(), sig, sig_len);
      }
   }

}
/*
* PKCS #8
* (C) 1999-2010,2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace PKCS8 {

namespace {

/*
* Get info from an EncryptedPrivateKeyInfo
*/
secure_vector<byte> PKCS8_extract(DataSource& source,
                                  AlgorithmIdentifier& pbe_alg_id)
   {
   secure_vector<byte> key_data;

   BER_Decoder(source)
      .start_cons(SEQUENCE)
         .decode(pbe_alg_id)
         .decode(key_data, OCTET_STRING)
      .verify_end();

   return key_data;
   }

/*
* PEM decode and/or decrypt a private key
*/
secure_vector<byte> PKCS8_decode(
   DataSource& source,
   std::function<std::string ()> get_passphrase,
   AlgorithmIdentifier& pk_alg_id)
   {
   AlgorithmIdentifier pbe_alg_id;
   secure_vector<byte> key_data, key;
   bool is_encrypted = true;

   try {
      if(ASN1::maybe_BER(source) && !PEM_Code::matches(source))
         key_data = PKCS8_extract(source, pbe_alg_id);
      else
         {
         std::string label;
         key_data = PEM_Code::decode(source, label);
         if(label == "PRIVATE KEY")
            is_encrypted = false;
         else if(label == "ENCRYPTED PRIVATE KEY")
            {
            DataSource_Memory key_source(key_data);
            key_data = PKCS8_extract(key_source, pbe_alg_id);
            }
         else
            throw PKCS8_Exception("Unknown PEM label " + label);
         }

      if(key_data.empty())
         throw PKCS8_Exception("No key data found");
      }
   catch(Decoding_Error& e)
      {
      throw Decoding_Error("PKCS #8 private key decoding failed: " + std::string(e.what()));
      }

   try
      {
      if(is_encrypted)
         {
         if(OIDS::lookup(pbe_alg_id.oid) != "PBE-PKCS5v20")
            throw std::runtime_error("Unknown PBE type " + pbe_alg_id.oid.as_string());
         key = pbes2_decrypt(key_data, get_passphrase(), pbe_alg_id.parameters);
         }
      else
         key = key_data;

      BER_Decoder(key)
         .start_cons(SEQUENCE)
         .decode_and_check<size_t>(0, "Unknown PKCS #8 version number")
            .decode(pk_alg_id)
            .decode(key, OCTET_STRING)
            .discard_remaining()
         .end_cons();
      }
   catch(std::exception& e)
      {
      throw Decoding_Error("PKCS #8 private key decoding failed: " + std::string(e.what()));
      }
   return key;
   }

}

/*
* BER encode a PKCS #8 private key, unencrypted
*/
secure_vector<byte> BER_encode(const Private_Key& key)
   {
   const size_t PKCS8_VERSION = 0;

   return DER_Encoder()
         .start_cons(SEQUENCE)
            .encode(PKCS8_VERSION)
            .encode(key.pkcs8_algorithm_identifier())
            .encode(key.pkcs8_private_key(), OCTET_STRING)
         .end_cons()
      .get_contents();
   }

/*
* PEM encode a PKCS #8 private key, unencrypted
*/
std::string PEM_encode(const Private_Key& key)
   {
   return PEM_Code::encode(PKCS8::BER_encode(key), "PRIVATE KEY");
   }

namespace {

std::pair<std::string, std::string>
choose_pbe_params(const std::string& pbe_algo, const std::string& key_algo)
   {
   if(pbe_algo == "")
      {
      // Defaults:
      if(key_algo == "Curve25519" || key_algo == "McEliece")
         return std::make_pair("AES-256/GCM", "SHA-512");
      else // for everything else (RSA, DSA, ECDSA, GOST, ...)
         return std::make_pair("AES-256/CBC", "SHA-256");
      }

   SCAN_Name request(pbe_algo);
   if(request.algo_name() != "PBE-PKCS5v20" || request.arg_count() != 2)
      throw std::runtime_error("Unsupported PBE " + pbe_algo);
   return std::make_pair(request.arg(1), request.arg(0));
   }

}

/*
* BER encode a PKCS #8 private key, encrypted
*/
std::vector<byte> BER_encode(const Private_Key& key,
                             RandomNumberGenerator& rng,
                             const std::string& pass,
                             std::chrono::milliseconds msec,
                             const std::string& pbe_algo)
   {
   const auto pbe_params = choose_pbe_params(pbe_algo, key.algo_name());

   const std::pair<AlgorithmIdentifier, std::vector<byte>> pbe_info =
      pbes2_encrypt(PKCS8::BER_encode(key), pass, msec,
                    pbe_params.first, pbe_params.second, rng);

   return DER_Encoder()
         .start_cons(SEQUENCE)
            .encode(pbe_info.first)
            .encode(pbe_info.second, OCTET_STRING)
         .end_cons()
      .get_contents_unlocked();
   }

/*
* PEM encode a PKCS #8 private key, encrypted
*/
std::string PEM_encode(const Private_Key& key,
                       RandomNumberGenerator& rng,
                       const std::string& pass,
                       std::chrono::milliseconds msec,
                       const std::string& pbe_algo)
   {
   if(pass == "")
      return PEM_encode(key);

   return PEM_Code::encode(PKCS8::BER_encode(key, rng, pass, msec, pbe_algo),
                           "ENCRYPTED PRIVATE KEY");
   }

/*
* Extract a private key and return it
*/
Private_Key* load_key(DataSource& source,
                      RandomNumberGenerator& rng,
                      std::function<std::string ()> get_pass)
   {
   AlgorithmIdentifier alg_id;
   secure_vector<byte> pkcs8_key = PKCS8_decode(source, get_pass, alg_id);

   const std::string alg_name = OIDS::lookup(alg_id.oid);
   if(alg_name == "" || alg_name == alg_id.oid.as_string())
      throw PKCS8_Exception("Unknown algorithm OID: " +
                            alg_id.oid.as_string());

   return make_private_key(alg_id, pkcs8_key, rng);
   }

/*
* Extract a private key and return it
*/
Private_Key* load_key(const std::string& fsname,
                      RandomNumberGenerator& rng,
                      std::function<std::string ()> get_pass)
   {
   DataSource_Stream source(fsname, true);
   return PKCS8::load_key(source, rng, get_pass);
   }

/*
* Extract a private key and return it
*/
Private_Key* load_key(DataSource& source,
                      RandomNumberGenerator& rng,
                      const std::string& pass)
   {
   return PKCS8::load_key(source, rng, [pass]() { return pass; });
   }

/*
* Extract a private key and return it
*/
Private_Key* load_key(const std::string& fsname,
                      RandomNumberGenerator& rng,
                      const std::string& pass)
   {
   return PKCS8::load_key(fsname, rng, [pass]() { return pass; });
   }

/*
* Make a copy of this private key
*/
Private_Key* copy_key(const Private_Key& key,
                      RandomNumberGenerator& rng)
   {
   DataSource_Memory source(PEM_encode(key));
   return PKCS8::load_key(source, rng);
   }

}

}
/*
* (C) 1999-2010,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

template<typename T, typename Key>
T* get_pk_op(const std::string& what, const Key& key, const std::string& pad,
             const std::string& provider = "")
   {
   if(T* p = Algo_Registry<T>::global_registry().make(typename T::Spec(key, pad), provider))
      return p;

   const std::string err = what + " with " + key.algo_name() + "/" + pad + " not supported";
   if(provider != "")
      throw Lookup_Error(err + " with provider " + provider);
   else
      throw Lookup_Error(err);
   }

}

PK_Encryptor_EME::PK_Encryptor_EME(const Public_Key& key,
                                   const std::string& padding,
                                   const std::string& provider)
   {
   m_op.reset(get_pk_op<PK_Ops::Encryption>("Encryption", key, padding, provider));
   }

std::vector<byte>
PK_Encryptor_EME::enc(const byte in[], size_t length, RandomNumberGenerator& rng) const
   {
   return unlock(m_op->encrypt(in, length, rng));
   }

size_t PK_Encryptor_EME::maximum_input_size() const
   {
   return m_op->max_input_bits() / 8;
   }

PK_Decryptor_EME::PK_Decryptor_EME(const Private_Key& key, const std::string& padding,
                                   const std::string& provider)
   {
   m_op.reset(get_pk_op<PK_Ops::Decryption>("Decryption", key, padding, provider));
   }

secure_vector<byte> PK_Decryptor_EME::dec(const byte msg[], size_t length) const
   {
   return m_op->decrypt(msg, length);
   }

PK_Key_Agreement::PK_Key_Agreement(const Private_Key& key, const std::string& kdf)
   {
   m_op.reset(get_pk_op<PK_Ops::Key_Agreement>("Key agreement", key, kdf));
   }

SymmetricKey PK_Key_Agreement::derive_key(size_t key_len,
                                          const byte in[], size_t in_len,
                                          const byte salt[],
                                          size_t salt_len) const
   {
   return m_op->agree(key_len, in, in_len, salt, salt_len);
   }

namespace {

std::vector<byte> der_encode_signature(const std::vector<byte>& sig, size_t parts)
   {
   if(sig.size() % parts)
      throw Encoding_Error("PK_Signer: strange signature size found");
   const size_t SIZE_OF_PART = sig.size() / parts;

   std::vector<BigInt> sig_parts(parts);
   for(size_t j = 0; j != sig_parts.size(); ++j)
      sig_parts[j].binary_decode(&sig[SIZE_OF_PART*j], SIZE_OF_PART);

   return DER_Encoder()
      .start_cons(SEQUENCE)
      .encode_list(sig_parts)
      .end_cons()
      .get_contents_unlocked();
   }

std::vector<byte> der_decode_signature(const byte sig[], size_t len,
                                       size_t part_size, size_t parts)
   {
   std::vector<byte> real_sig;
   BER_Decoder decoder(sig, len);
   BER_Decoder ber_sig = decoder.start_cons(SEQUENCE);

   size_t count = 0;
   while(ber_sig.more_items())
      {
      BigInt sig_part;
      ber_sig.decode(sig_part);
      real_sig += BigInt::encode_1363(sig_part, part_size);
      ++count;
      }

   if(count != parts)
      throw Decoding_Error("PK_Verifier: signature size invalid");
   return real_sig;
   }

}

PK_Signer::PK_Signer(const Private_Key& key,
                     const std::string& emsa,
                     Signature_Format format,
                     const std::string& provider)
   {
   m_op.reset(get_pk_op<PK_Ops::Signature>("Signing", key, emsa, provider));
   m_sig_format = format;
   }

void PK_Signer::update(const byte in[], size_t length)
   {
   m_op->update(in, length);
   }

std::vector<byte> PK_Signer::signature(RandomNumberGenerator& rng)
   {
   const std::vector<byte> plain_sig = unlock(m_op->sign(rng));
   const size_t parts = m_op->message_parts();

   if(parts == 1 || m_sig_format == IEEE_1363)
      return plain_sig;
   else if(m_sig_format == DER_SEQUENCE)
      return der_encode_signature(plain_sig, parts);
   else
      throw Encoding_Error("PK_Signer: Unknown signature format " +
                           std::to_string(m_sig_format));
   }

PK_Verifier::PK_Verifier(const Public_Key& key,
                         const std::string& emsa_name,
                         Signature_Format format,
                         const std::string& provider)
   {
   m_op.reset(get_pk_op<PK_Ops::Verification>("Verification", key, emsa_name, provider));
   m_sig_format = format;
   }

void PK_Verifier::set_input_format(Signature_Format format)
   {
   if(m_op->message_parts() == 1 && format != IEEE_1363)
      throw Invalid_State("PK_Verifier: This algorithm always uses IEEE 1363");
   m_sig_format = format;
   }

bool PK_Verifier::verify_message(const byte msg[], size_t msg_length,
                                 const byte sig[], size_t sig_length)
   {
   update(msg, msg_length);
   return check_signature(sig, sig_length);
   }

void PK_Verifier::update(const byte in[], size_t length)
   {
   m_op->update(in, length);
   }

bool PK_Verifier::check_signature(const byte sig[], size_t length)
   {
   try {
      if(m_sig_format == IEEE_1363)
         {
         return m_op->is_valid_signature(sig, length);
         }
      else if(m_sig_format == DER_SEQUENCE)
         {
         std::vector<byte> real_sig = der_decode_signature(sig, length,
                                                           m_op->message_part_size(),
                                                           m_op->message_parts());

         return m_op->is_valid_signature(real_sig.data(), real_sig.size());
         }
      else
         throw Decoding_Error("PK_Verifier: Unknown signature format " +
                              std::to_string(m_sig_format));
      }
   catch(Invalid_Argument) { return false; }
   }

}
/*
* Public Key Work Factor Functions
* (C) 1999-2007,2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <cmath>

namespace Botan {

size_t ecp_work_factor(size_t bits)
   {
   return bits / 2;
   }

size_t if_work_factor(size_t bits)
   {
   // RFC 3766: k * e^((1.92 + o(1)) * cubrt(ln(n) * (ln(ln(n)))^2))
   // It estimates k at .02 and o(1) to be effectively zero for sizes of interest
   const double k = .02;

   // approximates natural logarithm of p
   const double log2_e = std::log2(std::exp(1));
   const double log_p = bits / log2_e;

   const double est = 1.92 * std::pow(log_p * std::log(log_p) * std::log(log_p), 1.0/3.0);

   return static_cast<size_t>(std::log2(k) + log2_e * est);
   }

size_t dl_work_factor(size_t bits)
   {
   // Lacking better estimates...
   return if_work_factor(bits);
   }

size_t dl_exponent_size(size_t bits)
   {
   /*
   This uses a slightly tweaked version of the standard work factor
   function above. It assumes k is 1 (thus overestimating the strength
   of the prime group by 5-6 bits), and always returns at least 128 bits
   (this only matters for very small primes).
   */
   const size_t MIN_WORKFACTOR = 64;
   const double log2_e = std::log2(std::exp(1));
   const double log_p = bits / log2_e;

   const double strength = 1.92 * std::pow(log_p, 1.0/3.0) * std::pow(std::log(log_p), 2.0/3.0);

   return 2 * std::max<size_t>(MIN_WORKFACTOR, log2_e * strength);
   }

}
/*
* X.509 Public Key
* (C) 1999-2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace X509 {

std::vector<byte> BER_encode(const Public_Key& key)
   {
   return DER_Encoder()
         .start_cons(SEQUENCE)
            .encode(key.algorithm_identifier())
            .encode(key.x509_subject_public_key(), BIT_STRING)
         .end_cons()
      .get_contents_unlocked();
   }

/*
* PEM encode a X.509 public key
*/
std::string PEM_encode(const Public_Key& key)
   {
   return PEM_Code::encode(X509::BER_encode(key),
                           "PUBLIC KEY");
   }

/*
* Extract a public key and return it
*/
Public_Key* load_key(DataSource& source)
   {
   try {
      AlgorithmIdentifier alg_id;
      secure_vector<byte> key_bits;

      if(ASN1::maybe_BER(source) && !PEM_Code::matches(source))
         {
         BER_Decoder(source)
            .start_cons(SEQUENCE)
            .decode(alg_id)
            .decode(key_bits, BIT_STRING)
            .verify_end()
         .end_cons();
         }
      else
         {
         DataSource_Memory ber(
            PEM_Code::decode_check_label(source, "PUBLIC KEY")
            );

         BER_Decoder(ber)
            .start_cons(SEQUENCE)
            .decode(alg_id)
            .decode(key_bits, BIT_STRING)
            .verify_end()
         .end_cons();
         }

      if(key_bits.empty())
         throw Decoding_Error("X.509 public key decoding failed");

      return make_public_key(alg_id, key_bits);
      }
   catch(Decoding_Error& e)
      {
      throw Decoding_Error("X.509 public key decoding failed: " + std::string(e.what()));
      }
   }

/*
* Extract a public key and return it
*/
Public_Key* load_key(const std::string& fsname)
   {
   DataSource_Stream source(fsname, true);
   return X509::load_key(source);
   }

/*
* Extract a public key and return it
*/
Public_Key* load_key(const std::vector<byte>& mem)
   {
   DataSource_Memory source(mem);
   return X509::load_key(source);
   }

/*
* Make a copy of this public key
*/
Public_Key* copy_key(const Public_Key& key)
   {
   DataSource_Memory source(PEM_encode(key));
   return X509::load_key(source);
   }

}

}
/*
* Random Number Generator
* (C) 1999-2008 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

size_t RandomNumberGenerator::reseed(size_t bits_to_collect)
   {
   return this->reseed_with_timeout(bits_to_collect,
                                    BOTAN_RNG_RESEED_DEFAULT_TIMEOUT);
   }

size_t RandomNumberGenerator::reseed_with_timeout(size_t bits_to_collect,
                                                  std::chrono::milliseconds timeout)
   {
   return this->reseed_with_sources(Entropy_Sources::global_sources(),
                                    bits_to_collect,
                                    timeout);
   }

RandomNumberGenerator* RandomNumberGenerator::make_rng()
   {
   std::unique_ptr<MessageAuthenticationCode> h1(MessageAuthenticationCode::create("HMAC(SHA-512)"));
   std::unique_ptr<MessageAuthenticationCode> h2(MessageAuthenticationCode::create("HMAC(SHA-512)"));

   if(!h1 || !h2)
      throw Algorithm_Not_Found("HMAC_RNG HMACs");
   std::unique_ptr<RandomNumberGenerator> rng(new HMAC_RNG(h1.release(), h2.release()));

   rng->reseed(256);

   return rng.release();
   }

}
/*
* RSA operations provided by OpenSSL
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_OPENSSL)


#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/err.h>

namespace Botan {

namespace {

std::pair<int, size_t> get_openssl_enc_pad(const std::string& eme)
   {
   ERR_load_crypto_strings();
   if(eme == "Raw")
      return std::make_pair(RSA_NO_PADDING, 0);
   else if(eme == "EME-PKCS1-v1_5")
      return std::make_pair(RSA_PKCS1_PADDING, 11);
   else if(eme == "OAEP(SHA-1)")
      return std::make_pair(RSA_PKCS1_OAEP_PADDING, 41);
   else
      throw Lookup_Error("OpenSSL RSA does not support EME " + eme);
   }

class OpenSSL_RSA_Encryption_Operation : public PK_Ops::Encryption
   {
   public:
      typedef RSA_PublicKey Key_Type;

      static OpenSSL_RSA_Encryption_Operation* make(const Spec& spec)
         {
         try
            {
            if(auto* key = dynamic_cast<const RSA_PublicKey*>(&spec.key()))
               {
               auto pad_info = get_openssl_enc_pad(spec.padding());
               return new OpenSSL_RSA_Encryption_Operation(*key, pad_info.first, pad_info.second);
               }
            }
         catch(...) {}

         return nullptr;
         }

      OpenSSL_RSA_Encryption_Operation(const RSA_PublicKey& rsa, int pad, size_t pad_overhead) :
         m_openssl_rsa(nullptr, ::RSA_free), m_padding(pad)
         {
         const std::vector<byte> der = rsa.x509_subject_public_key();
         const byte* der_ptr = der.data();
         m_openssl_rsa.reset(::d2i_RSAPublicKey(nullptr, &der_ptr, der.size()));
         if(!m_openssl_rsa)
            throw OpenSSL_Error("d2i_RSAPublicKey");

         m_bits = 8 * (n_size() - pad_overhead) - 1;
         }

      size_t max_input_bits() const override { return m_bits; };

      secure_vector<byte> encrypt(const byte msg[], size_t msg_len,
                                  RandomNumberGenerator&) override
         {
         const size_t mod_sz = n_size();

         if(msg_len > mod_sz)
            throw Invalid_Argument("Input too large for RSA key");

         secure_vector<byte> outbuf(mod_sz);

         secure_vector<byte> inbuf;

         if(m_padding == RSA_NO_PADDING)
            {
            inbuf.resize(mod_sz);
            copy_mem(&inbuf[mod_sz - msg_len], msg, msg_len);
            }
         else
            {
            inbuf.assign(msg, msg + msg_len);
            }

         int rc = ::RSA_public_encrypt(inbuf.size(), inbuf.data(), outbuf.data(),
                                       m_openssl_rsa.get(), m_padding);
         if(rc < 0)
            throw OpenSSL_Error("RSA_public_encrypt");

         return outbuf;
         }

   private:
      size_t n_size() const { return ::RSA_size(m_openssl_rsa.get()); }
      std::unique_ptr<RSA, std::function<void (RSA*)>> m_openssl_rsa;
      size_t m_bits = 0;
      int m_padding = 0;
   };

class OpenSSL_RSA_Decryption_Operation : public PK_Ops::Decryption
   {
   public:
      typedef RSA_PrivateKey Key_Type;

      static OpenSSL_RSA_Decryption_Operation* make(const Spec& spec)
         {
         try
            {
            if(auto* key = dynamic_cast<const RSA_PrivateKey*>(&spec.key()))
               {
               auto pad_info = get_openssl_enc_pad(spec.padding());
               return new OpenSSL_RSA_Decryption_Operation(*key, pad_info.first);
               }
            }
         catch(...) {}

         return nullptr;
         }

      OpenSSL_RSA_Decryption_Operation(const RSA_PrivateKey& rsa, int pad) :
         m_openssl_rsa(nullptr, ::RSA_free), m_padding(pad)
         {
         const secure_vector<byte> der = rsa.pkcs8_private_key();
         const byte* der_ptr = der.data();
         m_openssl_rsa.reset(d2i_RSAPrivateKey(nullptr, &der_ptr, der.size()));
         if(!m_openssl_rsa)
            throw OpenSSL_Error("d2i_RSAPrivateKey");
         }

      size_t max_input_bits() const override { return ::BN_num_bits(m_openssl_rsa->n) - 1; }

      secure_vector<byte> decrypt(const byte msg[], size_t msg_len) override
         {
         secure_vector<byte> buf(::RSA_size(m_openssl_rsa.get()));
         int rc = ::RSA_private_decrypt(msg_len, msg, buf.data(), m_openssl_rsa.get(), m_padding);
         if(rc < 0 || static_cast<size_t>(rc) > buf.size())
            throw OpenSSL_Error("RSA_private_decrypt");
         buf.resize(rc);

         if(m_padding == RSA_NO_PADDING)
            {
            return CT::strip_leading_zeros(buf);
            }

         return buf;
         }

   private:
      std::unique_ptr<RSA, std::function<void (RSA*)>> m_openssl_rsa;
      int m_padding = 0;
   };

class OpenSSL_RSA_Verification_Operation : public PK_Ops::Verification_with_EMSA
   {
   public:
      typedef RSA_PublicKey Key_Type;

      static OpenSSL_RSA_Verification_Operation* make(const Spec& spec)
         {
         if(const RSA_PublicKey* rsa = dynamic_cast<const RSA_PublicKey*>(&spec.key()))
            {
            return new OpenSSL_RSA_Verification_Operation(*rsa, spec.padding());
            }

         return nullptr;
         }

      OpenSSL_RSA_Verification_Operation(const RSA_PublicKey& rsa, const std::string& emsa) :
         PK_Ops::Verification_with_EMSA(emsa),
         m_openssl_rsa(nullptr, ::RSA_free)
         {
         const std::vector<byte> der = rsa.x509_subject_public_key();
         const byte* der_ptr = der.data();
         m_openssl_rsa.reset(::d2i_RSAPublicKey(nullptr, &der_ptr, der.size()));
         }

      size_t max_input_bits() const override { return ::BN_num_bits(m_openssl_rsa->n) - 1; }

      bool with_recovery() const override { return true; }

      secure_vector<byte> verify_mr(const byte msg[], size_t msg_len) override
         {
         const size_t mod_sz = ::RSA_size(m_openssl_rsa.get());

         if(msg_len > mod_sz)
            throw Invalid_Argument("OpenSSL RSA verify input too large");

         secure_vector<byte> inbuf(mod_sz);
         copy_mem(&inbuf[mod_sz - msg_len], msg, msg_len);

         secure_vector<byte> outbuf(mod_sz);

         int rc = ::RSA_public_decrypt(inbuf.size(), inbuf.data(), outbuf.data(),
                                       m_openssl_rsa.get(), RSA_NO_PADDING);
         if(rc < 0)
            throw Invalid_Argument("RSA_public_decrypt");

         return CT::strip_leading_zeros(outbuf);
         }
   private:
      std::unique_ptr<RSA, std::function<void (RSA*)>> m_openssl_rsa;
   };

class OpenSSL_RSA_Signing_Operation : public PK_Ops::Signature_with_EMSA
   {
   public:
      typedef RSA_PrivateKey Key_Type;

      static OpenSSL_RSA_Signing_Operation* make(const Spec& spec)
         {
         if(const RSA_PrivateKey* rsa = dynamic_cast<const RSA_PrivateKey*>(&spec.key()))
            {
            return new OpenSSL_RSA_Signing_Operation(*rsa, spec.padding());
            }

         return nullptr;
         }

      OpenSSL_RSA_Signing_Operation(const RSA_PrivateKey& rsa, const std::string& emsa) :
         PK_Ops::Signature_with_EMSA(emsa),
         m_openssl_rsa(nullptr, ::RSA_free)
         {
         const secure_vector<byte> der = rsa.pkcs8_private_key();
         const byte* der_ptr = der.data();
         m_openssl_rsa.reset(d2i_RSAPrivateKey(nullptr, &der_ptr, der.size()));
         if(!m_openssl_rsa)
            throw OpenSSL_Error("d2i_RSAPrivateKey");
         }

      secure_vector<byte> raw_sign(const byte msg[], size_t msg_len,
                                   RandomNumberGenerator&) override
         {
         const size_t mod_sz = ::RSA_size(m_openssl_rsa.get());

         if(msg_len > mod_sz)
            throw Invalid_Argument("OpenSSL RSA sign input too large");

         secure_vector<byte> inbuf(mod_sz);
         copy_mem(&inbuf[mod_sz - msg_len], msg, msg_len);

         secure_vector<byte> outbuf(mod_sz);

         int rc = ::RSA_private_encrypt(inbuf.size(), inbuf.data(), outbuf.data(),
                                        m_openssl_rsa.get(), RSA_NO_PADDING);
         if(rc < 0)
            throw OpenSSL_Error("RSA_private_encrypt");

         return outbuf;
         }

      size_t max_input_bits() const override { return ::BN_num_bits(m_openssl_rsa->n) - 1; }

   private:
      std::unique_ptr<RSA, std::function<void (RSA*)>> m_openssl_rsa;
   };

BOTAN_REGISTER_TYPE(PK_Ops::Verification, OpenSSL_RSA_Verification_Operation, "RSA",
                    OpenSSL_RSA_Verification_Operation::make, "openssl", BOTAN_OPENSSL_RSA_PRIO);

BOTAN_REGISTER_TYPE(PK_Ops::Signature, OpenSSL_RSA_Signing_Operation, "RSA",
                    OpenSSL_RSA_Signing_Operation::make, "openssl", BOTAN_OPENSSL_RSA_PRIO);

BOTAN_REGISTER_TYPE(PK_Ops::Encryption, OpenSSL_RSA_Encryption_Operation, "RSA",
                    OpenSSL_RSA_Encryption_Operation::make, "openssl", BOTAN_OPENSSL_RSA_PRIO);

BOTAN_REGISTER_TYPE(PK_Ops::Decryption, OpenSSL_RSA_Decryption_Operation, "RSA",
                    OpenSSL_RSA_Decryption_Operation::make, "openssl", BOTAN_OPENSSL_RSA_PRIO);

}

}

#endif // BOTAN_HAS_OPENSSL
/*
* RSA
* (C) 1999-2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <future>

namespace Botan {

/*
* Create a RSA private key
*/
RSA_PrivateKey::RSA_PrivateKey(RandomNumberGenerator& rng,
                               size_t bits, size_t exp)
   {
   if(bits < 1024)
      throw Invalid_Argument(algo_name() + ": Can't make a key that is only " +
                             std::to_string(bits) + " bits long");
   if(exp < 3 || exp % 2 == 0)
      throw Invalid_Argument(algo_name() + ": Invalid encryption exponent");

   e = exp;

   do
      {
      p = random_prime(rng, (bits + 1) / 2, e);
      q = random_prime(rng, bits - p.bits(), e);
      n = p * q;
      } while(n.bits() != bits);

   d = inverse_mod(e, lcm(p - 1, q - 1));
   d1 = d % (p - 1);
   d2 = d % (q - 1);
   c = inverse_mod(q, p);

   gen_check(rng);
   }

/*
* Check Private RSA Parameters
*/
bool RSA_PrivateKey::check_key(RandomNumberGenerator& rng, bool strong) const
   {
   if(!IF_Scheme_PrivateKey::check_key(rng, strong))
      return false;

   if(!strong)
      return true;

   if((e * d) % lcm(p - 1, q - 1) != 1)
      return false;

   return KeyPair::signature_consistency_check(rng, *this, "EMSA4(SHA-1)");
   }

namespace {

/**
* RSA private (decrypt/sign) operation
*/
class RSA_Private_Operation
   {
   protected:
      size_t get_max_input_bits() const { return (n.bits() - 1); }

      RSA_Private_Operation(const RSA_PrivateKey& rsa) :
         n(rsa.get_n()),
         q(rsa.get_q()),
         c(rsa.get_c()),
         m_powermod_e_n(rsa.get_e(), rsa.get_n()),
         m_powermod_d1_p(rsa.get_d1(), rsa.get_p()),
         m_powermod_d2_q(rsa.get_d2(), rsa.get_q()),
         m_mod_p(rsa.get_p()),
         m_blinder(n,
                   [this](const BigInt& k) { return m_powermod_e_n(k); },
                   [this](const BigInt& k) { return inverse_mod(k, n); })
         {
         }

      BigInt blinded_private_op(const BigInt& m) const
         {
         if(m >= n)
            throw Invalid_Argument("RSA private op - input is too large");

         return m_blinder.unblind(private_op(m_blinder.blind(m)));
         }

      BigInt private_op(const BigInt& m) const
         {
         auto future_j1 = std::async(std::launch::async, m_powermod_d1_p, m);
         BigInt j2 = m_powermod_d2_q(m);
         BigInt j1 = future_j1.get();

         j1 = m_mod_p.reduce(sub_mul(j1, j2, c));

         return mul_add(j1, q, j2);
         }

      const BigInt& n;
      const BigInt& q;
      const BigInt& c;
      Fixed_Exponent_Power_Mod m_powermod_e_n, m_powermod_d1_p, m_powermod_d2_q;
      Modular_Reducer m_mod_p;
      Blinder m_blinder;
   };

class RSA_Signature_Operation : public PK_Ops::Signature_with_EMSA,
                                private RSA_Private_Operation
   {
   public:
      typedef RSA_PrivateKey Key_Type;

      size_t max_input_bits() const override { return get_max_input_bits(); };

      RSA_Signature_Operation(const RSA_PrivateKey& rsa, const std::string& emsa) :
         PK_Ops::Signature_with_EMSA(emsa),
         RSA_Private_Operation(rsa)
         {
         }

      secure_vector<byte> raw_sign(const byte msg[], size_t msg_len,
                                   RandomNumberGenerator&) override
         {
         const BigInt m(msg, msg_len);
         const BigInt x = blinded_private_op(m);
         const BigInt c = m_powermod_e_n(x);
         BOTAN_ASSERT(m == c, "RSA sign consistency check");
         return BigInt::encode_1363(x, n.bytes());
         }
   };

class RSA_Decryption_Operation : public PK_Ops::Decryption_with_EME,
                                 private RSA_Private_Operation
   {
   public:
      typedef RSA_PrivateKey Key_Type;

      size_t max_raw_input_bits() const override { return get_max_input_bits(); };

      RSA_Decryption_Operation(const RSA_PrivateKey& rsa, const std::string& eme) :
         PK_Ops::Decryption_with_EME(eme),
         RSA_Private_Operation(rsa)
         {
         }

      secure_vector<byte> raw_decrypt(const byte msg[], size_t msg_len) override
         {
         const BigInt m(msg, msg_len);
         const BigInt x = blinded_private_op(m);
         const BigInt c = m_powermod_e_n(x);
         BOTAN_ASSERT(m == c, "RSA sign consistency check");
         return BigInt::encode_locked(x);
         }
   };

/**
* RSA public (encrypt/verify) operation
*/
class RSA_Public_Operation
   {
   public:
      RSA_Public_Operation(const RSA_PublicKey& rsa) :
         n(rsa.get_n()), powermod_e_n(rsa.get_e(), rsa.get_n())
         {}

      size_t get_max_input_bits() const { return (n.bits() - 1); }

   protected:
      BigInt public_op(const BigInt& m) const
         {
         if(m >= n)
            throw Invalid_Argument("RSA public op - input is too large");
         return powermod_e_n(m);
         }

      const BigInt& n;
      Fixed_Exponent_Power_Mod powermod_e_n;
   };

class RSA_Encryption_Operation : public PK_Ops::Encryption_with_EME,
                                 private RSA_Public_Operation
   {
   public:
      typedef RSA_PublicKey Key_Type;

      RSA_Encryption_Operation(const RSA_PublicKey& rsa, const std::string& eme) :
         PK_Ops::Encryption_with_EME(eme),
         RSA_Public_Operation(rsa)
         {
         }

      size_t max_raw_input_bits() const override { return get_max_input_bits(); };

      secure_vector<byte> raw_encrypt(const byte msg[], size_t msg_len,
                                      RandomNumberGenerator&) override
         {
         BigInt m(msg, msg_len);
         return BigInt::encode_1363(public_op(m), n.bytes());
         }
   };

class RSA_Verify_Operation : public PK_Ops::Verification_with_EMSA,
                             private RSA_Public_Operation
   {
   public:
      typedef RSA_PublicKey Key_Type;

      size_t max_input_bits() const override { return get_max_input_bits(); };

      RSA_Verify_Operation(const RSA_PublicKey& rsa, const std::string& emsa) :
         PK_Ops::Verification_with_EMSA(emsa),
         RSA_Public_Operation(rsa)
         {
         }

      bool with_recovery() const override { return true; }

      secure_vector<byte> verify_mr(const byte msg[], size_t msg_len) override
         {
         BigInt m(msg, msg_len);
         return BigInt::encode_locked(public_op(m));
         }
   };

BOTAN_REGISTER_PK_ENCRYPTION_OP("RSA", RSA_Encryption_Operation);
BOTAN_REGISTER_PK_DECRYPTION_OP("RSA", RSA_Decryption_Operation);
BOTAN_REGISTER_PK_SIGNATURE_OP("RSA", RSA_Signature_Operation);
BOTAN_REGISTER_PK_VERIFY_OP("RSA", RSA_Verify_Operation);

}

}
/*
* SHA-160
* (C) 1999-2008,2011 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace SHA1_F {

namespace {

/*
* SHA-160 F1 Function
*/
inline void F1(u32bit A, u32bit& B, u32bit C, u32bit D, u32bit& E, u32bit msg)
   {
   E += (D ^ (B & (C ^ D))) + msg + 0x5A827999 + rotate_left(A, 5);
   B  = rotate_left(B, 30);
   }

/*
* SHA-160 F2 Function
*/
inline void F2(u32bit A, u32bit& B, u32bit C, u32bit D, u32bit& E, u32bit msg)
   {
   E += (B ^ C ^ D) + msg + 0x6ED9EBA1 + rotate_left(A, 5);
   B  = rotate_left(B, 30);
   }

/*
* SHA-160 F3 Function
*/
inline void F3(u32bit A, u32bit& B, u32bit C, u32bit D, u32bit& E, u32bit msg)
   {
   E += ((B & C) | ((B | C) & D)) + msg + 0x8F1BBCDC + rotate_left(A, 5);
   B  = rotate_left(B, 30);
   }

/*
* SHA-160 F4 Function
*/
inline void F4(u32bit A, u32bit& B, u32bit C, u32bit D, u32bit& E, u32bit msg)
   {
   E += (B ^ C ^ D) + msg + 0xCA62C1D6 + rotate_left(A, 5);
   B  = rotate_left(B, 30);
   }

}

}

/*
* SHA-160 Compression Function
*/
void SHA_160::compress_n(const byte input[], size_t blocks)
   {
   using namespace SHA1_F;

   u32bit A = digest[0], B = digest[1], C = digest[2],
          D = digest[3], E = digest[4];

   for(size_t i = 0; i != blocks; ++i)
      {
      load_be(W.data(), input, 16);

      for(size_t j = 16; j != 80; j += 8)
         {
         W[j  ] = rotate_left((W[j-3] ^ W[j-8] ^ W[j-14] ^ W[j-16]), 1);
         W[j+1] = rotate_left((W[j-2] ^ W[j-7] ^ W[j-13] ^ W[j-15]), 1);
         W[j+2] = rotate_left((W[j-1] ^ W[j-6] ^ W[j-12] ^ W[j-14]), 1);
         W[j+3] = rotate_left((W[j  ] ^ W[j-5] ^ W[j-11] ^ W[j-13]), 1);
         W[j+4] = rotate_left((W[j+1] ^ W[j-4] ^ W[j-10] ^ W[j-12]), 1);
         W[j+5] = rotate_left((W[j+2] ^ W[j-3] ^ W[j- 9] ^ W[j-11]), 1);
         W[j+6] = rotate_left((W[j+3] ^ W[j-2] ^ W[j- 8] ^ W[j-10]), 1);
         W[j+7] = rotate_left((W[j+4] ^ W[j-1] ^ W[j- 7] ^ W[j- 9]), 1);
         }

      F1(A, B, C, D, E, W[ 0]);   F1(E, A, B, C, D, W[ 1]);
      F1(D, E, A, B, C, W[ 2]);   F1(C, D, E, A, B, W[ 3]);
      F1(B, C, D, E, A, W[ 4]);   F1(A, B, C, D, E, W[ 5]);
      F1(E, A, B, C, D, W[ 6]);   F1(D, E, A, B, C, W[ 7]);
      F1(C, D, E, A, B, W[ 8]);   F1(B, C, D, E, A, W[ 9]);
      F1(A, B, C, D, E, W[10]);   F1(E, A, B, C, D, W[11]);
      F1(D, E, A, B, C, W[12]);   F1(C, D, E, A, B, W[13]);
      F1(B, C, D, E, A, W[14]);   F1(A, B, C, D, E, W[15]);
      F1(E, A, B, C, D, W[16]);   F1(D, E, A, B, C, W[17]);
      F1(C, D, E, A, B, W[18]);   F1(B, C, D, E, A, W[19]);

      F2(A, B, C, D, E, W[20]);   F2(E, A, B, C, D, W[21]);
      F2(D, E, A, B, C, W[22]);   F2(C, D, E, A, B, W[23]);
      F2(B, C, D, E, A, W[24]);   F2(A, B, C, D, E, W[25]);
      F2(E, A, B, C, D, W[26]);   F2(D, E, A, B, C, W[27]);
      F2(C, D, E, A, B, W[28]);   F2(B, C, D, E, A, W[29]);
      F2(A, B, C, D, E, W[30]);   F2(E, A, B, C, D, W[31]);
      F2(D, E, A, B, C, W[32]);   F2(C, D, E, A, B, W[33]);
      F2(B, C, D, E, A, W[34]);   F2(A, B, C, D, E, W[35]);
      F2(E, A, B, C, D, W[36]);   F2(D, E, A, B, C, W[37]);
      F2(C, D, E, A, B, W[38]);   F2(B, C, D, E, A, W[39]);

      F3(A, B, C, D, E, W[40]);   F3(E, A, B, C, D, W[41]);
      F3(D, E, A, B, C, W[42]);   F3(C, D, E, A, B, W[43]);
      F3(B, C, D, E, A, W[44]);   F3(A, B, C, D, E, W[45]);
      F3(E, A, B, C, D, W[46]);   F3(D, E, A, B, C, W[47]);
      F3(C, D, E, A, B, W[48]);   F3(B, C, D, E, A, W[49]);
      F3(A, B, C, D, E, W[50]);   F3(E, A, B, C, D, W[51]);
      F3(D, E, A, B, C, W[52]);   F3(C, D, E, A, B, W[53]);
      F3(B, C, D, E, A, W[54]);   F3(A, B, C, D, E, W[55]);
      F3(E, A, B, C, D, W[56]);   F3(D, E, A, B, C, W[57]);
      F3(C, D, E, A, B, W[58]);   F3(B, C, D, E, A, W[59]);

      F4(A, B, C, D, E, W[60]);   F4(E, A, B, C, D, W[61]);
      F4(D, E, A, B, C, W[62]);   F4(C, D, E, A, B, W[63]);
      F4(B, C, D, E, A, W[64]);   F4(A, B, C, D, E, W[65]);
      F4(E, A, B, C, D, W[66]);   F4(D, E, A, B, C, W[67]);
      F4(C, D, E, A, B, W[68]);   F4(B, C, D, E, A, W[69]);
      F4(A, B, C, D, E, W[70]);   F4(E, A, B, C, D, W[71]);
      F4(D, E, A, B, C, W[72]);   F4(C, D, E, A, B, W[73]);
      F4(B, C, D, E, A, W[74]);   F4(A, B, C, D, E, W[75]);
      F4(E, A, B, C, D, W[76]);   F4(D, E, A, B, C, W[77]);
      F4(C, D, E, A, B, W[78]);   F4(B, C, D, E, A, W[79]);

      A = (digest[0] += A);
      B = (digest[1] += B);
      C = (digest[2] += C);
      D = (digest[3] += D);
      E = (digest[4] += E);

      input += hash_block_size();
      }
   }

/*
* Copy out the digest
*/
void SHA_160::copy_out(byte output[])
   {
   copy_out_vec_be(output, output_length(), digest);
   }

/*
* Clear memory of sensitive data
*/
void SHA_160::clear()
   {
   MDx_HashFunction::clear();
   zeroise(W);
   digest[0] = 0x67452301;
   digest[1] = 0xEFCDAB89;
   digest[2] = 0x98BADCFE;
   digest[3] = 0x10325476;
   digest[4] = 0xC3D2E1F0;
   }

}
/*
* SHA-{224,256}
* (C) 1999-2010 Jack Lloyd
*     2007 FlexSecure GmbH
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

namespace SHA2_32 {

/*
* SHA-256 Rho Function
*/
inline u32bit rho(u32bit X, u32bit rot1, u32bit rot2, u32bit rot3)
   {
   return (rotate_right(X, rot1) ^ rotate_right(X, rot2) ^
           rotate_right(X, rot3));
   }

/*
* SHA-256 Sigma Function
*/
inline u32bit sigma(u32bit X, u32bit rot1, u32bit rot2, u32bit shift)
   {
   return (rotate_right(X, rot1) ^ rotate_right(X, rot2) ^ (X >> shift));
   }

/*
* SHA-256 F1 Function
*
* Use a macro as many compilers won't inline a function this big,
* even though it is much faster if inlined.
*/
#define SHA2_32_F(A, B, C, D, E, F, G, H, M1, M2, M3, M4, magic)   \
   do {                                                            \
      H += magic + rho(E, 6, 11, 25) + ((E & F) ^ (~E & G)) + M1;  \
      D += H;                                                      \
      H += rho(A, 2, 13, 22) + ((A & B) | ((A | B) & C));          \
      M1 += sigma(M2, 17, 19, 10) + M3 + sigma(M4, 7, 18, 3);      \
   } while(0);

/*
* SHA-224 / SHA-256 compression function
*/
void compress(secure_vector<u32bit>& digest,
              const byte input[], size_t blocks)
   {
   u32bit A = digest[0], B = digest[1], C = digest[2],
          D = digest[3], E = digest[4], F = digest[5],
          G = digest[6], H = digest[7];

   for(size_t i = 0; i != blocks; ++i)
      {
      u32bit W00 = load_be<u32bit>(input,  0);
      u32bit W01 = load_be<u32bit>(input,  1);
      u32bit W02 = load_be<u32bit>(input,  2);
      u32bit W03 = load_be<u32bit>(input,  3);
      u32bit W04 = load_be<u32bit>(input,  4);
      u32bit W05 = load_be<u32bit>(input,  5);
      u32bit W06 = load_be<u32bit>(input,  6);
      u32bit W07 = load_be<u32bit>(input,  7);
      u32bit W08 = load_be<u32bit>(input,  8);
      u32bit W09 = load_be<u32bit>(input,  9);
      u32bit W10 = load_be<u32bit>(input, 10);
      u32bit W11 = load_be<u32bit>(input, 11);
      u32bit W12 = load_be<u32bit>(input, 12);
      u32bit W13 = load_be<u32bit>(input, 13);
      u32bit W14 = load_be<u32bit>(input, 14);
      u32bit W15 = load_be<u32bit>(input, 15);

      SHA2_32_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x428A2F98);
      SHA2_32_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x71374491);
      SHA2_32_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0xB5C0FBCF);
      SHA2_32_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0xE9B5DBA5);
      SHA2_32_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x3956C25B);
      SHA2_32_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x59F111F1);
      SHA2_32_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x923F82A4);
      SHA2_32_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0xAB1C5ED5);
      SHA2_32_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0xD807AA98);
      SHA2_32_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x12835B01);
      SHA2_32_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x243185BE);
      SHA2_32_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x550C7DC3);
      SHA2_32_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x72BE5D74);
      SHA2_32_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0x80DEB1FE);
      SHA2_32_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x9BDC06A7);
      SHA2_32_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0xC19BF174);
      SHA2_32_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0xE49B69C1);
      SHA2_32_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0xEFBE4786);
      SHA2_32_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x0FC19DC6);
      SHA2_32_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x240CA1CC);
      SHA2_32_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x2DE92C6F);
      SHA2_32_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x4A7484AA);
      SHA2_32_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x5CB0A9DC);
      SHA2_32_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x76F988DA);
      SHA2_32_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x983E5152);
      SHA2_32_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0xA831C66D);
      SHA2_32_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0xB00327C8);
      SHA2_32_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0xBF597FC7);
      SHA2_32_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0xC6E00BF3);
      SHA2_32_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xD5A79147);
      SHA2_32_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x06CA6351);
      SHA2_32_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x14292967);
      SHA2_32_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x27B70A85);
      SHA2_32_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x2E1B2138);
      SHA2_32_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x4D2C6DFC);
      SHA2_32_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x53380D13);
      SHA2_32_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x650A7354);
      SHA2_32_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x766A0ABB);
      SHA2_32_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x81C2C92E);
      SHA2_32_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x92722C85);
      SHA2_32_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0xA2BFE8A1);
      SHA2_32_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0xA81A664B);
      SHA2_32_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0xC24B8B70);
      SHA2_32_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0xC76C51A3);
      SHA2_32_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0xD192E819);
      SHA2_32_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xD6990624);
      SHA2_32_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0xF40E3585);
      SHA2_32_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x106AA070);
      SHA2_32_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x19A4C116);
      SHA2_32_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x1E376C08);
      SHA2_32_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x2748774C);
      SHA2_32_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x34B0BCB5);
      SHA2_32_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x391C0CB3);
      SHA2_32_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x4ED8AA4A);
      SHA2_32_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x5B9CCA4F);
      SHA2_32_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x682E6FF3);
      SHA2_32_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x748F82EE);
      SHA2_32_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x78A5636F);
      SHA2_32_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x84C87814);
      SHA2_32_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x8CC70208);
      SHA2_32_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x90BEFFFA);
      SHA2_32_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xA4506CEB);
      SHA2_32_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0xBEF9A3F7);
      SHA2_32_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0xC67178F2);

      A = (digest[0] += A);
      B = (digest[1] += B);
      C = (digest[2] += C);
      D = (digest[3] += D);
      E = (digest[4] += E);
      F = (digest[5] += F);
      G = (digest[6] += G);
      H = (digest[7] += H);

      input += 64;
      }
   }

}

}

/*
* SHA-224 compression function
*/
void SHA_224::compress_n(const byte input[], size_t blocks)
   {
   SHA2_32::compress(digest, input, blocks);
   }

/*
* Copy out the digest
*/
void SHA_224::copy_out(byte output[])
   {
   copy_out_vec_be(output, output_length(), digest);
   }

/*
* Clear memory of sensitive data
*/
void SHA_224::clear()
   {
   MDx_HashFunction::clear();
   digest[0] = 0xC1059ED8;
   digest[1] = 0x367CD507;
   digest[2] = 0x3070DD17;
   digest[3] = 0xF70E5939;
   digest[4] = 0xFFC00B31;
   digest[5] = 0x68581511;
   digest[6] = 0x64F98FA7;
   digest[7] = 0xBEFA4FA4;
   }

/*
* SHA-256 compression function
*/
void SHA_256::compress_n(const byte input[], size_t blocks)
   {
   SHA2_32::compress(digest, input, blocks);
   }

/*
* Copy out the digest
*/
void SHA_256::copy_out(byte output[])
   {
   copy_out_vec_be(output, output_length(), digest);
   }

/*
* Clear memory of sensitive data
*/
void SHA_256::clear()
   {
   MDx_HashFunction::clear();
   digest[0] = 0x6A09E667;
   digest[1] = 0xBB67AE85;
   digest[2] = 0x3C6EF372;
   digest[3] = 0xA54FF53A;
   digest[4] = 0x510E527F;
   digest[5] = 0x9B05688C;
   digest[6] = 0x1F83D9AB;
   digest[7] = 0x5BE0CD19;
   }

}
/*
* SHA-{384,512}
* (C) 1999-2011,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

namespace SHA2_64 {

/*
* SHA-{384,512} Rho Function
*/
inline u64bit rho(u64bit X, u32bit rot1, u32bit rot2, u32bit rot3)
   {
   return (rotate_right(X, rot1) ^ rotate_right(X, rot2) ^
           rotate_right(X, rot3));
   }

/*
* SHA-{384,512} Sigma Function
*/
inline u64bit sigma(u64bit X, u32bit rot1, u32bit rot2, u32bit shift)
   {
   return (rotate_right(X, rot1) ^ rotate_right(X, rot2) ^ (X >> shift));
   }

/*
* SHA-512 F1 Function
*
* Use a macro as many compilers won't inline a function this big,
* even though it is much faster if inlined.
*/
#define SHA2_64_F(A, B, C, D, E, F, G, H, M1, M2, M3, M4, magic)   \
   do {                                                            \
      H += magic + rho(E, 14, 18, 41) + ((E & F) ^ (~E & G)) + M1; \
      D += H;                                                      \
      H += rho(A, 28, 34, 39) + ((A & B) | ((A | B) & C));         \
      M1 += sigma(M2, 19, 61, 6) + M3 + sigma(M4, 1, 8, 7);        \
   } while(0);

/*
* SHA-{384,512} Compression Function
*/
void compress(secure_vector<u64bit>& digest,
              const byte input[], size_t blocks)
   {
   u64bit A = digest[0], B = digest[1], C = digest[2],
          D = digest[3], E = digest[4], F = digest[5],
          G = digest[6], H = digest[7];

   for(size_t i = 0; i != blocks; ++i)
      {
      u64bit W00 = load_be<u64bit>(input,  0);
      u64bit W01 = load_be<u64bit>(input,  1);
      u64bit W02 = load_be<u64bit>(input,  2);
      u64bit W03 = load_be<u64bit>(input,  3);
      u64bit W04 = load_be<u64bit>(input,  4);
      u64bit W05 = load_be<u64bit>(input,  5);
      u64bit W06 = load_be<u64bit>(input,  6);
      u64bit W07 = load_be<u64bit>(input,  7);
      u64bit W08 = load_be<u64bit>(input,  8);
      u64bit W09 = load_be<u64bit>(input,  9);
      u64bit W10 = load_be<u64bit>(input, 10);
      u64bit W11 = load_be<u64bit>(input, 11);
      u64bit W12 = load_be<u64bit>(input, 12);
      u64bit W13 = load_be<u64bit>(input, 13);
      u64bit W14 = load_be<u64bit>(input, 14);
      u64bit W15 = load_be<u64bit>(input, 15);

      SHA2_64_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x428A2F98D728AE22);
      SHA2_64_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x7137449123EF65CD);
      SHA2_64_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0xB5C0FBCFEC4D3B2F);
      SHA2_64_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0xE9B5DBA58189DBBC);
      SHA2_64_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x3956C25BF348B538);
      SHA2_64_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x59F111F1B605D019);
      SHA2_64_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x923F82A4AF194F9B);
      SHA2_64_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0xAB1C5ED5DA6D8118);
      SHA2_64_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0xD807AA98A3030242);
      SHA2_64_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x12835B0145706FBE);
      SHA2_64_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x243185BE4EE4B28C);
      SHA2_64_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x550C7DC3D5FFB4E2);
      SHA2_64_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x72BE5D74F27B896F);
      SHA2_64_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0x80DEB1FE3B1696B1);
      SHA2_64_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x9BDC06A725C71235);
      SHA2_64_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0xC19BF174CF692694);
      SHA2_64_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0xE49B69C19EF14AD2);
      SHA2_64_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0xEFBE4786384F25E3);
      SHA2_64_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x0FC19DC68B8CD5B5);
      SHA2_64_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x240CA1CC77AC9C65);
      SHA2_64_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x2DE92C6F592B0275);
      SHA2_64_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x4A7484AA6EA6E483);
      SHA2_64_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x5CB0A9DCBD41FBD4);
      SHA2_64_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x76F988DA831153B5);
      SHA2_64_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x983E5152EE66DFAB);
      SHA2_64_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0xA831C66D2DB43210);
      SHA2_64_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0xB00327C898FB213F);
      SHA2_64_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0xBF597FC7BEEF0EE4);
      SHA2_64_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0xC6E00BF33DA88FC2);
      SHA2_64_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xD5A79147930AA725);
      SHA2_64_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x06CA6351E003826F);
      SHA2_64_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x142929670A0E6E70);
      SHA2_64_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x27B70A8546D22FFC);
      SHA2_64_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x2E1B21385C26C926);
      SHA2_64_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x4D2C6DFC5AC42AED);
      SHA2_64_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x53380D139D95B3DF);
      SHA2_64_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x650A73548BAF63DE);
      SHA2_64_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x766A0ABB3C77B2A8);
      SHA2_64_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x81C2C92E47EDAEE6);
      SHA2_64_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x92722C851482353B);
      SHA2_64_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0xA2BFE8A14CF10364);
      SHA2_64_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0xA81A664BBC423001);
      SHA2_64_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0xC24B8B70D0F89791);
      SHA2_64_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0xC76C51A30654BE30);
      SHA2_64_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0xD192E819D6EF5218);
      SHA2_64_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xD69906245565A910);
      SHA2_64_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0xF40E35855771202A);
      SHA2_64_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x106AA07032BBD1B8);
      SHA2_64_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x19A4C116B8D2D0C8);
      SHA2_64_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x1E376C085141AB53);
      SHA2_64_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x2748774CDF8EEB99);
      SHA2_64_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x34B0BCB5E19B48A8);
      SHA2_64_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x391C0CB3C5C95A63);
      SHA2_64_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x4ED8AA4AE3418ACB);
      SHA2_64_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x5B9CCA4F7763E373);
      SHA2_64_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x682E6FF3D6B2B8A3);
      SHA2_64_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x748F82EE5DEFB2FC);
      SHA2_64_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x78A5636F43172F60);
      SHA2_64_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x84C87814A1F0AB72);
      SHA2_64_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x8CC702081A6439EC);
      SHA2_64_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x90BEFFFA23631E28);
      SHA2_64_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xA4506CEBDE82BDE9);
      SHA2_64_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0xBEF9A3F7B2C67915);
      SHA2_64_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0xC67178F2E372532B);
      SHA2_64_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0xCA273ECEEA26619C);
      SHA2_64_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0xD186B8C721C0C207);
      SHA2_64_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0xEADA7DD6CDE0EB1E);
      SHA2_64_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0xF57D4F7FEE6ED178);
      SHA2_64_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x06F067AA72176FBA);
      SHA2_64_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x0A637DC5A2C898A6);
      SHA2_64_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x113F9804BEF90DAE);
      SHA2_64_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x1B710B35131C471B);
      SHA2_64_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x28DB77F523047D84);
      SHA2_64_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x32CAAB7B40C72493);
      SHA2_64_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x3C9EBE0A15C9BEBC);
      SHA2_64_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x431D67C49C100D4C);
      SHA2_64_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x4CC5D4BECB3E42B6);
      SHA2_64_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0x597F299CFC657E2A);
      SHA2_64_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x5FCB6FAB3AD6FAEC);
      SHA2_64_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x6C44198C4A475817);

      A = (digest[0] += A);
      B = (digest[1] += B);
      C = (digest[2] += C);
      D = (digest[3] += D);
      E = (digest[4] += E);
      F = (digest[5] += F);
      G = (digest[6] += G);
      H = (digest[7] += H);

      input += 128;
      }
   }

}

}

void SHA_512_256::compress_n(const byte input[], size_t blocks)
   {
   SHA2_64::compress(m_digest, input, blocks);
   }

void SHA_384::compress_n(const byte input[], size_t blocks)
   {
   SHA2_64::compress(m_digest, input, blocks);
   }

void SHA_512::compress_n(const byte input[], size_t blocks)
   {
   SHA2_64::compress(m_digest, input, blocks);
   }

void SHA_512_256::copy_out(byte output[])
   {
   copy_out_vec_be(output, output_length(), m_digest);
   }

void SHA_384::copy_out(byte output[])
   {
   copy_out_vec_be(output, output_length(), m_digest);
   }

void SHA_512::copy_out(byte output[])
   {
   copy_out_vec_be(output, output_length(), m_digest);
   }

void SHA_512_256::clear()
   {
   MDx_HashFunction::clear();
   m_digest[0] = 0x22312194FC2BF72C;
   m_digest[1] = 0x9F555FA3C84C64C2;
   m_digest[2] = 0x2393B86B6F53B151;
   m_digest[3] = 0x963877195940EABD;
   m_digest[4] = 0x96283EE2A88EFFE3;
   m_digest[5] = 0xBE5E1E2553863992;
   m_digest[6] = 0x2B0199FC2C85B8AA;
   m_digest[7] = 0x0EB72DDC81C52CA2;
   }

void SHA_384::clear()
   {
   MDx_HashFunction::clear();
   m_digest[0] = 0xCBBB9D5DC1059ED8;
   m_digest[1] = 0x629A292A367CD507;
   m_digest[2] = 0x9159015A3070DD17;
   m_digest[3] = 0x152FECD8F70E5939;
   m_digest[4] = 0x67332667FFC00B31;
   m_digest[5] = 0x8EB44A8768581511;
   m_digest[6] = 0xDB0C2E0D64F98FA7;
   m_digest[7] = 0x47B5481DBEFA4FA4;
   }

void SHA_512::clear()
   {
   MDx_HashFunction::clear();
   m_digest[0] = 0x6A09E667F3BCC908;
   m_digest[1] = 0xBB67AE8584CAA73B;
   m_digest[2] = 0x3C6EF372FE94F82B;
   m_digest[3] = 0xA54FF53A5F1D36F1;
   m_digest[4] = 0x510E527FADE682D1;
   m_digest[5] = 0x9B05688C2B3E6C1F;
   m_digest[6] = 0x1F83D9ABFB41BD6B;
   m_digest[7] = 0x5BE0CD19137E2179;
   }

}
/*
* SRP-6a (RFC 5054 compatatible)
* (C) 2011,2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

BigInt hash_seq(const std::string& hash_id,
                size_t pad_to,
                const BigInt& in1,
                const BigInt& in2)
   {
   std::unique_ptr<HashFunction> hash_fn(HashFunction::create(hash_id));

   if(!hash_fn)
      throw Algorithm_Not_Found(hash_id);

   hash_fn->update(BigInt::encode_1363(in1, pad_to));
   hash_fn->update(BigInt::encode_1363(in2, pad_to));

   return BigInt::decode(hash_fn->final());
   }

BigInt compute_x(const std::string& hash_id,
                 const std::string& identifier,
                 const std::string& password,
                 const std::vector<byte>& salt)
   {
   std::unique_ptr<HashFunction> hash_fn(HashFunction::create(hash_id));

   if(!hash_fn)
      throw Algorithm_Not_Found(hash_id);

   hash_fn->update(identifier);
   hash_fn->update(":");
   hash_fn->update(password);

   secure_vector<byte> inner_h = hash_fn->final();

   hash_fn->update(salt);
   hash_fn->update(inner_h);

   secure_vector<byte> outer_h = hash_fn->final();

   return BigInt::decode(outer_h);
   }

}

std::string srp6_group_identifier(const BigInt& N, const BigInt& g)
   {
   /*
   This function assumes that only one 'standard' SRP parameter set has
   been defined for a particular bitsize. As of this writing that is the case.
   */
   try
      {
      const std::string group_name = "modp/srp/" + std::to_string(N.bits());

      DL_Group group(group_name);

      if(group.get_p() == N && group.get_g() == g)
         return group_name;

      throw std::runtime_error("Unknown SRP params");
      }
   catch(...)
      {
      throw Invalid_Argument("Bad SRP group parameters");
      }
   }

std::pair<BigInt, SymmetricKey>
srp6_client_agree(const std::string& identifier,
                  const std::string& password,
                  const std::string& group_id,
                  const std::string& hash_id,
                  const std::vector<byte>& salt,
                  const BigInt& B,
                  RandomNumberGenerator& rng)
   {
   DL_Group group(group_id);
   const BigInt& g = group.get_g();
   const BigInt& p = group.get_p();

   const size_t p_bytes = group.get_p().bytes();

   if(B <= 0 || B >= p)
      throw std::runtime_error("Invalid SRP parameter from server");

   BigInt k = hash_seq(hash_id, p_bytes, p, g);

   BigInt a(rng, 256);

   BigInt A = power_mod(g, a, p);

   BigInt u = hash_seq(hash_id, p_bytes, A, B);

   const BigInt x = compute_x(hash_id, identifier, password, salt);

   BigInt S = power_mod((B - (k * power_mod(g, x, p))) % p, (a + (u * x)), p);

   SymmetricKey Sk(BigInt::encode_1363(S, p_bytes));

   return std::make_pair(A, Sk);
   }

BigInt generate_srp6_verifier(const std::string& identifier,
                              const std::string& password,
                              const std::vector<byte>& salt,
                              const std::string& group_id,
                              const std::string& hash_id)
   {
   const BigInt x = compute_x(hash_id, identifier, password, salt);

   DL_Group group(group_id);
   return power_mod(group.get_g(), x, group.get_p());
   }

BigInt SRP6_Server_Session::step1(const BigInt& v,
                                  const std::string& group_id,
                                  const std::string& hash_id,
                                  RandomNumberGenerator& rng)
   {
   DL_Group group(group_id);
   const BigInt& g = group.get_g();
   const BigInt& p = group.get_p();

   m_p_bytes = p.bytes();
   m_v = v;
   m_b = BigInt(rng, 256);
   m_p = p;
   m_hash_id = hash_id;

   const BigInt k = hash_seq(hash_id, m_p_bytes, p, g);

   m_B = (v*k + power_mod(g, m_b, p)) % p;

   return m_B;
   }

SymmetricKey SRP6_Server_Session::step2(const BigInt& A)
   {
   if(A <= 0 || A >= m_p)
      throw std::runtime_error("Invalid SRP parameter from client");

   const BigInt u = hash_seq(m_hash_id, m_p_bytes, A, m_B);

   const BigInt S = power_mod(A * power_mod(m_v, u, m_p), m_b, m_p);

   return BigInt::encode_1363(S, m_p_bytes);
   }

}
/*
* SRP-6a File Handling
* (C) 2011 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

SRP6_Authenticator_File::SRP6_Authenticator_File(const std::string& filename)
   {
   std::ifstream in(filename);

   if(!in)
      return; // no entries

   while(in.good())
      {
      std::string line;
      std::getline(in, line);

      std::vector<std::string> parts = split_on(line, ':');

      if(parts.size() != 4)
         throw Decoding_Error("Invalid line in SRP authenticator file");

      std::string username = parts[0];
      BigInt v = BigInt::decode(base64_decode(parts[1]));
      std::vector<byte> salt = unlock(base64_decode(parts[2]));
      BigInt group_id_idx = BigInt::decode(base64_decode(parts[3]));

      std::string group_id;

      if(group_id_idx == 1)
         group_id = "modp/srp/1024";
      else if(group_id_idx == 2)
         group_id = "modp/srp/1536";
      else if(group_id_idx == 3)
         group_id = "modp/srp/2048";
      else
         continue; // unknown group, ignored

      entries[username] = SRP6_Data(v, salt, group_id);
      }
   }

bool SRP6_Authenticator_File::lookup_user(const std::string& username,
                                          BigInt& v,
                                          std::vector<byte>& salt,
                                          std::string& group_id) const
   {
   std::map<std::string, SRP6_Data>::const_iterator i = entries.find(username);

   if(i == entries.end())
      return false;

   v = i->second.v;
   salt = i->second.salt;
   group_id = i->second.group_id;

   return true;
   }

}
/*
* Stream Ciphers
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_CHACHA)
#endif

#if defined(BOTAN_HAS_SALSA20)
#endif

#if defined(BOTAN_HAS_CTR_BE)
#endif

#if defined(BOTAN_HAS_OFB)
#endif

#if defined(BOTAN_HAS_RC4)
#endif

namespace Botan {

std::unique_ptr<StreamCipher> StreamCipher::create(const std::string& algo_spec,
                                                   const std::string& provider)
   {
   return std::unique_ptr<StreamCipher>(make_a<StreamCipher>(algo_spec, provider));
   }

std::vector<std::string> StreamCipher::providers(const std::string& algo_spec)
   {
   return providers_of<StreamCipher>(StreamCipher::Spec(algo_spec));
   }

StreamCipher::StreamCipher() {}
StreamCipher::~StreamCipher() {}

void StreamCipher::set_iv(const byte[], size_t iv_len)
   {
   if(!valid_iv_length(iv_len))
      throw Invalid_IV_Length(name(), iv_len);
   }

#if defined(BOTAN_HAS_CHACHA)
BOTAN_REGISTER_T_NOARGS(StreamCipher, ChaCha);
#endif

#if defined(BOTAN_HAS_SALSA20)
BOTAN_REGISTER_T_NOARGS(StreamCipher, Salsa20);
#endif

#if defined(BOTAN_HAS_CTR_BE)
BOTAN_REGISTER_NAMED_T(StreamCipher, "CTR-BE", CTR_BE, CTR_BE::make);
#endif

#if defined(BOTAN_HAS_OFB)
BOTAN_REGISTER_NAMED_T(StreamCipher, "OFB", OFB, OFB::make);
#endif

#if defined(BOTAN_HAS_RC4)
BOTAN_REGISTER_NAMED_T(StreamCipher, "RC4", RC4, RC4::make);
#endif

}
/*
* System RNG
* (C) 2014,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_TARGET_OS_HAS_CRYPTGENRANDOM)

#include <wincrypt.h>
#undef min
#undef max

#else

#include <errno.h>

#endif

namespace Botan {

namespace {

class System_RNG_Impl : public RandomNumberGenerator
   {
   public:
      System_RNG_Impl();
      ~System_RNG_Impl();

      void randomize(byte buf[], size_t len) override;

      bool is_seeded() const override { return true; }
      void clear() override {}
      std::string name() const override { return "system"; }

      size_t reseed_with_sources(Entropy_Sources&,
                                 size_t /*poll_bits*/,
                                 std::chrono::milliseconds /*timeout*/) override
         {
         // We ignore it and assert the PRNG is seeded.
         // TODO: could poll and write it to /dev/urandom to help seed it
         return 0;
         }

      void add_entropy(const byte[], size_t) override
         {
         }
   private:

#if defined(BOTAN_TARGET_OS_HAS_CRYPTGENRANDOM)
      HCRYPTPROV m_prov;
#else
      int m_fd;
#endif
   };

System_RNG_Impl::System_RNG_Impl()
   {
#if defined(BOTAN_TARGET_OS_HAS_CRYPTGENRANDOM)

   if(!CryptAcquireContext(&m_prov, 0, 0, BOTAN_SYSTEM_RNG_CRYPTOAPI_PROV_TYPE, CRYPT_VERIFYCONTEXT))
      throw std::runtime_error("System_RNG failed to acquire crypto provider");

#else

#ifndef O_NOCTTY
  #define O_NOCTTY 0
#endif

   m_fd = ::open(BOTAN_SYSTEM_RNG_DEVICE, O_RDONLY | O_NOCTTY);
   if(m_fd < 0)
      throw std::runtime_error("System_RNG failed to open RNG device");
#endif
   }

System_RNG_Impl::~System_RNG_Impl()
   {
#if defined(BOTAN_TARGET_OS_HAS_CRYPTGENRANDOM)
   ::CryptReleaseContext(m_prov, 0);
#else
   ::close(m_fd);
   m_fd = -1;
#endif
   }

void System_RNG_Impl::randomize(byte buf[], size_t len)
   {
#if defined(BOTAN_TARGET_OS_HAS_CRYPTGENRANDOM)
   ::CryptGenRandom(m_prov, static_cast<DWORD>(len), buf);
#else
   while(len)
      {
      ssize_t got = ::read(m_fd, buf, len);

      if(got < 0)
         {
         if(errno == EINTR)
            continue;
         throw std::runtime_error("System_RNG read failed error " + std::to_string(errno));
         }
      if(got == 0)
         throw std::runtime_error("System_RNG EOF on device"); // ?!?

      buf += got;
      len -= got;
      }
#endif
   }

}

RandomNumberGenerator& system_rng()
   {
   static System_RNG_Impl g_system_rng;
   return g_system_rng;
   }

}
/*
* Program List for Unix_EntropySource
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/**
* Default Commands for Entropy Gathering
*/
std::vector<std::vector<std::string>> Unix_EntropySource::get_default_sources()
   {
   std::vector<std::vector<std::string>> srcs;

   srcs.push_back({ "netstat", "-in" });
   srcs.push_back({ "pfstat" });
   srcs.push_back({ "vmstat", "-s" });
   srcs.push_back({ "vmstat" });

   srcs.push_back({ "arp", "-a", "-n" });
   srcs.push_back({ "ifconfig", "-a" });
   srcs.push_back({ "iostat" });
   srcs.push_back({ "ipcs", "-a" });
   srcs.push_back({ "mpstat" });
   srcs.push_back({ "netstat", "-an" });
   srcs.push_back({ "netstat", "-s" });
   srcs.push_back({ "nfsstat" });
   srcs.push_back({ "portstat" });
   srcs.push_back({ "procinfo", "-a" });
   srcs.push_back({ "pstat", "-T" });
   srcs.push_back({ "pstat", "-s" });
   srcs.push_back({ "uname", "-a" });
   srcs.push_back({ "uptime" });

   srcs.push_back({ "listarea" });
   srcs.push_back({ "listdev" });
   srcs.push_back({ "ps", "-A" });
   srcs.push_back({ "sysinfo" });

   srcs.push_back({ "finger" });
   srcs.push_back({ "mailstats" });
   srcs.push_back({ "rpcinfo", "-p", "localhost" });
   srcs.push_back({ "who" });

   srcs.push_back({ "df", "-l" });
   srcs.push_back({ "dmesg" });
   srcs.push_back({ "last", "-5" });
   srcs.push_back({ "ls", "-alni", "/proc" });
   srcs.push_back({ "ls", "-alni", "/tmp" });
   srcs.push_back({ "pstat", "-f" });

   srcs.push_back({ "ps", "-elf" });
   srcs.push_back({ "ps", "aux" });

   srcs.push_back({ "lsof", "-n" });
   srcs.push_back({ "sar", "-A" });

   return srcs;
   }

}
 /*
* Gather entropy by running various system commands in the hopes that
* some of the output cannot be guessed by a remote attacker.
*
* (C) 1999-2009,2013 Jack Lloyd
*     2012 Markus Wanner
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <atomic>

#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <signal.h>
#include <stdlib.h>

namespace Botan {

namespace {

std::string find_full_path_if_exists(const std::vector<std::string>& trusted_path,
                                     const std::string& proc)
   {
   for(auto dir : trusted_path)
      {
      const std::string full_path = dir + "/" + proc;
      if(::access(full_path.c_str(), X_OK) == 0)
         return full_path;
      }

   return "";
   }

size_t concurrent_processes(size_t user_request)
   {
   const size_t DEFAULT_CONCURRENT = 2;
   const size_t MAX_CONCURRENT = 8;

   if(user_request > 0)
      return std::min(user_request, MAX_CONCURRENT);

   const long online_cpus = ::sysconf(_SC_NPROCESSORS_ONLN);

   if(online_cpus > 0)
      return static_cast<size_t>(online_cpus); // maybe fewer?

   return DEFAULT_CONCURRENT;
   }

}

/**
* Unix_EntropySource Constructor
*/
Unix_EntropySource::Unix_EntropySource(const std::vector<std::string>& trusted_path,
                                       size_t proc_cnt) :
   m_trusted_paths(trusted_path),
   m_concurrent(concurrent_processes(proc_cnt))
   {
   }

void UnixProcessInfo_EntropySource::poll(Entropy_Accumulator& accum)
   {
   accum.add(::getpid(), BOTAN_ENTROPY_ESTIMATE_STATIC_SYSTEM_DATA);
   accum.add(::getppid(), BOTAN_ENTROPY_ESTIMATE_STATIC_SYSTEM_DATA);
   accum.add(::getuid(),  BOTAN_ENTROPY_ESTIMATE_STATIC_SYSTEM_DATA);
   accum.add(::getgid(),  BOTAN_ENTROPY_ESTIMATE_STATIC_SYSTEM_DATA);
   accum.add(::getpgrp(), BOTAN_ENTROPY_ESTIMATE_STATIC_SYSTEM_DATA);

   struct ::rusage usage;
   ::getrusage(RUSAGE_SELF, &usage);
   accum.add(usage, BOTAN_ENTROPY_ESTIMATE_STATIC_SYSTEM_DATA);
   }

void Unix_EntropySource::Unix_Process::spawn(const std::vector<std::string>& args)
   {
   if(args.empty())
      throw std::invalid_argument("Cannot spawn process without path");

   shutdown();

   int pipe[2];
   if(::pipe(pipe) != 0)
      return;

   pid_t pid = ::fork();

   if(pid == -1)
      {
      ::close(pipe[0]);
      ::close(pipe[1]);
      }
   else if(pid > 0) // in parent
      {
      m_pid = pid;
      m_fd = pipe[0];
      ::close(pipe[1]);
      }
   else // in child
      {
      if(::dup2(pipe[1], STDOUT_FILENO) == -1)
         ::exit(127);
      if(::close(pipe[0]) != 0 || ::close(pipe[1]) != 0)
         ::exit(127);
      if(close(STDERR_FILENO) != 0)
         ::exit(127);

      const char* arg0 = args[0].c_str();
      const char* arg1 = (args.size() > 1) ? args[1].c_str() : nullptr;
      const char* arg2 = (args.size() > 2) ? args[2].c_str() : nullptr;
      const char* arg3 = (args.size() > 3) ? args[3].c_str() : nullptr;
      const char* arg4 = (args.size() > 4) ? args[4].c_str() : nullptr;

      ::execl(arg0, arg0, arg1, arg2, arg3, arg4, NULL);
      ::exit(127);
      }
   }

void Unix_EntropySource::Unix_Process::shutdown()
   {
   if(m_pid == -1)
      return;

   ::close(m_fd);
   m_fd = -1;

   pid_t reaped = waitpid(m_pid, nullptr, WNOHANG);

   if(reaped == 0)
      {
      /*
      * Child is still alive - send it SIGTERM, sleep for a bit and
      * try to reap again, if still alive send SIGKILL
      */
      kill(m_pid, SIGTERM);

      struct ::timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 1000;
      select(0, nullptr, nullptr, nullptr, &tv);

      reaped = ::waitpid(m_pid, nullptr, WNOHANG);

      if(reaped == 0)
         {
         ::kill(m_pid, SIGKILL);
         do
            reaped = ::waitpid(m_pid, nullptr, 0);
         while(reaped == -1);
         }
      }

   m_pid = -1;
   }

const std::vector<std::string>& Unix_EntropySource::next_source()
   {
   const auto& src = m_sources.at(m_sources_idx);
   m_sources_idx = (m_sources_idx + 1) % m_sources.size();
   return src;
   }

void Unix_EntropySource::poll(Entropy_Accumulator& accum)
   {
   // refuse to run setuid or setgid, or as root
   if((getuid() != geteuid()) || (getgid() != getegid()) || (geteuid() == 0))
      return;

   std::lock_guard<std::mutex> lock(m_mutex);

   if(m_sources.empty())
      {
      auto sources = get_default_sources();

      for(auto src : sources)
         {
         const std::string path = find_full_path_if_exists(m_trusted_paths, src[0]);
         if(path != "")
            {
            src[0] = path;
            m_sources.push_back(src);
            }
         }
      }

   if(m_sources.empty())
      return; // still empty, really nothing to try

   const size_t MS_WAIT_TIME = 32;

   m_buf.resize(4096);

   while(!accum.polling_finished())
      {
      while(m_procs.size() < m_concurrent)
         m_procs.emplace_back(Unix_Process(next_source()));

      fd_set read_set;
      FD_ZERO(&read_set);

      std::vector<int> fds;

      for(auto& proc : m_procs)
         {
         int fd = proc.fd();
         if(fd > 0)
            {
            fds.push_back(fd);
            FD_SET(fd, &read_set);
            }
         }

      if(fds.empty())
         break;

      const int max_fd = *std::max_element(fds.begin(), fds.end());

      struct ::timeval timeout;
      timeout.tv_sec = (MS_WAIT_TIME / 1000);
      timeout.tv_usec = (MS_WAIT_TIME % 1000) * 1000;

      if(::select(max_fd + 1, &read_set, nullptr, nullptr, &timeout) < 0)
         return; // or continue?

      for(auto& proc : m_procs)
         {
         int fd = proc.fd();

         if(FD_ISSET(fd, &read_set))
            {
            const ssize_t got = ::read(fd, m_buf.data(), m_buf.size());
            if(got > 0)
               accum.add(m_buf.data(), got, BOTAN_ENTROPY_ESTIMATE_SYSTEM_TEXT);
            else
               proc.spawn(next_source());
            }
         }
      }
   }

}
/*
* Runtime assertion checking
* (C) 2010,2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

void assertion_failure(const char* expr_str,
                       const char* assertion_made,
                       const char* func,
                       const char* file,
                       int line)
   {
   std::ostringstream format;

   format << "False assertion ";

   if(assertion_made && assertion_made[0] != 0)
      format << "'" << assertion_made << "' (expression " << expr_str << ") ";
   else
      format << expr_str << " ";

   if(func)
      format << "in " << func << " ";

   format << "@" << file << ":" << line;

   throw std::runtime_error(format.str());
   }

}
/*
* Calendar Functions
* (C) 1999-2010 Jack Lloyd
* (C) 2015 Simon Warta (Kullo GmbH)
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <ctime>

#if defined(BOTAN_HAS_BOOST_DATETIME)
#include <boost/date_time/posix_time/posix_time_types.hpp>
#endif

namespace Botan {

namespace {

std::tm do_gmtime(std::time_t time_val)
   {
   std::tm tm;

#if defined(BOTAN_TARGET_OS_HAS_GMTIME_S)
   gmtime_s(&tm, &time_val); // Windows
#elif defined(BOTAN_TARGET_OS_HAS_GMTIME_R)
   gmtime_r(&time_val, &tm); // Unix/SUSv2
#else
   std::tm* tm_p = std::gmtime(&time_val);
   if (tm_p == nullptr)
      throw Encoding_Error("time_t_to_tm could not convert");
   tm = *tm_p;
#endif

   return tm;
   }

#if !defined(BOTAN_TARGET_OS_HAS_TIMEGM) && !defined(BOTAN_TARGET_OS_HAS_MKGMTIME)

#if defined(BOTAN_HAS_BOOST_DATETIME)

std::time_t boost_timegm(std::tm *tm)
   {
   const int sec  = tm->tm_sec;
   const int min  = tm->tm_min;
   const int hour = tm->tm_hour;
   const int day  = tm->tm_mday;
   const int mon  = tm->tm_mon + 1;
   const int year = tm->tm_year + 1900;

   std::time_t out;

      {
      using namespace boost::posix_time;
      using namespace boost::gregorian;
      const auto epoch = ptime(date(1970, 01, 01));
      const auto time = ptime(date(year, mon, day),
                              hours(hour) + minutes(min) + seconds(sec));
      const time_duration diff(time - epoch);
      out = diff.ticks() / diff.ticks_per_second();
      }

   return out;
   }

#else

#pragma message "Caution! A fallback version of timegm() is used which is not thread-safe"

std::mutex ENV_TZ;

std::time_t fallback_timegm(std::tm *tm)
   {
   std::time_t out;
   std::string tz_backup;

   ENV_TZ.lock();

   // Store current value of env variable TZ
   const char* tz_env_pointer = ::getenv("TZ");
   if (tz_env_pointer != nullptr)
      tz_backup = std::string(tz_env_pointer);

   // Clear value of TZ
   ::setenv("TZ", "", 1);
   ::tzset();

   out = ::mktime(tm);

   // Restore TZ
   if (!tz_backup.empty())
      {
      // setenv makes a copy of the second argument
      ::setenv("TZ", tz_backup.data(), 1);
      }
   else
      {
      ::unsetenv("TZ");
      }
   ::tzset();

   ENV_TZ.unlock();

   return out;
}
#endif // BOTAN_HAS_BOOST_DATETIME

#endif

}

std::chrono::system_clock::time_point calendar_point::to_std_timepoint() const
   {
   if (year < 1970)
      throw Invalid_Argument("calendar_point::to_std_timepoint() does not support years before 1970.");

   // 32 bit time_t ends at January 19, 2038
   // https://msdn.microsoft.com/en-us/library/2093ets1.aspx
   // For consistency reasons, throw after 2037 as long as
   // no other implementation is available.
   if (year > 2037)
      throw Invalid_Argument("calendar_point::to_std_timepoint() does not support years after 2037.");

   // std::tm: struct without any timezone information
   std::tm tm;
   tm.tm_isdst = -1; // i.e. no DST information available
   tm.tm_sec   = seconds;
   tm.tm_min   = minutes;
   tm.tm_hour  = hour;
   tm.tm_mday  = day;
   tm.tm_mon   = month - 1;
   tm.tm_year  = year - 1900;

   // Define a function alias `botan_timegm`
   #if defined(BOTAN_TARGET_OS_HAS_TIMEGM)
   std::time_t (&botan_timegm)(std::tm *tm) = timegm;
   #elif defined(BOTAN_TARGET_OS_HAS_MKGMTIME)
   // http://stackoverflow.com/questions/16647819/timegm-cross-platform
   std::time_t (&botan_timegm)(std::tm *tm) = _mkgmtime;
   #elif defined(BOTAN_HAS_BOOST_DATETIME)
   std::time_t (&botan_timegm)(std::tm *tm) = boost_timegm;
   #else
   std::time_t (&botan_timegm)(std::tm *tm) = fallback_timegm;
   #endif

   // Convert std::tm to std::time_t
   std::time_t tt = botan_timegm(&tm);
   if (tt == -1)
      throw Invalid_Argument("calendar_point couldn't be converted: " + to_string());

   return std::chrono::system_clock::from_time_t(tt);
   }

std::string calendar_point::to_string() const
   {
   // desired format: <YYYY>-<MM>-<dd>T<HH>:<mm>:<ss>
   std::stringstream output;
      {
      using namespace std;
      output << setfill('0')
             << setw(4) << year << "-" << setw(2) << month << "-" << setw(2) << day
             << "T"
             << setw(2) << hour << ":" << setw(2) << minutes << ":" << setw(2) << seconds;
      }
   return output.str();
   }


calendar_point calendar_value(
   const std::chrono::system_clock::time_point& time_point)
   {
   std::tm tm = do_gmtime(std::chrono::system_clock::to_time_t(time_point));

   return calendar_point(tm.tm_year + 1900,
                         tm.tm_mon + 1,
                         tm.tm_mday,
                         tm.tm_hour,
                         tm.tm_min,
                         tm.tm_sec);
   }

}
/*
* Character Set Handling
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <cctype>

namespace Botan {

namespace Charset {

namespace {

/*
* Convert from UCS-2 to ISO 8859-1
*/
std::string ucs2_to_latin1(const std::string& ucs2)
   {
   if(ucs2.size() % 2 == 1)
      throw Decoding_Error("UCS-2 string has an odd number of bytes");

   std::string latin1;

   for(size_t i = 0; i != ucs2.size(); i += 2)
      {
      const byte c1 = ucs2[i];
      const byte c2 = ucs2[i+1];

      if(c1 != 0)
         throw Decoding_Error("UCS-2 has non-Latin1 characters");

      latin1 += static_cast<char>(c2);
      }

   return latin1;
   }

/*
* Convert from UTF-8 to ISO 8859-1
*/
std::string utf8_to_latin1(const std::string& utf8)
   {
   std::string iso8859;

   size_t position = 0;
   while(position != utf8.size())
      {
      const byte c1 = static_cast<byte>(utf8[position++]);

      if(c1 <= 0x7F)
         iso8859 += static_cast<char>(c1);
      else if(c1 >= 0xC0 && c1 <= 0xC7)
         {
         if(position == utf8.size())
            throw Decoding_Error("UTF-8: sequence truncated");

         const byte c2 = static_cast<byte>(utf8[position++]);
         const byte iso_char = ((c1 & 0x07) << 6) | (c2 & 0x3F);

         if(iso_char <= 0x7F)
            throw Decoding_Error("UTF-8: sequence longer than needed");

         iso8859 += static_cast<char>(iso_char);
         }
      else
         throw Decoding_Error("UTF-8: Unicode chars not in Latin1 used");
      }

   return iso8859;
   }

/*
* Convert from ISO 8859-1 to UTF-8
*/
std::string latin1_to_utf8(const std::string& iso8859)
   {
   std::string utf8;
   for(size_t i = 0; i != iso8859.size(); ++i)
      {
      const byte c = static_cast<byte>(iso8859[i]);

      if(c <= 0x7F)
         utf8 += static_cast<char>(c);
      else
         {
         utf8 += static_cast<char>((0xC0 | (c >> 6)));
         utf8 += static_cast<char>((0x80 | (c & 0x3F)));
         }
      }
   return utf8;
   }

}

/*
* Perform character set transcoding
*/
std::string transcode(const std::string& str,
                      Character_Set to, Character_Set from)
   {
   if(to == LOCAL_CHARSET)
      to = LATIN1_CHARSET;
   if(from == LOCAL_CHARSET)
      from = LATIN1_CHARSET;

   if(to == from)
      return str;

   if(from == LATIN1_CHARSET && to == UTF8_CHARSET)
      return latin1_to_utf8(str);
   if(from == UTF8_CHARSET && to == LATIN1_CHARSET)
      return utf8_to_latin1(str);
   if(from == UCS2_CHARSET && to == LATIN1_CHARSET)
      return ucs2_to_latin1(str);

   throw Invalid_Argument("Unknown transcoding operation from " +
                          std::to_string(from) + " to " + std::to_string(to));
   }

/*
* Check if a character represents a digit
*/
bool is_digit(char c)
   {
   if(c == '0' || c == '1' || c == '2' || c == '3' || c == '4' ||
      c == '5' || c == '6' || c == '7' || c == '8' || c == '9')
      return true;
   return false;
   }

/*
* Check if a character represents whitespace
*/
bool is_space(char c)
   {
   if(c == ' ' || c == '\t' || c == '\n' || c == '\r')
      return true;
   return false;
   }

/*
* Convert a character to a digit
*/
byte char2digit(char c)
   {
   switch(c)
      {
      case '0': return 0;
      case '1': return 1;
      case '2': return 2;
      case '3': return 3;
      case '4': return 4;
      case '5': return 5;
      case '6': return 6;
      case '7': return 7;
      case '8': return 8;
      case '9': return 9;
      }

   throw Invalid_Argument("char2digit: Input is not a digit character");
   }

/*
* Convert a digit to a character
*/
char digit2char(byte b)
   {
   switch(b)
      {
      case 0: return '0';
      case 1: return '1';
      case 2: return '2';
      case 3: return '3';
      case 4: return '4';
      case 5: return '5';
      case 6: return '6';
      case 7: return '7';
      case 8: return '8';
      case 9: return '9';
      }

   throw Invalid_Argument("digit2char: Input is not a digit");
   }

/*
* Case-insensitive character comparison
*/
bool caseless_cmp(char a, char b)
   {
   return (std::tolower(static_cast<unsigned char>(a)) ==
           std::tolower(static_cast<unsigned char>(b)));
   }

}

}
/*
* Runtime CPU detection
* (C) 2009-2010,2013 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY)

#if defined(BOTAN_TARGET_OS_IS_DARWIN)
  #include <sys/sysctl.h>
#endif

#if defined(BOTAN_TARGET_OS_IS_OPENBSD)
  #include <sys/param.h>
  #include <machine/cpu.h>
#endif

#endif

#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)

#if defined(BOTAN_BUILD_COMPILER_IS_MSVC)

#include <intrin.h>

#define X86_CPUID(type, out) do { __cpuid((int*)out, type); } while(0)
#define X86_CPUID_SUBLEVEL(type, level, out) do { __cpuidex((int*)out, type, level); } while(0)

#elif defined(BOTAN_BUILD_COMPILER_IS_INTEL)

#include <ia32intrin.h>

#define X86_CPUID(type, out) do { __cpuid(out, type); } while(0)
#define X86_CPUID_SUBLEVEL(type, level, out) do { __cpuidex((int*)out, type, level); } while(0)

#elif defined(BOTAN_TARGET_ARCH_IS_X86_64) && defined(BOTAN_USE_GCC_INLINE_ASM)

#define X86_CPUID(type, out)                                                    \
   asm("cpuid\n\t" : "=a" (out[0]), "=b" (out[1]), "=c" (out[2]), "=d" (out[3]) \
       : "0" (type))

#define X86_CPUID_SUBLEVEL(type, level, out)                                    \
   asm("cpuid\n\t" : "=a" (out[0]), "=b" (out[1]), "=c" (out[2]), "=d" (out[3]) \
       : "0" (type), "2" (level))

#elif defined(BOTAN_BUILD_COMPILER_IS_GCC) || defined(BOTAN_BUILD_COMPILER_IS_CLANG)

#include <cpuid.h>

#define X86_CPUID(type, out) do { __get_cpuid(type, out, out+1, out+2, out+3); } while(0)

#define X86_CPUID_SUBLEVEL(type, level, out) \
   do { __cpuid_count(type, level, out[0], out[1], out[2], out[3]); } while(0)

#else

#warning "No way of calling cpuid for this compiler"

#define X86_CPUID(type, out) do { clear_mem(out, 4); } while(0)
#define X86_CPUID_SUBLEVEL(type, level, out) do { clear_mem(out, 4); } while(0)

#endif

#endif

namespace Botan {

u64bit CPUID::g_x86_processor_flags[2] = { 0, 0 };
size_t CPUID::g_cache_line_size = BOTAN_TARGET_CPU_DEFAULT_CACHE_LINE_SIZE;
bool CPUID::g_altivec_capable = false;
bool CPUID::g_initialized = false;

namespace {

#if defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY)

bool altivec_check_sysctl()
   {
#if defined(BOTAN_TARGET_OS_IS_DARWIN) || defined(BOTAN_TARGET_OS_IS_OPENBSD)

#if defined(BOTAN_TARGET_OS_IS_OPENBSD)
   int sels[2] = { CTL_MACHDEP, CPU_ALTIVEC };
#else
   // From Apple's docs
   int sels[2] = { CTL_HW, HW_VECTORUNIT };
#endif
   int vector_type = 0;
   size_t length = sizeof(vector_type);
   int error = sysctl(sels, 2, &vector_type, &length, NULL, 0);

   if(error == 0 && vector_type > 0)
      return true;
#endif

   return false;
   }

bool altivec_check_pvr_emul()
   {
   bool altivec_capable = false;

#if defined(BOTAN_TARGET_OS_IS_LINUX) || defined(BOTAN_TARGET_OS_IS_NETBSD)

   /*
   On PowerPC, MSR 287 is PVR, the Processor Version Number
   Normally it is only accessible to ring 0, but Linux and NetBSD
   (others, too, maybe?) will trap and emulate it for us.

   PVR identifiers for various AltiVec enabled CPUs. Taken from
   PearPC and Linux sources, mostly.
   */

   const u16bit PVR_G4_7400  = 0x000C;
   const u16bit PVR_G5_970   = 0x0039;
   const u16bit PVR_G5_970FX = 0x003C;
   const u16bit PVR_G5_970MP = 0x0044;
   const u16bit PVR_G5_970GX = 0x0045;
   const u16bit PVR_POWER6   = 0x003E;
   const u16bit PVR_POWER7   = 0x003F;
   const u16bit PVR_POWER8   = 0x004B;
   const u16bit PVR_CELL_PPU = 0x0070;

   // Motorola produced G4s with PVR 0x800[0123C] (at least)
   const u16bit PVR_G4_74xx_24  = 0x800;

   u32bit pvr = 0;

   asm volatile("mfspr %0, 287" : "=r" (pvr));

   // Top 16 bit suffice to identify model
   pvr >>= 16;

   altivec_capable |= (pvr == PVR_G4_7400);
   altivec_capable |= ((pvr >> 4) == PVR_G4_74xx_24);
   altivec_capable |= (pvr == PVR_G5_970);
   altivec_capable |= (pvr == PVR_G5_970FX);
   altivec_capable |= (pvr == PVR_G5_970MP);
   altivec_capable |= (pvr == PVR_G5_970GX);
   altivec_capable |= (pvr == PVR_POWER6);
   altivec_capable |= (pvr == PVR_POWER7);
   altivec_capable |= (pvr == PVR_POWER8);
   altivec_capable |= (pvr == PVR_CELL_PPU);
#endif

   return altivec_capable;
   }

#endif

}

bool CPUID::has_simd_32()
   {
#if defined(BOTAN_HAS_SIMD_SSE2)
   return CPUID::has_sse2();
#elif defined(BOTAN_HAS_SIMD_ALTIVEC)
   return CPUID::has_altivec();
#elif defined(BOTAN_HAS_SIMD_SCALAR)
   return true;
#else
   return false;
#endif
   }

void CPUID::print(std::ostream& o)
   {
   o << "CPUID flags: ";

#define CPUID_PRINT(flag) do { if(has_##flag()) o << #flag << " "; } while(0)
   CPUID_PRINT(sse2);
   CPUID_PRINT(ssse3);
   CPUID_PRINT(sse41);
   CPUID_PRINT(sse42);
   CPUID_PRINT(avx2);
   CPUID_PRINT(avx512f);
   CPUID_PRINT(altivec);

   CPUID_PRINT(rdtsc);
   CPUID_PRINT(bmi2);
   CPUID_PRINT(clmul);
   CPUID_PRINT(aes_ni);
   CPUID_PRINT(rdrand);
   CPUID_PRINT(rdseed);
   CPUID_PRINT(intel_sha);
   CPUID_PRINT(adx);
#undef CPUID_PRINT
   o << "\n";
   }

void CPUID::initialize()
   {
   if(g_initialized)
      return;

#if defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY)
      if(altivec_check_sysctl() || altivec_check_pvr_emul())
         g_altivec_capable = true;
#endif

#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)
   const u32bit INTEL_CPUID[3] = { 0x756E6547, 0x6C65746E, 0x49656E69 };
   const u32bit AMD_CPUID[3] = { 0x68747541, 0x444D4163, 0x69746E65 };

   u32bit cpuid[4] = { 0 };
   X86_CPUID(0, cpuid);

   const u32bit max_supported_sublevel = cpuid[0];

   if(max_supported_sublevel == 0)
      return;

   const bool is_intel = same_mem(cpuid + 1, INTEL_CPUID, 3);
   const bool is_amd = same_mem(cpuid + 1, AMD_CPUID, 3);

   X86_CPUID(1, cpuid);

   g_x86_processor_flags[0] = (static_cast<u64bit>(cpuid[2]) << 32) | cpuid[3];

   if(is_intel)
      g_cache_line_size = 8 * get_byte(2, cpuid[1]);

   if(max_supported_sublevel >= 7)
      {
      clear_mem(cpuid, 4);
      X86_CPUID_SUBLEVEL(7, 0, cpuid);
      g_x86_processor_flags[1] = (static_cast<u64bit>(cpuid[2]) << 32) | cpuid[1];
      }

   if(is_amd)
      {
      X86_CPUID(0x80000005, cpuid);
      g_cache_line_size = get_byte(3, cpuid[2]);
      }

#endif

#if defined(BOTAN_TARGET_ARCH_IS_X86_64)
   /*
   * If we don't have access to CPUID, we can still safely assume that
   * any x86-64 processor has SSE2 and RDTSC
   */
   if(g_x86_processor_flags[0] == 0)
      g_x86_processor_flags[0] = (1 << CPUID_SSE2_BIT) | (1 << CPUID_RDTSC_BIT);
#endif

   g_initialized = true;
   }

}
/*
* DataSource
* (C) 1999-2007 Jack Lloyd
*     2005 Matthew Gregan
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Read a single byte from the DataSource
*/
size_t DataSource::read_byte(byte& out)
   {
   return read(&out, 1);
   }

/*
* Peek a single byte from the DataSource
*/
size_t DataSource::peek_byte(byte& out) const
   {
   return peek(&out, 1, 0);
   }

/*
* Discard the next N bytes of the data
*/
size_t DataSource::discard_next(size_t n)
   {
   byte buf[64] = { 0 };
   size_t discarded = 0;

   while(n)
      {
      const size_t got = this->read(buf, std::min(n, sizeof(buf)));
      discarded += got;

      if(got == 0)
         break;
      }

   return discarded;
   }

/*
* Read from a memory buffer
*/
size_t DataSource_Memory::read(byte out[], size_t length)
   {
   size_t got = std::min<size_t>(source.size() - offset, length);
   copy_mem(out, source.data() + offset, got);
   offset += got;
   return got;
   }

bool DataSource_Memory::check_available(size_t n)
   {
   return (n <= (source.size() - offset));
   }

/*
* Peek into a memory buffer
*/
size_t DataSource_Memory::peek(byte out[], size_t length,
                               size_t peek_offset) const
   {
   const size_t bytes_left = source.size() - offset;
   if(peek_offset >= bytes_left) return 0;

   size_t got = std::min(bytes_left - peek_offset, length);
   copy_mem(out, &source[offset + peek_offset], got);
   return got;
   }

/*
* Check if the memory buffer is empty
*/
bool DataSource_Memory::end_of_data() const
   {
   return (offset == source.size());
   }

/*
* DataSource_Memory Constructor
*/
DataSource_Memory::DataSource_Memory(const std::string& in) :
   source(reinterpret_cast<const byte*>(in.data()),
          reinterpret_cast<const byte*>(in.data()) + in.length()),
   offset(0)
   {
   offset = 0;
   }

/*
* Read from a stream
*/
size_t DataSource_Stream::read(byte out[], size_t length)
   {
   source.read(reinterpret_cast<char*>(out), length);
   if(source.bad())
      throw Stream_IO_Error("DataSource_Stream::read: Source failure");

   size_t got = source.gcount();
   total_read += got;
   return got;
   }

bool DataSource_Stream::check_available(size_t n)
   {
   const std::streampos orig_pos = source.tellg();
   source.seekg(0, std::ios::end);
   const size_t avail = source.tellg() - orig_pos;
   source.seekg(orig_pos);
   return (avail >= n);
   }

/*
* Peek into a stream
*/
size_t DataSource_Stream::peek(byte out[], size_t length, size_t offset) const
   {
   if(end_of_data())
      throw Invalid_State("DataSource_Stream: Cannot peek when out of data");

   size_t got = 0;

   if(offset)
      {
      secure_vector<byte> buf(offset);
      source.read(reinterpret_cast<char*>(buf.data()), buf.size());
      if(source.bad())
         throw Stream_IO_Error("DataSource_Stream::peek: Source failure");
      got = source.gcount();
      }

   if(got == offset)
      {
      source.read(reinterpret_cast<char*>(out), length);
      if(source.bad())
         throw Stream_IO_Error("DataSource_Stream::peek: Source failure");
      got = source.gcount();
      }

   if(source.eof())
      source.clear();
   source.seekg(total_read, std::ios::beg);

   return got;
   }

/*
* Check if the stream is empty or in error
*/
bool DataSource_Stream::end_of_data() const
   {
   return (!source.good());
   }

/*
* Return a human-readable ID for this stream
*/
std::string DataSource_Stream::id() const
   {
   return identifier;
   }

/*
* DataSource_Stream Constructor
*/
DataSource_Stream::DataSource_Stream(const std::string& path,
                                     bool use_binary) :
   identifier(path),
   source_p(new std::ifstream(path,
                              use_binary ? std::ios::binary : std::ios::in)),
   source(*source_p),
   total_read(0)
   {
   if(!source.good())
      {
      delete source_p;
      throw Stream_IO_Error("DataSource: Failure opening file " + path);
      }
   }

/*
* DataSource_Stream Constructor
*/
DataSource_Stream::DataSource_Stream(std::istream& in,
                                     const std::string& name) :
   identifier(name),
   source_p(nullptr),
   source(in),
   total_read(0)
   {
   }

/*
* DataSource_Stream Destructor
*/
DataSource_Stream::~DataSource_Stream()
   {
   delete source_p;
   }

}
/*
* (C) 2015 Jack Lloyd
* (C) 2015 Simon Warta (Kullo GmbH)
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_TARGET_OS_HAS_STL_FILESYSTEM_MSVC) && defined(BOTAN_BUILD_COMPILER_IS_MSVC)
  #include <filesystem>
#elif defined(BOTAN_HAS_BOOST_FILESYSTEM)
  #include <boost/filesystem.hpp>
#elif defined(BOTAN_TARGET_OS_HAS_READDIR)
#endif

namespace Botan {

namespace {

#if defined(BOTAN_TARGET_OS_HAS_STL_FILESYSTEM_MSVC) && defined(BOTAN_BUILD_COMPILER_IS_MSVC)
std::vector<std::string> impl_stl_filesystem(const std::string& dir)
   {
   using namespace std::tr2::sys;

   std::vector<std::string> out;

   path p(dir);

   if (is_directory(p))
      {
      for (recursive_directory_iterator itr(p), end; itr != end; ++itr)
         {
         if (is_regular_file(itr->path()))
            {
            out.push_back(itr->path().string());
            }
         }
      }

   return out;
   }
#elif defined(BOTAN_HAS_BOOST_FILESYSTEM)
std::vector<std::string> impl_boost_filesystem(const std::string& dir_path)
{
   namespace fs = boost::filesystem;

   std::vector<std::string> out;

   for(fs::recursive_directory_iterator dir(dir_path), end; dir != end; ++dir)
      {
      if(fs::is_regular_file(dir->path()))
         {
         out.push_back(dir->path().string());
         }
      }

   return out;
}
#elif defined(BOTAN_TARGET_OS_HAS_READDIR)
std::vector<std::string> impl_readdir(const std::string& dir_path)
   {
   std::vector<std::string> out;
   std::deque<std::string> dir_list;
   dir_list.push_back(dir_path);

   while(!dir_list.empty())
      {
      const std::string cur_path = dir_list[0];
      dir_list.pop_front();

      std::unique_ptr<DIR, std::function<int (DIR*)>> dir(::opendir(cur_path.c_str()), ::closedir);

      if(dir)
         {
         while(struct dirent* dirent = ::readdir(dir.get()))
            {
            const std::string filename = dirent->d_name;
            if(filename == "." || filename == "..")
               continue;
            const std::string full_path = cur_path + "/" + filename;

            struct stat stat_buf;

            if(::lstat(full_path.c_str(), &stat_buf) == -1)
               continue;

            if(S_ISDIR(stat_buf.st_mode))
               dir_list.push_back(full_path);
            else if(S_ISREG(stat_buf.st_mode))
               out.push_back(full_path);
            }
         }
      }

   return out;
   }
#endif

}

std::vector<std::string> get_files_recursive(const std::string& dir)
   {
   std::vector<std::string> files;

#if defined(BOTAN_TARGET_OS_HAS_STL_FILESYSTEM_MSVC) && defined(BOTAN_BUILD_COMPILER_IS_MSVC)
   files = impl_stl_filesystem(dir);
#elif defined(BOTAN_HAS_BOOST_FILESYSTEM)
   files = impl_boost_filesystem(dir);
#elif defined(BOTAN_TARGET_OS_HAS_READDIR)
   files = impl_readdir(dir);
#else
   throw No_Filesystem_Access();
#endif

   std::sort(files.begin(), files.end());

   return files;
   }

}
/*
* OS and machine specific utility functions
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


//TODO: defined(BOTAN_TARGET_OS_TYPE_IS_POSIX)

#if defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK)
  #include <sys/mman.h>
#endif

namespace Botan {

namespace OS {

size_t get_memory_locking_limit()
   {
#if defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK)
   /*
   * Linux defaults to only 64 KiB of mlockable memory per process
   * (too small) but BSDs offer a small fraction of total RAM (more
   * than we need). Bound the total mlock size to 512 KiB which is
   * enough to run the entire test suite without spilling to non-mlock
   * memory (and thus presumably also enough for many useful
   * programs), but small enough that we should not cause problems
   * even if many processes are mlocking on the same machine.
   */
   size_t mlock_requested = BOTAN_MLOCK_ALLOCATOR_MAX_LOCKED_KB;

   /*
   * Allow override via env variable
   */
   if(const char* env = ::getenv("BOTAN_MLOCK_POOL_SIZE"))
      {
      try
         {
         const size_t user_req = std::stoul(env, nullptr);
         mlock_requested = std::min(user_req, mlock_requested);
         }
      catch(std::exception&) { /* ignore it */ }
      }

   if(mlock_requested > 0)
      {
      struct ::rlimit limits;

      ::getrlimit(RLIMIT_MEMLOCK, &limits);

      if(limits.rlim_cur < limits.rlim_max)
         {
         limits.rlim_cur = limits.rlim_max;
         ::setrlimit(RLIMIT_MEMLOCK, &limits);
         ::getrlimit(RLIMIT_MEMLOCK, &limits);
         }

      return std::min<size_t>(limits.rlim_cur, mlock_requested * 1024);
      }
#endif

   return 0;
   }

void* allocate_locked_pages(size_t length)
   {
#if defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK)

#if !defined(MAP_NOCORE)
   #define MAP_NOCORE 0
#endif

#if !defined(MAP_ANONYMOUS)
   #define MAP_ANONYMOUS MAP_ANON
#endif

   void* ptr = ::mmap(nullptr,
                      length,
                      PROT_READ | PROT_WRITE,
                      MAP_ANONYMOUS | MAP_SHARED | MAP_NOCORE,
                      /*fd*/-1,
                      /*offset*/0);

   if(ptr == MAP_FAILED)
      {
      return nullptr;
      }

#if defined(MADV_DONTDUMP)
   ::madvise(ptr, length, MADV_DONTDUMP);
#endif

   if(::mlock(ptr, length) != 0)
      {
      ::munmap(ptr, length);
      return nullptr; // failed to lock
      }

   ::memset(ptr, 0, length);

   return ptr;
#else
   return nullptr; /* not implemented */
#endif
   }

void free_locked_pages(void* ptr, size_t length)
   {
   if(ptr == nullptr || length == 0)
      return;

#if defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK)
   zero_mem(ptr, length);
   ::munlock(ptr, length);
   ::munmap(ptr, length);
#else
   // Invalid argument because no way this pointer was allocated by us
   throw Invalid_Argument("Invalid ptr to free_locked_pages");
#endif
   }

}

}
/*
* Various string utils and parsing functions
* (C) 1999-2007,2013,2014,2015 Jack Lloyd
* (C) 2015 Simon Warta (Kullo GmbH)
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <limits>

namespace Botan {

u32bit to_u32bit(const std::string& str)
   {
   try
      {
      // std::stoul is not strict enough. Ensure that str is digit only [0-9]*
      for (const char chr : str)
         {
         if (chr < '0' || chr > '9')
            {
            auto chrAsString = std::string(1, chr);
            throw Invalid_Argument("String contains non-digit char: " + chrAsString);
            }
         }

      const auto integerValue = std::stoul(str);

      // integerValue might be uint64
      if (integerValue > std::numeric_limits<u32bit>::max())
         {
         throw Invalid_Argument("Integer value exceeds 32 bit range: " + std::to_string(integerValue));
         }

      return integerValue;
      }
   catch(std::exception& e)
      {
      auto message = std::string("Could not read '" + str + "' as decimal string");
      auto exceptionMessage = std::string(e.what());
      if (!exceptionMessage.empty()) message += ": " + exceptionMessage;
      throw std::runtime_error(message);
      }
   }

/*
* Convert a string into a time duration
*/
u32bit timespec_to_u32bit(const std::string& timespec)
   {
   if(timespec == "")
      return 0;

   const char suffix = timespec[timespec.size()-1];
   std::string value = timespec.substr(0, timespec.size()-1);

   u32bit scale = 1;

   if(Charset::is_digit(suffix))
      value += suffix;
   else if(suffix == 's')
      scale = 1;
   else if(suffix == 'm')
      scale = 60;
   else if(suffix == 'h')
      scale = 60 * 60;
   else if(suffix == 'd')
      scale = 24 * 60 * 60;
   else if(suffix == 'y')
      scale = 365 * 24 * 60 * 60;
   else
      throw Decoding_Error("timespec_to_u32bit: Bad input " + timespec);

   return scale * to_u32bit(value);
   }

/*
* Parse a SCAN-style algorithm name
*/
std::vector<std::string> parse_algorithm_name(const std::string& namex)
   {
   if(namex.find('(') == std::string::npos &&
      namex.find(')') == std::string::npos)
      return std::vector<std::string>(1, namex);

   std::string name = namex, substring;
   std::vector<std::string> elems;
   size_t level = 0;

   elems.push_back(name.substr(0, name.find('(')));
   name = name.substr(name.find('('));

   for(auto i = name.begin(); i != name.end(); ++i)
      {
      char c = *i;

      if(c == '(')
         ++level;
      if(c == ')')
         {
         if(level == 1 && i == name.end() - 1)
            {
            if(elems.size() == 1)
               elems.push_back(substring.substr(1));
            else
               elems.push_back(substring);
            return elems;
            }

         if(level == 0 || (level == 1 && i != name.end() - 1))
            throw Invalid_Algorithm_Name(namex);
         --level;
         }

      if(c == ',' && level == 1)
         {
         if(elems.size() == 1)
            elems.push_back(substring.substr(1));
         else
            elems.push_back(substring);
         substring.clear();
         }
      else
         substring += c;
      }

   if(substring != "")
      throw Invalid_Algorithm_Name(namex);

   return elems;
   }

std::vector<std::string> split_on(const std::string& str, char delim)
   {
   return split_on_pred(str, [delim](char c) { return c == delim; });
   }

std::vector<std::string> split_on_pred(const std::string& str,
                                       std::function<bool (char)> pred)
   {
   std::vector<std::string> elems;
   if(str == "") return elems;

   std::string substr;
   for(auto i = str.begin(); i != str.end(); ++i)
      {
      if(pred(*i))
         {
         if(substr != "")
            elems.push_back(substr);
         substr.clear();
         }
      else
         substr += *i;
      }

   if(substr == "")
      throw Invalid_Argument("Unable to split string: " + str);
   elems.push_back(substr);

   return elems;
   }

/*
* Join a string
*/
std::string string_join(const std::vector<std::string>& strs, char delim)
   {
   std::string out = "";

   for(size_t i = 0; i != strs.size(); ++i)
      {
      if(i != 0)
         out += delim;
      out += strs[i];
      }

   return out;
   }

/*
* Parse an ASN.1 OID string
*/
std::vector<u32bit> parse_asn1_oid(const std::string& oid)
   {
   std::string substring;
   std::vector<u32bit> oid_elems;

   for(auto i = oid.begin(); i != oid.end(); ++i)
      {
      char c = *i;

      if(c == '.')
         {
         if(substring == "")
            throw Invalid_OID(oid);
         oid_elems.push_back(to_u32bit(substring));
         substring.clear();
         }
      else
         substring += c;
      }

   if(substring == "")
      throw Invalid_OID(oid);
   oid_elems.push_back(to_u32bit(substring));

   if(oid_elems.size() < 2)
      throw Invalid_OID(oid);

   return oid_elems;
   }

/*
* X.500 String Comparison
*/
bool x500_name_cmp(const std::string& name1, const std::string& name2)
   {
   auto p1 = name1.begin();
   auto p2 = name2.begin();

   while((p1 != name1.end()) && Charset::is_space(*p1)) ++p1;
   while((p2 != name2.end()) && Charset::is_space(*p2)) ++p2;

   while(p1 != name1.end() && p2 != name2.end())
      {
      if(Charset::is_space(*p1))
         {
         if(!Charset::is_space(*p2))
            return false;

         while((p1 != name1.end()) && Charset::is_space(*p1)) ++p1;
         while((p2 != name2.end()) && Charset::is_space(*p2)) ++p2;

         if(p1 == name1.end() && p2 == name2.end())
            return true;
         }

      if(!Charset::caseless_cmp(*p1, *p2))
         return false;
      ++p1;
      ++p2;
      }

   while((p1 != name1.end()) && Charset::is_space(*p1)) ++p1;
   while((p2 != name2.end()) && Charset::is_space(*p2)) ++p2;

   if((p1 != name1.end()) || (p2 != name2.end()))
      return false;
   return true;
   }

/*
* Convert a decimal-dotted string to binary IP
*/
u32bit string_to_ipv4(const std::string& str)
   {
   std::vector<std::string> parts = split_on(str, '.');

   if(parts.size() != 4)
      throw Decoding_Error("Invalid IP string " + str);

   u32bit ip = 0;

   for(auto part = parts.begin(); part != parts.end(); ++part)
      {
      u32bit octet = to_u32bit(*part);

      if(octet > 255)
         throw Decoding_Error("Invalid IP string " + str);

      ip = (ip << 8) | (octet & 0xFF);
      }

   return ip;
   }

/*
* Convert an IP address to decimal-dotted string
*/
std::string ipv4_to_string(u32bit ip)
   {
   std::string str;

   for(size_t i = 0; i != sizeof(ip); ++i)
      {
      if(i)
         str += ".";
      str += std::to_string(get_byte(i, ip));
      }

   return str;
   }

std::string erase_chars(const std::string& str, const std::set<char>& chars)
   {
   std::string out;

   for(auto c: str)
      if(chars.count(c) == 0)
         out += c;

   return out;
   }

std::string replace_chars(const std::string& str,
                          const std::set<char>& chars,
                          char to_char)
   {
   std::string out = str;

   for(size_t i = 0; i != out.size(); ++i)
      if(chars.count(out[i]))
         out[i] = to_char;

   return out;
   }

std::string replace_char(const std::string& str, char from_char, char to_char)
   {
   std::string out = str;

   for(size_t i = 0; i != out.size(); ++i)
      if(out[i] == from_char)
         out[i] = to_char;

   return out;
   }

bool host_wildcard_match(const std::string& issued, const std::string& host)
   {
   if(issued == host)
      return true;

   if(issued.size() > 2 && issued[0] == '*' && issued[1] == '.')
      {
      size_t host_i = host.find('.');
      if(host_i == std::string::npos || host_i == host.size() - 1)
         return false;

      const std::string host_base = host.substr(host_i + 1);
      const std::string issued_base = issued.substr(2);

      if(host_base == issued_base)
         return true;
         }

   return false;
   }

}
/*
* Simple config/test file reader
* (C) 2013,2014,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

std::string clean_ws(const std::string& s)
   {
   const char* ws = " \t\n";
   auto start = s.find_first_not_of(ws);
   auto end = s.find_last_not_of(ws);

   if(start == std::string::npos)
      return "";

   if(end == std::string::npos)
      return s.substr(start, end);
   else
      return s.substr(start, start + end + 1);
   }

std::map<std::string, std::string> read_cfg(std::istream& is)
   {
   std::map<std::string, std::string> kv;
   size_t line = 0;

   while(is.good())
      {
      std::string s;

      std::getline(is, s);

      ++line;

      if(s == "" || s[0] == '#')
         continue;

      s = clean_ws(s.substr(0, s.find('#')));

      if(s == "")
         continue;

      auto eq = s.find("=");

      if(eq == std::string::npos || eq == 0 || eq == s.size() - 1)
         throw std::runtime_error("Bad read_cfg input '" + s + "' on line " + std::to_string(line));

      const std::string key = clean_ws(s.substr(0, eq));
      const std::string val = clean_ws(s.substr(eq + 1, std::string::npos));

      kv[key] = val;
      }

   return kv;
   }

}
/*
* Semaphore
* (C) 2013 Joel Low
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


// Based on code by Pierre Gaston (http://p9as.blogspot.com/2012/06/c11-semaphores.html)

namespace Botan {

void Semaphore::release(size_t n)
   {
   for(size_t i = 0; i != n; ++i)
      {
      std::lock_guard<std::mutex> lock(m_mutex);

      ++m_value;

      if(m_value <= 0)
         {
         ++m_wakeups;
         m_cond.notify_one();
         }
      }
   }

void Semaphore::acquire()
   {
   std::unique_lock<std::mutex> lock(m_mutex);
   --m_value;
   if(m_value < 0)
      {
      m_cond.wait(lock, [this] { return m_wakeups > 0; });
      --m_wakeups;
      }
   }

}
/*
* Version Information
* (C) 1999-2013 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
  These are intentionally compiled rather than inlined, so an
  application running against a shared library can test the true
  version they are running against.
*/

/*
* Return the version as a string
*/
std::string version_string()
   {
   return std::string(version_cstr());
   }

const char* version_cstr()
   {
#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)

   /*
   It is intentional that this string is a compile-time constant;
   it makes it much easier to find in binaries.
   */

   return "Botan " STR(BOTAN_VERSION_MAJOR) "."
                   STR(BOTAN_VERSION_MINOR) "."
                   STR(BOTAN_VERSION_PATCH) " ("
                   BOTAN_VERSION_RELEASE_TYPE
#if (BOTAN_VERSION_DATESTAMP != 0)
                   ", dated " STR(BOTAN_VERSION_DATESTAMP)
#endif
                   ", revision " BOTAN_VERSION_VC_REVISION
                   ", distribution " BOTAN_DISTRIBUTION_INFO ")";

#undef STR
#undef QUOTE
   }

u32bit version_datestamp() { return BOTAN_VERSION_DATESTAMP; }

/*
* Return parts of the version as integers
*/
u32bit version_major() { return BOTAN_VERSION_MAJOR; }
u32bit version_minor() { return BOTAN_VERSION_MINOR; }
u32bit version_patch() { return BOTAN_VERSION_PATCH; }

}
 /*
* Zero Memory
* (C) 2012,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_TARGET_OS_HAS_RTLSECUREZEROMEMORY)
#endif

namespace Botan {

void zero_mem(void* ptr, size_t n)
   {
#if defined(BOTAN_TARGET_OS_HAS_RTLSECUREZEROMEMORY)
   ::RtlSecureZeroMemory(ptr, n);
#elif defined(BOTAN_USE_VOLATILE_MEMSET_FOR_ZERO) && (BOTAN_USE_VOLATILE_MEMSET_FOR_ZERO == 1)
   static void* (*const volatile memset_ptr)(void*, int, size_t) = std::memset;
   (memset_ptr)(ptr, 0, n);
#else
   volatile byte* p = reinterpret_cast<volatile byte*>(ptr);

   for(size_t i = 0; i != n; ++i)
      p[i] = 0;
#endif
   }

}
/*
* Zlib Compressor
* (C) 2001 Peter J Jones
*     2001-2007,2014 Jack Lloyd
*     2006 Matt Johnston
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <zlib.h>

namespace Botan {

BOTAN_REGISTER_COMPRESSION(Zlib_Compression, Zlib_Decompression);
BOTAN_REGISTER_COMPRESSION(Gzip_Compression, Gzip_Decompression);
BOTAN_REGISTER_COMPRESSION(Deflate_Compression, Deflate_Decompression);

namespace {

class Zlib_Stream : public Zlib_Style_Stream<z_stream, Bytef>
   {
   public:
      Zlib_Stream()
         {
         streamp()->opaque = alloc();
         streamp()->zalloc = Compression_Alloc_Info::malloc<unsigned int>;
         streamp()->zfree = Compression_Alloc_Info::free;
         }

      u32bit run_flag() const override { return Z_NO_FLUSH; }
      u32bit flush_flag() const override { return Z_SYNC_FLUSH; }
      u32bit finish_flag() const override { return Z_FINISH; }

      int compute_window_bits(int wbits, int wbits_offset) const
         {
         if(wbits_offset == -1)
            return -wbits;
         else
            return wbits + wbits_offset;
         }
   };

class Zlib_Compression_Stream : public Zlib_Stream
   {
   public:
      Zlib_Compression_Stream(size_t level, int wbits, int wbits_offset = 0)
         {
         wbits = compute_window_bits(wbits, wbits_offset);

         int rc = deflateInit2(streamp(), level, Z_DEFLATED, wbits,
                               8, Z_DEFAULT_STRATEGY);
         if(rc != Z_OK)
            throw std::runtime_error("zlib deflate initialization failed");
         }

      ~Zlib_Compression_Stream()
         {
         deflateEnd(streamp());
         }

      bool run(u32bit flags) override
         {
         int rc = deflate(streamp(), flags);

         if(rc == Z_MEM_ERROR)
            throw std::bad_alloc();
         else if(rc != Z_OK && rc != Z_STREAM_END && rc != Z_BUF_ERROR)
            throw std::runtime_error("zlib deflate error " + std::to_string(rc));

         return (rc == Z_STREAM_END);
         }
   };

class Zlib_Decompression_Stream : public Zlib_Stream
   {
   public:
      Zlib_Decompression_Stream(int wbits, int wbits_offset = 0)
         {
         int rc = inflateInit2(streamp(), compute_window_bits(wbits, wbits_offset));

         if(rc == Z_MEM_ERROR)
            throw std::bad_alloc();
         else if(rc != Z_OK)
            throw std::runtime_error("zlib inflate initialization failed");
         }

      ~Zlib_Decompression_Stream()
         {
         inflateEnd(streamp());
         }

      bool run(u32bit flags) override
         {
         int rc = inflate(streamp(), flags);

         if(rc == Z_MEM_ERROR)
            throw std::bad_alloc();
         else if(rc != Z_OK && rc != Z_STREAM_END && rc != Z_BUF_ERROR)
            throw std::runtime_error("zlib inflate error " + std::to_string(rc));

         return (rc == Z_STREAM_END);
         }
   };

class Deflate_Compression_Stream : public Zlib_Compression_Stream
   {
   public:
      Deflate_Compression_Stream(size_t level, int wbits) :
         Zlib_Compression_Stream(level, wbits, -1) {}
   };

class Deflate_Decompression_Stream : public Zlib_Decompression_Stream
   {
   public:
      Deflate_Decompression_Stream(int wbits) : Zlib_Decompression_Stream(wbits, -1) {}
   };

class Gzip_Compression_Stream : public Zlib_Compression_Stream
   {
   public:
      Gzip_Compression_Stream(size_t level, int wbits, byte os_code) :
         Zlib_Compression_Stream(level, wbits, 16)
         {
         clear_mem(&m_header, 1);
         m_header.os = os_code;
         m_header.time = std::time(nullptr);

         int rc = deflateSetHeader(streamp(), &m_header);
         if(rc != Z_OK)
            throw std::runtime_error("setting gzip header failed");
         }

   private:
      ::gz_header m_header;
   };

class Gzip_Decompression_Stream : public Zlib_Decompression_Stream
   {
   public:
      Gzip_Decompression_Stream(int wbits) : Zlib_Decompression_Stream(wbits, 16) {}
   };

}

Compression_Stream* Zlib_Compression::make_stream() const
   {
   return new Zlib_Compression_Stream(m_level, 15);
   }

Compression_Stream* Zlib_Decompression::make_stream() const
   {
   return new Zlib_Decompression_Stream(15);
   }

Compression_Stream* Deflate_Compression::make_stream() const
   {
   return new Deflate_Compression_Stream(m_level, 15);
   }

Compression_Stream* Deflate_Decompression::make_stream() const
   {
   return new Deflate_Decompression_Stream(15);
   }

Compression_Stream* Gzip_Compression::make_stream() const
   {
   return new Gzip_Compression_Stream(m_level, 15, m_os_code);
   }

Compression_Stream* Gzip_Decompression::make_stream() const
   {
   return new Gzip_Decompression_Stream(15);
   }

}
