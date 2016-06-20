/*
* Botan 1.11.30 Amalgamation
* (C) 1999-2013,2014,2015 Jack Lloyd and others
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_AMALGAMATION_H__
#define BOTAN_AMALGAMATION_H__

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <deque>
#include <exception>
#include <functional>
#include <initializer_list>
#include <iosfwd>
#include <istream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

/*
* This file was automatically generated Mon Jun 20 14:53:05 2016 UTC by
* daniel@twentyone running '/tmp/update-botan/1.11.30/Botan-1.11.30/configure.py --via-amalgamation --no-autoload --disable-shared --with-zlib --enable-modules=aes,sha1,sha2_32,sha2_64,auto_rng,codec_filt,eme_oaep,emsa_pssr,gcm,hres_timer,rsa,srp6,system_rng,darwin_secrandom --os=darwin --cpu=armv8-a --cc=clang'
*
* Target
*  - Compiler: clang++  -pthread -std=c++11 -D_REENTRANT -fstack-protector -O3
*  - Arch: armv8-a/arm64
*  - OS: darwin
*/

#define BOTAN_VERSION_MAJOR 1
#define BOTAN_VERSION_MINOR 11
#define BOTAN_VERSION_PATCH 30
#define BOTAN_VERSION_DATESTAMP 20160619

#define BOTAN_VERSION_RELEASE_TYPE "released"

#define BOTAN_VERSION_VC_REVISION "git:0b6d2a853638206dfa43420d5419e8b719505cc7"

#define BOTAN_DISTRIBUTION_INFO "unspecified"

#define BOTAN_INSTALL_PREFIX R"(/usr/local)"
#define BOTAN_INSTALL_HEADER_DIR "include/botan-1.11"
#define BOTAN_INSTALL_LIB_DIR "lib"
#define BOTAN_LIB_LINK "-lz -framework Security"

#ifndef BOTAN_DLL
  #define BOTAN_DLL 
#endif

/* How much to allocate for a buffer of no particular size */
#define BOTAN_DEFAULT_BUFFER_SIZE 1024

/* Minimum and maximum sizes to allocate out of the mlock pool (bytes)
   Default min is 16 as smaller values are easily bruteforceable and thus
   likely not cryptographic keys.
*/
#define BOTAN_MLOCK_ALLOCATOR_MIN_ALLOCATION 16
#define BOTAN_MLOCK_ALLOCATOR_MAX_ALLOCATION 128

/*
* Total maximum amount of RAM (in KiB) we will lock into memory, even
* if the OS would let us lock more
*/
#define BOTAN_MLOCK_ALLOCATOR_MAX_LOCKED_KB 512

/* Multiplier on a block cipher's native parallelism */
#define BOTAN_BLOCK_CIPHER_PAR_MULT 4

/* How many bits per limb in a BigInt */
#define BOTAN_MP_WORD_BITS 64

/*
* If enabled uses memset via volatile function pointer to zero memory,
* otherwise does a byte at a time write via a volatile pointer.
*/
#define BOTAN_USE_VOLATILE_MEMSET_FOR_ZERO 1

/*
* If enabled the ECC implementation will use Montgomery ladder
* instead of a fixed window implementation.
*/
#define BOTAN_POINTGFP_BLINDED_MULTIPLY_USE_MONTGOMERY_LADDER 0

/*
* Set number of bits used to generate mask for blinding the scalar of
* a point multiplication. Set to zero to disable this side-channel
* countermeasure.
*/
#define BOTAN_POINTGFP_SCALAR_BLINDING_BITS 20

/*
* Set number of bits used to generate mask for blinding the
* representation of an ECC point. Set to zero to diable this
* side-channel countermeasure.
*/
#define BOTAN_POINTGFP_RANDOMIZE_BLINDING_BITS 80

/*
* Normally blinding is performed by choosing a random starting point (plus
* its inverse, of a form appropriate to the algorithm being blinded), and
* then choosing new blinding operands by successive squaring of both
* values. This is much faster than computing a new starting point but
* introduces some possible coorelation
*
* To avoid possible leakage problems in long-running processes, the blinder
* periodically reinitializes the sequence. This value specifies how often
* a new sequence should be started.
*/
#define BOTAN_BLINDING_REINIT_INTERVAL 32

/* PK key consistency checking toggles */
#define BOTAN_PUBLIC_KEY_STRONG_CHECKS_ON_LOAD 1
#define BOTAN_PRIVATE_KEY_STRONG_CHECKS_ON_LOAD 0
#define BOTAN_PRIVATE_KEY_STRONG_CHECKS_ON_GENERATE 1

/*
* RNGs will automatically poll the system for additional seed material
* after producing this many bytes of output.
*/
#define BOTAN_RNG_MAX_OUTPUT_BEFORE_RESEED 4096
#define BOTAN_RNG_RESEED_POLL_BITS 128
#define BOTAN_RNG_AUTO_RESEED_TIMEOUT std::chrono::milliseconds(10)
#define BOTAN_RNG_RESEED_DEFAULT_TIMEOUT std::chrono::milliseconds(50)

/*
* Specifies (in order) the list of entropy sources that will be used
* to seed an in-memory RNG. The first few in the default list
* ("timer", "proc_info", etc) do not count as contributing any entropy
* but are included as they are fast and help protect against a
* seriously broken system RNG.
*/
#define BOTAN_ENTROPY_DEFAULT_SOURCES \
   { "timestamp", "rdseed", "rdrand", "proc_info", \
   "darwin_secrandom", "dev_random", "win32_cryptoapi", "egd", \
   "proc_walk", "system_stats", "unix_procs" }

/*
* These control the RNG used by the system RNG interface
*/
#define BOTAN_SYSTEM_RNG_DEVICE "/dev/urandom"
#define BOTAN_SYSTEM_RNG_CRYPTOAPI_PROV_TYPE PROV_RSA_FULL

/*
* These paramaters control how many bytes to read from the system
* PRNG, and how long to block if applicable.
*
* Timeout is ignored on Windows as CryptGenRandom doesn't block
*/
#define BOTAN_SYSTEM_RNG_POLL_DEVICES { "/dev/urandom", "/dev/random", "/dev/srandom" }

#define BOTAN_SYSTEM_RNG_POLL_REQUEST 64
#define BOTAN_SYSTEM_RNG_POLL_TIMEOUT_MS 20

#define BOTAN_ENTROPY_EGD_PATHS { "/var/run/egd-pool", "/dev/egd-pool" }
#define BOTAN_ENTROPY_PROC_FS_PATH "/proc"
#define BOTAN_ENTROPY_SAFE_PATHS { "/bin", "/sbin", "/usr/bin", "/usr/sbin" }

/*
* Defines the static entropy estimates which each type of source uses.
* These values are expressed as the bits of entropy per byte of
* output (in double format) and should be conservative. These are used
* unless an entropy source has some more specific opinion on the entropy
* of the underlying source.
*/

// We include some high resolution timestamps because it can't hurt
#define BOTAN_ENTROPY_ESTIMATE_TIMESTAMPS 0

// Data which is system or process specific, but otherwise static
#define BOTAN_ENTROPY_ESTIMATE_STATIC_SYSTEM_DATA 0

// Binary system data of some kind
#define BOTAN_ENTROPY_ESTIMATE_SYSTEM_DATA 0.5

// Human readable text which has entropy
#define BOTAN_ENTROPY_ESTIMATE_SYSTEM_TEXT (1.0 / 64)

/*
The output of a hardware RNG such as RDRAND / RDSEED

By default such RNGs are used but not trusted, so that the standard
softare-based entropy polling is still used.
*/
#define BOTAN_ENTROPY_ESTIMATE_HARDWARE_RNG 0.0

/*
How often should the RdRand/RdSeed RNGs be polled

Each poll generates 32 bit entropy
*/
#define BOTAN_ENTROPY_INTEL_RNG_POLLS 32

// According to Intel RdRand is guaranteed to generate a random number within 10 retries on a working CPU
#define BOTAN_ENTROPY_RDRAND_RETRIES 10

/*
* RdSeed is not guaranteed to generate a random number within a specific number of retries
* Define the number of retries here
*/
#define BOTAN_ENTROPY_RDSEED_RETRIES 20

// The output of a PRNG we are trusting to be strong
#define BOTAN_ENTROPY_ESTIMATE_STRONG_RNG 7.0


/*
* Compiler and target specific flags
*/

/* Should we use GCC-style inline assembler? */
#if !defined(BOTAN_USE_GCC_INLINE_ASM) && defined(__GNUG__)
  #define BOTAN_USE_GCC_INLINE_ASM 1
#endif

#ifdef __GNUC__
  #define BOTAN_GCC_VERSION \
     (__GNUC__ * 100 + __GNUC_MINOR__ * 10 + __GNUC_PATCHLEVEL__)
#else
  #define BOTAN_GCC_VERSION 0
#endif

/* Target identification and feature test macros */
#define BOTAN_TARGET_OS_IS_DARWIN
#define BOTAN_TARGET_OS_TYPE_IS_UNIX
#define BOTAN_TARGET_OS_HAS_DLOPEN
#define BOTAN_TARGET_OS_HAS_GETTIMEOFDAY
#define BOTAN_TARGET_OS_HAS_GMTIME_R
#define BOTAN_TARGET_OS_HAS_MEMSET_S
#define BOTAN_TARGET_OS_HAS_READDIR
#define BOTAN_TARGET_OS_HAS_SOCKETS
#define BOTAN_TARGET_OS_HAS_TIMEGM

#define BOTAN_TARGET_ARCH_IS_ARM64
#define BOTAN_TARGET_CPU_IS_ARMV8_A
#define BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN
#define BOTAN_TARGET_CPU_IS_ARM_FAMILY
#define BOTAN_TARGET_CPU_NATIVE_WORD_SIZE 64
#define BOTAN_TARGET_CPU_HAS_NATIVE_64BIT
#define BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK 0

#if defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN) || \
    defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
  #define BOTAN_TARGET_CPU_HAS_KNOWN_ENDIANNESS
#endif

/*
* If no way of dynamically determining the cache line size for the
* system exists, this value is used as the default. Used by the side
* channel countermeasures rather than for alignment purposes, so it is
* better to be on the smaller side if the exact value cannot be
* determined. Typically 32 or 64 bytes on modern CPUs.
*/
#if !defined(BOTAN_TARGET_CPU_DEFAULT_CACHE_LINE_SIZE)
  #define BOTAN_TARGET_CPU_DEFAULT_CACHE_LINE_SIZE 32
#endif

#define BOTAN_BUILD_COMPILER_IS_CLANG

/*
* Compile-time deprecatation warnings
*/
#if !defined(BOTAN_NO_DEPRECATED_WARNINGS)

  #if defined(__clang__)
    #define BOTAN_DEPRECATED(msg) __attribute__ ((deprecated))

  #elif defined(_MSC_VER)
    #define BOTAN_DEPRECATED(msg) __declspec(deprecated(msg))

  #elif defined(__GNUG__)

    #if BOTAN_GCC_VERSION >= 450
      #define BOTAN_DEPRECATED(msg) __attribute__ ((deprecated(msg)))
    #else
      #define BOTAN_DEPRECATED(msg) __attribute__ ((deprecated))
    #endif

  #endif

#endif

#if !defined(BOTAN_NORETURN)

  #if defined (__clang__) || defined (__GNUG__)
    #define BOTAN_NORETURN __attribute__ ((__noreturn__))

  #elif defined (_MSC_VER)
    #define BOTAN_NORETURN __declspec(noreturn)

  #else
    #define BOTAN_NORETURN
	
  #endif

#endif


#if defined(_MSC_VER)
  #define BOTAN_CURRENT_FUNCTION __FUNCTION__
#else
  #define BOTAN_CURRENT_FUNCTION __func__
#endif

#if !defined(BOTAN_DEPRECATED)
  #define BOTAN_DEPRECATED(msg)
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1900)
  // noexcept is not supported in VS 2013
  #include <yvals.h>
  #define BOTAN_NOEXCEPT _NOEXCEPT
#else
  #define BOTAN_NOEXCEPT noexcept
#endif

/*
* Module availability definitions
*/
#define BOTAN_HAS_AEAD_GCM 20131128
#define BOTAN_HAS_AEAD_MODES 20131128
#define BOTAN_HAS_AES 20131128
#define BOTAN_HAS_ASN1 20131128
#define BOTAN_HAS_AUTO_SEEDING_RNG 20131128
#define BOTAN_HAS_BASE64_CODEC 20131128
#define BOTAN_HAS_BIGINT 20131128
#define BOTAN_HAS_BIGINT_MP 20151225
#define BOTAN_HAS_BLOCK_CIPHER 20131128
#define BOTAN_HAS_CIPHER_MODE_PADDING 20131128
#define BOTAN_HAS_CODEC_FILTERS 20131128
#define BOTAN_HAS_COMPRESSION 20141117
#define BOTAN_HAS_CTR_BE 20131128
#define BOTAN_HAS_DL_GROUP 20131128
#define BOTAN_HAS_EME_OAEP 20140118
#define BOTAN_HAS_EMSA_PSSR 20131128
#define BOTAN_HAS_ENTROPY_SOURCE 20151120
#define BOTAN_HAS_ENTROPY_SRC_DARWIN_SECRANDOM 20150925
#define BOTAN_HAS_ENTROPY_SRC_HIGH_RESOLUTION_TIMER 20131128
#define BOTAN_HAS_FILTERS 20160415
#define BOTAN_HAS_HEX_CODEC 20131128
#define BOTAN_HAS_HMAC 20131128
#define BOTAN_HAS_HMAC_RNG 20131128
#define BOTAN_HAS_IF_PUBLIC_KEY_FAMILY 20131128
#define BOTAN_HAS_KDF_BASE 20131128
#define BOTAN_HAS_KEYPAIR_TESTING 20131128
#define BOTAN_HAS_MAC 20150626
#define BOTAN_HAS_MDX_HASH_FUNCTION 20131128
#define BOTAN_HAS_MGF1 20140118
#define BOTAN_HAS_MODES 20150626
#define BOTAN_HAS_MODE_CBC 20131128
#define BOTAN_HAS_NUMBERTHEORY 20131128
#define BOTAN_HAS_OID_LOOKUP 20131128
#define BOTAN_HAS_PBKDF 20150626
#define BOTAN_HAS_PBKDF2 20131128
#define BOTAN_HAS_PEM_CODEC 20131128
#define BOTAN_HAS_PKCS5_PBES2 20141119
#define BOTAN_HAS_PK_PADDING 20131128
#define BOTAN_HAS_PUBLIC_KEY_CRYPTO 20131128
#define BOTAN_HAS_RSA 20131128
#define BOTAN_HAS_SHA1 20131128
#define BOTAN_HAS_SHA2_32 20131128
#define BOTAN_HAS_SHA2_64 20131128
#define BOTAN_HAS_SRP6 20131128
#define BOTAN_HAS_STREAM_CIPHER 20131128
#define BOTAN_HAS_SYSTEM_RNG 20141202
#define BOTAN_HAS_UTIL_FUNCTIONS 20150919
#define BOTAN_HAS_ZLIB 20160412

/*
* Local configuration options (if any) follow
*/


// Append to macros living outside of functions, so that invocations must end with a semicolon.
// The struct is only declared to force the semicolon, it is never defined.
#define BOTAN_FORCE_SEMICOLON struct BOTAN_DUMMY_STRUCT


namespace Botan {

/**
* Called when an assertion fails
*/
BOTAN_NORETURN void BOTAN_DLL assertion_failure(const char* expr_str,
                                 const char* assertion_made,
                                 const char* func,
                                 const char* file,
                                 int line);

/**
* Make an assertion
*/
#define BOTAN_ASSERT(expr, assertion_made)                \
   do {                                                   \
      if(!(expr))                                         \
         Botan::assertion_failure(#expr,                  \
                                  assertion_made,         \
                                  BOTAN_CURRENT_FUNCTION, \
                                  __FILE__,               \
                                  __LINE__);              \
   } while(0)

/**
* Assert that value1 == value2
*/
#define BOTAN_ASSERT_EQUAL(expr1, expr2, assertion_made)   \
   do {                                                    \
     if((expr1) != (expr2))                                \
       Botan::assertion_failure(#expr1 " == " #expr2,      \
                                  assertion_made,          \
                                  BOTAN_CURRENT_FUNCTION,  \
                                  __FILE__,                \
                                  __LINE__);               \
   } while(0)

/**
* Assert that expr1 (if true) implies expr2 is also true
*/
#define BOTAN_ASSERT_IMPLICATION(expr1, expr2, msg)        \
   do {                                                    \
     if((expr1) && !(expr2))                               \
       Botan::assertion_failure(#expr1 " implies " #expr2, \
                                msg,                       \
                                  BOTAN_CURRENT_FUNCTION,  \
                                  __FILE__,                \
                                  __LINE__);               \
   } while(0)

/**
* Assert that a pointer is not null
*/
#define BOTAN_ASSERT_NONNULL(ptr)                          \
   do {                                                    \
     if((ptr) == nullptr)                                  \
         Botan::assertion_failure(#ptr " is not null",     \
                                  "",                      \
                                  BOTAN_CURRENT_FUNCTION,  \
                                  __FILE__,                \
                                  __LINE__);               \
   } while(0)

/**
* Mark variable as unused
*/
#define BOTAN_UNUSED(v) static_cast<void>(v)

}


/**
* The primary namespace for the botan library
*/
namespace Botan {

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;
using std::int32_t;
using std::int64_t;
using std::size_t;

using byte   = std::uint8_t;
using u16bit = std::uint16_t;
using u32bit = std::uint32_t;
using u64bit = std::uint64_t;
using s32bit = std::int32_t;

/**
* A default buffer size; typically a memory page
*/
static const size_t DEFAULT_BUFFERSIZE = BOTAN_DEFAULT_BUFFER_SIZE;

}


namespace Botan {

/**
* Zeroize memory
* @param ptr a pointer to memory to zero out
* @param n the number of bytes pointed to by ptr
*/
BOTAN_DLL void zero_mem(void* ptr, size_t n);

/**
* Zeroize memory
* @param ptr a pointer to an array
* @param n the number of Ts pointed to by ptr
*/
template<typename T> inline void clear_mem(T* ptr, size_t n)
   {
   if(n > 0)
      {
      std::memset(ptr, 0, sizeof(T)*n);
      }
   }

/**
* Copy memory
* @param out the destination array
* @param in the source array
* @param n the number of elements of in/out
*/
template<typename T> inline void copy_mem(T* out, const T* in, size_t n)
   {
   if(n > 0)
      {
      std::memmove(out, in, sizeof(T)*n);
      }
   }

/**
* Set memory to a fixed value
* @param ptr a pointer to an array
* @param n the number of Ts pointed to by ptr
* @param val the value to set each byte to
*/
template<typename T>
inline void set_mem(T* ptr, size_t n, byte val)
   {
   if(n > 0)
      {
      std::memset(ptr, val, sizeof(T)*n);
      }
   }

/**
* Memory comparison, input insensitive
* @param p1 a pointer to an array
* @param p2 a pointer to another array
* @param n the number of Ts in p1 and p2
* @return true iff p1[i] == p2[i] forall i in [0...n)
*/
template<typename T> inline bool same_mem(const T* p1, const T* p2, size_t n)
   {
   volatile T difference = 0;

   for(size_t i = 0; i != n; ++i)
      difference |= (p1[i] ^ p2[i]);

   return difference == 0;
   }

/**
* XOR_ arrays. Postcondition out[i] = in[i] ^ out[i] forall i = 0...length
* @param out the input/output buffer
* @param in the read-only input buffer
* @param length the length of the buffers
*/
template<typename T>
void xor_buf(T out[], const T in[], size_t length)
   {
   for(size_t i = 0; i != length; ++i)
      {
      out[i] ^= in[i];
      }
   }

/**
* XOR arrays. Postcondition out[i] = in[i] ^ in2[i] forall i = 0...length
* @param out the output buffer
* @param in the first input buffer
* @param in2 the second output buffer
* @param length the length of the three buffers
*/
template<typename T> void xor_buf(T out[],
                                  const T in[],
                                  const T in2[],
                                  size_t length)
   {
   for(size_t i = 0; i != length; ++i)
      {
      out[i] = in[i] ^ in2[i];
      }
   }

template<typename Alloc, typename Alloc2>
void xor_buf(std::vector<byte, Alloc>& out,
             const std::vector<byte, Alloc2>& in,
             size_t n)
   {
   xor_buf(out.data(), in.data(), n);
   }

template<typename Alloc>
void xor_buf(std::vector<byte, Alloc>& out,
             const byte* in,
             size_t n)
   {
   xor_buf(out.data(), in, n);
   }

template<typename Alloc, typename Alloc2>
void xor_buf(std::vector<byte, Alloc>& out,
             const byte* in,
             const std::vector<byte, Alloc2>& in2,
             size_t n)
   {
   xor_buf(out.data(), in, in2.data(), n);
   }

template<typename T, typename Alloc, typename Alloc2>
std::vector<T, Alloc>&
operator^=(std::vector<T, Alloc>& out,
           const std::vector<T, Alloc2>& in)
   {
   if(out.size() < in.size())
      out.resize(in.size());

   xor_buf(out.data(), in.data(), in.size());
   return out;
   }

}


#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)
#endif

namespace Botan {

template<typename T>
class secure_allocator
   {
   public:
      typedef T          value_type;

      typedef T*         pointer;
      typedef const T*   const_pointer;

      typedef T&         reference;
      typedef const T&   const_reference;

      typedef std::size_t     size_type;
      typedef std::ptrdiff_t  difference_type;

      secure_allocator() BOTAN_NOEXCEPT {}

      template<typename U>
      secure_allocator(const secure_allocator<U>&) BOTAN_NOEXCEPT {}

      ~secure_allocator() BOTAN_NOEXCEPT {}

      pointer address(reference x) const BOTAN_NOEXCEPT
         { return std::addressof(x); }

      const_pointer address(const_reference x) const BOTAN_NOEXCEPT
         { return std::addressof(x); }

      pointer allocate(size_type n, const void* = 0)
         {
#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)
         if(pointer p = static_cast<pointer>(mlock_allocator::instance().allocate(n, sizeof(T))))
            return p;
#endif

         pointer p = new T[n];
         clear_mem(p, n);
         return p;
         }

      void deallocate(pointer p, size_type n)
         {
         zero_mem(p, n);

#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)
         if(mlock_allocator::instance().deallocate(p, n, sizeof(T)))
            return;
#endif

         delete [] p;
         }

      size_type max_size() const BOTAN_NOEXCEPT
         {
         return static_cast<size_type>(-1) / sizeof(T);
         }

      template<typename U, typename... Args>
      void construct(U* p, Args&&... args)
         {
         ::new(static_cast<void*>(p)) U(std::forward<Args>(args)...);
         }

      template<typename U> void destroy(U* p) { p->~U(); }
   };

template<typename T, typename U> inline bool
operator==(const secure_allocator<T>&, const secure_allocator<U>&)
   { return true; }

template<typename T, typename U> inline bool
operator!=(const secure_allocator<T>&, const secure_allocator<U>&)
   { return false; }

template<typename T> using secure_vector = std::vector<T, secure_allocator<T>>;
template<typename T> using secure_deque = std::deque<T, secure_allocator<T>>;

template<typename T>
std::vector<T> unlock(const secure_vector<T>& in)
   {
   std::vector<T> out(in.size());
   copy_mem(out.data(), in.data(), in.size());
   return out;
   }

template<typename T, typename Alloc>
size_t buffer_insert(std::vector<T, Alloc>& buf,
                     size_t buf_offset,
                     const T input[],
                     size_t input_length)
   {
   const size_t to_copy = std::min(input_length, buf.size() - buf_offset);
   if (to_copy > 0)
      {
      copy_mem(&buf[buf_offset], input, to_copy);
      }
   return to_copy;
   }

template<typename T, typename Alloc, typename Alloc2>
size_t buffer_insert(std::vector<T, Alloc>& buf,
                     size_t buf_offset,
                     const std::vector<T, Alloc2>& input)
   {
   const size_t to_copy = std::min(input.size(), buf.size() - buf_offset);
   if (to_copy > 0)
      {
      copy_mem(&buf[buf_offset], input.data(), to_copy);
      }
   return to_copy;
   }

template<typename T, typename Alloc, typename Alloc2>
std::vector<T, Alloc>&
operator+=(std::vector<T, Alloc>& out,
           const std::vector<T, Alloc2>& in)
   {
   const size_t copy_offset = out.size();
   out.resize(out.size() + in.size());
   if (in.size() > 0)
      {
      copy_mem(&out[copy_offset], in.data(), in.size());
      }
   return out;
   }

template<typename T, typename Alloc>
std::vector<T, Alloc>& operator+=(std::vector<T, Alloc>& out, T in)
   {
   out.push_back(in);
   return out;
   }

template<typename T, typename Alloc, typename L>
std::vector<T, Alloc>& operator+=(std::vector<T, Alloc>& out,
                                  const std::pair<const T*, L>& in)
   {
   const size_t copy_offset = out.size();
   out.resize(out.size() + in.second);
   if (in.second > 0)
      {
      copy_mem(&out[copy_offset], in.first, in.second);
      }
   return out;
   }

template<typename T, typename Alloc, typename L>
std::vector<T, Alloc>& operator+=(std::vector<T, Alloc>& out,
                                  const std::pair<T*, L>& in)
   {
   const size_t copy_offset = out.size();
   out.resize(out.size() + in.second);
   if (in.second > 0)
      {
      copy_mem(&out[copy_offset], in.first, in.second);
      }
   return out;
   }

/**
* Zeroise the values; length remains unchanged
* @param vec the vector to zeroise
*/
template<typename T, typename Alloc>
void zeroise(std::vector<T, Alloc>& vec)
   {
   clear_mem(vec.data(), vec.size());
   }

/**
* Zeroise the values then free the memory
* @param vec the vector to zeroise and free
*/
template<typename T, typename Alloc>
void zap(std::vector<T, Alloc>& vec)
   {
   zeroise(vec);
   vec.clear();
   vec.shrink_to_fit();
   }

}


namespace Botan {

/**
* Represents the length requirements on an algorithm key
*/
class BOTAN_DLL Key_Length_Specification
   {
   public:
      /**
      * Constructor for fixed length keys
      * @param keylen the supported key length
      */
      explicit Key_Length_Specification(size_t keylen) :
         m_min_keylen(keylen),
         m_max_keylen(keylen),
         m_keylen_mod(1)
         {
         }

      /**
      * Constructor for variable length keys
      * @param min_k the smallest supported key length
      * @param max_k the largest supported key length
      * @param k_mod the number of bytes the key must be a multiple of
      */
      Key_Length_Specification(size_t min_k,
                               size_t max_k,
                               size_t k_mod = 1) :
         m_min_keylen(min_k),
         m_max_keylen(max_k ? max_k : min_k),
         m_keylen_mod(k_mod)
         {
         }

      /**
      * @param length is a key length in bytes
      * @return true iff this length is a valid length for this algo
      */
      bool valid_keylength(size_t length) const
         {
         return ((length >= m_min_keylen) &&
                 (length <= m_max_keylen) &&
                 (length % m_keylen_mod == 0));
         }

      /**
      * @return minimum key length in bytes
      */
      size_t minimum_keylength() const
         {
         return m_min_keylen;
         }

      /**
      * @return maximum key length in bytes
      */
      size_t maximum_keylength() const
         {
         return m_max_keylen;
         }

      /**
      * @return key length multiple in bytes
      */
      size_t keylength_multiple() const
         {
         return m_keylen_mod;
         }

      Key_Length_Specification multiple(size_t n) const
         {
         return Key_Length_Specification(n * m_min_keylen,
                                         n * m_max_keylen,
                                         n * m_keylen_mod);
         }

   private:
      size_t m_min_keylen, m_max_keylen, m_keylen_mod;
   };

}



namespace Botan {

/**
* Parse a SCAN-style algorithm name
* @param scan_name the name
* @return the name components
*/
BOTAN_DLL std::vector<std::string>
parse_algorithm_name(const std::string& scan_name);

/**
* Split a string
* @param str the input string
* @param delim the delimitor
* @return string split by delim
*/
BOTAN_DLL std::vector<std::string> split_on(
   const std::string& str, char delim);

/**
* Split a string on a character predicate
* @param str the input string
*/
BOTAN_DLL std::vector<std::string>
split_on_pred(const std::string& str,
              std::function<bool (char)> pred);

/**
* Erase characters from a string
*/
BOTAN_DLL std::string erase_chars(const std::string& str, const std::set<char>& chars);

/**
* Replace a character in a string
* @param str the input string
* @param from_char the character to replace
* @param to_char the character to replace it with
* @return str with all instances of from_char replaced by to_char
*/
BOTAN_DLL std::string replace_char(const std::string& str,
                                   char from_char,
                                   char to_char);

/**
* Replace a character in a string
* @param str the input string
* @param from_chars the characters to replace
* @param to_char the character to replace it with
* @return str with all instances of from_chars replaced by to_char
*/
BOTAN_DLL std::string replace_chars(const std::string& str,
                                    const std::set<char>& from_chars,
                                    char to_char);

/**
* Join a string
* @param strs strings to join
* @param delim the delimitor
* @return string joined by delim
*/
BOTAN_DLL std::string string_join(const std::vector<std::string>& strs,
                                  char delim);

/**
* Parse an ASN.1 OID
* @param oid the OID in string form
* @return OID components
*/
BOTAN_DLL std::vector<u32bit> parse_asn1_oid(const std::string& oid);

/**
* Compare two names using the X.509 comparison algorithm
* @param name1 the first name
* @param name2 the second name
* @return true if name1 is the same as name2 by the X.509 comparison rules
*/
BOTAN_DLL bool x500_name_cmp(const std::string& name1,
                             const std::string& name2);

/**
* Convert a string to a number
* @param str the string to convert
* @return number value of the string
*/
BOTAN_DLL u32bit to_u32bit(const std::string& str);

/**
* Convert a time specification to a number
* @param timespec the time specification
* @return number of seconds represented by timespec
*/
BOTAN_DLL u32bit timespec_to_u32bit(const std::string& timespec);

/**
* Convert a string representation of an IPv4 address to a number
* @param ip_str the string representation
* @return integer IPv4 address
*/
BOTAN_DLL u32bit string_to_ipv4(const std::string& ip_str);

/**
* Convert an IPv4 address to a string
* @param ip_addr the IPv4 address to convert
* @return string representation of the IPv4 address
*/
BOTAN_DLL std::string ipv4_to_string(u32bit ip_addr);

std::map<std::string, std::string> BOTAN_DLL read_cfg(std::istream& is);

std::string BOTAN_DLL clean_ws(const std::string& s);

bool BOTAN_DLL host_wildcard_match(const std::string& wildcard, const std::string& host);


}


namespace Botan {

/**
* Base class for all exceptions thrown by the library
*/
class BOTAN_DLL Exception : public std::exception
   {
   public:
      explicit Exception(const std::string& msg) : m_msg(msg) {}
      Exception(const char* prefix, const std::string& msg) : m_msg(std::string(prefix) + " " + msg) {}
      const char* what() const BOTAN_NOEXCEPT override { return m_msg.c_str(); }
   private:
      std::string m_msg;
   };

/**
* An invalid argument which caused
*/
class BOTAN_DLL Invalid_Argument : public Exception
   {
   public:
      explicit Invalid_Argument(const std::string& msg) :
         Exception("Invalid argument", msg) {}
   };

/**
* Unsupported_Argument Exception
*
* An argument that is invalid because it is not supported by Botan.
* It might or might not be valid in another context like a standard.
*/
struct BOTAN_DLL Unsupported_Argument : public Invalid_Argument
   {
   explicit Unsupported_Argument(const std::string& msg) : Invalid_Argument(msg) {}
   };

/**
* Invalid_State Exception
*/
struct BOTAN_DLL Invalid_State : public Exception
   {
   explicit Invalid_State(const std::string& err) :
      Exception(err)
      {}
   };

/**
* Lookup_Error Exception
*/
struct BOTAN_DLL Lookup_Error : public Exception
   {
   explicit Lookup_Error(const std::string& err) :
      Exception(err)
      {}
   };

/**
* Internal_Error Exception
*/
struct BOTAN_DLL Internal_Error : public Exception
   {
   explicit Internal_Error(const std::string& err) :
      Exception("Internal error: " + err)
      {}
   };

/**
* Invalid_Key_Length Exception
*/
struct BOTAN_DLL Invalid_Key_Length : public Invalid_Argument
   {
   Invalid_Key_Length(const std::string& name, size_t length) :
      Invalid_Argument(name + " cannot accept a key of length " +
                       std::to_string(length))
      {}
   };

/**
* Invalid_IV_Length Exception
*/
struct BOTAN_DLL Invalid_IV_Length : public Invalid_Argument
   {
   Invalid_IV_Length(const std::string& mode, size_t bad_len) :
      Invalid_Argument("IV length " + std::to_string(bad_len) +
                       " is invalid for " + mode)
      {}
   };

/**
* PRNG_Unseeded Exception
*/
struct BOTAN_DLL PRNG_Unseeded : public Invalid_State
   {
   explicit PRNG_Unseeded(const std::string& algo) :
      Invalid_State("PRNG not seeded: " + algo)
      {}
   };

/**
* Policy_Violation Exception
*/
struct BOTAN_DLL Policy_Violation : public Invalid_State
   {
   explicit Policy_Violation(const std::string& err) :
      Invalid_State("Policy violation: " + err)
      {}
   };

/**
* Algorithm_Not_Found Exception
*/
struct BOTAN_DLL Algorithm_Not_Found : public Lookup_Error
   {
   explicit Algorithm_Not_Found(const std::string& name) :
      Lookup_Error("Could not find any algorithm named \"" + name + "\"")
      {}
   };

/**
* No_Provider_Found Exception
*/
struct BOTAN_DLL No_Provider_Found : public Exception
   {
   explicit No_Provider_Found(const std::string& name) :
      Exception("Could not find any provider for algorithm named \"" + name + "\"")
      {}
   };

/**
* Invalid_Algorithm_Name Exception
*/
struct BOTAN_DLL Invalid_Algorithm_Name : public Invalid_Argument
   {
   explicit Invalid_Algorithm_Name(const std::string& name):
      Invalid_Argument("Invalid algorithm name: " + name)
      {}
   };

/**
* Encoding_Error Exception
*/
struct BOTAN_DLL Encoding_Error : public Invalid_Argument
   {
   explicit Encoding_Error(const std::string& name) :
      Invalid_Argument("Encoding error: " + name) {}
   };

/**
* Decoding_Error Exception
*/
struct BOTAN_DLL Decoding_Error : public Invalid_Argument
   {
   explicit Decoding_Error(const std::string& name) :
      Invalid_Argument("Decoding error: " + name) {}
   };

/**
* Integrity_Failure Exception
*/
struct BOTAN_DLL Integrity_Failure : public Exception
   {
   explicit Integrity_Failure(const std::string& msg) :
      Exception("Integrity failure: " + msg) {}
   };

/**
* Invalid_OID Exception
*/
struct BOTAN_DLL Invalid_OID : public Decoding_Error
   {
   explicit Invalid_OID(const std::string& oid) :
      Decoding_Error("Invalid ASN.1 OID: " + oid) {}
   };

/**
* Stream_IO_Error Exception
*/
struct BOTAN_DLL Stream_IO_Error : public Exception
   {
   explicit Stream_IO_Error(const std::string& err) :
      Exception("I/O error: " + err)
      {}
   };

/**
* No_Filesystem_Access Exception
*/
struct BOTAN_DLL No_Filesystem_Access : public Exception
   {
   No_Filesystem_Access() : Exception("No filesystem access enabled.") {}
   };

/**
* Self Test Failure Exception
*/
struct BOTAN_DLL Self_Test_Failure : public Internal_Error
   {
   explicit Self_Test_Failure(const std::string& err) :
      Internal_Error("Self test failed: " + err)
      {}
   };

/**
* Not Implemented Exception
*/
struct BOTAN_DLL Not_Implemented : public Exception
   {
   explicit Not_Implemented(const std::string& err) :
      Exception("Not implemented", err)
      {}
   };

}


namespace Botan {

/**
* Octet String
*/
class BOTAN_DLL OctetString
   {
   public:
      /**
      * @return size of this octet string in bytes
      */
      size_t length() const { return m_data.size(); }
      size_t size() const { return m_data.size(); }

      /**
      * @return this object as a secure_vector<byte>
      */
      secure_vector<byte> bits_of() const { return m_data; }

      /**
      * @return start of this string
      */
      const byte* begin() const { return m_data.data(); }

      /**
      * @return end of this string
      */
      const byte* end() const   { return begin() + m_data.size(); }

      /**
      * @return this encoded as hex
      */
      std::string as_string() const;

      /**
      * XOR the contents of another octet string into this one
      * @param other octet string
      * @return reference to this
      */
      OctetString& operator^=(const OctetString& other);

      /**
      * Force to have odd parity
      */
      void set_odd_parity();

      /**
      * Create a new OctetString
      * @param str is a hex encoded string
      */
      explicit OctetString(const std::string& str = "");

      /**
      * Create a new random OctetString
      * @param rng is a random number generator
      * @param len is the desired length in bytes
      */
      OctetString(class RandomNumberGenerator& rng, size_t len);

      /**
      * Create a new OctetString
      * @param in is an array
      * @param len is the length of in in bytes
      */
      OctetString(const byte in[], size_t len);

      /**
      * Create a new OctetString
      * @param in a bytestring
      */
      OctetString(const secure_vector<byte>& in) : m_data(in) {}

      /**
      * Create a new OctetString
      * @param in a bytestring
      */
      OctetString(const std::vector<byte>& in) : m_data(in.begin(), in.end()) {}

   private:
      secure_vector<byte> m_data;
   };

/**
* Compare two strings
* @param x an octet string
* @param y an octet string
* @return if x is equal to y
*/
BOTAN_DLL bool operator==(const OctetString& x,
                          const OctetString& y);

/**
* Compare two strings
* @param x an octet string
* @param y an octet string
* @return if x is not equal to y
*/
BOTAN_DLL bool operator!=(const OctetString& x,
                          const OctetString& y);

/**
* Concatenate two strings
* @param x an octet string
* @param y an octet string
* @return x concatenated with y
*/
BOTAN_DLL OctetString operator+(const OctetString& x,
                                const OctetString& y);

/**
* XOR two strings
* @param x an octet string
* @param y an octet string
* @return x XORed with y
*/
BOTAN_DLL OctetString operator^(const OctetString& x,
                                const OctetString& y);


/**
* Alternate name for octet string showing intent to use as a key
*/
using SymmetricKey = OctetString;

/**
* Alternate name for octet string showing intent to use as an IV
*/
using InitializationVector = OctetString;

}


namespace Botan {

/**
A class encapsulating a SCAN name (similar to JCE conventions)
http://www.users.zetnet.co.uk/hopwood/crypto/scan/
*/
class BOTAN_DLL SCAN_Name
   {
   public:
      /**
      * @param algo_spec A SCAN-format name
      */
      explicit SCAN_Name(const char* algo_spec);

      /**
      * @param algo_spec A SCAN-format name
      */
      explicit SCAN_Name(std::string algo_spec);

      /**
      * @param algo_spec A SCAN-format name
      */
      SCAN_Name(std::string algo_spec, const std::string& extra);

      /**
      * @return original input string
      */
      const std::string& as_string() const { return m_orig_algo_spec; }

      /**
      * @return algorithm name
      */
      const std::string& algo_name() const { return m_alg_name; }

      /**
      * @return algorithm name plus any arguments
      */
      std::string algo_name_and_args() const { return algo_name() + all_arguments(); }

      /**
      * @return all arguments
      */
      std::string all_arguments() const;

      /**
      * @return number of arguments
      */
      size_t arg_count() const { return m_args.size(); }

      /**
      * @param lower is the lower bound
      * @param upper is the upper bound
      * @return if the number of arguments is between lower and upper
      */
      bool arg_count_between(size_t lower, size_t upper) const
         { return ((arg_count() >= lower) && (arg_count() <= upper)); }

      /**
      * @param i which argument
      * @return ith argument
      */
      std::string arg(size_t i) const;

      /**
      * @param i which argument
      * @param def_value the default value
      * @return ith argument or the default value
      */
      std::string arg(size_t i, const std::string& def_value) const;

      /**
      * @param i which argument
      * @param def_value the default value
      * @return ith argument as an integer, or the default value
      */
      size_t arg_as_integer(size_t i, size_t def_value) const;

      /**
      * @return cipher mode (if any)
      */
      std::string cipher_mode() const
         { return (m_mode_info.size() >= 1) ? m_mode_info[0] : ""; }

      /**
      * @return cipher mode padding (if any)
      */
      std::string cipher_mode_pad() const
         { return (m_mode_info.size() >= 2) ? m_mode_info[1] : ""; }

      static void add_alias(const std::string& alias, const std::string& basename);

      static std::string deref_alias(const std::string& alias);
   private:
      static std::mutex g_alias_map_mutex;
      static std::map<std::string, std::string> g_alias_map;

      std::string m_orig_algo_spec;
      std::string m_alg_name;
      std::vector<std::string> m_args;
      std::vector<std::string> m_mode_info;
   };

}


namespace Botan {

/**
* Interface for cipher modes
*/
class BOTAN_DLL Cipher_Mode
   {
   public:
      typedef SCAN_Name Spec;

      virtual ~Cipher_Mode() {}

      /**
      * Begin processing a message.
      * @param nonce the per message nonce
      */
      template<typename Alloc>
      secure_vector<byte> start(const std::vector<byte, Alloc>& nonce)
         {
         return start(nonce.data(), nonce.size());
         }

      /**
      * Begin processing a message.
      * @param nonce the per message nonce
      */
      template<typename Alloc>
      BOTAN_DEPRECATED("Use Transform::start")
      secure_vector<byte> start_vec(const std::vector<byte, Alloc>& nonce)
         {
         return start(nonce.data(), nonce.size());
         }

      /**
      * Begin processing a message.
      * @param nonce the per message nonce
      * @param nonce_len length of nonce
      */
      secure_vector<byte> start(const byte nonce[], size_t nonce_len)
         {
         return start_raw(nonce, nonce_len);
         }

      /**
      * Begin processing a message.
      */
      secure_vector<byte> start()
         {
         return start_raw(nullptr, 0);
         }

      virtual secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) = 0;

      /**
      * Process some data. Input must be in size update_granularity() byte blocks.
      * @param blocks in/out parameter which will possibly be resized
      * @param offset an offset into blocks to begin processing
      */
      virtual void update(secure_vector<byte>& blocks, size_t offset = 0) = 0;

      /**
      * Complete processing of a message.
      *
      * @param final_block in/out parameter which must be at least
      *        minimum_final_size() bytes, and will be set to any final output
      * @param offset an offset into final_block to begin processing
      */
      virtual void finish(secure_vector<byte>& final_block, size_t offset = 0) = 0;

      /**
      * Returns the size of the output if this transform is used to process a
      * message with input_length bytes. Will throw if unable to give a precise
      * answer.
      */
      virtual size_t output_length(size_t input_length) const = 0;

      /**
      * @return size of required blocks to update
      */
      virtual size_t update_granularity() const = 0;

      /**
      * @return required minimium size to finalize() - may be any
      *         length larger than this.
      */
      virtual size_t minimum_final_size() const = 0;

      /**
      * Return the default size for a nonce
      */
      virtual size_t default_nonce_length() const = 0;

      /**
      * Return true iff nonce_len is a valid length for the nonce
      */
      virtual bool valid_nonce_length(size_t nonce_len) const = 0;

      /**
      * Return some short name describing the provider of this tranformation.
      * Useful in cases where multiple implementations are available (eg,
      * different implementations of AES). Default "core" is used for the
      * 'standard' implementation included in the library.
      */
      virtual std::string provider() const { return "core"; }

      virtual std::string name() const = 0;

      virtual void clear() = 0;

      /**
      * Returns true iff this mode provides authentication as well as
      * confidentiality.
      */
      virtual bool authenticated() const { return false; }

      /**
      * Return the size of the authentication tag used (in bytes)
      */
      virtual size_t tag_size() const { return 0; }

      /**
      * @return object describing limits on key size
      */
      virtual Key_Length_Specification key_spec() const = 0;

      /**
      * Check whether a given key length is valid for this algorithm.
      * @param length the key length to be checked.
      * @return true if the key length is valid.
      */
      bool valid_keylength(size_t length) const
         {
         return key_spec().valid_keylength(length);
         }

      template<typename Alloc>
      void set_key(const std::vector<byte, Alloc>& key)
         {
         set_key(key.data(), key.size());
         }

      void set_key(const SymmetricKey& key)
         {
         set_key(key.begin(), key.length());
         }

      /**
      * Set the symmetric key of this transform
      * @param key contains the key material
      * @param length in bytes of key param
      */
      void set_key(const byte key[], size_t length)
         {
         if(!valid_keylength(length))
            throw Invalid_Key_Length(name(), length);
         key_schedule(key, length);
         }

   private:
      virtual void key_schedule(const byte key[], size_t length) = 0;
   };

/**
* The two possible directions for cipher filters, determining whether they
* actually perform encryption or decryption.
*/
enum Cipher_Dir { ENCRYPTION, DECRYPTION };

BOTAN_DLL Cipher_Mode* get_cipher_mode(const std::string& algo_spec, Cipher_Dir direction);

}


namespace Botan {

/**
* Interface for AEAD (Authenticated Encryption with Associated Data)
* modes. These modes provide both encryption and message
* authentication, and can authenticate additional per-message data
* which is not included in the ciphertext (for instance a sequence
* number).
*/
class BOTAN_DLL AEAD_Mode : public Cipher_Mode
   {
   public:
      bool authenticated() const override { return true; }

      /**
      * Set associated data that is not included in the ciphertext but
      * that should be authenticated. Must be called after set_key and
      * before start.
      *
      * Unless reset by another call, the associated data is kept
      * between messages. Thus, if the AD does not change, calling
      * once (after set_key) is the optimum.
      *
      * @param ad the associated data
      * @param ad_len length of add in bytes
      */
      virtual void set_associated_data(const byte ad[], size_t ad_len) = 0;

      template<typename Alloc>
      void set_associated_data_vec(const std::vector<byte, Alloc>& ad)
         {
         set_associated_data(ad.data(), ad.size());
         }

      template<typename Alloc>
      void set_ad(const std::vector<byte, Alloc>& ad)
         {
         set_associated_data(ad.data(), ad.size());
         }

      /**
      * Default AEAD nonce size (a commonly supported value among AEAD
      * modes, and large enough that random collisions are unlikely).
      */
      size_t default_nonce_length() const override { return 12; }

      virtual ~AEAD_Mode();
   };

/**
* Get an AEAD mode by name (eg "AES-128/GCM" or "Serpent/EAX")
*/
BOTAN_DLL AEAD_Mode* get_aead(const std::string& name, Cipher_Dir direction);

}


namespace Botan {

/**
* This class represents a symmetric algorithm object.
*/
class BOTAN_DLL SymmetricAlgorithm
   {
   public:
      virtual ~SymmetricAlgorithm() {}

      virtual void clear() = 0;

      /**
      * @return object describing limits on key size
      */
      virtual Key_Length_Specification key_spec() const = 0;

      /**
      * @return minimum allowed key length
      */
      size_t maximum_keylength() const
         {
         return key_spec().maximum_keylength();
         }

      /**
      * @return maxmium allowed key length
      */
      size_t minimum_keylength() const
         {
         return key_spec().minimum_keylength();
         }

      /**
      * Check whether a given key length is valid for this algorithm.
      * @param length the key length to be checked.
      * @return true if the key length is valid.
      */
      bool valid_keylength(size_t length) const
         {
         return key_spec().valid_keylength(length);
         }

      /**
      * Set the symmetric key of this object.
      * @param key the SymmetricKey to be set.
      */
      void set_key(const SymmetricKey& key)
         {
         set_key(key.begin(), key.length());
         }

      template<typename Alloc>
      void set_key(const std::vector<byte, Alloc>& key)
         {
         set_key(key.data(), key.size());
         }

      /**
      * Set the symmetric key of this object.
      * @param key the to be set as a byte array.
      * @param length in bytes of key param
      */
      void set_key(const byte key[], size_t length)
         {
         if(!valid_keylength(length))
            throw Invalid_Key_Length(name(), length);
         key_schedule(key, length);
         }

      virtual std::string name() const = 0;

   private:
      /**
      * Run the key schedule
      * @param key the key
      * @param length of key
      */
      virtual void key_schedule(const byte key[], size_t length) = 0;
   };

}


namespace Botan {

/**
* This class represents a block cipher object.
*/
class BOTAN_DLL BlockCipher : public SymmetricAlgorithm
   {
   public:
      typedef SCAN_Name Spec;

      /**
      * Create an instance based on a name
      * Will return a null pointer if the algo/provider combination cannot
      * be found. If provider is empty then best available is chosen.
      */
      static std::unique_ptr<BlockCipher> create(const std::string& algo_spec,
                                                 const std::string& provider = "");

      /**
      * Returns the list of available providers for this algorithm, empty if not available
      */
      static std::vector<std::string> providers(const std::string& algo_spec);

      /**
      * @return block size of this algorithm
      */
      virtual size_t block_size() const = 0;

      /**
      * @return native parallelism of this cipher in blocks
      */
      virtual size_t parallelism() const { return 1; }

      /**
      * @return prefererred parallelism of this cipher in bytes
      */
      size_t parallel_bytes() const
         {
         return parallelism() * block_size() * BOTAN_BLOCK_CIPHER_PAR_MULT;
         }

      /**
      * Encrypt a block.
      * @param in The plaintext block to be encrypted as a byte array.
      * Must be of length block_size().
      * @param out The byte array designated to hold the encrypted block.
      * Must be of length block_size().
      */
      void encrypt(const byte in[], byte out[]) const
         { encrypt_n(in, out, 1); }

      /**
      * Decrypt a block.
      * @param in The ciphertext block to be decypted as a byte array.
      * Must be of length block_size().
      * @param out The byte array designated to hold the decrypted block.
      * Must be of length block_size().
      */
      void decrypt(const byte in[], byte out[]) const
         { decrypt_n(in, out, 1); }

      /**
      * Encrypt a block.
      * @param block the plaintext block to be encrypted
      * Must be of length block_size(). Will hold the result when the function
      * has finished.
      */
      void encrypt(byte block[]) const { encrypt_n(block, block, 1); }

      /**
      * Decrypt a block.
      * @param block the ciphertext block to be decrypted
      * Must be of length block_size(). Will hold the result when the function
      * has finished.
      */
      void decrypt(byte block[]) const { decrypt_n(block, block, 1); }

      /**
      * Encrypt one or more blocks
      * @param block the input/output buffer (multiple of block_size())
      */
      template<typename Alloc>
      void encrypt(std::vector<byte, Alloc>& block) const
         {
         return encrypt_n(block.data(), block.data(), block.size() / block_size());
         }

      /**
      * Decrypt one or more blocks
      * @param block the input/output buffer (multiple of block_size())
      */
      template<typename Alloc>
      void decrypt(std::vector<byte, Alloc>& block) const
         {
         return decrypt_n(block.data(), block.data(), block.size() / block_size());
         }

      /**
      * Encrypt one or more blocks
      * @param in the input buffer (multiple of block_size())
      * @param out the output buffer (same size as in)
      */
      template<typename Alloc, typename Alloc2>
      void encrypt(const std::vector<byte, Alloc>& in,
                   std::vector<byte, Alloc2>& out) const
         {
         return encrypt_n(in.data(), out.data(), in.size() / block_size());
         }

      /**
      * Decrypt one or more blocks
      * @param in the input buffer (multiple of block_size())
      * @param out the output buffer (same size as in)
      */
      template<typename Alloc, typename Alloc2>
      void decrypt(const std::vector<byte, Alloc>& in,
                   std::vector<byte, Alloc2>& out) const
         {
         return decrypt_n(in.data(), out.data(), in.size() / block_size());
         }

      /**
      * Encrypt one or more blocks
      * @param in the input buffer (multiple of block_size())
      * @param out the output buffer (same size as in)
      * @param blocks the number of blocks to process
      */
      virtual void encrypt_n(const byte in[], byte out[],
                             size_t blocks) const = 0;

      /**
      * Decrypt one or more blocks
      * @param in the input buffer (multiple of block_size())
      * @param out the output buffer (same size as in)
      * @param blocks the number of blocks to process
      */
      virtual void decrypt_n(const byte in[], byte out[],
                             size_t blocks) const = 0;

      /**
      * @return new object representing the same algorithm as *this
      */
      virtual BlockCipher* clone() const = 0;

      virtual ~BlockCipher();
   };

/**
* Represents a block cipher with a single fixed block size
*/
template<size_t BS, size_t KMIN, size_t KMAX = 0, size_t KMOD = 1>
class Block_Cipher_Fixed_Params : public BlockCipher
   {
   public:
      enum { BLOCK_SIZE = BS };
      size_t block_size() const override { return BS; }

      Key_Length_Specification key_spec() const override
         {
         return Key_Length_Specification(KMIN, KMAX, KMOD);
         }
   };

}


namespace Botan {

/**
* AES-128
*/
class BOTAN_DLL AES_128 final : public Block_Cipher_Fixed_Params<16, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void clear() override;

      std::string name() const override { return "AES-128"; }
      BlockCipher* clone() const override { return new AES_128; }
   private:
      void key_schedule(const byte key[], size_t length) override;

      secure_vector<u32bit> m_EK, m_DK;
      secure_vector<byte> m_ME, m_MD;
   };

/**
* AES-192
*/
class BOTAN_DLL AES_192 final : public Block_Cipher_Fixed_Params<16, 24>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void clear() override;

      std::string name() const override { return "AES-192"; }
      BlockCipher* clone() const override { return new AES_192; }
   private:
      void key_schedule(const byte key[], size_t length) override;

      secure_vector<u32bit> m_EK, m_DK;
      secure_vector<byte> m_ME, m_MD;
   };

/**
* AES-256
*/
class BOTAN_DLL AES_256 final : public Block_Cipher_Fixed_Params<16, 32>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void clear() override;

      std::string name() const override { return "AES-256"; }
      BlockCipher* clone() const override { return new AES_256; }
   private:
      void key_schedule(const byte key[], size_t length) override;

      secure_vector<u32bit> m_EK, m_DK;
      secure_vector<byte> m_ME, m_MD;
   };

}


namespace Botan {

class BER_Decoder;
class DER_Encoder;

/**
* ASN.1 Type and Class Tags
*/
enum ASN1_Tag {
   UNIVERSAL        = 0x00,
   APPLICATION      = 0x40,
   CONTEXT_SPECIFIC = 0x80,

   CONSTRUCTED      = 0x20,

   PRIVATE          = CONSTRUCTED | CONTEXT_SPECIFIC,

   EOC              = 0x00,
   BOOLEAN          = 0x01,
   INTEGER          = 0x02,
   BIT_STRING       = 0x03,
   OCTET_STRING     = 0x04,
   NULL_TAG         = 0x05,
   OBJECT_ID        = 0x06,
   ENUMERATED       = 0x0A,
   SEQUENCE         = 0x10,
   SET              = 0x11,

   UTF8_STRING      = 0x0C,
   NUMERIC_STRING   = 0x12,
   PRINTABLE_STRING = 0x13,
   T61_STRING       = 0x14,
   IA5_STRING       = 0x16,
   VISIBLE_STRING   = 0x1A,
   BMP_STRING       = 0x1E,

   UTC_TIME                = 0x17,
   GENERALIZED_TIME        = 0x18,
   UTC_OR_GENERALIZED_TIME = 0x19,

   NO_OBJECT        = 0xFF00,
   DIRECTORY_STRING = 0xFF01
};

/**
* Basic ASN.1 Object Interface
*/
class BOTAN_DLL ASN1_Object
   {
   public:
      /**
      * Encode whatever this object is into to
      * @param to the DER_Encoder that will be written to
      */
      virtual void encode_into(DER_Encoder& to) const = 0;

      /**
      * Decode whatever this object is from from
      * @param from the BER_Decoder that will be read from
      */
      virtual void decode_from(BER_Decoder& from) = 0;

      virtual ~ASN1_Object() {}
   };

/**
* BER Encoded Object
*/
class BOTAN_DLL BER_Object
   {
   public:
      void assert_is_a(ASN1_Tag, ASN1_Tag);

      // public member variable:
      ASN1_Tag type_tag, class_tag;

      // public member variable:
      secure_vector<byte> value;
   };

/*
* ASN.1 Utility Functions
*/
class DataSource;

namespace ASN1 {

std::vector<byte> put_in_sequence(const std::vector<byte>& val);
std::string to_string(const BER_Object& obj);

/**
* Heuristics tests; is this object possibly BER?
* @param src a data source that will be peeked at but not modified
*/
bool maybe_BER(DataSource& src);

}

/**
* General BER Decoding Error Exception
*/
struct BOTAN_DLL BER_Decoding_Error : public Decoding_Error
   {
   explicit BER_Decoding_Error(const std::string&);
   };

/**
* Exception For Incorrect BER Taggings
*/
struct BOTAN_DLL BER_Bad_Tag : public BER_Decoding_Error
   {
   BER_Bad_Tag(const std::string& msg, ASN1_Tag tag);
   BER_Bad_Tag(const std::string& msg, ASN1_Tag tag1, ASN1_Tag tag2);
   };

}


namespace Botan {

/**
* This class represents ASN.1 object identifiers.
*/
class BOTAN_DLL OID final : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;

      /**
      * Find out whether this OID is empty
      * @return true is no OID value is set
      */
      bool empty() const { return m_id.size() == 0; }

      /**
      * Get this OID as list (vector) of its components.
      * @return vector representing this OID
      */
      const std::vector<u32bit>& get_id() const { return m_id; }

      /**
      * Get this OID as a string
      * @return string representing this OID
      */
      std::string as_string() const;

      /**
      * Compare two OIDs.
      * @return true if they are equal, false otherwise
      */
      bool operator==(const OID&) const;

      /**
      * Reset this instance to an empty OID.
      */
      void clear();

      /**
      * Add a component to this OID.
      * @param new_comp the new component to add to the end of this OID
      * @return reference to *this
      */
      OID& operator+=(u32bit new_comp);

      /**
      * Construct an OID from a string.
      * @param str a string in the form "a.b.c" etc., where a,b,c are numbers
      */
      OID(const std::string& str = "");
   private:
      std::vector<u32bit> m_id;
   };

/**
* Append another component onto the OID.
* @param oid the OID to add the new component to
* @param new_comp the new component to add
*/
OID BOTAN_DLL operator+(const OID& oid, u32bit new_comp);

/**
* Compare two OIDs.
* @param a the first OID
* @param b the second OID
* @return true if a is not equal to b
*/
bool BOTAN_DLL operator!=(const OID& a, const OID& b);

/**
* Compare two OIDs.
* @param a the first OID
* @param b the second OID
* @return true if a is lexicographically smaller than b
*/
bool BOTAN_DLL operator<(const OID& a, const OID& b);

}


namespace Botan {

/**
* Algorithm Identifier
*/
class BOTAN_DLL AlgorithmIdentifier final : public ASN1_Object
   {
   public:
      enum Encoding_Option { USE_NULL_PARAM };

      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;

      AlgorithmIdentifier() {}
      AlgorithmIdentifier(const OID&, Encoding_Option);
      AlgorithmIdentifier(const std::string&, Encoding_Option);

      AlgorithmIdentifier(const OID&, const std::vector<byte>&);
      AlgorithmIdentifier(const std::string&, const std::vector<byte>&);

      // public member variable:
      OID oid;

      // public member variable:
      std::vector<byte> parameters;
   };

/*
* Comparison Operations
*/
bool BOTAN_DLL operator==(const AlgorithmIdentifier&,
                          const AlgorithmIdentifier&);
bool BOTAN_DLL operator!=(const AlgorithmIdentifier&,
                          const AlgorithmIdentifier&);

}


namespace Botan {

/**
* Simple String
*/
class BOTAN_DLL ASN1_String final : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;

      std::string value() const;
      std::string iso_8859() const;

      ASN1_Tag tagging() const;

      explicit ASN1_String(const std::string& = "");
      ASN1_String(const std::string&, ASN1_Tag);
   private:
      std::string m_iso_8859_str;
      ASN1_Tag m_tag;
   };

}


namespace Botan {

/**
* Alternative Name
*/
class BOTAN_DLL AlternativeName final : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;

      std::multimap<std::string, std::string> contents() const;

      void add_attribute(const std::string&, const std::string&);
      std::multimap<std::string, std::string> get_attributes() const;

      void add_othername(const OID&, const std::string&, ASN1_Tag);
      std::multimap<OID, ASN1_String> get_othernames() const;

      bool has_items() const;

      AlternativeName(const std::string& = "", const std::string& = "",
                      const std::string& = "", const std::string& = "");
   private:
      std::multimap<std::string, std::string> m_alt_info;
      std::multimap<OID, ASN1_String> m_othernames;
   };

}


namespace Botan {

/**
* Attribute
*/
class BOTAN_DLL Attribute final : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder& to) const override;
      void decode_from(class BER_Decoder& from) override;

      // public member variable:
      OID oid;

      // public member variable:
      std::vector<byte> parameters;

      Attribute() {}
      Attribute(const OID&, const std::vector<byte>&);
      Attribute(const std::string&, const std::vector<byte>&);
   };

}


namespace Botan {

/**
* X.509 Time
*/
class BOTAN_DLL X509_Time final : public ASN1_Object
   {
   public:
      /// DER encode a X509_Time
      void encode_into(DER_Encoder&) const override;

      // Decode a BER encoded X509_Time
      void decode_from(BER_Decoder&) override;

      /// Return an internal string representation of the time
      std::string to_string() const;

      /// Returns a human friendly string replesentation of no particular formatting
      std::string readable_string() const;

      /// Return if the time has been set somehow
      bool time_is_set() const;

      ///  Compare this time against another
      s32bit cmp(const X509_Time& other) const;

      /// Create an invalid X509_Time
      X509_Time() {}

      /// Create a X509_Time from a time point
      explicit X509_Time(const std::chrono::system_clock::time_point& time);

      /// Create an X509_Time from string
      X509_Time(const std::string& t_spec, ASN1_Tag tag);

   private:
      void set_to(const std::string& t_spec, ASN1_Tag);
      bool passes_sanity_check() const;

      u32bit m_year = 0;
      u32bit m_month = 0;
      u32bit m_day = 0;
      u32bit m_hour = 0;
      u32bit m_minute = 0;
      u32bit m_second = 0;
      ASN1_Tag m_tag = NO_OBJECT;
   };

/*
* Comparison Operations
*/
bool BOTAN_DLL operator==(const X509_Time&, const X509_Time&);
bool BOTAN_DLL operator!=(const X509_Time&, const X509_Time&);
bool BOTAN_DLL operator<=(const X509_Time&, const X509_Time&);
bool BOTAN_DLL operator>=(const X509_Time&, const X509_Time&);
bool BOTAN_DLL operator<(const X509_Time&, const X509_Time&);
bool BOTAN_DLL operator>(const X509_Time&, const X509_Time&);

}


namespace Botan {

class Entropy_Sources;

/**
* This class represents a random number (RNG) generator object.
*/
class BOTAN_DLL RandomNumberGenerator
   {
   public:
      /**
      * Create a seeded and active RNG object for general application use
      * Added in 1.8.0
      */
      static RandomNumberGenerator* make_rng();

      /**
      * Randomize a byte array.
      * @param output the byte array to hold the random output.
      * @param length the length of the byte array output.
      */
      virtual void randomize(byte output[], size_t length) = 0;

      /**
      * Return a random vector
      * @param bytes number of bytes in the result
      * @return randomized vector of length bytes
      */
      virtual secure_vector<byte> random_vec(size_t bytes)
         {
         secure_vector<byte> output(bytes);
         randomize(output.data(), output.size());
         return output;
         }

      /**
      * Only usable with POD types, only useful with integers
      * get_random<u64bit>()
      */
      template<typename T> T get_random()
         {
         T r;
         this->randomize(reinterpret_cast<byte*>(&r), sizeof(r));
         return r;
         }

      /**
      * Return a random byte
      * @return random byte
      */
      byte next_byte() { return get_random<byte>(); }

      byte next_nonzero_byte()
         {
         byte b = next_byte();
         while(b == 0)
            b = next_byte();
         return b;
         }

      /**
      * Check whether this RNG is seeded.
      * @return true if this RNG was already seeded, false otherwise.
      */
      virtual bool is_seeded() const = 0;

      /**
      * Clear all internally held values of this RNG.
      */
      virtual void clear() = 0;

      /**
      * Return the name of this object
      */
      virtual std::string name() const = 0;

      /**
      * Seed this RNG using the global entropy sources and default timeout
      * @param bits_to_collect is the number of bits of entropy to
               attempt to gather from the entropy sources
      */
      size_t reseed(size_t bits_to_collect);

      /**
      * Seed this RNG using the global entropy sources
      * @param bits_to_collect is the number of bits of entropy to
               attempt to gather from the entropy sources
      * @param poll_timeout try not to run longer than this, no matter what
      */
      size_t reseed_with_timeout(size_t bits_to_collect,
                                 std::chrono::milliseconds poll_timeout);

      /**
      * Poll provided sources for up to poll_bits bits of entropy
      * or until the timeout expires. Returns estimate of the number
      * of bits collected.
      */
      virtual size_t reseed_with_sources(Entropy_Sources& srcs,
                                         size_t poll_bits,
                                         std::chrono::milliseconds poll_timeout) = 0;

      /**
      * Add entropy to this RNG.
      * @param in a byte array containg the entropy to be added
      * @param length the length of the byte array in
      */
      virtual void add_entropy(const byte in[], size_t length) = 0;

      /*
      * Never copy a RNG, create a new one
      */
      RandomNumberGenerator(const RandomNumberGenerator& rng) = delete;
      RandomNumberGenerator& operator=(const RandomNumberGenerator& rng) = delete;

      RandomNumberGenerator() {}
      virtual ~RandomNumberGenerator() {}
   };

typedef RandomNumberGenerator RNG;

/**
* Null/stub RNG - fails if you try to use it for anything
*/
class BOTAN_DLL Null_RNG : public RandomNumberGenerator
   {
   public:
      void randomize(byte[], size_t) override { throw PRNG_Unseeded("Null_RNG"); }

      void clear() override {}

      std::string name() const override { return "Null_RNG"; }

      size_t reseed_with_sources(Entropy_Sources&, size_t,
                                 std::chrono::milliseconds) override
         {
         return 0;
         }

      bool is_seeded() const override { return false; }
      void add_entropy(const byte[], size_t) override {}
   };

/**
* Wraps access to a RNG in a mutex
*/
class BOTAN_DLL Serialized_RNG : public RandomNumberGenerator
   {
   public:
      void randomize(byte out[], size_t len) override
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         m_rng->randomize(out, len);
         }

      bool is_seeded() const override
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         return m_rng->is_seeded();
         }

      void clear() override
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         m_rng->clear();
         }

      std::string name() const override
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         return m_rng->name();
         }

      size_t reseed_with_sources(Entropy_Sources& src,
                                 size_t bits,
                                 std::chrono::milliseconds msec) override
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         return m_rng->reseed_with_sources(src, bits, msec);
         }

      void add_entropy(const byte in[], size_t len) override
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         m_rng->add_entropy(in, len);
         }

      Serialized_RNG() : m_rng(RandomNumberGenerator::make_rng()) {}
      explicit Serialized_RNG(RandomNumberGenerator* rng) : m_rng(rng) {}
   private:
      mutable std::mutex m_mutex;
      std::unique_ptr<RandomNumberGenerator> m_rng;
   };

}


namespace Botan {

class AutoSeeded_RNG : public RandomNumberGenerator
   {
   public:
      void randomize(byte out[], size_t len) override
         { m_rng->randomize(out, len); }

      bool is_seeded() const override { return m_rng->is_seeded(); }

      void clear() override { m_rng->clear(); }

      std::string name() const override { return m_rng->name(); }

      size_t reseed_with_sources(Entropy_Sources& srcs,
                               size_t poll_bits,
                               std::chrono::milliseconds poll_timeout) override
         {
         return m_rng->reseed_with_sources(srcs, poll_bits, poll_timeout);
         }

      void add_entropy(const byte in[], size_t len) override
         { m_rng->add_entropy(in, len); }

      AutoSeeded_RNG() : m_rng(RandomNumberGenerator::make_rng()) {}
   private:
      std::unique_ptr<RandomNumberGenerator> m_rng;
   };

}


namespace Botan {

/**
* This class represents general abstract filter objects.
*/
class BOTAN_DLL Filter
   {
   public:
      /**
      * @return descriptive name for this filter
      */
      virtual std::string name() const = 0;

      /**
      * Write a portion of a message to this filter.
      * @param input the input as a byte array
      * @param length the length of the byte array input
      */
      virtual void write(const byte input[], size_t length) = 0;

      /**
      * Start a new message. Must be closed by end_msg() before another
      * message can be started.
      */
      virtual void start_msg() {}

      /**
      * Notify that the current message is finished; flush buffers and
      * do end-of-message processing (if any).
      */
      virtual void end_msg() {}

      /**
      * Check whether this filter is an attachable filter.
      * @return true if this filter is attachable, false otherwise
      */
      virtual bool attachable() { return true; }

      virtual ~Filter() {}
   protected:
      /**
      * @param in some input for the filter
      * @param length the length of in
      */
      virtual void send(const byte in[], size_t length);

      /**
      * @param in some input for the filter
      */
      void send(byte in) { send(&in, 1); }

      /**
      * @param in some input for the filter
      */
      void send(const secure_vector<byte>& in) { send(in.data(), in.size()); }

      /**
      * @param in some input for the filter
      */
      void send(const std::vector<byte>& in) { send(in.data(), in.size()); }

      /**
      * @param in some input for the filter
      * @param length the number of bytes of in to send
      */
      void send(const secure_vector<byte>& in, size_t length)
         {
         send(in.data(), length);
         }

      /**
      * @param in some input for the filter
      * @param length the number of bytes of in to send
      */
      void send(const std::vector<byte>& in, size_t length)
         {
         send(in.data(), length);
         }

      Filter();

      Filter(const Filter&) = delete;

      Filter& operator=(const Filter&) = delete;

   private:
      /**
      * Start a new message in *this and all following filters. Only for
      * internal use, not intended for use in client applications.
      */
      void new_msg();

      /**
      * End a new message in *this and all following filters. Only for
      * internal use, not intended for use in client applications.
      */
      void finish_msg();

      friend class Pipe;
      friend class Fanout_Filter;

      size_t total_ports() const;
      size_t current_port() const { return m_port_num; }

      /**
      * Set the active port
      * @param new_port the new value
      */
      void set_port(size_t new_port);

      size_t owns() const { return m_filter_owns; }

      /**
      * Attach another filter to this one
      * @param f filter to attach
      */
      void attach(Filter* f);

      /**
      * @param filters the filters to set
      * @param count number of items in filters
      */
      void set_next(Filter* filters[], size_t count);
      Filter* get_next() const;

      secure_vector<byte> m_write_queue;
      std::vector<Filter*> m_next;
      size_t m_port_num, m_filter_owns;

      // true if filter belongs to a pipe --> prohibit filter sharing!
      bool m_owned;
   };

/**
* This is the abstract Fanout_Filter base class.
**/
class BOTAN_DLL Fanout_Filter : public Filter
   {
   protected:
      /**
      * Increment the number of filters past us that we own
      */
      void incr_owns() { ++m_filter_owns; }

      void set_port(size_t n) { Filter::set_port(n); }

      void set_next(Filter* f[], size_t n) { Filter::set_next(f, n); }

      void attach(Filter* f) { Filter::attach(f); }

   private:
      friend class Threaded_Fork;
      using Filter::m_write_queue;
      using Filter::total_ports;
      using Filter::m_next;
   };

/**
* The type of checking to be performed by decoders:
* NONE - no checks, IGNORE_WS - perform checks, but ignore
* whitespaces, FULL_CHECK - perform checks, also complain
* about white spaces.
*/
enum Decoder_Checking { NONE, IGNORE_WS, FULL_CHECK };

}


namespace Botan {

/**
* This class represents a Base64 encoder.
*/
class BOTAN_DLL Base64_Encoder final : public Filter
   {
   public:
      std::string name() const override { return "Base64_Encoder"; }

      /**
      * Input a part of a message to the encoder.
      * @param input the message to input as a byte array
      * @param length the length of the byte array input
      */
      void write(const byte input[], size_t length) override;

      /**
      * Inform the Encoder that the current message shall be closed.
      */
      void end_msg() override;

      /**
      * Create a base64 encoder.
      * @param breaks whether to use line breaks in the output
      * @param length the length of the lines of the output
      * @param t_n whether to use a trailing newline
      */
      Base64_Encoder(bool breaks = false, size_t length = 72,
                     bool t_n = false);
   private:
      void encode_and_send(const byte input[], size_t length,
                           bool final_inputs = false);
      void do_output(const byte output[], size_t length);

      const size_t m_line_length;
      const bool m_trailing_newline;
      std::vector<byte> m_in, m_out;
      size_t m_position, m_out_position;
   };

/**
* This object represents a Base64 decoder.
*/
class BOTAN_DLL Base64_Decoder final : public Filter
   {
   public:
      std::string name() const override { return "Base64_Decoder"; }

      /**
      * Input a part of a message to the decoder.
      * @param input the message to input as a byte array
      * @param length the length of the byte array input
      */
      void write(const byte input[], size_t length) override;

      /**
      * Finish up the current message
      */
      void end_msg() override;

      /**
      * Create a base64 decoder.
      * @param checking the type of checking that shall be performed by
      * the decoder
      */
      explicit Base64_Decoder(Decoder_Checking checking = NONE);
   private:
      const Decoder_Checking m_checking;
      std::vector<byte> m_in, m_out;
      size_t m_position;
   };

}


namespace Botan {

/**
* Perform base64 encoding
* @param output an array of at least input_length*4/3 bytes
* @param input is some binary data
* @param input_length length of input in bytes
* @param input_consumed is an output parameter which says how many
*        bytes of input were actually consumed. If less than
*        input_length, then the range input[consumed:length]
*        should be passed in later along with more input.
* @param final_inputs true iff this is the last input, in which case
         padding chars will be applied if needed
* @return number of bytes written to output
*/
size_t BOTAN_DLL base64_encode(char output[],
                               const byte input[],
                               size_t input_length,
                               size_t& input_consumed,
                               bool final_inputs);

/**
* Perform base64 encoding
* @param input some input
* @param input_length length of input in bytes
* @return base64adecimal representation of input
*/
std::string BOTAN_DLL base64_encode(const byte input[],
                                    size_t input_length);

/**
* Perform base64 encoding
* @param input some input
* @return base64adecimal representation of input
*/
template<typename Alloc>
std::string base64_encode(const std::vector<byte, Alloc>& input)
   {
   return base64_encode(input.data(), input.size());
   }

/**
* Perform base64 decoding
* @param output an array of at least input_length*3/4 bytes
* @param input some base64 input
* @param input_length length of input in bytes
* @param input_consumed is an output parameter which says how many
*        bytes of input were actually consumed. If less than
*        input_length, then the range input[consumed:length]
*        should be passed in later along with more input.
* @param final_inputs true iff this is the last input, in which case
         padding is allowed
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL base64_decode(byte output[],
                               const char input[],
                               size_t input_length,
                               size_t& input_consumed,
                               bool final_inputs,
                               bool ignore_ws = true);

/**
* Perform base64 decoding
* @param output an array of at least input_length*3/4 bytes
* @param input some base64 input
* @param input_length length of input in bytes
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL base64_decode(byte output[],
                               const char input[],
                               size_t input_length,
                               bool ignore_ws = true);

/**
* Perform base64 decoding
* @param output an array of at least input_length/3*4 bytes
* @param input some base64 input
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL base64_decode(byte output[],
                               const std::string& input,
                               bool ignore_ws = true);

/**
* Perform base64 decoding
* @param input some base64 input
* @param input_length the length of input in bytes
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded base64 output
*/
secure_vector<byte> BOTAN_DLL base64_decode(const char input[],
                                           size_t input_length,
                                           bool ignore_ws = true);

/**
* Perform base64 decoding
* @param input some base64 input
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded base64 output
*/
secure_vector<byte> BOTAN_DLL base64_decode(const std::string& input,
                                           bool ignore_ws = true);

}


namespace Botan {

/**
* BitBucket is a filter which simply discards all inputs
*/
struct BOTAN_DLL BitBucket final : public Filter
   {
   void write(const byte[], size_t) override {}

   std::string name() const override { return "BitBucket"; }
   };

/**
* This class represents Filter chains. A Filter chain is an ordered
* concatenation of Filters, the input to a Chain sequentially passes
* through all the Filters contained in the Chain.
*/

class BOTAN_DLL Chain : public Fanout_Filter
   {
   public:
      void write(const byte input[], size_t length) override { send(input, length); }

      std::string name() const override;

      /**
      * Construct a chain of up to four filters. The filters are set
      * up in the same order as the arguments.
      */
      Chain(Filter* = nullptr, Filter* = nullptr,
            Filter* = nullptr, Filter* = nullptr);

      /**
      * Construct a chain from range of filters
      * @param filter_arr the list of filters
      * @param length how many filters
      */
      Chain(Filter* filter_arr[], size_t length);
   };

/**
* This class represents a fork filter, whose purpose is to fork the
* flow of data. It causes an input message to result in n messages at
* the end of the filter, where n is the number of forks.
*/
class BOTAN_DLL Fork : public Fanout_Filter
   {
   public:
      void write(const byte input[], size_t length) override { send(input, length); }
      void set_port(size_t n) { Fanout_Filter::set_port(n); }

      std::string name() const override;

      /**
      * Construct a Fork filter with up to four forks.
      */
      Fork(Filter*, Filter*, Filter* = nullptr, Filter* = nullptr);

      /**
      * Construct a Fork from range of filters
      * @param filter_arr the list of filters
      * @param length how many filters
      */
      Fork(Filter* filter_arr[], size_t length);
   };

/**
* This class is a threaded version of the Fork filter. While this uses
* threads, the class itself is NOT thread-safe. This is meant as a drop-
* in replacement for Fork where performance gains are possible.
*/
class BOTAN_DLL Threaded_Fork : public Fork
   {
   public:
      std::string name() const override;

      /**
      * Construct a Threaded_Fork filter with up to four forks.
      */
      Threaded_Fork(Filter*, Filter*, Filter* = nullptr, Filter* = nullptr);

      /**
      * Construct a Threaded_Fork from range of filters
      * @param filter_arr the list of filters
      * @param length how many filters
      */
      Threaded_Fork(Filter* filter_arr[], size_t length);

      ~Threaded_Fork();

   protected:
      void set_next(Filter* f[], size_t n);
      void send(const byte in[], size_t length) override;

   private:
      void thread_delegate_work(const byte input[], size_t length);
      void thread_entry(Filter* filter);

      std::vector<std::shared_ptr<std::thread>> m_threads;
      std::unique_ptr<struct Threaded_Fork_Data> m_thread_data;
   };

}


namespace Botan {

/**
* This class represents an abstract data source object.
*/
class BOTAN_DLL DataSource
   {
   public:
      /**
      * Read from the source. Moves the internal offset so that every
      * call to read will return a new portion of the source.
      *
      * @param out the byte array to write the result to
      * @param length the length of the byte array out
      * @return length in bytes that was actually read and put
      * into out
      */
      virtual size_t read(byte out[], size_t length) = 0;

      virtual bool check_available(size_t n) = 0;

      /**
      * Read from the source but do not modify the internal
      * offset. Consecutive calls to peek() will return portions of
      * the source starting at the same position.
      *
      * @param out the byte array to write the output to
      * @param length the length of the byte array out
      * @param peek_offset the offset into the stream to read at
      * @return length in bytes that was actually read and put
      * into out
      */
      virtual size_t peek(byte out[], size_t length,
                          size_t peek_offset) const = 0;

      /**
      * Test whether the source still has data that can be read.
      * @return true if there is still data to read, false otherwise
      */
      virtual bool end_of_data() const = 0;
      /**
      * return the id of this data source
      * @return std::string representing the id of this data source
      */
      virtual std::string id() const { return ""; }

      /**
      * Read one byte.
      * @param out the byte to read to
      * @return length in bytes that was actually read and put
      * into out
      */
      size_t read_byte(byte& out);

      /**
      * Peek at one byte.
      * @param out an output byte
      * @return length in bytes that was actually read and put
      * into out
      */
      size_t peek_byte(byte& out) const;

      /**
      * Discard the next N bytes of the data
      * @param N the number of bytes to discard
      * @return number of bytes actually discarded
      */
      size_t discard_next(size_t N);

      /**
      * @return number of bytes read so far.
      */
      virtual size_t get_bytes_read() const = 0;

      DataSource() {}
      virtual ~DataSource() {}
      DataSource& operator=(const DataSource&) = delete;
      DataSource(const DataSource&) = delete;
   };

/**
* This class represents a Memory-Based DataSource
*/
class BOTAN_DLL DataSource_Memory : public DataSource
   {
   public:
      size_t read(byte[], size_t) override;
      size_t peek(byte[], size_t, size_t) const override;
      bool check_available(size_t n) override;
      bool end_of_data() const override;

      /**
      * Construct a memory source that reads from a string
      * @param in the string to read from
      */
      explicit DataSource_Memory(const std::string& in);

      /**
      * Construct a memory source that reads from a byte array
      * @param in the byte array to read from
      * @param length the length of the byte array
      */
      DataSource_Memory(const byte in[], size_t length) :
         m_source(in, in + length), m_offset(0) {}

      /**
      * Construct a memory source that reads from a secure_vector
      * @param in the MemoryRegion to read from
      */
      explicit DataSource_Memory(const secure_vector<byte>& in) :
         m_source(in), m_offset(0) {}

      /**
      * Construct a memory source that reads from a std::vector
      * @param in the MemoryRegion to read from
      */
      explicit DataSource_Memory(const std::vector<byte>& in) :
         m_source(in.begin(), in.end()), m_offset(0) {}

      size_t get_bytes_read() const override { return m_offset; }
   private:
      secure_vector<byte> m_source;
      size_t m_offset;
   };

/**
* This class represents a Stream-Based DataSource.
*/
class BOTAN_DLL DataSource_Stream : public DataSource
   {
   public:
      size_t read(byte[], size_t) override;
      size_t peek(byte[], size_t, size_t) const override;
      bool check_available(size_t n) override;
      bool end_of_data() const override;
      std::string id() const override;

      DataSource_Stream(std::istream&,
                        const std::string& id = "<std::istream>");

      /**
      * Construct a Stream-Based DataSource from file
      * @param file the name of the file
      * @param use_binary whether to treat the file as binary or not
      */
      DataSource_Stream(const std::string& file, bool use_binary = false);

      DataSource_Stream(const DataSource_Stream&) = delete;

      DataSource_Stream& operator=(const DataSource_Stream&) = delete;

      ~DataSource_Stream();

      size_t get_bytes_read() const override { return m_total_read; }
   private:
      const std::string m_identifier;

      std::istream* m_source_p;
      std::istream& m_source;
      size_t m_total_read;
   };

}


namespace Botan {

/**
* BER Decoding Object
*/
class BOTAN_DLL BER_Decoder
   {
   public:
      BER_Object get_next_object();

      std::vector<byte> get_next_octet_string();

      void push_back(const BER_Object& obj);

      bool more_items() const;
      BER_Decoder& verify_end();
      BER_Decoder& discard_remaining();

      BER_Decoder  start_cons(ASN1_Tag type_tag, ASN1_Tag class_tag = UNIVERSAL);
      BER_Decoder& end_cons();

      BER_Decoder& get_next(BER_Object& ber);

      BER_Decoder& raw_bytes(secure_vector<byte>& v);
      BER_Decoder& raw_bytes(std::vector<byte>& v);

      BER_Decoder& decode_null();
      BER_Decoder& decode(bool& v);
      BER_Decoder& decode(size_t& v);
      BER_Decoder& decode(class BigInt& v);
      BER_Decoder& decode(std::vector<byte>& v, ASN1_Tag type_tag);
      BER_Decoder& decode(secure_vector<byte>& v, ASN1_Tag type_tag);

      BER_Decoder& decode(bool& v,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      BER_Decoder& decode(size_t& v,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      BER_Decoder& decode(class BigInt& v,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      BER_Decoder& decode(std::vector<byte>& v,
                          ASN1_Tag real_type,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      BER_Decoder& decode(secure_vector<byte>& v,
                          ASN1_Tag real_type,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      BER_Decoder& decode(class ASN1_Object& obj,
                          ASN1_Tag type_tag = NO_OBJECT,
                          ASN1_Tag class_tag = NO_OBJECT);

      BER_Decoder& decode_octet_string_bigint(class BigInt& b);

      u64bit decode_constrained_integer(ASN1_Tag type_tag,
                                        ASN1_Tag class_tag,
                                        size_t T_bytes);

      template<typename T> BER_Decoder& decode_integer_type(T& out)
         {
         return decode_integer_type<T>(out, INTEGER, UNIVERSAL);
         }

      template<typename T>
         BER_Decoder& decode_integer_type(T& out,
                                          ASN1_Tag type_tag,
                                          ASN1_Tag class_tag = CONTEXT_SPECIFIC)
         {
         out = static_cast<T>(decode_constrained_integer(type_tag, class_tag, sizeof(out)));
         return (*this);
         }

      template<typename T>
         BER_Decoder& decode_optional(T& out,
                                      ASN1_Tag type_tag,
                                      ASN1_Tag class_tag,
                                      const T& default_value = T());

      template<typename T>
         BER_Decoder& decode_optional_implicit(
            T& out,
            ASN1_Tag type_tag,
            ASN1_Tag class_tag,
            ASN1_Tag real_type,
            ASN1_Tag real_class,
            const T& default_value = T());

      template<typename T>
         BER_Decoder& decode_list(std::vector<T>& out,
                                  ASN1_Tag type_tag = SEQUENCE,
                                  ASN1_Tag class_tag = UNIVERSAL);

      template<typename T>
         BER_Decoder& decode_and_check(const T& expected,
                                       const std::string& error_msg)
         {
         T actual;
         decode(actual);

         if(actual != expected)
            throw Decoding_Error(error_msg);

         return (*this);
         }

      /*
      * Decode an OPTIONAL string type
      */
      template<typename Alloc>
      BER_Decoder& decode_optional_string(std::vector<byte, Alloc>& out,
                                          ASN1_Tag real_type,
                                          u16bit type_no,
                                          ASN1_Tag class_tag = CONTEXT_SPECIFIC)
         {
         BER_Object obj = get_next_object();

         ASN1_Tag type_tag = static_cast<ASN1_Tag>(type_no);

         if(obj.type_tag == type_tag && obj.class_tag == class_tag)
            {
            if((class_tag & CONSTRUCTED) && (class_tag & CONTEXT_SPECIFIC))
               BER_Decoder(obj.value).decode(out, real_type).verify_end();
            else
               {
               push_back(obj);
               decode(out, real_type, type_tag, class_tag);
               }
            }
         else
            {
            out.clear();
            push_back(obj);
            }

         return (*this);
         }

      BER_Decoder& operator=(const BER_Decoder&) = delete;

      explicit BER_Decoder(DataSource&);

      BER_Decoder(const byte[], size_t);

      explicit BER_Decoder(const secure_vector<byte>&);

      explicit BER_Decoder(const std::vector<byte>& vec);

      BER_Decoder(const BER_Decoder&);
      ~BER_Decoder();
   private:
      BER_Decoder* m_parent;
      DataSource* m_source;
      BER_Object m_pushed;
      mutable bool m_owns;
   };

/*
* Decode an OPTIONAL or DEFAULT element
*/
template<typename T>
BER_Decoder& BER_Decoder::decode_optional(T& out,
                                          ASN1_Tag type_tag,
                                          ASN1_Tag class_tag,
                                          const T& default_value)
   {
   BER_Object obj = get_next_object();

   if(obj.type_tag == type_tag && obj.class_tag == class_tag)
      {
      if((class_tag & CONSTRUCTED) && (class_tag & CONTEXT_SPECIFIC))
         BER_Decoder(obj.value).decode(out).verify_end();
      else
         {
         push_back(obj);
         decode(out, type_tag, class_tag);
         }
      }
   else
      {
      out = default_value;
      push_back(obj);
      }

   return (*this);
   }

/*
* Decode an OPTIONAL or DEFAULT element
*/
template<typename T>
BER_Decoder& BER_Decoder::decode_optional_implicit(
   T& out,
   ASN1_Tag type_tag,
   ASN1_Tag class_tag,
   ASN1_Tag real_type,
   ASN1_Tag real_class,
   const T& default_value)
   {
   BER_Object obj = get_next_object();

   if(obj.type_tag == type_tag && obj.class_tag == class_tag)
      {
      obj.type_tag = real_type;
      obj.class_tag = real_class;
      push_back(obj);
      decode(out, real_type, real_class);
      }
   else
      {
      out = default_value;
      push_back(obj);
      }

   return (*this);
   }
/*
* Decode a list of homogenously typed values
*/
template<typename T>
BER_Decoder& BER_Decoder::decode_list(std::vector<T>& vec,
                                      ASN1_Tag type_tag,
                                      ASN1_Tag class_tag)
   {
   BER_Decoder list = start_cons(type_tag, class_tag);

   while(list.more_items())
      {
      T value;
      list.decode(value);
      vec.push_back(value);
      }

   list.end_cons();

   return (*this);
   }

}


namespace Botan {

#if (BOTAN_MP_WORD_BITS == 8)
  typedef byte word;
#elif (BOTAN_MP_WORD_BITS == 16)
  typedef u16bit word;
#elif (BOTAN_MP_WORD_BITS == 32)
  typedef u32bit word;
#elif (BOTAN_MP_WORD_BITS == 64)
  typedef u64bit word;
#else
  #error BOTAN_MP_WORD_BITS must be 8, 16, 32, or 64
#endif

const word MP_WORD_MASK = ~static_cast<word>(0);
const word MP_WORD_TOP_BIT = static_cast<word>(1) << (8*sizeof(word) - 1);
const word MP_WORD_MAX = MP_WORD_MASK;

}


namespace Botan {

/**
* Bit rotation left
* @param input the input word
* @param rot the number of bits to rotate
* @return input rotated left by rot bits
*/
template<typename T> inline T rotate_left(T input, size_t rot)
   {
   if(rot == 0)
      return input;
   return static_cast<T>((input << rot) | (input >> (8*sizeof(T)-rot)));;
   }

/**
* Bit rotation right
* @param input the input word
* @param rot the number of bits to rotate
* @return input rotated right by rot bits
*/
template<typename T> inline T rotate_right(T input, size_t rot)
   {
   if(rot == 0)
      return input;
   return static_cast<T>((input >> rot) | (input << (8*sizeof(T)-rot)));
   }

}


#if defined(BOTAN_TARGET_CPU_HAS_SSE2) && !defined(BOTAN_NO_SSE_INTRINSICS)
  #include <emmintrin.h>
#endif

namespace Botan {

/**
* Swap a 16 bit integer
*/
inline u16bit reverse_bytes(u16bit val)
   {
   return rotate_left(val, 8);
   }

/**
* Swap a 32 bit integer
*/
inline u32bit reverse_bytes(u32bit val)
   {
#if BOTAN_GCC_VERSION >= 430 && !defined(BOTAN_TARGET_ARCH_IS_ARM32)
   /*
   GCC intrinsic added in 4.3, works for a number of CPUs

   However avoid under ARM, as it branches to a function in libgcc
   instead of generating inline asm, so slower even than the generic
   rotate version below.
   */
   return __builtin_bswap32(val);

#elif defined(BOTAN_USE_GCC_INLINE_ASM) && defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)

   // GCC-style inline assembly for x86 or x86-64
   asm("bswapl %0" : "=r" (val) : "0" (val));
   return val;

#elif defined(BOTAN_USE_GCC_INLINE_ASM) && defined(BOTAN_TARGET_ARCH_IS_ARM32)

   asm ("eor r3, %1, %1, ror #16\n\t"
        "bic r3, r3, #0x00FF0000\n\t"
        "mov %0, %1, ror #8\n\t"
        "eor %0, %0, r3, lsr #8"
        : "=r" (val)
        : "0" (val)
        : "r3", "cc");

   return val;

#elif defined(_MSC_VER) && defined(BOTAN_TARGET_ARCH_IS_X86_32)

   // Visual C++ inline asm for 32-bit x86, by Yves Jerschow
   __asm mov eax, val;
   __asm bswap eax;

#else

   // Generic implementation
   return (rotate_right(val, 8) & 0xFF00FF00) |
          (rotate_left (val, 8) & 0x00FF00FF);

#endif
   }

/**
* Swap a 64 bit integer
*/
inline u64bit reverse_bytes(u64bit val)
   {
#if BOTAN_GCC_VERSION >= 430

   // GCC intrinsic added in 4.3, works for a number of CPUs
   return __builtin_bswap64(val);

#elif defined(BOTAN_USE_GCC_INLINE_ASM) && defined(BOTAN_TARGET_ARCH_IS_X86_64)
   // GCC-style inline assembly for x86-64
   asm("bswapq %0" : "=r" (val) : "0" (val));
   return val;

#else
   /* Generic implementation. Defined in terms of 32-bit bswap so any
    * optimizations in that version can help here (particularly
    * useful for 32-bit x86).
    */

   u32bit hi = static_cast<u32bit>(val >> 32);
   u32bit lo = static_cast<u32bit>(val);

   hi = reverse_bytes(hi);
   lo = reverse_bytes(lo);

   return (static_cast<u64bit>(lo) << 32) | hi;
#endif
   }

/**
* Swap 4 Ts in an array
*/
template<typename T>
inline void bswap_4(T x[4])
   {
   x[0] = reverse_bytes(x[0]);
   x[1] = reverse_bytes(x[1]);
   x[2] = reverse_bytes(x[2]);
   x[3] = reverse_bytes(x[3]);
   }

#if defined(BOTAN_TARGET_CPU_HAS_SSE2) && !defined(BOTAN_NO_SSE_INTRINSICS)

/**
* Swap 4 u32bits in an array using SSE2 shuffle instructions
*/
template<>
inline void bswap_4(u32bit x[4])
   {
   __m128i T = _mm_loadu_si128(reinterpret_cast<const __m128i*>(x));

   T = _mm_shufflehi_epi16(T, _MM_SHUFFLE(2, 3, 0, 1));
   T = _mm_shufflelo_epi16(T, _MM_SHUFFLE(2, 3, 0, 1));

   T =  _mm_or_si128(_mm_srli_epi16(T, 8), _mm_slli_epi16(T, 8));

   _mm_storeu_si128(reinterpret_cast<__m128i*>(x), T);
   }

#endif

}


#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK

#if defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)

#define BOTAN_ENDIAN_N2B(x) (x)
#define BOTAN_ENDIAN_B2N(x) (x)

#define BOTAN_ENDIAN_N2L(x) reverse_bytes(x)
#define BOTAN_ENDIAN_L2N(x) reverse_bytes(x)

#elif defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN)

#define BOTAN_ENDIAN_N2L(x) (x)
#define BOTAN_ENDIAN_L2N(x) (x)

#define BOTAN_ENDIAN_N2B(x) reverse_bytes(x)
#define BOTAN_ENDIAN_B2N(x) reverse_bytes(x)

#endif

#endif

namespace Botan {

/**
* Byte extraction
* @param byte_num which byte to extract, 0 == highest byte
* @param input the value to extract from
* @return byte byte_num of input
*/
template<typename T> inline byte get_byte(size_t byte_num, T input)
   {
   return static_cast<byte>(
      input >> (((~byte_num)&(sizeof(T)-1)) << 3)
      );
   }

/**
* Make a u16bit from two bytes
* @param i0 the first byte
* @param i1 the second byte
* @return i0 || i1
*/
inline u16bit make_u16bit(byte i0, byte i1)
   {
   return ((static_cast<u16bit>(i0) << 8) | i1);
   }

/**
* Make a u32bit from four bytes
* @param i0 the first byte
* @param i1 the second byte
* @param i2 the third byte
* @param i3 the fourth byte
* @return i0 || i1 || i2 || i3
*/
inline u32bit make_u32bit(byte i0, byte i1, byte i2, byte i3)
   {
   return ((static_cast<u32bit>(i0) << 24) |
           (static_cast<u32bit>(i1) << 16) |
           (static_cast<u32bit>(i2) <<  8) |
           (static_cast<u32bit>(i3)));
   }

/**
* Make a u32bit from eight bytes
* @param i0 the first byte
* @param i1 the second byte
* @param i2 the third byte
* @param i3 the fourth byte
* @param i4 the fifth byte
* @param i5 the sixth byte
* @param i6 the seventh byte
* @param i7 the eighth byte
* @return i0 || i1 || i2 || i3 || i4 || i5 || i6 || i7
*/
inline u64bit make_u64bit(byte i0, byte i1, byte i2, byte i3,
                          byte i4, byte i5, byte i6, byte i7)
    {
   return ((static_cast<u64bit>(i0) << 56) |
           (static_cast<u64bit>(i1) << 48) |
           (static_cast<u64bit>(i2) << 40) |
           (static_cast<u64bit>(i3) << 32) |
           (static_cast<u64bit>(i4) << 24) |
           (static_cast<u64bit>(i5) << 16) |
           (static_cast<u64bit>(i6) <<  8) |
           (static_cast<u64bit>(i7)));
    }

/**
* Load a big-endian word
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th T of in, as a big-endian value
*/
template<typename T>
inline T load_be(const byte in[], size_t off)
   {
   in += off * sizeof(T);
   T out = 0;
   for(size_t i = 0; i != sizeof(T); ++i)
      out = (out << 8) | in[i];
   return out;
   }

/**
* Load a little-endian word
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th T of in, as a litte-endian value
*/
template<typename T>
inline T load_le(const byte in[], size_t off)
   {
   in += off * sizeof(T);
   T out = 0;
   for(size_t i = 0; i != sizeof(T); ++i)
      out = (out << 8) | in[sizeof(T)-1-i];
   return out;
   }

/**
* Load a big-endian u16bit
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th u16bit of in, as a big-endian value
*/
template<>
inline u16bit load_be<u16bit>(const byte in[], size_t off)
   {
   in += off * sizeof(u16bit);

#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   u16bit x;
   std::memcpy(&x, in, sizeof(x));
   return BOTAN_ENDIAN_N2B(x);
#else
   return make_u16bit(in[0], in[1]);
#endif
   }

/**
* Load a little-endian u16bit
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th u16bit of in, as a little-endian value
*/
template<>
inline u16bit load_le<u16bit>(const byte in[], size_t off)
   {
   in += off * sizeof(u16bit);

#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   u16bit x;
   std::memcpy(&x, in, sizeof(x));
   return BOTAN_ENDIAN_N2L(x);
#else
   return make_u16bit(in[1], in[0]);
#endif
   }

/**
* Load a big-endian u32bit
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th u32bit of in, as a big-endian value
*/
template<>
inline u32bit load_be<u32bit>(const byte in[], size_t off)
   {
   in += off * sizeof(u32bit);
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   u32bit x;
   std::memcpy(&x, in, sizeof(x));
   return BOTAN_ENDIAN_N2B(x);
#else
   return make_u32bit(in[0], in[1], in[2], in[3]);
#endif
   }

/**
* Load a little-endian u32bit
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th u32bit of in, as a little-endian value
*/
template<>
inline u32bit load_le<u32bit>(const byte in[], size_t off)
   {
   in += off * sizeof(u32bit);
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   u32bit x;
   std::memcpy(&x, in, sizeof(x));
   return BOTAN_ENDIAN_N2L(x);
#else
   return make_u32bit(in[3], in[2], in[1], in[0]);
#endif
   }

/**
* Load a big-endian u64bit
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th u64bit of in, as a big-endian value
*/
template<>
inline u64bit load_be<u64bit>(const byte in[], size_t off)
   {
   in += off * sizeof(u64bit);
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   u64bit x;
   std::memcpy(&x, in, sizeof(x));
   return BOTAN_ENDIAN_N2B(x);
#else
   return make_u64bit(in[0], in[1], in[2], in[3],
                      in[4], in[5], in[6], in[7]);
#endif
   }

/**
* Load a little-endian u64bit
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th u64bit of in, as a little-endian value
*/
template<>
inline u64bit load_le<u64bit>(const byte in[], size_t off)
   {
   in += off * sizeof(u64bit);
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   u64bit x;
   std::memcpy(&x, in, sizeof(x));
   return BOTAN_ENDIAN_N2L(x);
#else
   return make_u64bit(in[7], in[6], in[5], in[4],
                      in[3], in[2], in[1], in[0]);
#endif
   }

/**
* Load two little-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
*/
template<typename T>
inline void load_le(const byte in[], T& x0, T& x1)
   {
   x0 = load_le<T>(in, 0);
   x1 = load_le<T>(in, 1);
   }

/**
* Load four little-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
* @param x2 where the third word will be written
* @param x3 where the fourth word will be written
*/
template<typename T>
inline void load_le(const byte in[],
                    T& x0, T& x1, T& x2, T& x3)
   {
   x0 = load_le<T>(in, 0);
   x1 = load_le<T>(in, 1);
   x2 = load_le<T>(in, 2);
   x3 = load_le<T>(in, 3);
   }

/**
* Load eight little-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
* @param x2 where the third word will be written
* @param x3 where the fourth word will be written
* @param x4 where the fifth word will be written
* @param x5 where the sixth word will be written
* @param x6 where the seventh word will be written
* @param x7 where the eighth word will be written
*/
template<typename T>
inline void load_le(const byte in[],
                    T& x0, T& x1, T& x2, T& x3,
                    T& x4, T& x5, T& x6, T& x7)
   {
   x0 = load_le<T>(in, 0);
   x1 = load_le<T>(in, 1);
   x2 = load_le<T>(in, 2);
   x3 = load_le<T>(in, 3);
   x4 = load_le<T>(in, 4);
   x5 = load_le<T>(in, 5);
   x6 = load_le<T>(in, 6);
   x7 = load_le<T>(in, 7);
   }

/**
* Load a variable number of little-endian words
* @param out the output array of words
* @param in the input array of bytes
* @param count how many words are in in
*/
template<typename T>
inline void load_le(T out[],
                    const byte in[],
                    size_t count)
   {
   if(count > 0)
      {
#if defined(BOTAN_TARGET_CPU_HAS_KNOWN_ENDIANNESS)
      std::memcpy(out, in, sizeof(T)*count);

#if defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
      const size_t blocks = count - (count % 4);
      const size_t left = count - blocks;

      for(size_t i = 0; i != blocks; i += 4)
         bswap_4(out + i);

      for(size_t i = 0; i != left; ++i)
         out[blocks+i] = reverse_bytes(out[blocks+i]);
#endif

#else
      for(size_t i = 0; i != count; ++i)
         out[i] = load_le<T>(in, i);
#endif
      }
   }

/**
* Load two big-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
*/
template<typename T>
inline void load_be(const byte in[], T& x0, T& x1)
   {
   x0 = load_be<T>(in, 0);
   x1 = load_be<T>(in, 1);
   }

/**
* Load four big-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
* @param x2 where the third word will be written
* @param x3 where the fourth word will be written
*/
template<typename T>
inline void load_be(const byte in[],
                    T& x0, T& x1, T& x2, T& x3)
   {
   x0 = load_be<T>(in, 0);
   x1 = load_be<T>(in, 1);
   x2 = load_be<T>(in, 2);
   x3 = load_be<T>(in, 3);
   }

/**
* Load eight big-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
* @param x2 where the third word will be written
* @param x3 where the fourth word will be written
* @param x4 where the fifth word will be written
* @param x5 where the sixth word will be written
* @param x6 where the seventh word will be written
* @param x7 where the eighth word will be written
*/
template<typename T>
inline void load_be(const byte in[],
                    T& x0, T& x1, T& x2, T& x3,
                    T& x4, T& x5, T& x6, T& x7)
   {
   x0 = load_be<T>(in, 0);
   x1 = load_be<T>(in, 1);
   x2 = load_be<T>(in, 2);
   x3 = load_be<T>(in, 3);
   x4 = load_be<T>(in, 4);
   x5 = load_be<T>(in, 5);
   x6 = load_be<T>(in, 6);
   x7 = load_be<T>(in, 7);
   }

/**
* Load a variable number of big-endian words
* @param out the output array of words
* @param in the input array of bytes
* @param count how many words are in in
*/
template<typename T>
inline void load_be(T out[],
                    const byte in[],
                    size_t count)
   {
   if(count > 0)
      {
#if defined(BOTAN_TARGET_CPU_HAS_KNOWN_ENDIANNESS)
      std::memcpy(out, in, sizeof(T)*count);

#if defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN)
      const size_t blocks = count - (count % 4);
      const size_t left = count - blocks;

      for(size_t i = 0; i != blocks; i += 4)
         bswap_4(out + i);

      for(size_t i = 0; i != left; ++i)
         out[blocks+i] = reverse_bytes(out[blocks+i]);
#endif

#else
      for(size_t i = 0; i != count; ++i)
         out[i] = load_be<T>(in, i);
#endif
      }
   }

/**
* Store a big-endian u16bit
* @param in the input u16bit
* @param out the byte array to write to
*/
inline void store_be(u16bit in, byte out[2])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   u16bit o = BOTAN_ENDIAN_N2B(in);
   std::memcpy(out, &o, sizeof(o));
#else
   out[0] = get_byte(0, in);
   out[1] = get_byte(1, in);
#endif
   }

/**
* Store a little-endian u16bit
* @param in the input u16bit
* @param out the byte array to write to
*/
inline void store_le(u16bit in, byte out[2])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   u16bit o = BOTAN_ENDIAN_N2L(in);
   std::memcpy(out, &o, sizeof(o));
#else
   out[0] = get_byte(1, in);
   out[1] = get_byte(0, in);
#endif
   }

/**
* Store a big-endian u32bit
* @param in the input u32bit
* @param out the byte array to write to
*/
inline void store_be(u32bit in, byte out[4])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   u32bit o = BOTAN_ENDIAN_B2N(in);
   std::memcpy(out, &o, sizeof(o));
#else
   out[0] = get_byte(0, in);
   out[1] = get_byte(1, in);
   out[2] = get_byte(2, in);
   out[3] = get_byte(3, in);
#endif
   }

/**
* Store a little-endian u32bit
* @param in the input u32bit
* @param out the byte array to write to
*/
inline void store_le(u32bit in, byte out[4])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   u32bit o = BOTAN_ENDIAN_L2N(in);
   std::memcpy(out, &o, sizeof(o));
#else
   out[0] = get_byte(3, in);
   out[1] = get_byte(2, in);
   out[2] = get_byte(1, in);
   out[3] = get_byte(0, in);
#endif
   }

/**
* Store a big-endian u64bit
* @param in the input u64bit
* @param out the byte array to write to
*/
inline void store_be(u64bit in, byte out[8])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   u64bit o = BOTAN_ENDIAN_B2N(in);
   std::memcpy(out, &o, sizeof(o));
#else
   out[0] = get_byte(0, in);
   out[1] = get_byte(1, in);
   out[2] = get_byte(2, in);
   out[3] = get_byte(3, in);
   out[4] = get_byte(4, in);
   out[5] = get_byte(5, in);
   out[6] = get_byte(6, in);
   out[7] = get_byte(7, in);
#endif
   }

/**
* Store a little-endian u64bit
* @param in the input u64bit
* @param out the byte array to write to
*/
inline void store_le(u64bit in, byte out[8])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   u64bit o = BOTAN_ENDIAN_L2N(in);
   std::memcpy(out, &o, sizeof(o));
#else
   out[0] = get_byte(7, in);
   out[1] = get_byte(6, in);
   out[2] = get_byte(5, in);
   out[3] = get_byte(4, in);
   out[4] = get_byte(3, in);
   out[5] = get_byte(2, in);
   out[6] = get_byte(1, in);
   out[7] = get_byte(0, in);
#endif
   }

/**
* Store two little-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
*/
template<typename T>
inline void store_le(byte out[], T x0, T x1)
   {
   store_le(x0, out + (0 * sizeof(T)));
   store_le(x1, out + (1 * sizeof(T)));
   }

/**
* Store two big-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
*/
template<typename T>
inline void store_be(byte out[], T x0, T x1)
   {
   store_be(x0, out + (0 * sizeof(T)));
   store_be(x1, out + (1 * sizeof(T)));
   }

/**
* Store four little-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
* @param x2 the third word
* @param x3 the fourth word
*/
template<typename T>
inline void store_le(byte out[], T x0, T x1, T x2, T x3)
   {
   store_le(x0, out + (0 * sizeof(T)));
   store_le(x1, out + (1 * sizeof(T)));
   store_le(x2, out + (2 * sizeof(T)));
   store_le(x3, out + (3 * sizeof(T)));
   }

/**
* Store four big-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
* @param x2 the third word
* @param x3 the fourth word
*/
template<typename T>
inline void store_be(byte out[], T x0, T x1, T x2, T x3)
   {
   store_be(x0, out + (0 * sizeof(T)));
   store_be(x1, out + (1 * sizeof(T)));
   store_be(x2, out + (2 * sizeof(T)));
   store_be(x3, out + (3 * sizeof(T)));
   }

/**
* Store eight little-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
* @param x2 the third word
* @param x3 the fourth word
* @param x4 the fifth word
* @param x5 the sixth word
* @param x6 the seventh word
* @param x7 the eighth word
*/
template<typename T>
inline void store_le(byte out[], T x0, T x1, T x2, T x3,
                                 T x4, T x5, T x6, T x7)
   {
   store_le(x0, out + (0 * sizeof(T)));
   store_le(x1, out + (1 * sizeof(T)));
   store_le(x2, out + (2 * sizeof(T)));
   store_le(x3, out + (3 * sizeof(T)));
   store_le(x4, out + (4 * sizeof(T)));
   store_le(x5, out + (5 * sizeof(T)));
   store_le(x6, out + (6 * sizeof(T)));
   store_le(x7, out + (7 * sizeof(T)));
   }

/**
* Store eight big-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
* @param x2 the third word
* @param x3 the fourth word
* @param x4 the fifth word
* @param x5 the sixth word
* @param x6 the seventh word
* @param x7 the eighth word
*/
template<typename T>
inline void store_be(byte out[], T x0, T x1, T x2, T x3,
                                 T x4, T x5, T x6, T x7)
   {
   store_be(x0, out + (0 * sizeof(T)));
   store_be(x1, out + (1 * sizeof(T)));
   store_be(x2, out + (2 * sizeof(T)));
   store_be(x3, out + (3 * sizeof(T)));
   store_be(x4, out + (4 * sizeof(T)));
   store_be(x5, out + (5 * sizeof(T)));
   store_be(x6, out + (6 * sizeof(T)));
   store_be(x7, out + (7 * sizeof(T)));
   }

template<typename T>
void copy_out_be(byte out[], size_t out_bytes, const T in[])
   {
   while(out_bytes >= sizeof(T))
      {
      store_be(in[0], out);
      out += sizeof(T);
      out_bytes -= sizeof(T);
      in += 1;
   }

   for(size_t i = 0; i != out_bytes; ++i)
      out[i] = get_byte(i%8, in[0]);
   }

template<typename T, typename Alloc>
void copy_out_vec_be(byte out[], size_t out_bytes, const std::vector<T, Alloc>& in)
   {
   copy_out_be(out, out_bytes, in.data());
   }

template<typename T>
void copy_out_le(byte out[], size_t out_bytes, const T in[])
   {
   while(out_bytes >= sizeof(T))
      {
      store_le(in[0], out);
      out += sizeof(T);
      out_bytes -= sizeof(T);
      in += 1;
   }

   for(size_t i = 0; i != out_bytes; ++i)
      out[i] = get_byte(sizeof(T) - 1 - (i % 8), in[0]);
   }

template<typename T, typename Alloc>
void copy_out_vec_le(byte out[], size_t out_bytes, const std::vector<T, Alloc>& in)
   {
   copy_out_le(out, out_bytes, in.data());
   }

}


namespace Botan {

/**
* Arbitrary precision integer
*/
class BOTAN_DLL BigInt
   {
   public:
     /**
     * Base enumerator for encoding and decoding
     */
     enum Base { Decimal = 10, Hexadecimal = 16, Binary = 256 };

     /**
     * Sign symbol definitions for positive and negative numbers
     */
     enum Sign { Negative = 0, Positive = 1 };

     /**
     * DivideByZero Exception
     */
     struct BOTAN_DLL DivideByZero : public Exception
        { DivideByZero() : Exception("BigInt divide by zero") {} };

     /**
     * Create empty BigInt
     */
     BigInt() {}

     /**
     * Create BigInt from 64 bit integer
     * @param n initial value of this BigInt
     */
     BigInt(u64bit n);

     /**
     * Copy Constructor
     * @param other the BigInt to copy
     */
     BigInt(const BigInt& other);

     /**
     * Create BigInt from a string. If the string starts with 0x the
     * rest of the string will be interpreted as hexadecimal digits.
     * Otherwise, it will be interpreted as a decimal number.
     *
     * @param str the string to parse for an integer value
     */
     BigInt(const std::string& str);

     /**
     * Create a BigInt from an integer in a byte array
     * @param buf the byte array holding the value
     * @param length size of buf
     * @param base is the number base of the integer in buf
     */
     BigInt(const byte buf[], size_t length, Base base = Binary);

     /**
     * \brief Create a random BigInt of the specified size
     *
     * @param rng random number generator
     * @param bits size in bits
     * @param set_high_bit if true, the highest bit is always set
     *
     * @see randomize
     */
     BigInt(RandomNumberGenerator& rng, size_t bits, bool set_high_bit = true);

     /**
     * Create BigInt of specified size, all zeros
     * @param sign the sign
     * @param n size of the internal register in words
     */
     BigInt(Sign sign, size_t n);

     /**
     * Move constructor
     */
     BigInt(BigInt&& other)
        {
        this->swap(other);
        }

     /**
     * Move assignment
     */
     BigInt& operator=(BigInt&& other)
        {
        if(this != &other)
           this->swap(other);

        return (*this);
        }

     /**
     * Copy assignment
     */
     BigInt& operator=(const BigInt&) = default;

     /**
     * Swap this value with another
     * @param other BigInt to swap values with
     */
     void swap(BigInt& other)
        {
        m_reg.swap(other.m_reg);
        std::swap(m_signedness, other.m_signedness);
        }

     void swap_reg(secure_vector<word>& reg)
        {
        m_reg.swap(reg);
        }

     /**
     * += operator
     * @param y the BigInt to add to this
     */
     BigInt& operator+=(const BigInt& y);

     /**
     * -= operator
     * @param y the BigInt to subtract from this
     */
     BigInt& operator-=(const BigInt& y);

     /**
     * *= operator
     * @param y the BigInt to multiply with this
     */
     BigInt& operator*=(const BigInt& y);

     /**
     * /= operator
     * @param y the BigInt to divide this by
     */
     BigInt& operator/=(const BigInt& y);

     /**
     * Modulo operator
     * @param y the modulus to reduce this by
     */
     BigInt& operator%=(const BigInt& y);

     /**
     * Modulo operator
     * @param y the modulus (word) to reduce this by
     */
     word    operator%=(word y);

     /**
     * Left shift operator
     * @param shift the number of bits to shift this left by
     */
     BigInt& operator<<=(size_t shift);

     /**
     * Right shift operator
     * @param shift the number of bits to shift this right by
     */
     BigInt& operator>>=(size_t shift);

     /**
     * Increment operator
     */
     BigInt& operator++() { return (*this += 1); }

     /**
     * Decrement operator
     */
     BigInt& operator--() { return (*this -= 1); }

     /**
     * Postfix increment operator
     */
     BigInt  operator++(int) { BigInt x = (*this); ++(*this); return x; }

     /**
     * Postfix decrement operator
     */
     BigInt  operator--(int) { BigInt x = (*this); --(*this); return x; }

     /**
     * Unary negation operator
     * @return negative this
     */
     BigInt operator-() const;

     /**
     * ! operator
     * @return true iff this is zero, otherwise false
     */
     bool operator !() const { return (!is_nonzero()); }

     /**
     * Zeroize the BigInt. The size of the underlying register is not
     * modified.
     */
     void clear() { zeroise(m_reg); }

     /**
     * Compare this to another BigInt
     * @param n the BigInt value to compare with
     * @param check_signs include sign in comparison?
     * @result if (this<n) return -1, if (this>n) return 1, if both
     * values are identical return 0 [like Perl's <=> operator]
     */
     s32bit cmp(const BigInt& n, bool check_signs = true) const;

     /**
     * Test if the integer has an even value
     * @result true if the integer is even, false otherwise
     */
     bool is_even() const { return (get_bit(0) == 0); }

     /**
     * Test if the integer has an odd value
     * @result true if the integer is odd, false otherwise
     */
     bool is_odd()  const { return (get_bit(0) == 1); }

     /**
     * Test if the integer is not zero
     * @result true if the integer is non-zero, false otherwise
     */
     bool is_nonzero() const { return (!is_zero()); }

     /**
     * Test if the integer is zero
     * @result true if the integer is zero, false otherwise
     */
     bool is_zero() const
        {
        const size_t sw = sig_words();

        for(size_t i = 0; i != sw; ++i)
           if(m_reg[i])
              return false;
        return true;
        }

     /**
     * Set bit at specified position
     * @param n bit position to set
     */
     void set_bit(size_t n);

     /**
     * Clear bit at specified position
     * @param n bit position to clear
     */
     void clear_bit(size_t n);

     /**
     * Clear all but the lowest n bits
     * @param n amount of bits to keep
     */
     void mask_bits(size_t n)
        {
        if(n == 0) { clear(); return; }

        const size_t top_word = n / BOTAN_MP_WORD_BITS;
        const word mask = (static_cast<word>(1) << (n % BOTAN_MP_WORD_BITS)) - 1;

        if(top_word < size())
           {
           const size_t len = size() - (top_word + 1);
           if (len > 0)
              {
              clear_mem(&m_reg[top_word+1], len);
              }
           m_reg[top_word] &= mask;
           }
        }

     /**
     * Return bit value at specified position
     * @param n the bit offset to test
     * @result true, if the bit at position n is set, false otherwise
     */
     bool get_bit(size_t n) const
        {
        return ((word_at(n / BOTAN_MP_WORD_BITS) >> (n % BOTAN_MP_WORD_BITS)) & 1);
        }

     /**
     * Return (a maximum of) 32 bits of the complete value
     * @param offset the offset to start extracting
     * @param length amount of bits to extract (starting at offset)
     * @result the integer extracted from the register starting at
     * offset with specified length
     */
     u32bit get_substring(size_t offset, size_t length) const;

     /**
     * Convert this value into a u32bit, if it is in the range
     * [0 ... 2**32-1], or otherwise throw an exception.
     * @result the value as a u32bit if conversion is possible
     */
     u32bit to_u32bit() const;

     /**
     * @param n the offset to get a byte from
     * @result byte at offset n
     */
     byte byte_at(size_t n) const
        {
        return get_byte(sizeof(word) - (n % sizeof(word)) - 1,
                        word_at(n / sizeof(word)));
        }

     /**
     * Return the word at a specified position of the internal register
     * @param n position in the register
     * @return value at position n
     */
     word word_at(size_t n) const
        { return ((n < size()) ? m_reg[n] : 0); }

     void set_word_at(size_t i, word w)
        {
        grow_to(i + 1);
        m_reg[i] = w;
        }

     /**
     * Tests if the sign of the integer is negative
     * @result true, iff the integer has a negative sign
     */
     bool is_negative() const { return (sign() == Negative); }

     /**
     * Tests if the sign of the integer is positive
     * @result true, iff the integer has a positive sign
     */
     bool is_positive() const { return (sign() == Positive); }

     /**
     * Return the sign of the integer
     * @result the sign of the integer
     */
     Sign sign() const { return (m_signedness); }

     /**
     * @result the opposite sign of the represented integer value
     */
     Sign reverse_sign() const;

     /**
     * Flip the sign of this BigInt
     */
     void flip_sign();

     /**
     * Set sign of the integer
     * @param sign new Sign to set
     */
     void set_sign(Sign sign);

     /**
     * @result absolute (positive) value of this
     */
     BigInt abs() const;

     /**
     * Give size of internal register
     * @result size of internal register in words
     */
     size_t size() const { return m_reg.size(); }

     /**
     * Return how many words we need to hold this value
     * @result significant words of the represented integer value
     */
     size_t sig_words() const
        {
        const word* x = m_reg.data();
        size_t sig = m_reg.size();

        while(sig && (x[sig-1] == 0))
           sig--;
        return sig;
        }

     /**
     * Give byte length of the integer
     * @result byte length of the represented integer value
     */
     size_t bytes() const;

     /**
     * Get the bit length of the integer
     * @result bit length of the represented integer value
     */
     size_t bits() const;

     /**
     * Return a mutable pointer to the register
     * @result a pointer to the start of the internal register
     */
     word* mutable_data() { return m_reg.data(); }

     /**
     * Return a const pointer to the register
     * @result a pointer to the start of the internal register
     */
     const word* data() const { return m_reg.data(); }

     secure_vector<word>& get_word_vector() { return m_reg; }
     const secure_vector<word>& get_word_vector() const { return m_reg; }

     /**
     * Increase internal register buffer to at least n words
     * @param n new size of register
     */
     void grow_to(size_t n);

     /**
     * Fill BigInt with a random number with size of bitsize
     *
     * If \p set_high_bit is true, the highest bit will be set, which causes
     * the entropy to be \a bits-1. Otherwise the highest bit is randomly chosen
     * by the rng, causing the entropy to be \a bits.
     *
     * @param rng the random number generator to use
     * @param bitsize number of bits the created random value should have
     * @param set_high_bit if true, the highest bit is always set
     */
     void randomize(RandomNumberGenerator& rng, size_t bitsize, bool set_high_bit = true);

     /**
     * Store BigInt-value in a given byte array
     * @param buf destination byte array for the integer value
     */
     void binary_encode(byte buf[]) const;

     /**
     * Read integer value from a byte array with given size
     * @param buf byte array buffer containing the integer
     * @param length size of buf
     */
     void binary_decode(const byte buf[], size_t length);

     /**
     * Read integer value from a byte array (secure_vector<byte>)
     * @param buf the array to load from
     */
     void binary_decode(const secure_vector<byte>& buf)
        {
        binary_decode(buf.data(), buf.size());
        }

     /**
     * @param base the base to measure the size for
     * @return size of this integer in base base
     */
     size_t encoded_size(Base base = Binary) const;

     /**
     * @param rng a random number generator
     * @param min the minimum value
     * @param max the maximum value
     * @return random integer in [min,max)
     */
     static BigInt random_integer(RandomNumberGenerator& rng,
                                  const BigInt& min,
                                  const BigInt& max);

     /**
     * Create a power of two
     * @param n the power of two to create
     * @return bigint representing 2^n
     */
     static BigInt power_of_2(size_t n)
        {
        BigInt b;
        b.set_bit(n);
        return b;
        }

     /**
     * Encode the integer value from a BigInt to a std::vector of bytes
     * @param n the BigInt to use as integer source
     * @param base number-base of resulting byte array representation
     * @result secure_vector of bytes containing the integer with given base
     */
     static std::vector<byte> encode(const BigInt& n, Base base = Binary);

     /**
     * Encode the integer value from a BigInt to a secure_vector of bytes
     * @param n the BigInt to use as integer source
     * @param base number-base of resulting byte array representation
     * @result secure_vector of bytes containing the integer with given base
     */
     static secure_vector<byte> encode_locked(const BigInt& n,
                                              Base base = Binary);

     /**
     * Encode the integer value from a BigInt to a byte array
     * @param buf destination byte array for the encoded integer
     * value with given base
     * @param n the BigInt to use as integer source
     * @param base number-base of resulting byte array representation
     */
     static void encode(byte buf[], const BigInt& n, Base base = Binary);

     /**
     * Create a BigInt from an integer in a byte array
     * @param buf the binary value to load
     * @param length size of buf
     * @param base number-base of the integer in buf
     * @result BigInt representing the integer in the byte array
     */
     static BigInt decode(const byte buf[], size_t length,
                          Base base = Binary);

     /**
     * Create a BigInt from an integer in a byte array
     * @param buf the binary value to load
     * @param base number-base of the integer in buf
     * @result BigInt representing the integer in the byte array
     */
     static BigInt decode(const secure_vector<byte>& buf,
                          Base base = Binary)
        {
        return BigInt::decode(buf.data(), buf.size(), base);
        }

     /**
     * Create a BigInt from an integer in a byte array
     * @param buf the binary value to load
     * @param base number-base of the integer in buf
     * @result BigInt representing the integer in the byte array
     */
     static BigInt decode(const std::vector<byte>& buf,
                          Base base = Binary)
        {
        return BigInt::decode(buf.data(), buf.size(), base);
        }

     /**
     * Encode a BigInt to a byte array according to IEEE 1363
     * @param n the BigInt to encode
     * @param bytes the length of the resulting secure_vector<byte>
     * @result a secure_vector<byte> containing the encoded BigInt
     */
     static secure_vector<byte> encode_1363(const BigInt& n, size_t bytes);

     static void encode_1363(byte out[], size_t bytes, const BigInt& n);

   private:
      secure_vector<word> m_reg;
      Sign m_signedness = Positive;
   };

/*
* Arithmetic Operators
*/
BigInt BOTAN_DLL operator+(const BigInt& x, const BigInt& y);
BigInt BOTAN_DLL operator-(const BigInt& x, const BigInt& y);
BigInt BOTAN_DLL operator*(const BigInt& x, const BigInt& y);
BigInt BOTAN_DLL operator/(const BigInt& x, const BigInt& d);
BigInt BOTAN_DLL operator%(const BigInt& x, const BigInt& m);
word   BOTAN_DLL operator%(const BigInt& x, word m);
BigInt BOTAN_DLL operator<<(const BigInt& x, size_t n);
BigInt BOTAN_DLL operator>>(const BigInt& x, size_t n);

/*
* Comparison Operators
*/
inline bool operator==(const BigInt& a, const BigInt& b)
   { return (a.cmp(b) == 0); }
inline bool operator!=(const BigInt& a, const BigInt& b)
   { return (a.cmp(b) != 0); }
inline bool operator<=(const BigInt& a, const BigInt& b)
   { return (a.cmp(b) <= 0); }
inline bool operator>=(const BigInt& a, const BigInt& b)
   { return (a.cmp(b) >= 0); }
inline bool operator<(const BigInt& a, const BigInt& b)
   { return (a.cmp(b) < 0); }
inline bool operator>(const BigInt& a, const BigInt& b)
   { return (a.cmp(b) > 0); }

/*
* I/O Operators
*/
BOTAN_DLL std::ostream& operator<<(std::ostream&, const BigInt&);
BOTAN_DLL std::istream& operator>>(std::istream&, BigInt&);

}

namespace std {

template<>
inline void swap<Botan::BigInt>(Botan::BigInt& x, Botan::BigInt& y)
   {
   x.swap(y);
   }

}


namespace Botan {

/**
* Modular Exponentiator Interface
*/
class BOTAN_DLL Modular_Exponentiator
   {
   public:
      virtual void set_base(const BigInt&) = 0;
      virtual void set_exponent(const BigInt&) = 0;
      virtual BigInt execute() const = 0;
      virtual Modular_Exponentiator* copy() const = 0;
      virtual ~Modular_Exponentiator() {}
   };

/**
* Modular Exponentiator Proxy
*/
class BOTAN_DLL Power_Mod
   {
   public:

      enum Usage_Hints {
         NO_HINTS        = 0x0000,

         BASE_IS_FIXED   = 0x0001,
         BASE_IS_SMALL   = 0x0002,
         BASE_IS_LARGE   = 0x0004,
         BASE_IS_2       = 0x0008,

         EXP_IS_FIXED    = 0x0100,
         EXP_IS_SMALL    = 0x0200,
         EXP_IS_LARGE    = 0x0400
      };

      /*
      * Try to choose a good window size
      */
      static size_t window_bits(size_t exp_bits, size_t base_bits,
                                Power_Mod::Usage_Hints hints);

      void set_modulus(const BigInt&, Usage_Hints = NO_HINTS) const;
      void set_base(const BigInt&) const;
      void set_exponent(const BigInt&) const;

      BigInt execute() const;

      Power_Mod& operator=(const Power_Mod&);

      Power_Mod(const BigInt& = 0, Usage_Hints = NO_HINTS);
      Power_Mod(const Power_Mod&);
      virtual ~Power_Mod();
   private:
      mutable Modular_Exponentiator* m_core;
   };

/**
* Fixed Exponent Modular Exponentiator Proxy
*/
class BOTAN_DLL Fixed_Exponent_Power_Mod : public Power_Mod
   {
   public:
      BigInt operator()(const BigInt& b) const
         { set_base(b); return execute(); }

      Fixed_Exponent_Power_Mod() {}

      Fixed_Exponent_Power_Mod(const BigInt& exponent,
                               const BigInt& modulus,
                               Usage_Hints hints = NO_HINTS);
   };

/**
* Fixed Base Modular Exponentiator Proxy
*/
class BOTAN_DLL Fixed_Base_Power_Mod : public Power_Mod
   {
   public:
      BigInt operator()(const BigInt& e) const
         { set_exponent(e); return execute(); }

      Fixed_Base_Power_Mod() {}

      Fixed_Base_Power_Mod(const BigInt& base,
                           const BigInt& modulus,
                           Usage_Hints hints = NO_HINTS);
   };

}


namespace Botan {

/**
* Fused multiply-add
* @param a an integer
* @param b an integer
* @param c an integer
* @return (a*b)+c
*/
BigInt BOTAN_DLL mul_add(const BigInt& a,
                         const BigInt& b,
                         const BigInt& c);

/**
* Fused subtract-multiply
* @param a an integer
* @param b an integer
* @param c an integer
* @return (a-b)*c
*/
BigInt BOTAN_DLL sub_mul(const BigInt& a,
                         const BigInt& b,
                         const BigInt& c);

/**
* Fused multiply-subtract
* @param a an integer
* @param b an integer
* @param c an integer
* @return (a*b)-c
*/
BigInt BOTAN_DLL mul_sub(const BigInt& a,
                         const BigInt& b,
                         const BigInt& c);

/**
* Return the absolute value
* @param n an integer
* @return absolute value of n
*/
inline BigInt abs(const BigInt& n) { return n.abs(); }

/**
* Compute the greatest common divisor
* @param x a positive integer
* @param y a positive integer
* @return gcd(x,y)
*/
BigInt BOTAN_DLL gcd(const BigInt& x, const BigInt& y);

/**
* Least common multiple
* @param x a positive integer
* @param y a positive integer
* @return z, smallest integer such that z % x == 0 and z % y == 0
*/
BigInt BOTAN_DLL lcm(const BigInt& x, const BigInt& y);

/**
* @param x an integer
* @return (x*x)
*/
BigInt BOTAN_DLL square(const BigInt& x);

/**
* Modular inversion
* @param x a positive integer
* @param modulus a positive integer
* @return y st (x*y) % modulus == 1 or 0 if no such value
* Not const time
*/
BigInt BOTAN_DLL inverse_mod(const BigInt& x,
                             const BigInt& modulus);

/**
* Const time modular inversion
* Requires the modulus be odd
*/
BigInt BOTAN_DLL ct_inverse_mod_odd_modulus(const BigInt& n, const BigInt& mod);

/**
* Return a^-1 * 2^k mod b
* Returns k, between n and 2n
* Not const time
*/
size_t BOTAN_DLL almost_montgomery_inverse(BigInt& result,
                                           const BigInt& a,
                                           const BigInt& b);

/**
* Call almost_montgomery_inverse and correct the result to a^-1 mod b
*/
BigInt BOTAN_DLL normalized_montgomery_inverse(const BigInt& a, const BigInt& b);


/**
* Compute the Jacobi symbol. If n is prime, this is equivalent
* to the Legendre symbol.
* @see http://mathworld.wolfram.com/JacobiSymbol.html
*
* @param a is a non-negative integer
* @param n is an odd integer > 1
* @return (n / m)
*/
s32bit BOTAN_DLL jacobi(const BigInt& a,
                        const BigInt& n);

/**
* Modular exponentation
* @param b an integer base
* @param x a positive exponent
* @param m a positive modulus
* @return (b^x) % m
*/
BigInt BOTAN_DLL power_mod(const BigInt& b,
                           const BigInt& x,
                           const BigInt& m);

/**
* Compute the square root of x modulo a prime using the
* Shanks-Tonnelli algorithm
*
* @param x the input
* @param p the prime
* @return y such that (y*y)%p == x, or -1 if no such integer
*/
BigInt BOTAN_DLL ressol(const BigInt& x, const BigInt& p);

/*
* Compute -input^-1 mod 2^MP_WORD_BITS. Returns zero if input
* is even. If input is odd, input and 2^n are relatively prime
* and an inverse exists.
*/
word BOTAN_DLL monty_inverse(word input);

/**
* @param x a positive integer
* @return count of the zero bits in x, or, equivalently, the largest
*         value of n such that 2^n divides x evenly. Returns zero if
*         n is less than or equal to zero.
*/
size_t BOTAN_DLL low_zero_bits(const BigInt& x);

/**
* Check for primality
* @param n a positive integer to test for primality
* @param rng a random number generator
* @param prob chance of false positive is bounded by 1/2**prob
* @param is_random true if n was randomly chosen by us
* @return true if all primality tests passed, otherwise false
*/
bool BOTAN_DLL is_prime(const BigInt& n,
                        RandomNumberGenerator& rng,
                        size_t prob = 56,
                        bool is_random = false);

inline bool quick_check_prime(const BigInt& n, RandomNumberGenerator& rng)
   { return is_prime(n, rng, 32); }

inline bool check_prime(const BigInt& n, RandomNumberGenerator& rng)
   { return is_prime(n, rng, 56); }

inline bool verify_prime(const BigInt& n, RandomNumberGenerator& rng)
   { return is_prime(n, rng, 80); }


/**
* Randomly generate a prime
* @param rng a random number generator
* @param bits how large the resulting prime should be in bits
* @param coprime a positive integer the result should be coprime to
* @param equiv a non-negative number that the result should be
               equivalent to modulo equiv_mod
* @param equiv_mod the modulus equiv should be checked against
* @return random prime with the specified criteria
*/
BigInt BOTAN_DLL random_prime(RandomNumberGenerator& rng,
                              size_t bits, const BigInt& coprime = 1,
                              size_t equiv = 1, size_t equiv_mod = 2);

/**
* Return a 'safe' prime, of the form p=2*q+1 with q prime
* @param rng a random number generator
* @param bits is how long the resulting prime should be
* @return prime randomly chosen from safe primes of length bits
*/
BigInt BOTAN_DLL random_safe_prime(RandomNumberGenerator& rng,
                                   size_t bits);

/**
* Generate DSA parameters using the FIPS 186 kosherizer
* @param rng a random number generator
* @param af an algorithm factory
* @param p_out where the prime p will be stored
* @param q_out where the prime q will be stored
* @param pbits how long p will be in bits
* @param qbits how long q will be in bits
* @return random seed used to generate this parameter set
*/
std::vector<byte> BOTAN_DLL
generate_dsa_primes(RandomNumberGenerator& rng,
                    BigInt& p_out, BigInt& q_out,
                    size_t pbits, size_t qbits);

/**
* Generate DSA parameters using the FIPS 186 kosherizer
* @param rng a random number generator
* @param af an algorithm factory
* @param p_out where the prime p will be stored
* @param q_out where the prime q will be stored
* @param pbits how long p will be in bits
* @param qbits how long q will be in bits
* @param seed the seed used to generate the parameters
* @return true if seed generated a valid DSA parameter set, otherwise
          false. p_out and q_out are only valid if true was returned.
*/
bool BOTAN_DLL
generate_dsa_primes(RandomNumberGenerator& rng,
                    BigInt& p_out, BigInt& q_out,
                    size_t pbits, size_t qbits,
                    const std::vector<byte>& seed);

/**
* The size of the PRIMES[] array
*/
const size_t PRIME_TABLE_SIZE = 6541;

/**
* A const array of all primes less than 65535
*/
extern const u16bit BOTAN_DLL PRIMES[];

}


namespace Botan {

/**
* Modular Reducer (using Barrett's technique)
*/
class BOTAN_DLL Modular_Reducer
   {
   public:
      const BigInt& get_modulus() const { return m_modulus; }

      BigInt reduce(const BigInt& x) const;

      /**
      * Multiply mod p
      * @param x
      * @param y
      * @return (x * y) % p
      */
      BigInt multiply(const BigInt& x, const BigInt& y) const
         { return reduce(x * y); }

      /**
      * Square mod p
      * @param x
      * @return (x * x) % p
      */
      BigInt square(const BigInt& x) const
         { return reduce(Botan::square(x)); }

      /**
      * Cube mod p
      * @param x
      * @return (x * x * x) % p
      */
      BigInt cube(const BigInt& x) const
         { return multiply(x, this->square(x)); }

      bool initialized() const { return (m_mod_words != 0); }

      Modular_Reducer() { m_mod_words = 0; }
      explicit Modular_Reducer(const BigInt& mod);
   private:
      BigInt m_modulus, m_modulus_2, m_mu;
      size_t m_mod_words;
   };

}


namespace Botan {

class RandomNumberGenerator;

/**
* Blinding Function Object
*/
class BOTAN_DLL Blinder
   {
   public:
      BigInt blind(const BigInt& x) const;

      BigInt unblind(const BigInt& x) const;

      bool initialized() const { return m_reducer.initialized(); }

      Blinder() {}

      Blinder(const BigInt& modulus,
              std::function<BigInt (const BigInt&)> fwd_func,
              std::function<BigInt (const BigInt&)> inv_func);

      Blinder(const Blinder&) = delete;

      Blinder& operator=(const Blinder&) = delete;

   private:
      BigInt blinding_nonce() const;

      Modular_Reducer m_reducer;
      std::unique_ptr<RandomNumberGenerator> m_rng;
      std::function<BigInt (const BigInt&)> m_fwd_fn;
      std::function<BigInt (const BigInt&)> m_inv_fn;
      size_t m_modulus_bits = 0;

      mutable BigInt m_e, m_d;
      mutable size_t m_counter = 0;
   };

}


namespace Botan {

/**
* Base class for all stream ciphers
*/
class BOTAN_DLL StreamCipher : public SymmetricAlgorithm
   {
   public:
      typedef SCAN_Name Spec;

      /**
      * Create an instance based on a name
      * Will return a null pointer if the algo/provider combination cannot
      * be found. If provider is empty then best available is chosen.
      */
      static std::unique_ptr<StreamCipher> create(const std::string& algo_spec,
                                                  const std::string& provider = "");

      /**
      * Returns the list of available providers for this algorithm, empty if not available
      */
      static std::vector<std::string> providers(const std::string& algo_spec);

      /**
      * Encrypt or decrypt a message
      * @param in the plaintext
      * @param out the byte array to hold the output, i.e. the ciphertext
      * @param len the length of both in and out in bytes
      */
      virtual void cipher(const byte in[], byte out[], size_t len) = 0;

      /**
      * Encrypt or decrypt a message
      * @param buf the plaintext / ciphertext
      * @param len the length of buf in bytes
      */
      void cipher1(byte buf[], size_t len)
         { cipher(buf, buf, len); }

      template<typename Alloc>
         void encipher(std::vector<byte, Alloc>& inout)
         { cipher(inout.data(), inout.data(), inout.size()); }

      template<typename Alloc>
         void encrypt(std::vector<byte, Alloc>& inout)
         { cipher(inout.data(), inout.data(), inout.size()); }

      template<typename Alloc>
         void decrypt(std::vector<byte, Alloc>& inout)
         { cipher(inout.data(), inout.data(), inout.size()); }

      /**
      * Resync the cipher using the IV
      * @param iv the initialization vector
      * @param iv_len the length of the IV in bytes
      */
      virtual void set_iv(const byte[], size_t iv_len);

      /**
      * @param iv_len the length of the IV in bytes
      * @return if the length is valid for this algorithm
      */
      virtual bool valid_iv_length(size_t iv_len) const { return (iv_len == 0); }

      /**
      * Get a new object representing the same algorithm as *this
      */
      virtual StreamCipher* clone() const = 0;

      /**
      * Set the offset and the state used later to generate the keystream
      * @param offset the offset where we begin to generate the keystream
      */
      virtual void seek(u64bit offset) = 0;

      StreamCipher();
      virtual ~StreamCipher();
   };

}


namespace Botan {

/**
* This class represents any kind of computation which uses an internal
* state, such as hash functions or MACs
*/
class BOTAN_DLL Buffered_Computation
   {
   public:
      /**
      * @return length of the output of this function in bytes
      */
      virtual size_t output_length() const = 0;

      /**
      * Add new input to process.
      * @param in the input to process as a byte array
      * @param length of param in in bytes
      */
      void update(const byte in[], size_t length) { add_data(in, length); }

      /**
      * Add new input to process.
      * @param in the input to process as a secure_vector
      */
      void update(const secure_vector<byte>& in)
         {
         add_data(in.data(), in.size());
         }

      /**
      * Add new input to process.
      * @param in the input to process as a std::vector
      */
      void update(const std::vector<byte>& in)
         {
         add_data(in.data(), in.size());
         }

      /**
      * Add an integer in big-endian order
      * @param in the value
      */
      template<typename T> void update_be(const T in)
         {
         for(size_t i = 0; i != sizeof(T); ++i)
            {
            byte b = get_byte(i, in);
            add_data(&b, 1);
            }
         }

      /**
      * Add new input to process.
      * @param str the input to process as a std::string. Will be interpreted
      * as a byte array based on
      * the strings encoding.
      */
      void update(const std::string& str)
         {
         add_data(reinterpret_cast<const byte*>(str.data()), str.size());
         }

      /**
      * Process a single byte.
      * @param in the byte to process
      */
      void update(byte in) { add_data(&in, 1); }

      /**
      * Complete the computation and retrieve the
      * final result.
      * @param out The byte array to be filled with the result.
      * Must be of length output_length()
      */
      void final(byte out[]) { final_result(out); }

      /**
      * Complete the computation and retrieve the
      * final result.
      * @return secure_vector holding the result
      */
      secure_vector<byte> final()
         {
         secure_vector<byte> output(output_length());
         final_result(output.data());
         return output;
         }

      template<typename Alloc>
         void final(std::vector<byte, Alloc>& out)
         {
         out.resize(output_length());
         final_result(out.data());
         }

      /**
      * Update and finalize computation. Does the same as calling update()
      * and final() consecutively.
      * @param in the input to process as a byte array
      * @param length the length of the byte array
      * @result the result of the call to final()
      */
      secure_vector<byte> process(const byte in[], size_t length)
         {
         add_data(in, length);
         return final();
         }

      /**
      * Update and finalize computation. Does the same as calling update()
      * and final() consecutively.
      * @param in the input to process
      * @result the result of the call to final()
      */
      secure_vector<byte> process(const secure_vector<byte>& in)
         {
         add_data(in.data(), in.size());
         return final();
         }

      /**
      * Update and finalize computation. Does the same as calling update()
      * and final() consecutively.
      * @param in the input to process
      * @result the result of the call to final()
      */
      secure_vector<byte> process(const std::vector<byte>& in)
         {
         add_data(in.data(), in.size());
         return final();
         }

      /**
      * Update and finalize computation. Does the same as calling update()
      * and final() consecutively.
      * @param in the input to process as a string
      * @result the result of the call to final()
      */
      secure_vector<byte> process(const std::string& in)
         {
         update(in);
         return final();
         }

      virtual ~Buffered_Computation() {}
   private:
      /**
      * Add more data to the computation
      * @param input is an input buffer
      * @param length is the length of input in bytes
      */
      virtual void add_data(const byte input[], size_t length) = 0;

      /**
      * Write the final output to out
      * @param out is an output buffer of output_length()
      */
      virtual void final_result(byte out[]) = 0;
   };

}


namespace Botan {

/**
* This class represents hash function (message digest) objects
*/
class BOTAN_DLL HashFunction : public Buffered_Computation
   {
   public:
      typedef SCAN_Name Spec;

      /**
      * Create an instance based on a name
      * Will return a null pointer if the algo/provider combination cannot
      * be found. If provider is empty then best available is chosen.
      */
      static std::unique_ptr<HashFunction> create(const std::string& algo_spec,
                                                  const std::string& provider = "");

      /**
      * Returns the list of available providers for this algorithm, empty if not available
      */
      static std::vector<std::string> providers(const std::string& algo_spec);

      /**
      * @return new object representing the same algorithm as *this
      */
      virtual HashFunction* clone() const = 0;

      HashFunction();

      virtual ~HashFunction();

      virtual void clear() = 0;

      virtual std::string name() const = 0;

      /**
      * @return hash block size as defined for this algorithm
      */
      virtual size_t hash_block_size() const { return 0; }
   };

}


namespace Botan {

/**
* This class represents Message Authentication Code (MAC) objects.
*/
class BOTAN_DLL MessageAuthenticationCode : public Buffered_Computation,
                                            public SymmetricAlgorithm
   {
   public:
      typedef SCAN_Name Spec;

      /**
      * Create an instance based on a name
      * Will return a null pointer if the algo/provider combination cannot
      * be found. If provider is empty then best available is chosen.
      */
      static std::unique_ptr<MessageAuthenticationCode> create(const std::string& algo_spec,
                                                               const std::string& provider = "");

      /**
      * Returns the list of available providers for this algorithm, empty if not available
      */
      static std::vector<std::string> providers(const std::string& algo_spec);

      virtual ~MessageAuthenticationCode();

      /**
      * Verify a MAC.
      * @param in the MAC to verify as a byte array
      * @param length the length of param in
      * @return true if the MAC is valid, false otherwise
      */
      virtual bool verify_mac(const byte in[], size_t length);

      /**
      * Get a new object representing the same algorithm as *this
      */
      virtual MessageAuthenticationCode* clone() const = 0;
   };

typedef MessageAuthenticationCode MAC;

}


namespace Botan {

/*
* As of 1.11.26 this header is deprecated. Instead use the calls T::create and
* T::providers (as demonstrated in the implementation below).
*/

/*
* Get an algorithm object
*  NOTE: these functions create and return new objects, letting the
* caller assume ownership of them
*/

/**
* Block cipher factory method.
*
* @param algo_spec the name of the desired block cipher
* @return pointer to the block cipher object
*/
BOTAN_DEPRECATED("Use BlockCipher::create")
inline BlockCipher* get_block_cipher(const std::string& algo_spec,
                                     const std::string& provider = "")
   {
   return BlockCipher::create(algo_spec, provider).release();
   }

BOTAN_DEPRECATED("Use BlockCipher::create")
inline std::unique_ptr<BlockCipher> make_block_cipher(const std::string& algo_spec,
                                                      const std::string& provider = "")
   {
   std::unique_ptr<BlockCipher> p(BlockCipher::create(algo_spec, provider));
   if(p)
      return p;
   throw Algorithm_Not_Found(algo_spec);
   }

BOTAN_DEPRECATED("Use BlockCipher::providers")
inline std::vector<std::string> get_block_cipher_providers(const std::string& algo_spec)
   {
   return BlockCipher::providers(algo_spec);
   }

/**
* Stream cipher factory method.
*
* @param algo_spec the name of the desired stream cipher
* @return pointer to the stream cipher object
*/
BOTAN_DEPRECATED("Use StreamCipher::create")
inline StreamCipher* get_stream_cipher(const std::string& algo_spec,
                                       const std::string& provider = "")
   {
   return StreamCipher::create(algo_spec, provider).release();
   }

BOTAN_DEPRECATED("Use StreamCipher::create")
inline std::unique_ptr<StreamCipher> make_stream_cipher(const std::string& algo_spec,
                                                        const std::string& provider = "")
   {
   std::unique_ptr<StreamCipher> p(StreamCipher::create(algo_spec, provider));
   if(p)
      return p;
   throw Algorithm_Not_Found(algo_spec);
   }

BOTAN_DEPRECATED("Use StreamCipher::providers")
inline std::vector<std::string> get_stream_cipher_providers(const std::string& algo_spec)
   {
   return StreamCipher::providers(algo_spec);
   }

/**
* Hash function factory method.
*
* @param algo_spec the name of the desired hash function
* @return pointer to the hash function object
*/
BOTAN_DEPRECATED("Use HashFunction::create")
inline HashFunction* get_hash_function(const std::string& algo_spec,
                                       const std::string& provider = "")
   {
   return HashFunction::create(algo_spec, provider).release();
   }

BOTAN_DEPRECATED("Use HashFunction::create")
inline std::unique_ptr<HashFunction> make_hash_function(const std::string& algo_spec,
                                                        const std::string& provider = "")
   {
   std::unique_ptr<HashFunction> p(HashFunction::create(algo_spec, provider));
   if(p)
      return p;
   throw Algorithm_Not_Found(algo_spec);
   }

BOTAN_DEPRECATED("Use HashFunction::create")
inline HashFunction* get_hash(const std::string& algo_spec,
                              const std::string& provider = "")
   {
   return HashFunction::create(algo_spec, provider).release();
   }

BOTAN_DEPRECATED("Use HashFunction::providers")
inline std::vector<std::string> get_hash_function_providers(const std::string& algo_spec)
   {
   return HashFunction::providers(algo_spec);
   }

/**
* MAC factory method.
*
* @param algo_spec the name of the desired MAC
* @return pointer to the MAC object
*/
BOTAN_DEPRECATED("MessageAuthenticationCode::create")
inline MessageAuthenticationCode* get_mac(const std::string& algo_spec,
                                             const std::string& provider = "")
   {
   return MessageAuthenticationCode::create(algo_spec, provider).release();
   }

BOTAN_DEPRECATED("MessageAuthenticationCode::create")
inline std::unique_ptr<MessageAuthenticationCode> make_message_auth(const std::string& algo_spec,
                                                                       const std::string& provider = "")
   {
   std::unique_ptr<MessageAuthenticationCode> p(MessageAuthenticationCode::create(algo_spec, provider));
   if(p)
      return p;
   throw Algorithm_Not_Found(algo_spec);
   }

BOTAN_DEPRECATED("MessageAuthenticationCode::providers")
inline std::vector<std::string> get_mac_providers(const std::string& algo_spec)
   {
   return MessageAuthenticationCode::providers(algo_spec);
   }

}


namespace Botan {

/*
* Get information describing the version
*/

/**
* Get a human-readable string identifying the version of Botan.
* No particular format should be assumed.
* @return version string
*/
BOTAN_DLL std::string version_string();

BOTAN_DLL const char* version_cstr();

/**
* Return the date this version of botan was released, in an integer of
* the form YYYYMMDD. For instance a version released on May 21, 2013
* would return the integer 20130521. If the currently running version
* is not an official release, this function will return 0 instead.
*
* @return release date, or zero if unreleased
*/
BOTAN_DLL u32bit version_datestamp();

/**
* Get the major version number.
* @return major version number
*/
BOTAN_DLL u32bit version_major();

/**
* Get the minor version number.
* @return minor version number
*/
BOTAN_DLL u32bit version_minor();

/**
* Get the patch number.
* @return patch number
*/
BOTAN_DLL u32bit version_patch();

/**
* Usable for checking that the DLL version loaded at runtime exactly
* matches the compile-time version. Call using BOTAN_VERSION_* macro
* values. Returns the empty string if an exact match, otherwise an
* appropriate message. @added 1.11.26
*/
BOTAN_DLL std::string
runtime_version_check(u32bit major,
                      u32bit minor,
                      u32bit patch);

/*
* Macros for compile-time version checks
*/
#define BOTAN_VERSION_CODE_FOR(a,b,c) ((a << 16) | (b << 8) | (c))

/**
* Compare using BOTAN_VERSION_CODE_FOR, as in
*  # if BOTAN_VERSION_CODE < BOTAN_VERSION_CODE_FOR(1,8,0)
*  #    error "Botan version too old"
*  # endif
*/
#define BOTAN_VERSION_CODE BOTAN_VERSION_CODE_FOR(BOTAN_VERSION_MAJOR, \
                                                  BOTAN_VERSION_MINOR, \
                                                  BOTAN_VERSION_PATCH)

}



#if defined(BOTAN_HAS_AUTO_SEEDING_RNG)
#endif


namespace Botan {

/**
* Filter mixin that breaks input into blocks, useful for
* cipher modes
*/
class BOTAN_DLL Buffered_Filter
   {
   public:
      /**
      * Write bytes into the buffered filter, which will them emit them
      * in calls to buffered_block in the subclass
      * @param in the input bytes
      * @param length of in in bytes
      */
      void write(const byte in[], size_t length);

      template<typename Alloc>
         void write(const std::vector<byte, Alloc>& in, size_t length)
         {
         write(in.data(), length);
         }

      /**
      * Finish a message, emitting to buffered_block and buffered_final
      * Will throw an exception if less than final_minimum bytes were
      * written into the filter.
      */
      void end_msg();

      /**
      * Initialize a Buffered_Filter
      * @param block_size the function buffered_block will be called
      *        with inputs which are a multiple of this size
      * @param final_minimum the function buffered_final will be called
      *        with at least this many bytes.
      */
      Buffered_Filter(size_t block_size, size_t final_minimum);

      virtual ~Buffered_Filter() {}
   protected:
      /**
      * The block processor, implemented by subclasses
      * @param input some input bytes
      * @param length the size of input, guaranteed to be a multiple
      *        of block_size
      */
      virtual void buffered_block(const byte input[], size_t length) = 0;

      /**
      * The final block, implemented by subclasses
      * @param input some input bytes
      * @param length the size of input, guaranteed to be at least
      *        final_minimum bytes
      */
      virtual void buffered_final(const byte input[], size_t length) = 0;

      /**
      * @return block size of inputs
      */
      size_t buffered_block_size() const { return m_main_block_mod; }

      /**
      * @return current position in the buffer
      */
      size_t current_position() const { return m_buffer_pos; }

      /**
      * Reset the buffer position
      */
      void buffer_reset() { m_buffer_pos = 0; }
   private:
      size_t m_main_block_mod, m_final_minimum;

      secure_vector<byte> m_buffer;
      size_t m_buffer_pos;
   };

}


namespace Botan {

/**
* Struct representing a particular date and time
*/
struct BOTAN_DLL calendar_point
   {
   /** The year */
   u32bit year;

   /** The month, 1 through 12 for Jan to Dec */
   u32bit month;

   /** The day of the month, 1 through 31 (or 28 or 30 based on month */
   u32bit day;

   /** Hour in 24-hour form, 0 to 23 */
   u32bit hour;

   /** Minutes in the hour, 0 to 60 */
   u32bit minutes;

   /** Seconds in the minute, 0 to 60, but might be slightly
       larger to deal with leap seconds on some systems
   */
   u32bit seconds;

   /**
   * Initialize a calendar_point
   * @param y the year
   * @param mon the month
   * @param d the day
   * @param h the hour
   * @param min the minute
   * @param sec the second
   */
   calendar_point(u32bit y, u32bit mon, u32bit d, u32bit h, u32bit min, u32bit sec) :
      year(y), month(mon), day(d), hour(h), minutes(min), seconds(sec) {}

   /**
   * Returns an STL timepoint object
   */
   std::chrono::system_clock::time_point to_std_timepoint() const;

   /**
   * Returns a human readable string of the struct's components.
   * Formatting might change over time. Currently it is RFC339 'iso-date-time'.
   */
   std::string to_string() const;
   };

/**
* Convert a time_point to a calendar_point
* @param time_point a time point from the system clock
* @return calendar_point object representing this time point
*/
BOTAN_DLL calendar_point calendar_value(
   const std::chrono::system_clock::time_point& time_point);

}


namespace Botan {

/**
* Block Cipher Mode Padding Method
* This class is pretty limited, it cannot deal well with
* randomized padding methods, or any padding method that
* wants to add more than one block. For instance, it should
* be possible to define cipher text stealing mode as simply
* a padding mode for CBC, which happens to consume the last
* two block (and requires use of the block cipher).
*/
class BOTAN_DLL BlockCipherModePaddingMethod
   {
   public:
      virtual void add_padding(secure_vector<byte>& buffer,
                               size_t final_block_bytes,
                               size_t block_size) const = 0;

      /**
      * @param block the last block
      * @param size the of the block
      */
      virtual size_t unpad(const byte block[],
                           size_t size) const = 0;

      /**
      * @param block_size of the cipher
      * @return valid block size for this padding mode
      */
      virtual bool valid_blocksize(size_t block_size) const = 0;

      /**
      * @return name of the mode
      */
      virtual std::string name() const = 0;

      /**
      * virtual destructor
      */
      virtual ~BlockCipherModePaddingMethod() {}
   };

/**
* PKCS#7 Padding
*/
class BOTAN_DLL PKCS7_Padding final : public BlockCipherModePaddingMethod
   {
   public:
      void add_padding(secure_vector<byte>& buffer,
                       size_t final_block_bytes,
                       size_t block_size) const override;

      size_t unpad(const byte[], size_t) const override;

      bool valid_blocksize(size_t bs) const override { return (bs > 0 && bs < 256); }

      std::string name() const override { return "PKCS7"; }
   };

/**
* ANSI X9.23 Padding
*/
class BOTAN_DLL ANSI_X923_Padding final : public BlockCipherModePaddingMethod
   {
   public:
      void add_padding(secure_vector<byte>& buffer,
                       size_t final_block_bytes,
                       size_t block_size) const override;

      size_t unpad(const byte[], size_t) const override;

      bool valid_blocksize(size_t bs) const override { return (bs > 0 && bs < 256); }

      std::string name() const override { return "X9.23"; }
   };

/**
* One And Zeros Padding
*/
class BOTAN_DLL OneAndZeros_Padding final : public BlockCipherModePaddingMethod
   {
   public:
      void add_padding(secure_vector<byte>& buffer,
                       size_t final_block_bytes,
                       size_t block_size) const override;

      size_t unpad(const byte[], size_t) const override;

      bool valid_blocksize(size_t bs) const override { return (bs > 0); }

      std::string name() const override { return "OneAndZeros"; }
   };

/**
* Null Padding
*/
class BOTAN_DLL Null_Padding final : public BlockCipherModePaddingMethod
   {
   public:
      void add_padding(secure_vector<byte>&, size_t, size_t) const override {}

      size_t unpad(const byte[], size_t size) const override { return size; }

      bool valid_blocksize(size_t) const override { return true; }

      std::string name() const override { return "NoPadding"; }
   };

BlockCipherModePaddingMethod* get_bc_pad(const std::string& algo_spec);

}


namespace Botan {

/**
* CBC Mode
*/
class BOTAN_DLL CBC_Mode : public Cipher_Mode
   {
   public:
      std::string name() const override;

      size_t update_granularity() const override;

      Key_Length_Specification key_spec() const override;

      size_t default_nonce_length() const override;

      bool valid_nonce_length(size_t n) const override;

      void clear() override;
   protected:
      CBC_Mode(BlockCipher* cipher, BlockCipherModePaddingMethod* padding);

      const BlockCipher& cipher() const { return *m_cipher; }

      const BlockCipherModePaddingMethod& padding() const
         {
         BOTAN_ASSERT_NONNULL(m_padding);
         return *m_padding;
         }

      secure_vector<byte>& state() { return m_state; }

      byte* state_ptr() { return m_state.data(); }

   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override;

      void key_schedule(const byte key[], size_t length) override;

      std::unique_ptr<BlockCipher> m_cipher;
      std::unique_ptr<BlockCipherModePaddingMethod> m_padding;
      secure_vector<byte> m_state;
   };

/**
* CBC Encryption
*/
class BOTAN_DLL CBC_Encryption : public CBC_Mode
   {
   public:
      CBC_Encryption(BlockCipher* cipher, BlockCipherModePaddingMethod* padding) :
         CBC_Mode(cipher, padding) {}

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override;

      size_t minimum_final_size() const override;
   };

/**
* CBC Encryption with ciphertext stealing (CBC-CS3 variant)
*/
class BOTAN_DLL CTS_Encryption final : public CBC_Encryption
   {
   public:
      explicit CTS_Encryption(BlockCipher* cipher) : CBC_Encryption(cipher, nullptr) {}

      size_t output_length(size_t input_length) const override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;

      size_t minimum_final_size() const override;

      bool valid_nonce_length(size_t n) const override;
   };

/**
* CBC Decryption
*/
class BOTAN_DLL CBC_Decryption : public CBC_Mode
   {
   public:
      CBC_Decryption(BlockCipher* cipher, BlockCipherModePaddingMethod* padding) :
         CBC_Mode(cipher, padding), m_tempbuf(update_granularity()) {}

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override;

      size_t minimum_final_size() const override;
   private:
      secure_vector<byte> m_tempbuf;
   };

/**
* CBC Decryption with ciphertext stealing (CBC-CS3 variant)
*/
class BOTAN_DLL CTS_Decryption final : public CBC_Decryption
   {
   public:
      explicit CTS_Decryption(BlockCipher* cipher) : CBC_Decryption(cipher, nullptr) {}

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;

      size_t minimum_final_size() const override;

      bool valid_nonce_length(size_t n) const override;
   };

}


namespace Botan {

/**
* The different charsets (nominally) supported by Botan.
*/
enum Character_Set {
   LOCAL_CHARSET,
   UCS2_CHARSET,
   UTF8_CHARSET,
   LATIN1_CHARSET
};

namespace Charset {

/*
* Character Set Handling
*/
std::string BOTAN_DLL transcode(const std::string& str,
                                Character_Set to,
                                Character_Set from);

bool BOTAN_DLL is_digit(char c);
bool BOTAN_DLL is_space(char c);
bool BOTAN_DLL caseless_cmp(char x, char y);

byte BOTAN_DLL char2digit(char c);
char BOTAN_DLL digit2char(byte b);

}

}


namespace Botan {

/**
* This class represents keyed filters, i.e. filters that have to be
* fed with a key in order to function.
*/
class BOTAN_DLL Keyed_Filter : public Filter
   {
   public:
      /**
      * Set the key of this filter
      * @param key the key to use
      */
      virtual void set_key(const SymmetricKey& key) = 0;

      /**
      * Set the initialization vector of this filter. Note: you should
      * call set_iv() only after you have called set_key()
      * @param iv the initialization vector to use
      */
      virtual void set_iv(const InitializationVector& iv);

      /**
      * Check whether a key length is valid for this filter
      * @param length the key length to be checked for validity
      * @return true if the key length is valid, false otherwise
      */
      bool valid_keylength(size_t length) const
         {
         return key_spec().valid_keylength(length);
         }

      /**
      * @return object describing limits on key size
      */
      virtual Key_Length_Specification key_spec() const = 0;

      /**
      * Check whether an IV length is valid for this filter
      * @param length the IV length to be checked for validity
      * @return true if the IV length is valid, false otherwise
      */
      virtual bool valid_iv_length(size_t length) const
         { return (length == 0); }
   };



/*
* Get a cipher object
*/

/**
* Factory method for general symmetric cipher filters.
* @param algo_spec the name of the desired cipher
* @param key the key to be used for encryption/decryption performed by
* the filter
* @param iv the initialization vector to be used
* @param direction determines whether the filter will be an encrypting
* or decrypting filter
* @return pointer to newly allocated encryption or decryption filter
*/
BOTAN_DLL Keyed_Filter* get_cipher(const std::string& algo_spec,
                                   const SymmetricKey& key,
                                   const InitializationVector& iv,
                                   Cipher_Dir direction);

/**
* Factory method for general symmetric cipher filters.
* @param algo_spec the name of the desired cipher
* @param key the key to be used for encryption/decryption performed by
* the filter
* @param direction determines whether the filter will be an encrypting
* or decrypting filter
* @return pointer to the encryption or decryption filter
*/
BOTAN_DLL Keyed_Filter* get_cipher(const std::string& algo_spec,
                                   const SymmetricKey& key,
                                   Cipher_Dir direction);

/**
* Factory method for general symmetric cipher filters. No key will be
* set in the filter.
*
* @param algo_spec the name of the desired cipher
* @param direction determines whether the filter will be an encrypting or
* decrypting filter
* @return pointer to the encryption or decryption filter
*/
BOTAN_DLL Keyed_Filter* get_cipher(const std::string& algo_spec,
                                   Cipher_Dir direction);

}


namespace Botan {

/**
* Filter interface for cipher modes
*/
class BOTAN_DLL Cipher_Mode_Filter : public Keyed_Filter,
                                     private Buffered_Filter
   {
   public:
      explicit Cipher_Mode_Filter(Cipher_Mode* t);

      void set_iv(const InitializationVector& iv) override;

      void set_key(const SymmetricKey& key) override;

      Key_Length_Specification key_spec() const override;

      bool valid_iv_length(size_t length) const override;

      std::string name() const override;

   protected:
      const Cipher_Mode& get_mode() const { return *m_mode; }

      Cipher_Mode& get_mode() { return *m_mode; }

   private:
      void write(const byte input[], size_t input_length) override;
      void start_msg() override;
      void end_msg() override;

      void buffered_block(const byte input[], size_t input_length) override;
      void buffered_final(const byte input[], size_t input_length) override;

      class Nonce_State
         {
         public:
            explicit Nonce_State(bool allow_null_nonce) : m_fresh_nonce(allow_null_nonce) {}

            void update(const InitializationVector& iv);
            std::vector<byte> get();
         private:
            bool m_fresh_nonce;
            std::vector<byte> m_nonce;
         };

      Nonce_State m_nonce;
      std::unique_ptr<Cipher_Mode> m_mode;
      secure_vector<byte> m_buffer;
   };

// deprecated aliases, will be removed before 2.0
typedef Cipher_Mode_Filter Transform_Filter;
typedef Transform_Filter Transformation_Filter;

}


namespace Botan {

class Compression_Algorithm;
class Decompression_Algorithm;

/**
* Filter interface for compression
*/
class BOTAN_DLL Compression_Filter : public Filter
   {
   public:
      void start_msg() override;
      void write(const byte input[], size_t input_length) override;
      void end_msg() override;

      void flush();

      std::string name() const override;

      Compression_Filter(const std::string& type,
                         size_t compression_level,
                         size_t buffer_size = 4096);
   private:
      std::unique_ptr<Compression_Algorithm> m_comp;
      size_t m_buffersize, m_level;
      secure_vector<byte> m_buffer;
   };

/**
* Filter interface for decompression
*/
class BOTAN_DLL Decompression_Filter : public Filter
   {
   public:
      void start_msg() override;
      void write(const byte input[], size_t input_length) override;
      void end_msg() override;

      std::string name() const override;

      Decompression_Filter(const std::string& type,
                           size_t buffer_size = 4096);
   private:
      std::unique_ptr<Decompression_Algorithm> m_comp;
      std::size_t m_buffersize;
      secure_vector<byte> m_buffer;
   };

}


namespace Botan {

class BOTAN_DLL Compression_Algorithm
   {
   public:
      typedef SCAN_Name Spec;

      /**
      * Begin compressing. Most compression algorithms offer a tunable
      * time/compression tradeoff parameter generally represented by
      * an integer in the range of 1 to 9.
      *
      * If 0 or a value out of range is provided, a compression algorithm
      * specific default is used.
      */
      virtual void start(size_t comp_level = 0) = 0;

      /**
      * Process some data. Input must be in size update_granularity() byte blocks.
      * @param blocks in/out parameter which will possibly be resized or swapped
      * @param offset an offset into blocks to begin processing
      * @param flush if true the compressor will be told to flush state
      */
      virtual void update(secure_vector<byte>& buf, size_t offset = 0, bool flush = false) = 0;

      /**
      * Finish compressing
      *
      * @param final_block in/out parameter
      * @param offset an offset into final_block to begin processing
      */
      virtual void finish(secure_vector<byte>& final_block, size_t offset = 0) = 0;

      virtual std::string name() const = 0;

      /**
      * Reset the state and abort the current message; start can be
      * called again to process a new message.
      */
      virtual void clear() = 0;

      virtual ~Compression_Algorithm() {}
   };

class BOTAN_DLL Decompression_Algorithm
   {
   public:
      typedef SCAN_Name Spec;

      /**
      * Decompression does not support levels
      */
      virtual void start() = 0;

      virtual void update(secure_vector<byte>& buf, size_t offset = 0) = 0;

      virtual void finish(secure_vector<byte>& final_block, size_t offset = 0) = 0;

      virtual std::string name() const = 0;

      virtual void clear() = 0;

      virtual ~Decompression_Algorithm() {}
   };

BOTAN_DLL Compression_Algorithm* make_compressor(const std::string& type);
BOTAN_DLL Decompression_Algorithm* make_decompressor(const std::string& type);

class Compression_Stream
   {
   public:
      virtual ~Compression_Stream() {}

      virtual void next_in(byte* b, size_t len) = 0;

      virtual void next_out(byte* b, size_t len) = 0;

      virtual size_t avail_in() const = 0;

      virtual size_t avail_out() const = 0;

      virtual u32bit run_flag() const = 0;
      virtual u32bit flush_flag() const = 0;
      virtual u32bit finish_flag() const = 0;

      virtual bool run(u32bit flags) = 0;
   };

class Stream_Compression : public Compression_Algorithm
   {
   public:
      void update(secure_vector<byte>& buf, size_t offset, bool flush) final override;

      void finish(secure_vector<byte>& buf, size_t offset) final override;

      void clear() final override;

   private:
      void start(size_t level) final override;

      void process(secure_vector<byte>& buf, size_t offset, u32bit flags);

      virtual Compression_Stream* make_stream(size_t level) const = 0;

      secure_vector<byte> m_buffer;
      std::unique_ptr<Compression_Stream> m_stream;
   };

class Stream_Decompression : public Decompression_Algorithm
   {
   public:
      void update(secure_vector<byte>& buf, size_t offset) final override;

      void finish(secure_vector<byte>& buf, size_t offset) final override;

      void clear() final override;

   private:
      void start() final override;

      void process(secure_vector<byte>& buf, size_t offset, u32bit flags);

      virtual Compression_Stream* make_stream() const = 0;

      secure_vector<byte> m_buffer;
      std::unique_ptr<Compression_Stream> m_stream;
   };

}


namespace Botan {

/**
* A class handling runtime CPU feature detection
*/
class BOTAN_DLL CPUID
   {
   public:
      /**
      * Probe the CPU and see what extensions are supported
      */
      static void initialize();

      /**
      * Return a best guess of the cache line size
      */
      static size_t cache_line_size()
         {
         if(!g_initialized)
            {
            initialize();
            }
         return g_cache_line_size;
         }

      /**
      * Check if the processor supports AltiVec/VMX
      */
      static bool has_altivec()
         {
         if(!g_initialized)
            {
            initialize();
            }

         return g_altivec_capable;
         }

      /**
      * Check if the processor supports RDTSC
      */
      static bool has_rdtsc()
         { return x86_processor_flags_has(CPUID_RDTSC_BIT); }

      /**
      * Check if the processor supports SSE2
      */
      static bool has_sse2()
         { return x86_processor_flags_has(CPUID_SSE2_BIT); }

      /**
      * Check if the processor supports SSSE3
      */
      static bool has_ssse3()
         { return x86_processor_flags_has(CPUID_SSSE3_BIT); }

      /**
      * Check if the processor supports SSE4.1
      */
      static bool has_sse41()
         { return x86_processor_flags_has(CPUID_SSE41_BIT); }

      /**
      * Check if the processor supports SSE4.2
      */
      static bool has_sse42()
         { return x86_processor_flags_has(CPUID_SSE42_BIT); }

      /**
      * Check if the processor supports AVX2
      */
      static bool has_avx2()
         { return x86_processor_flags_has(CPUID_AVX2_BIT); }

      /**
      * Check if the processor supports AVX-512F
      */
      static bool has_avx512f()
         { return x86_processor_flags_has(CPUID_AVX512F_BIT); }

      /**
      * Check if the processor supports BMI2
      */
      static bool has_bmi2()
         { return x86_processor_flags_has(CPUID_BMI2_BIT); }

      /**
      * Check if the processor supports AES-NI
      */
      static bool has_aes_ni()
         { return x86_processor_flags_has(CPUID_AESNI_BIT); }

      /**
      * Check if the processor supports CLMUL
      */
      static bool has_clmul()
         { return x86_processor_flags_has(CPUID_CLMUL_BIT); }

      /**
      * Check if the processor supports Intel SHA extension
      */
      static bool has_intel_sha()
         { return x86_processor_flags_has(CPUID_SHA_BIT); }

      /**
      * Check if the processor supports ADX extension
      */
      static bool has_adx()
         { return x86_processor_flags_has(CPUID_ADX_BIT); }

      /**
      * Check if the processor supports RDRAND
      */
      static bool has_rdrand()
         { return x86_processor_flags_has(CPUID_RDRAND_BIT); }

      /**
      * Check if the processor supports RDSEED
      */
      static bool has_rdseed()
         { return x86_processor_flags_has(CPUID_RDSEED_BIT); }

      static bool has_simd_32();

      static void print(std::ostream& o);
   private:
      enum CPUID_bits {
         CPUID_RDTSC_BIT = 4,
         CPUID_SSE2_BIT = 26,
         CPUID_CLMUL_BIT = 33,
         CPUID_SSSE3_BIT = 41,
         CPUID_SSE41_BIT = 51,
         CPUID_SSE42_BIT = 52,
         CPUID_AESNI_BIT = 57,
         CPUID_RDRAND_BIT = 62,

         CPUID_AVX2_BIT = 64+5,
         CPUID_BMI2_BIT = 64+8,
         CPUID_AVX512F_BIT = 64+16,
         CPUID_RDSEED_BIT = 64+18,
         CPUID_ADX_BIT = 64+19,
         CPUID_SHA_BIT = 64+29,
      };

      static bool x86_processor_flags_has(u64bit bit)
         {
         if(!g_initialized)
            initialize();
         return ((g_x86_processor_flags[bit/64] >> (bit % 64)) & 1);
         }

      static bool g_initialized;
      static u64bit g_x86_processor_flags[2];
      static size_t g_cache_line_size;
      static bool g_altivec_capable;
   };

}


namespace Botan {

/**
* CTR-BE (Counter mode, big-endian)
*/
class BOTAN_DLL CTR_BE final : public StreamCipher
   {
   public:
      void cipher(const byte in[], byte out[], size_t length) override;

      void set_iv(const byte iv[], size_t iv_len) override;

      bool valid_iv_length(size_t iv_len) const override
         { return (iv_len <= m_cipher->block_size()); }

      Key_Length_Specification key_spec() const override
         {
         return m_cipher->key_spec();
         }

      std::string name() const override;

      CTR_BE* clone() const override
         { return new CTR_BE(m_cipher->clone()); }

      void clear() override;

      static CTR_BE* make(const Spec& spec);

      /**
      * @param cipher the underlying block cipher to use
      */
      explicit CTR_BE(BlockCipher* cipher);

      CTR_BE(BlockCipher* cipher, size_t ctr_size);

      void seek(u64bit offset) override;
   private:
      void key_schedule(const byte key[], size_t key_len) override;
      void increment_counter();

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<byte> m_counter, m_pad;
      size_t m_ctr_size;
      size_t m_pad_pos;
   };

}


namespace Botan {

/**
* This class represents abstract data sink objects.
*/
class BOTAN_DLL DataSink : public Filter
   {
   public:
      bool attachable() override { return false; }
      DataSink() {}
      virtual ~DataSink() {}

      DataSink& operator=(const DataSink&) = delete;
      DataSink(const DataSink&) = delete;
   };

/**
* This class represents a data sink which writes its output to a stream.
*/
class BOTAN_DLL DataSink_Stream : public DataSink
   {
   public:
      std::string name() const override { return m_identifier; }

      void write(const byte[], size_t) override;

      /**
      * Construct a DataSink_Stream from a stream.
      * @param stream the stream to write to
      * @param name identifier
      */
      DataSink_Stream(std::ostream& stream,
                      const std::string& name = "<std::ostream>");

      /**
      * Construct a DataSink_Stream from a stream.
      * @param pathname the name of the file to open a stream to
      * @param use_binary indicates whether to treat the file
      * as a binary file or not
      */
      DataSink_Stream(const std::string& pathname,
                      bool use_binary = false);

      ~DataSink_Stream();
   private:
      const std::string m_identifier;

      std::ostream* m_sink_p;
      std::ostream& m_sink;
   };

}


namespace Botan {

class BOTAN_DLL SQL_Database
   {
   public:

      class BOTAN_DLL SQL_DB_Error : public Exception
         {
         public:
            explicit SQL_DB_Error(const std::string& what) : Exception("SQL database", what) {}
         };

      class BOTAN_DLL Statement
         {
         public:
            /* Bind statement parameters */
            virtual void bind(int column, const std::string& str) = 0;

            virtual void bind(int column, size_t i) = 0;

            virtual void bind(int column, std::chrono::system_clock::time_point time) = 0;

            virtual void bind(int column, const std::vector<byte>& blob) = 0;

            /* Get output */
            virtual std::pair<const byte*, size_t> get_blob(int column) = 0;

            virtual size_t get_size_t(int column) = 0;

            /* Run to completion */
            virtual size_t spin() = 0;

            /* Maybe update */
            virtual bool step() = 0;

            virtual ~Statement() {}
         };

      /*
      * Create a new statement for execution.
      * Use ?1, ?2, ?3, etc for parameters to set later with bind
      */
      virtual std::shared_ptr<Statement> new_statement(const std::string& base_sql) const = 0;

      virtual size_t row_count(const std::string& table_name) = 0;

      virtual void create_table(const std::string& table_schema) = 0;

      virtual ~SQL_Database() {}
};

}


namespace Botan {

class BigInt;
class ASN1_Object;

/**
* General DER Encoding Object
*/
class BOTAN_DLL DER_Encoder
   {
   public:
      secure_vector<byte> get_contents();

      std::vector<byte> get_contents_unlocked()
         { return unlock(get_contents()); }

      DER_Encoder& start_cons(ASN1_Tag type_tag,
                              ASN1_Tag class_tag = UNIVERSAL);
      DER_Encoder& end_cons();

      DER_Encoder& start_explicit(u16bit type_tag);
      DER_Encoder& end_explicit();

      DER_Encoder& raw_bytes(const byte val[], size_t len);
      DER_Encoder& raw_bytes(const secure_vector<byte>& val);
      DER_Encoder& raw_bytes(const std::vector<byte>& val);

      DER_Encoder& encode_null();
      DER_Encoder& encode(bool b);
      DER_Encoder& encode(size_t s);
      DER_Encoder& encode(const BigInt& n);
      DER_Encoder& encode(const secure_vector<byte>& v, ASN1_Tag real_type);
      DER_Encoder& encode(const std::vector<byte>& v, ASN1_Tag real_type);
      DER_Encoder& encode(const byte val[], size_t len, ASN1_Tag real_type);

      DER_Encoder& encode(bool b,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      DER_Encoder& encode(size_t s,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      DER_Encoder& encode(const BigInt& n,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      DER_Encoder& encode(const std::vector<byte>& v,
                          ASN1_Tag real_type,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      DER_Encoder& encode(const secure_vector<byte>& v,
                          ASN1_Tag real_type,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      DER_Encoder& encode(const byte v[], size_t len,
                          ASN1_Tag real_type,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      template<typename T>
      DER_Encoder& encode_optional(const T& value, const T& default_value)
         {
         if(value != default_value)
            encode(value);
         return (*this);
         }

      template<typename T>
      DER_Encoder& encode_list(const std::vector<T>& values)
         {
         for(size_t i = 0; i != values.size(); ++i)
            encode(values[i]);
         return (*this);
         }

      DER_Encoder& encode(const ASN1_Object& obj);
      DER_Encoder& encode_if(bool pred, DER_Encoder& enc);
      DER_Encoder& encode_if(bool pred, const ASN1_Object& obj);

      DER_Encoder& add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                              const byte rep[], size_t length);

      DER_Encoder& add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                              const std::vector<byte>& rep)
         {
         return add_object(type_tag, class_tag, rep.data(), rep.size());
         }

      DER_Encoder& add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                              const secure_vector<byte>& rep)
         {
         return add_object(type_tag, class_tag, rep.data(), rep.size());
         }

      DER_Encoder& add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                              const std::string& str);

      DER_Encoder& add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                              byte val);

   private:
      class DER_Sequence
         {
         public:
            ASN1_Tag tag_of() const;
            secure_vector<byte> get_contents();
            void add_bytes(const byte[], size_t);
            DER_Sequence(ASN1_Tag, ASN1_Tag);
         private:
            ASN1_Tag m_type_tag, m_class_tag;
            secure_vector<byte> m_contents;
            std::vector< secure_vector<byte> > m_set_contents;
         };

      secure_vector<byte> m_contents;
      std::vector<DER_Sequence> m_subsequences;
   };

}


namespace Botan {

/**
* BigInt Division
* @param x an integer
* @param y a non-zero integer
* @param q will be set to x / y
* @param r will be set to x % y
*/
void BOTAN_DLL divide(const BigInt& x,
                      const BigInt& y,
                      BigInt& q,
                      BigInt& r);

}


namespace Botan {

/**
* This class represents discrete logarithm groups. It holds a prime p,
* a prime q = (p-1)/2 and g = x^((p-1)/q) mod p.
*/
class BOTAN_DLL DL_Group
   {
   public:

      /**
      * Get the prime p.
      * @return prime p
      */
      const BigInt& get_p() const;

      /**
      * Get the prime q.
      * @return prime q
      */
      const BigInt& get_q() const;

      /**
      * Get the base g.
      * @return base g
      */
      const BigInt& get_g() const;

      /**
      * The DL group encoding format variants.
      */
      enum Format {
         ANSI_X9_42,
         ANSI_X9_57,
         PKCS_3,

         DSA_PARAMETERS = ANSI_X9_57,
         DH_PARAMETERS = ANSI_X9_42,
         X942_DH_PARAMETERS = ANSI_X9_42,
         PKCS3_DH_PARAMETERS = PKCS_3
      };

      /**
      * Determine the prime creation for DL groups.
      */
      enum PrimeType { Strong, Prime_Subgroup, DSA_Kosherizer };

      /**
      * Perform validity checks on the group.
      * @param rng the rng to use
      * @param strong whether to perform stronger by lengthier tests
      * @return true if the object is consistent, false otherwise
      */
      bool verify_group(RandomNumberGenerator& rng, bool strong) const;

      /**
      * Encode this group into a string using PEM encoding.
      * @param format the encoding format
      * @return string holding the PEM encoded group
      */
      std::string PEM_encode(Format format) const;

      /**
      * Encode this group into a string using DER encoding.
      * @param format the encoding format
      * @return string holding the DER encoded group
      */
      std::vector<byte> DER_encode(Format format) const;

      /**
      * Decode a DER/BER encoded group into this instance.
      * @param ber a vector containing the DER/BER encoded group
      * @param format the format of the encoded group
      */
      void BER_decode(const std::vector<byte>& ber,
                      Format format);

      /**
      * Decode a PEM encoded group into this instance.
      * @param pem the PEM encoding of the group
      */
      void PEM_decode(const std::string& pem);

      /**
      * Construct a DL group with uninitialized internal value.
      * Use this constructor is you wish to set the groups values
      * from a DER or PEM encoded group.
      */
      DL_Group();

      /**
      * Construct a DL group that is registered in the configuration.
      * @param name the name that is configured in the global configuration
      * for the desired group. If no configuration file is specified,
      * the default values from the file policy.cpp will be used. For instance,
      * use "modp/ietf/768" as name.
      */
      DL_Group(const std::string& name);

      /**
      * Create a new group randomly.
      * @param rng the random number generator to use
      * @param type specifies how the creation of primes p and q shall
      * be performed. If type=Strong, then p will be determined as a
      * safe prime, and q will be chosen as (p-1)/2. If
      * type=Prime_Subgroup and qbits = 0, then the size of q will be
      * determined according to the estimated difficulty of the DL
      * problem. If type=DSA_Kosherizer, DSA primes will be created.
      * @param pbits the number of bits of p
      * @param qbits the number of bits of q. Leave it as 0 to have
      * the value determined according to pbits.
      */
      DL_Group(RandomNumberGenerator& rng, PrimeType type,
               size_t pbits, size_t qbits = 0);

      /**
      * Create a DSA group with a given seed.
      * @param rng the random number generator to use
      * @param seed the seed to use to create the random primes
      * @param pbits the desired bit size of the prime p
      * @param qbits the desired bit size of the prime q.
      */
      DL_Group(RandomNumberGenerator& rng,
               const std::vector<byte>& seed,
               size_t pbits = 1024, size_t qbits = 0);

      /**
      * Create a DL group. The prime q will be determined according to p.
      * @param p the prime p
      * @param g the base g
      */
      DL_Group(const BigInt& p, const BigInt& g);

      /**
      * Create a DL group.
      * @param p the prime p
      * @param q the prime q
      * @param g the base g
      */
      DL_Group(const BigInt& p, const BigInt& q, const BigInt& g);

      /**
      * Return PEM representation of named DL group
      */
      static const char* PEM_for_named_group(const std::string& name);
   private:
      static BigInt make_dsa_generator(const BigInt&, const BigInt&);

      void init_check() const;
      void initialize(const BigInt&, const BigInt&, const BigInt&);
      bool m_initialized;
      BigInt m_p, m_q, m_g;
   };

}


namespace Botan {

/**
* Encoding Method for Encryption
*/
class BOTAN_DLL EME
   {
   public:
      typedef SCAN_Name Spec;

      virtual ~EME() = default;

      /**
      * Return the maximum input size in bytes we can support
      * @param keybits the size of the key in bits
      * @return upper bound of input in bytes
      */
      virtual size_t maximum_input_size(size_t keybits) const = 0;

      /**
      * Encode an input
      * @param in the plaintext
      * @param in_length length of plaintext in bytes
      * @param key_length length of the key in bits
      * @param rng a random number generator
      * @return encoded plaintext
      */
      secure_vector<byte> encode(const byte in[],
                                 size_t in_length,
                                 size_t key_length,
                                 RandomNumberGenerator& rng) const;

      /**
      * Encode an input
      * @param in the plaintext
      * @param key_length length of the key in bits
      * @param rng a random number generator
      * @return encoded plaintext
      */
      secure_vector<byte> encode(const secure_vector<byte>& in,
                                 size_t key_length,
                                 RandomNumberGenerator& rng) const;

      /**
      * Decode an input
      * @param valid_mask written to specifies if output is valid
      * @param in the encoded plaintext
      * @param in_len length of encoded plaintext in bytes
      * @return bytes of out[] written to along with
      *         validity mask (0xFF if valid, else 0x00)
      */
      virtual secure_vector<byte> unpad(byte& valid_mask,
                                        const byte in[],
                                        size_t in_len) const = 0;

      /**
      * Encode an input
      * @param in the plaintext
      * @param in_length length of plaintext in bytes
      * @param key_length length of the key in bits
      * @param rng a random number generator
      * @return encoded plaintext
      */
      virtual secure_vector<byte> pad(const byte in[],
                                      size_t in_length,
                                      size_t key_length,
                                      RandomNumberGenerator& rng) const = 0;
   };

/**
* Factory method for EME (message-encoding methods for encryption) objects
* @param algo_spec the name of the EME to create
* @return pointer to newly allocated object of that type
*/
BOTAN_DLL EME*  get_eme(const std::string& algo_spec);

}


namespace Botan {

/**
* EMSA, from IEEE 1363s Encoding Method for Signatures, Appendix
*
* Any way of encoding/padding signatures
*/
class BOTAN_DLL EMSA
   {
   public:
      typedef SCAN_Name Spec;

      /**
      * Add more data to the signature computation
      * @param input some data
      * @param length length of input in bytes
      */
      virtual void update(const byte input[], size_t length) = 0;

      /**
      * @return raw hash
      */
      virtual secure_vector<byte> raw_data() = 0;

      /**
      * Return the encoding of a message
      * @param msg the result of raw_data()
      * @param output_bits the desired output bit size
      * @param rng a random number generator
      * @return encoded signature
      */
      virtual secure_vector<byte> encoding_of(const secure_vector<byte>& msg,
                                             size_t output_bits,
                                             RandomNumberGenerator& rng) = 0;

      /**
      * Verify the encoding
      * @param coded the received (coded) message representative
      * @param raw the computed (local, uncoded) message representative
      * @param key_bits the size of the key in bits
      * @return true if coded is a valid encoding of raw, otherwise false
      */
      virtual bool verify(const secure_vector<byte>& coded,
                          const secure_vector<byte>& raw,
                          size_t key_bits) = 0;

      virtual ~EMSA();
   };

/**
* Factory method for EMSA (message-encoding methods for signatures
* with appendix) objects
* @param algo_spec the name of the EME to create
* @return pointer to newly allocated object of that type
*/
BOTAN_DLL EMSA* get_emsa(const std::string& algo_spec);

}


namespace Botan {

/**
* Class used to accumulate the poll results of EntropySources
*/
class BOTAN_DLL Entropy_Accumulator final
   {
   public:
      /**
      * Initialize an Entropy_Accumulator
      *
      * @param accum will be called with poll results, first params the data and
      * length, the second a best estimate of min-entropy for the entire buffer;
      * out of an abundance of caution this will be zero for many sources.
      * accum should return true if it wants the polling to stop, though it may
      * still be called again a few more times, and should be careful to return
      * true then as well.
      */
      explicit Entropy_Accumulator(std::function<bool (const byte[], size_t, double)> accum) :
         m_accum_fn(accum) {}

      /**
      * @return if our polling goal has been achieved
      */
      bool polling_goal_achieved() const { return m_done; }

      bool polling_finished() const { return m_done; }

      /**
      * Add entropy to the accumulator
      * @param bytes the input bytes
      * @param length specifies how many bytes the input is
      * @param entropy_bits_per_byte is a best guess at how much
      * entropy per byte is in this input
      */
      void add(const void* bytes, size_t length, double entropy_bits_per_byte)
         {
         m_done = m_accum_fn(reinterpret_cast<const byte*>(bytes),
                             length, entropy_bits_per_byte * length) || m_done;
         }

      /**
      * Add entropy to the accumulator
      * @param v is some value
      * @param entropy_bits_per_byte is a best guess at how much
      * entropy per byte is in this input
      */
      template<typename T>
      void add(const T& v, double entropy_bits_per_byte)
         {
         add(&v, sizeof(T), entropy_bits_per_byte);
         }

      secure_vector<byte>& get_io_buf(size_t sz) { m_io_buf.resize(sz); return m_io_buf; }
   private:
      std::function<bool (const byte[], size_t, double)> m_accum_fn;
      secure_vector<byte> m_io_buf;
      bool m_done = false;
   };

/**
* Abstract interface to a source of entropy
*/
class BOTAN_DLL Entropy_Source
   {
   public:
      /*
      * Return a new entropy source of a particular type, or null
      * Each entropy source may require substantial resources (eg, a file handle
      * or socket instance), so try to share them among multiple RNGs, or just
      * use the preconfigured global list accessed by global_entropy_sources()
      */
      static std::unique_ptr<Entropy_Source> create(const std::string& type);

      /**
      * @return name identifying this entropy source
      */
      virtual std::string name() const = 0;

      /**
      * Perform an entropy gathering poll
      * @param accum is an accumulator object that will be given entropy
      */
      virtual void poll(Entropy_Accumulator& accum) = 0;

      virtual ~Entropy_Source() {}
   };

class BOTAN_DLL Entropy_Sources final
   {
   public:
      static Entropy_Sources& global_sources();

      void add_source(std::unique_ptr<Entropy_Source> src);

      std::vector<std::string> enabled_sources() const;

      void poll(Entropy_Accumulator& accum);
      bool poll_just(Entropy_Accumulator& accum, const std::string& src);

      Entropy_Sources() {}
      explicit Entropy_Sources(const std::vector<std::string>& sources);

      ~Entropy_Sources();
   private:
      std::vector<Entropy_Source*> m_srcs;
   };

}



namespace Botan {

/**
* This class represents pipe objects.
* A set of filters can be placed into a pipe, and information flows
* through the pipe until it reaches the end, where the output is
* collected for retrieval.  If you're familiar with the Unix shell
* environment, this design will sound quite familiar.
*/
class BOTAN_DLL Pipe final : public DataSource
   {
   public:
      /**
      * An opaque type that identifies a message in this Pipe
      */
      typedef size_t message_id;

      /**
      * Exception if you use an invalid message as an argument to
      * read, remaining, etc
      */
      struct BOTAN_DLL Invalid_Message_Number : public Invalid_Argument
         {
         /**
         * @param where the error occurred
         * @param msg the invalid message id that was used
         */
         Invalid_Message_Number(const std::string& where, message_id msg) :
            Invalid_Argument("Pipe::" + where + ": Invalid message number " +
                             std::to_string(msg))
            {}
         };

      /**
      * A meta-id for whatever the last message is
      */
      static const message_id LAST_MESSAGE;

      /**
      * A meta-id for the default message (set with set_default_msg)
      */
      static const message_id DEFAULT_MESSAGE;

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in the byte array to write
      * @param length the length of the byte array in
      */
      void write(const byte in[], size_t length);

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in the secure_vector containing the data to write
      */
      void write(const secure_vector<byte>& in)
         { write(in.data(), in.size()); }

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in the std::vector containing the data to write
      */
      void write(const std::vector<byte>& in)
         { write(in.data(), in.size()); }

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in the string containing the data to write
      */
      void write(const std::string& in);

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in the DataSource to read the data from
      */
      void write(DataSource& in);

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in a single byte to be written
      */
      void write(byte in);

      /**
      * Perform start_msg(), write() and end_msg() sequentially.
      * @param in the byte array containing the data to write
      * @param length the length of the byte array to write
      */
      void process_msg(const byte in[], size_t length);

      /**
      * Perform start_msg(), write() and end_msg() sequentially.
      * @param in the secure_vector containing the data to write
      */
      void process_msg(const secure_vector<byte>& in);

      /**
      * Perform start_msg(), write() and end_msg() sequentially.
      * @param in the secure_vector containing the data to write
      */
      void process_msg(const std::vector<byte>& in);

      /**
      * Perform start_msg(), write() and end_msg() sequentially.
      * @param in the string containing the data to write
      */
      void process_msg(const std::string& in);

      /**
      * Perform start_msg(), write() and end_msg() sequentially.
      * @param in the DataSource providing the data to write
      */
      void process_msg(DataSource& in);

      /**
      * Find out how many bytes are ready to read.
      * @param msg the number identifying the message
      * for which the information is desired
      * @return number of bytes that can still be read
      */
      size_t remaining(message_id msg = DEFAULT_MESSAGE) const;

      /**
      * Read the default message from the pipe. Moves the internal
      * offset so that every call to read will return a new portion of
      * the message.
      *
      * @param output the byte array to write the read bytes to
      * @param length the length of the byte array output
      * @return number of bytes actually read into output
      */
      size_t read(byte output[], size_t length) override;

      /**
      * Read a specified message from the pipe. Moves the internal
      * offset so that every call to read will return a new portion of
      * the message.
      * @param output the byte array to write the read bytes to
      * @param length the length of the byte array output
      * @param msg the number identifying the message to read from
      * @return number of bytes actually read into output
      */
      size_t read(byte output[], size_t length, message_id msg);

      /**
      * Read a single byte from the pipe. Moves the internal offset so
      * that every call to read will return a new portion of the
      * message.
      *
      * @param output the byte to write the result to
      * @param msg the message to read from
      * @return number of bytes actually read into output
      */
      size_t read(byte& output, message_id msg = DEFAULT_MESSAGE);

      /**
      * Read the full contents of the pipe.
      * @param msg the number identifying the message to read from
      * @return secure_vector holding the contents of the pipe
      */
      secure_vector<byte> read_all(message_id msg = DEFAULT_MESSAGE);

      /**
      * Read the full contents of the pipe.
      * @param msg the number identifying the message to read from
      * @return string holding the contents of the pipe
      */
      std::string read_all_as_string(message_id = DEFAULT_MESSAGE);

      /** Read from the default message but do not modify the internal
      * offset. Consecutive calls to peek() will return portions of
      * the message starting at the same position.
      * @param output the byte array to write the peeked message part to
      * @param length the length of the byte array output
      * @param offset the offset from the current position in message
      * @return number of bytes actually peeked and written into output
      */
      size_t peek(byte output[], size_t length, size_t offset) const override;

      /** Read from the specified message but do not modify the
      * internal offset. Consecutive calls to peek() will return
      * portions of the message starting at the same position.
      * @param output the byte array to write the peeked message part to
      * @param length the length of the byte array output
      * @param offset the offset from the current position in message
      * @param msg the number identifying the message to peek from
      * @return number of bytes actually peeked and written into output
      */
      size_t peek(byte output[], size_t length,
                  size_t offset, message_id msg) const;

      /** Read a single byte from the specified message but do not
      * modify the internal offset. Consecutive calls to peek() will
      * return portions of the message starting at the same position.
      * @param output the byte to write the peeked message byte to
      * @param offset the offset from the current position in message
      * @param msg the number identifying the message to peek from
      * @return number of bytes actually peeked and written into output
      */
      size_t peek(byte& output, size_t offset,
                  message_id msg = DEFAULT_MESSAGE) const;

      /**
      * @return the number of bytes read from the default message.
      */
      size_t get_bytes_read() const override;

      /**
      * @return the number of bytes read from the specified message.
      */
      size_t get_bytes_read(message_id msg) const;

      bool check_available(size_t n) override;
      bool check_available_msg(size_t n, message_id msg);

      /**
      * @return currently set default message
      */
      size_t default_msg() const { return m_default_read; }

      /**
      * Set the default message
      * @param msg the number identifying the message which is going to
      * be the new default message
      */
      void set_default_msg(message_id msg);

      /**
      * Get the number of messages the are in this pipe.
      * @return number of messages the are in this pipe
      */
      message_id message_count() const;

      /**
      * Test whether this pipe has any data that can be read from.
      * @return true if there is more data to read, false otherwise
      */
      bool end_of_data() const override;

      /**
      * Start a new message in the pipe. A potential other message in this pipe
      * must be closed with end_msg() before this function may be called.
      */
      void start_msg();

      /**
      * End the current message.
      */
      void end_msg();

      /**
      * Insert a new filter at the front of the pipe
      * @param filt the new filter to insert
      */
      void prepend(Filter* filt);

      /**
      * Insert a new filter at the back of the pipe
      * @param filt the new filter to insert
      */
      void append(Filter* filt);

      /**
      * Remove the first filter at the front of the pipe.
      */
      void pop();

      /**
      * Reset this pipe to an empty pipe.
      */
      void reset();

      /**
      * Construct a Pipe of up to four filters. The filters are set up
      * in the same order as the arguments.
      */
      Pipe(Filter* = nullptr, Filter* = nullptr,
           Filter* = nullptr, Filter* = nullptr);

      /**
      * Construct a Pipe from a list of filters
      * @param filters the set of filters to use
      */
      explicit Pipe(std::initializer_list<Filter*> filters);

      Pipe(const Pipe&) = delete;
      Pipe& operator=(const Pipe&) = delete;

      ~Pipe();
   private:
      void init();
      void destruct(Filter*);
      void find_endpoints(Filter*);
      void clear_endpoints(Filter*);

      message_id get_message_no(const std::string&, message_id) const;

      Filter* m_pipe;
      class Output_Buffers* m_outputs;
      message_id m_default_read;
      bool m_inside_msg;
   };

/**
* Stream output operator; dumps the results from pipe's default
* message to the output stream.
* @param out an output stream
* @param pipe the pipe
*/
BOTAN_DLL std::ostream& operator<<(std::ostream& out, Pipe& pipe);

/**
* Stream input operator; dumps the remaining bytes of input
* to the (assumed open) pipe message.
* @param in the input stream
* @param pipe the pipe
*/
BOTAN_DLL std::istream& operator>>(std::istream& in, Pipe& pipe);

}

#if defined(BOTAN_HAS_PIPE_UNIXFD_IO)
#endif



#if defined(BOTAN_HAS_CODEC_FILTERS)

namespace Botan {

/**
* Converts arbitrary binary data to hex strings, optionally with
* newlines inserted
*/
class BOTAN_DLL Hex_Encoder final : public Filter
   {
   public:
      /**
      * Whether to use uppercase or lowercase letters for the encoded string.
      */
      enum Case { Uppercase, Lowercase };

      std::string name() const override { return "Hex_Encoder"; }

      void write(const byte in[], size_t length) override;
      void end_msg() override;

      /**
      * Create a hex encoder.
      * @param the_case the case to use in the encoded strings.
      */
      explicit Hex_Encoder(Case the_case);

      /**
      * Create a hex encoder.
      * @param newlines should newlines be used
      * @param line_length if newlines are used, how long are lines
      * @param the_case the case to use in the encoded strings
      */
      Hex_Encoder(bool newlines = false,
                  size_t line_length = 72,
                  Case the_case = Uppercase);
   private:
      void encode_and_send(const byte[], size_t);

      const Case m_casing;
      const size_t m_line_length;
      std::vector<byte> m_in, m_out;
      size_t m_position, m_counter;
   };

/**
* Converts hex strings to bytes
*/
class BOTAN_DLL Hex_Decoder final : public Filter
   {
   public:
      std::string name() const override { return "Hex_Decoder"; }

      void write(const byte[], size_t) override;
      void end_msg() override;

      /**
      * Construct a Hex Decoder using the specified
      * character checking.
      * @param checking the checking to use during decoding.
      */
      explicit Hex_Decoder(Decoder_Checking checking = NONE);
   private:
      const Decoder_Checking m_checking;
      std::vector<byte> m_in, m_out;
      size_t m_position;
   };

}

#endif

namespace Botan {

/**
* Stream Cipher Filter
*/
class BOTAN_DLL StreamCipher_Filter : public Keyed_Filter
   {
   public:

      std::string name() const override { return m_cipher->name(); }

      /**
      * Write input data
      * @param input data
      * @param input_len length of input in bytes
      */
      void write(const byte input[], size_t input_len) override;

      bool valid_iv_length(size_t iv_len) const override
         { return m_cipher->valid_iv_length(iv_len); }

      /**
      * Set the initialization vector for this filter.
      * @param iv the initialization vector to set
      */
      void set_iv(const InitializationVector& iv) override
         {
         m_cipher->set_iv(iv.begin(), iv.length());
         }

      /**
      * Set the key of this filter.
      * @param key the key to set
      */
      void set_key(const SymmetricKey& key) override { m_cipher->set_key(key); }

      Key_Length_Specification key_spec() const override { return m_cipher->key_spec(); }

      /**
      * Construct a stream cipher filter.
      * @param cipher a cipher object to use
      */
      explicit StreamCipher_Filter(StreamCipher* cipher);

      /**
      * Construct a stream cipher filter.
      * @param cipher a cipher object to use
      * @param key the key to use inside this filter
      */
      StreamCipher_Filter(StreamCipher* cipher, const SymmetricKey& key);

      /**
      * Construct a stream cipher filter.
      * @param cipher the name of the desired cipher
      */
      explicit StreamCipher_Filter(const std::string& cipher);

      /**
      * Construct a stream cipher filter.
      * @param cipher the name of the desired cipher
      * @param key the key to use inside this filter
      */
      StreamCipher_Filter(const std::string& cipher, const SymmetricKey& key);
   private:
      secure_vector<byte> m_buffer;
      std::unique_ptr<StreamCipher> m_cipher;
   };

/**
* Hash Filter.
*/
class BOTAN_DLL Hash_Filter : public Filter
   {
   public:
      void write(const byte input[], size_t len) override { m_hash->update(input, len); }
      void end_msg() override;

      std::string name() const override { return m_hash->name(); }

      /**
      * Construct a hash filter.
      * @param hash_fun the hash function to use
      * @param len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the hashfunction
      * hash. Otherwise, specify a smaller value here so that the
      * output of the hash algorithm will be cut off.
      */
      Hash_Filter(HashFunction* hash, size_t len = 0) :
         m_hash(hash), m_out_len(len) {}

      /**
      * Construct a hash filter.
      * @param request the name of the hash algorithm to use
      * @param len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the hashfunction
      * hash. Otherwise, specify a smaller value here so that the
      * output of the hash algorithm will be cut off.
      */
      Hash_Filter(const std::string& request, size_t len = 0);

   private:
      std::unique_ptr<HashFunction> m_hash;
      const size_t m_out_len;
   };

/**
* MessageAuthenticationCode Filter.
*/
class BOTAN_DLL MAC_Filter : public Keyed_Filter
   {
   public:
      void write(const byte input[], size_t len) override { m_mac->update(input, len); }
      void end_msg() override;

      std::string name() const override { return m_mac->name(); }

      /**
      * Set the key of this filter.
      * @param key the key to set
      */
      void set_key(const SymmetricKey& key) override { m_mac->set_key(key); }

      Key_Length_Specification key_spec() const override { return m_mac->key_spec(); }

      /**
      * Construct a MAC filter. The MAC key will be left empty.
      * @param mac the MAC to use
      * @param out_len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the
      * MAC. Otherwise, specify a smaller value here so that the
      * output of the MAC will be cut off.
      */
      MAC_Filter(MessageAuthenticationCode* mac,
                 size_t out_len = 0) :
         m_mac(mac),
         m_out_len(out_len)
         {
         }

      /**
      * Construct a MAC filter.
      * @param mac the MAC to use
      * @param key the MAC key to use
      * @param out_len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the
      * MAC. Otherwise, specify a smaller value here so that the
      * output of the MAC will be cut off.
      */
      MAC_Filter(MessageAuthenticationCode* mac,
                 const SymmetricKey& key,
                 size_t out_len = 0) :
         m_mac(mac),
         m_out_len(out_len)
         {
         m_mac->set_key(key);
         }

      /**
      * Construct a MAC filter. The MAC key will be left empty.
      * @param mac the name of the MAC to use
      * @param len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the
      * MAC. Otherwise, specify a smaller value here so that the
      * output of the MAC will be cut off.
      */
      MAC_Filter(const std::string& mac, size_t len = 0);

      /**
      * Construct a MAC filter.
      * @param mac the name of the MAC to use
      * @param key the MAC key to use
      * @param len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the
      * MAC. Otherwise, specify a smaller value here so that the
      * output of the MAC will be cut off.
      */
      MAC_Filter(const std::string& mac, const SymmetricKey& key,
                 size_t len = 0);
   private:
      std::unique_ptr<MessageAuthenticationCode> m_mac;
      const size_t m_out_len;
   };

}


namespace Botan {

class GHASH;

/**
* GCM Mode
*/
class BOTAN_DLL GCM_Mode : public AEAD_Mode
   {
   public:
      void set_associated_data(const byte ad[], size_t ad_len) override;

      std::string name() const override;

      size_t update_granularity() const override;

      Key_Length_Specification key_spec() const override;

      // GCM supports arbitrary nonce lengths
      bool valid_nonce_length(size_t) const override { return true; }

      size_t tag_size() const override { return m_tag_size; }

      void clear() override;
   protected:
      GCM_Mode(BlockCipher* cipher, size_t tag_size);

      const size_t m_BS = 16;

      const size_t m_tag_size;
      const std::string m_cipher_name;

      std::unique_ptr<StreamCipher> m_ctr;
      std::unique_ptr<GHASH> m_ghash;
   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override;

      void key_schedule(const byte key[], size_t length) override;
   };

/**
* GCM Encryption
*/
class BOTAN_DLL GCM_Encryption final : public GCM_Mode
   {
   public:
      /**
      * @param cipher the 128 bit block cipher to use
      * @param tag_size is how big the auth tag will be
      */
      GCM_Encryption(BlockCipher* cipher, size_t tag_size = 16) :
         GCM_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         { return input_length + tag_size(); }

      size_t minimum_final_size() const override { return 0; }

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   };

/**
* GCM Decryption
*/
class BOTAN_DLL GCM_Decryption final : public GCM_Mode
   {
   public:
      /**
      * @param cipher the 128 bit block cipher to use
      * @param tag_size is how big the auth tag will be
      */
      GCM_Decryption(BlockCipher* cipher, size_t tag_size = 16) :
         GCM_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         {
         BOTAN_ASSERT(input_length > tag_size(), "Sufficient input");
         return input_length - tag_size();
         }

      size_t minimum_final_size() const override { return tag_size(); }

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   };

/**
* GCM's GHASH
* Maybe a Transform?
*/
class BOTAN_DLL GHASH : public SymmetricAlgorithm
   {
   public:
      void set_associated_data(const byte ad[], size_t ad_len);

      secure_vector<byte> nonce_hash(const byte nonce[], size_t len);

      void start(const byte nonce[], size_t len);

      /*
      * Assumes input len is multiple of 16
      */
      void update(const byte in[], size_t len);

      secure_vector<byte> final();

      Key_Length_Specification key_spec() const override
         { return Key_Length_Specification(16); }

      void clear() override;

      std::string name() const override { return "GHASH"; }
   private:
      void key_schedule(const byte key[], size_t key_len) override;

      void gcm_multiply(secure_vector<byte>& x) const;

      void ghash_update(secure_vector<byte>& x,
                        const byte input[], size_t input_len);

      void add_final_block(secure_vector<byte>& x,
                           size_t ad_len, size_t pt_len);

      secure_vector<byte> m_H;
      secure_vector<byte> m_H_ad;
      secure_vector<byte> m_nonce;
      secure_vector<byte> m_ghash;
      size_t m_ad_len = 0, m_text_len = 0;
   };

}


namespace Botan {

/**
* Perform hex encoding
* @param output an array of at least input_length*2 bytes
* @param input is some binary data
* @param input_length length of input in bytes
* @param uppercase should output be upper or lower case?
*/
void BOTAN_DLL hex_encode(char output[],
                          const byte input[],
                          size_t input_length,
                          bool uppercase = true);

/**
* Perform hex encoding
* @param input some input
* @param input_length length of input in bytes
* @param uppercase should output be upper or lower case?
* @return hexadecimal representation of input
*/
std::string BOTAN_DLL hex_encode(const byte input[],
                                 size_t input_length,
                                 bool uppercase = true);

/**
* Perform hex encoding
* @param input some input
* @param uppercase should output be upper or lower case?
* @return hexadecimal representation of input
*/
template<typename Alloc>
std::string hex_encode(const std::vector<byte, Alloc>& input,
                       bool uppercase = true)
   {
   return hex_encode(input.data(), input.size(), uppercase);
   }

/**
* Perform hex decoding
* @param output an array of at least input_length/2 bytes
* @param input some hex input
* @param input_length length of input in bytes
* @param input_consumed is an output parameter which says how many
*        bytes of input were actually consumed. If less than
*        input_length, then the range input[consumed:length]
*        should be passed in later along with more input.
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL hex_decode(byte output[],
                            const char input[],
                            size_t input_length,
                            size_t& input_consumed,
                            bool ignore_ws = true);

/**
* Perform hex decoding
* @param output an array of at least input_length/2 bytes
* @param input some hex input
* @param input_length length of input in bytes
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL hex_decode(byte output[],
                            const char input[],
                            size_t input_length,
                            bool ignore_ws = true);

/**
* Perform hex decoding
* @param output an array of at least input_length/2 bytes
* @param input some hex input
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL hex_decode(byte output[],
                            const std::string& input,
                            bool ignore_ws = true);

/**
* Perform hex decoding
* @param input some hex input
* @param input_length the length of input in bytes
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded hex output
*/
std::vector<byte> BOTAN_DLL
hex_decode(const char input[],
           size_t input_length,
           bool ignore_ws = true);

/**
* Perform hex decoding
* @param input some hex input
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded hex output
*/
std::vector<byte> BOTAN_DLL
hex_decode(const std::string& input,
           bool ignore_ws = true);


/**
* Perform hex decoding
* @param input some hex input
* @param input_length the length of input in bytes
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded hex output
*/
secure_vector<byte> BOTAN_DLL
hex_decode_locked(const char input[],
                  size_t input_length,
                  bool ignore_ws = true);

/**
* Perform hex decoding
* @param input some hex input
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded hex output
*/
secure_vector<byte> BOTAN_DLL
hex_decode_locked(const std::string& input,
                  bool ignore_ws = true);

}


namespace Botan {

/**
* HMAC
*/
class BOTAN_DLL HMAC final : public MessageAuthenticationCode
   {
   public:
      void clear() override;
      std::string name() const override;
      MessageAuthenticationCode* clone() const override;

      size_t output_length() const override { return m_hash->output_length(); }

      Key_Length_Specification key_spec() const override
         {
         // Absurd max length here is to support PBKDF2
         return Key_Length_Specification(0, 512);
         }

      /**
      * @param hash the hash to use for HMACing
      */
      explicit HMAC(HashFunction* hash);

      static HMAC* make(const Spec& spec);

      HMAC(const HMAC&) = delete;
      HMAC& operator=(const HMAC&) = delete;
   private:
      void add_data(const byte[], size_t) override;
      void final_result(byte[]) override;
      void key_schedule(const byte[], size_t) override;

      std::unique_ptr<HashFunction> m_hash;
      secure_vector<byte> m_ikey, m_okey;
   };

}


namespace Botan {

/**
* HMAC_RNG - based on the design described in "On Extract-then-Expand
* Key Derivation Functions and an HMAC-based KDF" by Hugo Krawczyk
* (henceforce, 'E-t-E')
*
* However it actually can be parameterized with any two MAC functions,
* not restricted to HMAC (this variation is also described in
* Krawczyk's paper), for instance one could use HMAC(SHA-512) as the
* extractor and CMAC(AES-256) as the PRF.
*/
class BOTAN_DLL HMAC_RNG : public RandomNumberGenerator
   {
   public:
      void randomize(byte buf[], size_t len) override;
      bool is_seeded() const override;
      void clear() override;
      std::string name() const override;

      size_t reseed_with_sources(Entropy_Sources& srcs,
                                 size_t poll_bits,
                                 std::chrono::milliseconds poll_timeout) override;

      void add_entropy(const byte[], size_t) override;

      /**
      * @param extractor a MAC used for extracting the entropy
      * @param prf a MAC used as a PRF using HKDF construction
      */
      HMAC_RNG(MessageAuthenticationCode* extractor,
               MessageAuthenticationCode* prf);
   private:
      std::unique_ptr<MessageAuthenticationCode> m_extractor;
      std::unique_ptr<MessageAuthenticationCode> m_prf;

      enum HMAC_PRF_Label {
         Running,
         Reseed,
         ExtractorSeed,
      };
      void new_K_value(byte label);

      size_t m_collected_entropy_estimate = 0;
      size_t m_output_since_reseed = 0;

      secure_vector<byte> m_K;
      u32bit m_counter = 0;
      u32bit m_pid = 0;
   };

}


namespace Botan {

/**
* Public Key Base Class.
*/
class BOTAN_DLL Public_Key
   {
   public:
      /**
      * Get the name of the underlying public key scheme.
      * @return name of the public key scheme
      */
      virtual std::string algo_name() const = 0;

      /**
      * Return the estimated strength of the underlying key against
      * the best currently known attack. Note that this ignores anything
      * but pure attacks against the key itself and do not take into
      * account padding schemes, usage mistakes, etc which might reduce
      * the strength. However it does suffice to provide an upper bound.
      *
      * @return estimated strength in bits
      */
      virtual size_t estimated_strength() const = 0;

      /**
      * Get the OID of the underlying public key scheme.
      * @return OID of the public key scheme
      */
      virtual OID get_oid() const;

      /**
      * Test the key values for consistency.
      * @param rng rng to use
      * @param strong whether to perform strong and lengthy version
      * of the test
      * @return true if the test is passed
      */
      virtual bool check_key(RandomNumberGenerator& rng,
                             bool strong) const = 0;

      /**
      * Find out the number of message parts supported by this scheme.
      * @return number of message parts
      */
      virtual size_t message_parts() const { return 1; }

      /**
      * Find out the message part size supported by this scheme/key.
      * @return size of the message parts in bits
      */
      virtual size_t message_part_size() const { return 0; }

      /**
      * Get the maximum message size in bits supported by this public key.
      * @return maximum message size in bits
      */
      virtual size_t max_input_bits() const = 0;

      /**
      * @return X.509 AlgorithmIdentifier for this key
      */
      virtual AlgorithmIdentifier algorithm_identifier() const = 0;

      /**
      * @return X.509 subject key encoding for this key object
      */
      virtual std::vector<byte> x509_subject_public_key() const = 0;

      virtual ~Public_Key() {}
   protected:
      /**
      * Self-test after loading a key
      * @param rng a random number generator
      */
      virtual void load_check(RandomNumberGenerator& rng) const;
   };

/**
* Private Key Base Class
*/
class BOTAN_DLL Private_Key : public virtual Public_Key
   {
   public:
      /**
      * @return PKCS #8 private key encoding for this key object
      */
      virtual secure_vector<byte> pkcs8_private_key() const = 0;

      /**
      * @return PKCS #8 AlgorithmIdentifier for this key
      * Might be different from the X.509 identifier, but normally is not
      */
      virtual AlgorithmIdentifier pkcs8_algorithm_identifier() const
         { return algorithm_identifier(); }

   protected:
      /**
      * Self-test after loading a key
      * @param rng a random number generator
      */
      void load_check(RandomNumberGenerator& rng) const override;

      /**
      * Self-test after generating a key
      * @param rng a random number generator
      */
      void gen_check(RandomNumberGenerator& rng) const;
   };

/**
* PK Secret Value Derivation Key
*/
class BOTAN_DLL PK_Key_Agreement_Key : public virtual Private_Key
   {
   public:
      /*
      * @return public component of this key
      */
      virtual std::vector<byte> public_value() const = 0;

      virtual ~PK_Key_Agreement_Key() {}
   };

/*
* Typedefs
*/
typedef PK_Key_Agreement_Key PK_KA_Key;
typedef Public_Key X509_PublicKey;
typedef Private_Key PKCS8_PrivateKey;

}


namespace Botan {

/**
* The two types of X509 encoding supported by Botan.
*/
enum X509_Encoding { RAW_BER, PEM };

/**
* This namespace contains functions for handling X.509 public keys
*/
namespace X509 {

/**
* BER encode a key
* @param key the public key to encode
* @return BER encoding of this key
*/
BOTAN_DLL std::vector<byte> BER_encode(const Public_Key& key);

/**
* PEM encode a public key into a string.
* @param key the key to encode
* @return PEM encoded key
*/
BOTAN_DLL std::string PEM_encode(const Public_Key& key);

/**
* Create a public key from a data source.
* @param source the source providing the DER or PEM encoded key
* @return new public key object
*/
BOTAN_DLL Public_Key* load_key(DataSource& source);

/**
* Create a public key from a file
* @param filename pathname to the file to load
* @return new public key object
*/
BOTAN_DLL Public_Key* load_key(const std::string& filename);

/**
* Create a public key from a memory region.
* @param enc the memory region containing the DER or PEM encoded key
* @return new public key object
*/
BOTAN_DLL Public_Key* load_key(const std::vector<byte>& enc);

/**
* Copy a key.
* @param key the public key to copy
* @return new public key object
*/
BOTAN_DLL Public_Key* copy_key(const Public_Key& key);

}

}


namespace Botan {

/**
* This class represents public keys
* of integer factorization based (IF) public key schemes.
*/
class BOTAN_DLL IF_Scheme_PublicKey : public virtual Public_Key
   {
   public:
      IF_Scheme_PublicKey(const AlgorithmIdentifier& alg_id,
                          const secure_vector<byte>& key_bits);

      IF_Scheme_PublicKey(const BigInt& n, const BigInt& e) :
         m_n(n), m_e(e) {}

      bool check_key(RandomNumberGenerator& rng, bool) const override;

      AlgorithmIdentifier algorithm_identifier() const override;

      std::vector<byte> x509_subject_public_key() const override;

      /**
      * @return public modulus
      */
      const BigInt& get_n() const { return m_n; }

      /**
      * @return public exponent
      */
      const BigInt& get_e() const { return m_e; }

      size_t max_input_bits() const override { return (m_n.bits() - 1); }

      size_t estimated_strength() const override;

   protected:
      IF_Scheme_PublicKey() {}

      BigInt m_n, m_e;
   };

/**
* This class represents public keys
* of integer factorization based (IF) public key schemes.
*/
class BOTAN_DLL IF_Scheme_PrivateKey : public virtual IF_Scheme_PublicKey,
                                       public virtual Private_Key
   {
   public:

      IF_Scheme_PrivateKey(RandomNumberGenerator& rng,
                           const BigInt& prime1, const BigInt& prime2,
                           const BigInt& exp, const BigInt& d_exp,
                           const BigInt& mod);

      IF_Scheme_PrivateKey(RandomNumberGenerator& rng,
                           const AlgorithmIdentifier& alg_id,
                           const secure_vector<byte>& key_bits);

      bool check_key(RandomNumberGenerator& rng, bool) const override;

      /**
      * Get the first prime p.
      * @return prime p
      */
      const BigInt& get_p() const { return m_p; }

      /**
      * Get the second prime q.
      * @return prime q
      */
      const BigInt& get_q() const { return m_q; }

      /**
      * Get d with exp * d = 1 mod (p - 1, q - 1).
      * @return d
      */
      const BigInt& get_d() const { return m_d; }

      const BigInt& get_c() const { return m_c; }
      const BigInt& get_d1() const { return m_d1; }
      const BigInt& get_d2() const { return m_d2; }

      secure_vector<byte> pkcs8_private_key() const override;

   protected:
      IF_Scheme_PrivateKey() {}

      BigInt m_d, m_p, m_q, m_d1, m_d2, m_c;
   };

}


namespace Botan {

/*
* Previously botan had state whose lifetime had to be explicitly
* managed by the application. As of 1.11.14 this is no longer the
* case, and this class is no longer needed and kept only for backwards
* compatibility.
*/
class BOTAN_DLL LibraryInitializer
   {
   public:
      explicit LibraryInitializer(const std::string& s = "") { initialize(s); }
      ~LibraryInitializer() { deinitialize(); }

      static void initialize(const std::string& = "");
      static void deinitialize();
   };

}


namespace Botan {

/**
* Key Derivation Function
*/
class BOTAN_DLL KDF
   {
   public:
      virtual ~KDF();

      /**
      * Create an instance based on a name
      * Will return a null pointer if the algo/provider combination cannot
      * be found. If provider is empty then best available is chosen.
      */
      static std::unique_ptr<KDF> create(const std::string& algo_spec,
                                         const std::string& provider = "");

      /**
      * Returns the list of available providers for this algorithm, empty if not available
      */
      static std::vector<std::string> providers(const std::string& algo_spec);

      virtual std::string name() const = 0;

      virtual size_t kdf(byte key[], size_t key_len,
                         const byte secret[], size_t secret_len,
                         const byte salt[], size_t salt_len,
                         const byte label[], size_t label_len) const = 0;


      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param secret_len size of secret in bytes
      * @param salt a diversifier
      * @param salt_len size of salt in bytes
      * @param label purpose for the derived keying material
      * @param label_len size of label in bytes
      */
      secure_vector<byte> derive_key(size_t key_len,
                                    const byte secret[],
                                    size_t secret_len,
                                    const byte salt[],
                                    size_t salt_len,
                                    const byte label[] = nullptr,
                                    size_t label_len = 0) const
         {
         secure_vector<byte> key(key_len);
         key.resize(kdf(key.data(), key.size(), secret, secret_len, salt, salt_len, label, label_len));
         return key;
         }

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param salt a diversifier
      * @param label purpose for the derived keying material
      */
      secure_vector<byte> derive_key(size_t key_len,
                                    const secure_vector<byte>& secret,
                                    const std::string& salt = "",
                                    const std::string& label = "") const
         {
         return derive_key(key_len, secret.data(), secret.size(),
                           reinterpret_cast<const byte*>(salt.data()),
                           salt.length(),
                           reinterpret_cast<const byte*>(label.data()),
                           label.length());

         }

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param salt a diversifier
      * @param label purpose for the derived keying material
      */
      template<typename Alloc, typename Alloc2, typename Alloc3>
      secure_vector<byte> derive_key(size_t key_len,
                                     const std::vector<byte, Alloc>& secret,
                                     const std::vector<byte, Alloc2>& salt,
                                     const std::vector<byte, Alloc3>& label) const
         {
         return derive_key(key_len,
                           secret.data(), secret.size(),
                           salt.data(), salt.size(),
                           label.data(), label.size());
         }

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param salt a diversifier
      * @param salt_len size of salt in bytes
      * @param label purpose for the derived keying material
      */
      secure_vector<byte> derive_key(size_t key_len,
                                    const secure_vector<byte>& secret,
                                    const byte salt[],
                                    size_t salt_len,
                                    const std::string& label = "") const
         {
         return derive_key(key_len,
                           secret.data(), secret.size(),
                           salt, salt_len,
                           reinterpret_cast<const byte*>(label.data()),
                           label.size());
         }

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param secret_len size of secret in bytes
      * @param salt a diversifier
      * @param label purpose for the derived keying material
      */
      secure_vector<byte> derive_key(size_t key_len,
                                    const byte secret[],
                                    size_t secret_len,
                                    const std::string& salt = "",
                                    const std::string& label = "") const
         {
         return derive_key(key_len, secret, secret_len,
                           reinterpret_cast<const byte*>(salt.data()),
                           salt.length(),
                           reinterpret_cast<const byte*>(label.data()),
                           label.length());
         }

      virtual KDF* clone() const = 0;

      typedef SCAN_Name Spec;

   };

/**
* Factory method for KDF (key derivation function)
* @param algo_spec the name of the KDF to create
* @return pointer to newly allocated object of that type
*/
BOTAN_DLL KDF*  get_kdf(const std::string& algo_spec);

}


namespace Botan {

namespace KeyPair {

/**
* Tests whether the key is consistent for encryption; whether
* encrypting and then decrypting gives to the original plaintext.
* @param rng the rng to use
* @param key the key to test
* @param padding the encryption padding method to use
* @return true if consistent otherwise false
*/
BOTAN_DLL bool
encryption_consistency_check(RandomNumberGenerator& rng,
                             const Private_Key& key,
                             const std::string& padding);

/**
* Tests whether the key is consistent for signatures; whether a
* signature can be created and then verified
* @param rng the rng to use
* @param key the key to test
* @param padding the signature padding method to use
* @return true if consistent otherwise false
*/
BOTAN_DLL bool
signature_consistency_check(RandomNumberGenerator& rng,
                            const Private_Key& key,
                            const std::string& padding);

}

}


namespace Botan {

/**
* MDx Hash Function Base Class
*/
class BOTAN_DLL MDx_HashFunction : public HashFunction
   {
   public:
      /**
      * @param block_length is the number of bytes per block
      * @param big_byte_endian specifies if the hash uses big-endian bytes
      * @param big_bit_endian specifies if the hash uses big-endian bits
      * @param counter_size specifies the size of the counter var in bytes
      */
      MDx_HashFunction(size_t block_length,
                       bool big_byte_endian,
                       bool big_bit_endian,
                       size_t counter_size = 8);

      size_t hash_block_size() const override { return m_buffer.size(); }
   protected:
      void add_data(const byte input[], size_t length) override;
      void final_result(byte output[]) override;

      /**
      * Run the hash's compression function over a set of blocks
      * @param blocks the input
      * @param block_n the number of blocks
      */
      virtual void compress_n(const byte blocks[], size_t block_n) = 0;

      void clear() override;

      /**
      * Copy the output to the buffer
      * @param buffer to put the output into
      */
      virtual void copy_out(byte buffer[]) = 0;

      /**
      * Write the count, if used, to this spot
      * @param out where to write the counter to
      */
      virtual void write_count(byte out[]);
   private:
      secure_vector<byte> m_buffer;
      u64bit m_count;
      size_t m_position;

      const bool BIG_BYTE_ENDIAN, BIG_BIT_ENDIAN;
      const size_t COUNT_SIZE;
   };

}


namespace Botan {

/**
* MGF1 from PKCS #1 v2.0
*/
void BOTAN_DLL mgf1_mask(HashFunction& hash,
                         const byte in[], size_t in_len,
                         byte out[], size_t out_len);

}


namespace Botan {

// Prefer TI mode over __int128 as GCC rejects the latter in pendantic mode
#if (BOTAN_GCC_VERSION > 440) && defined(BOTAN_TARGET_CPU_HAS_NATIVE_64BIT)
   #define BOTAN_TARGET_HAS_NATIVE_UINT128
   typedef unsigned int uint128_t __attribute__((mode(TI)));
#elif defined(__SIZEOF_INT128__)
   #define BOTAN_TARGET_HAS_NATIVE_UINT128
   typedef unsigned __int128 uint128_t;
#endif

}

#if defined(BOTAN_TARGET_HAS_NATIVE_UINT128)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi)      \
   do {                                      \
      const uint128_t r = static_cast<uint128_t>(a) * b;   \
      *hi = (r >> 64) & 0xFFFFFFFFFFFFFFFF;  \
      *lo = (r      ) & 0xFFFFFFFFFFFFFFFF;  \
   } while(0)

#elif defined(BOTAN_BUILD_COMPILER_IS_MSVC) && defined(BOTAN_TARGET_CPU_HAS_NATIVE_64BIT)

#include <intrin.h>
#pragma intrinsic(_umul128)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi) \
   do { *lo = _umul128(a, b, hi); } while(0)

#elif defined(BOTAN_USE_GCC_INLINE_ASM)

#if defined(BOTAN_TARGET_ARCH_IS_X86_64)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi) do {                           \
   asm("mulq %3" : "=d" (*hi), "=a" (*lo) : "a" (a), "rm" (b) : "cc"); \
   } while(0)

#elif defined(BOTAN_TARGET_ARCH_IS_ALPHA)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi) do {              \
   asm("umulh %1,%2,%0" : "=r" (*hi) : "r" (a), "r" (b)); \
   *lo = a * b;                                           \
} while(0)

#elif defined(BOTAN_TARGET_ARCH_IS_IA64)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi) do {                \
   asm("xmpy.hu %0=%1,%2" : "=f" (*hi) : "f" (a), "f" (b)); \
   *lo = a * b;                                             \
} while(0)

#elif defined(BOTAN_TARGET_ARCH_IS_PPC64)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi) do {                      \
   asm("mulhdu %0,%1,%2" : "=r" (*hi) : "r" (a), "r" (b) : "cc"); \
   *lo = a * b;                                                   \
} while(0)

#endif

#endif

namespace Botan {

/**
* Perform a 64x64->128 bit multiplication
*/
inline void mul64x64_128(u64bit a, u64bit b, u64bit* lo, u64bit* hi)
   {
#if defined(BOTAN_FAST_64X64_MUL)
   BOTAN_FAST_64X64_MUL(a, b, lo, hi);
#else

   /*
   * Do a 64x64->128 multiply using four 32x32->64 multiplies plus
   * some adds and shifts. Last resort for CPUs like UltraSPARC (with
   * 64-bit registers/ALU, but no 64x64->128 multiply) or 32-bit CPUs.
   */
   const size_t HWORD_BITS = 32;
   const u32bit HWORD_MASK = 0xFFFFFFFF;

   const u32bit a_hi = (a >> HWORD_BITS);
   const u32bit a_lo = (a  & HWORD_MASK);
   const u32bit b_hi = (b >> HWORD_BITS);
   const u32bit b_lo = (b  & HWORD_MASK);

   u64bit x0 = static_cast<u64bit>(a_hi) * b_hi;
   u64bit x1 = static_cast<u64bit>(a_lo) * b_hi;
   u64bit x2 = static_cast<u64bit>(a_hi) * b_lo;
   u64bit x3 = static_cast<u64bit>(a_lo) * b_lo;

   // this cannot overflow as (2^32-1)^2 + 2^32-1 < 2^64-1
   x2 += x3 >> HWORD_BITS;

   // this one can overflow
   x2 += x1;

   // propagate the carry if any
   x0 += static_cast<u64bit>(static_cast<bool>(x2 < x1)) << HWORD_BITS;

   *hi = x0 + (x2 >> HWORD_BITS);
   *lo  = ((x2 & HWORD_MASK) << HWORD_BITS) + (x3 & HWORD_MASK);
#endif
   }

}


namespace Botan {

/**
* OAEP (called EME1 in IEEE 1363 and in earlier versions of the library)
*/
class BOTAN_DLL OAEP final : public EME
   {
   public:
      size_t maximum_input_size(size_t) const override;

      static OAEP* make(const Spec& spec);

      /**
      * @param hash object to use for hashing (takes ownership)
      * @param P an optional label. Normally empty.
      */
      OAEP(HashFunction* hash, const std::string& P = "");
   private:
      secure_vector<byte> pad(const byte in[],
                              size_t in_length,
                              size_t key_length,
                              RandomNumberGenerator& rng) const override;

      secure_vector<byte> unpad(byte& valid_mask,
                                const byte in[],
                                size_t in_len) const override;

      secure_vector<byte> m_Phash;
      std::unique_ptr<HashFunction> m_hash;
   };

}


namespace Botan {

namespace OIDS {

/**
* Register an OID to string mapping.
* @param oid the oid to register
* @param name the name to be associated with the oid
*/
BOTAN_DLL void add_oid(const OID& oid, const std::string& name);

BOTAN_DLL void add_oid2str(const OID& oid, const std::string& name);
BOTAN_DLL void add_str2oid(const OID& oid, const std::string& name);

BOTAN_DLL void add_oidstr(const char* oidstr, const char* name);

/**
* See if an OID exists in the internal table.
* @param oid the oid to check for
* @return true if the oid is registered
*/
BOTAN_DLL bool have_oid(const std::string& oid);

/**
* Resolve an OID
* @param oid the OID to look up
* @return name associated with this OID
*/
BOTAN_DLL std::string lookup(const OID& oid);

/**
* Find the OID to a name. The lookup will be performed in the
* general OID section of the configuration.
* @param name the name to resolve
* @return OID associated with the specified name
*/
BOTAN_DLL OID lookup(const std::string& name);

/**
* Tests whether the specified OID stands for the specified name.
* @param oid the OID to check
* @param name the name to check
* @return true if the specified OID stands for the specified name
*/
BOTAN_DLL bool name_of(const OID& oid, const std::string& name);

BOTAN_DLL const char* default_oid_list();

}

}


namespace Botan {

/**
* Encrypt with PBES2 from PKCS #5 v2.0
* @param passphrase the passphrase to use for encryption
* @param msec how many milliseconds to run PBKDF2
* @param cipher specifies the block cipher to use to encrypt
* @param digest specifies the PRF to use with PBKDF2 (eg "HMAC(SHA-1)")
* @param rng a random number generator
*/
std::pair<AlgorithmIdentifier, std::vector<byte>>
BOTAN_DLL pbes2_encrypt(const secure_vector<byte>& key_bits,
                        const std::string& passphrase,
                        std::chrono::milliseconds msec,
                        const std::string& cipher,
                        const std::string& digest,
                        RandomNumberGenerator& rng);

/**
* Decrypt a PKCS #5 v2.0 encrypted stream
* @param key_bits the input
* @param passphrase the passphrase to use for decryption
* @param params the PBES2 parameters
*/
secure_vector<byte>
BOTAN_DLL pbes2_decrypt(const secure_vector<byte>& key_bits,
                        const std::string& passphrase,
                        const std::vector<byte>& params);

}


namespace Botan {

/**
* Base class for PBKDF (password based key derivation function)
* implementations. Converts a password into a key using a salt
* and iterated hashing to make brute force attacks harder.
*/
class BOTAN_DLL PBKDF
   {
   public:
      /**
      * Create an instance based on a name
      * Will return a null pointer if the algo/provider combination cannot
      * be found. If provider is empty then best available is chosen.
      */
      static std::unique_ptr<PBKDF> create(const std::string& algo_spec,
                                           const std::string& provider = "");

      /**
      * Returns the list of available providers for this algorithm, empty if not available
      */
      static std::vector<std::string> providers(const std::string& algo_spec);

      typedef SCAN_Name Spec;

      /**
      * @return new instance of this same algorithm
      */
      virtual PBKDF* clone() const = 0;

      virtual std::string name() const = 0;

      virtual ~PBKDF();

      /**
      * Derive a key from a passphrase for a number of iterations
      * specified by either iterations or if iterations == 0 then
      * running until seconds time has elapsed.
      *
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param salt_len length of salt in bytes
      * @param iterations the number of iterations to use (use 10K or more)
      * @param msec if iterations is zero, then instead the PBKDF is
      *        run until msec milliseconds has passed.
      * @return the number of iterations performed
      */
      virtual size_t pbkdf(byte out[], size_t out_len,
                           const std::string& passphrase,
                           const byte salt[], size_t salt_len,
                           size_t iterations,
                           std::chrono::milliseconds msec) const = 0;

      void pbkdf_iterations(byte out[], size_t out_len,
                            const std::string& passphrase,
                            const byte salt[], size_t salt_len,
                            size_t iterations) const;

      void pbkdf_timed(byte out[], size_t out_len,
                         const std::string& passphrase,
                         const byte salt[], size_t salt_len,
                         std::chrono::milliseconds msec,
                         size_t& iterations) const;

      secure_vector<byte> pbkdf_iterations(size_t out_len,
                                           const std::string& passphrase,
                                           const byte salt[], size_t salt_len,
                                           size_t iterations) const;

      secure_vector<byte> pbkdf_timed(size_t out_len,
                                      const std::string& passphrase,
                                      const byte salt[], size_t salt_len,
                                      std::chrono::milliseconds msec,
                                      size_t& iterations) const;

      // Following kept for compat with 1.10:

      /**
      * Derive a key from a passphrase
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param salt_len length of salt in bytes
      * @param iterations the number of iterations to use (use 10K or more)
      */
      OctetString derive_key(size_t out_len,
                             const std::string& passphrase,
                             const byte salt[], size_t salt_len,
                             size_t iterations) const
         {
         return pbkdf_iterations(out_len, passphrase, salt, salt_len, iterations);
         }

      /**
      * Derive a key from a passphrase
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param iterations the number of iterations to use (use 10K or more)
      */
      template<typename Alloc>
      OctetString derive_key(size_t out_len,
                             const std::string& passphrase,
                             const std::vector<byte, Alloc>& salt,
                             size_t iterations) const
         {
         return pbkdf_iterations(out_len, passphrase, salt.data(), salt.size(), iterations);
         }

      /**
      * Derive a key from a passphrase
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param salt_len length of salt in bytes
      * @param msec is how long to run the PBKDF
      * @param iterations is set to the number of iterations used
      */
      OctetString derive_key(size_t out_len,
                             const std::string& passphrase,
                             const byte salt[], size_t salt_len,
                             std::chrono::milliseconds msec,
                             size_t& iterations) const
         {
         return pbkdf_timed(out_len, passphrase, salt, salt_len, msec, iterations);
         }

      /**
      * Derive a key from a passphrase using a certain amount of time
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param msec is how long to run the PBKDF
      * @param iterations is set to the number of iterations used
      */
      template<typename Alloc>
      OctetString derive_key(size_t out_len,
                             const std::string& passphrase,
                             const std::vector<byte, Alloc>& salt,
                             std::chrono::milliseconds msec,
                             size_t& iterations) const
         {
         return pbkdf_timed(out_len, passphrase, salt.data(), salt.size(), msec, iterations);
         }
   };

/**
* Password based key derivation function factory method
* @param algo_spec the name of the desired PBKDF algorithm
* @return pointer to newly allocated object of that type
*/
inline PBKDF* get_pbkdf(const std::string& algo_spec,
                           const std::string& provider = "")
   {
   std::unique_ptr<PBKDF> p(PBKDF::create(algo_spec, provider));
   if(p)
      return p.release();
   throw Algorithm_Not_Found(algo_spec);
   }

}


namespace Botan {

BOTAN_DLL size_t pbkdf2(MessageAuthenticationCode& prf,
                        byte out[],
                        size_t out_len,
                        const std::string& passphrase,
                        const byte salt[], size_t salt_len,
                        size_t iterations,
                        std::chrono::milliseconds msec);

/**
* PKCS #5 PBKDF2
*/
class BOTAN_DLL PKCS5_PBKDF2 final : public PBKDF
   {
   public:
      std::string name() const override
         {
         return "PBKDF2(" + m_mac->name() + ")";
         }

      PBKDF* clone() const override
         {
         return new PKCS5_PBKDF2(m_mac->clone());
         }

      size_t pbkdf(byte output_buf[], size_t output_len,
                   const std::string& passphrase,
                   const byte salt[], size_t salt_len,
                   size_t iterations,
                   std::chrono::milliseconds msec) const override;

      /**
      * Create a PKCS #5 instance using the specified message auth code
      * @param mac_fn the MAC object to use as PRF
      */
      explicit PKCS5_PBKDF2(MessageAuthenticationCode* mac_fn) : m_mac(mac_fn) {}

      static PKCS5_PBKDF2* make(const Spec& spec);
   private:
      std::unique_ptr<MessageAuthenticationCode> m_mac;
   };

}


namespace Botan {

namespace PEM_Code {

/**
* Encode some binary data in PEM format
*/
BOTAN_DLL std::string encode(const byte data[],
                             size_t data_len,
                             const std::string& label,
                             size_t line_width = 64);

/**
* Encode some binary data in PEM format
*/
inline std::string encode(const std::vector<byte>& data,
                          const std::string& label,
                          size_t line_width = 64)
   {
   return encode(data.data(), data.size(), label, line_width);
   }

/**
* Encode some binary data in PEM format
*/
inline std::string encode(const secure_vector<byte>& data,
                          const std::string& label,
                          size_t line_width = 64)
   {
   return encode(data.data(), data.size(), label, line_width);
   }

/**
* Decode PEM data
* @param pem a datasource containing PEM encoded data
* @param label is set to the PEM label found for later inspection
*/
BOTAN_DLL secure_vector<byte> decode(DataSource& pem,
                                     std::string& label);

/**
* Decode PEM data
* @param pem a string containing PEM encoded data
* @param label is set to the PEM label found for later inspection
*/
BOTAN_DLL secure_vector<byte> decode(const std::string& pem,
                                     std::string& label);

/**
* Decode PEM data
* @param pem a datasource containing PEM encoded data
* @param label is what we expect the label to be
*/
BOTAN_DLL secure_vector<byte> decode_check_label(
   DataSource& pem,
   const std::string& label);

/**
* Decode PEM data
* @param pem a string containing PEM encoded data
* @param label is what we expect the label to be
*/
BOTAN_DLL secure_vector<byte> decode_check_label(
   const std::string& pem,
   const std::string& label);

/**
* Heuristic test for PEM data.
*/
BOTAN_DLL bool matches(DataSource& source,
                       const std::string& extra = "",
                       size_t search_range = 4096);

}

}


namespace Botan {

class EME;
class KDF;
class EMSA;

namespace PK_Ops {

template<typename Key>
class PK_Spec
   {
   public:
      PK_Spec(const Key& key, const std::string& pad) :
         m_key(key), m_pad(pad) {}

      std::string algo_name() const { return m_key.algo_name(); }

      std::string as_string() const { return algo_name() + "/" + padding(); }

      const Key& key() const { return m_key; }
      const std::string& padding() const { return m_pad; }
   private:
      const Key& m_key;
      const std::string m_pad;
   };

typedef PK_Spec<Public_Key> PK_Spec_Public_Key;
typedef PK_Spec<Private_Key> PK_Spec_Private_Key;

/**
* Public key encryption interface
*/
class BOTAN_DLL Encryption
   {
   public:
      typedef PK_Spec_Public_Key Spec;

      virtual size_t max_input_bits() const = 0;

      virtual secure_vector<byte> encrypt(const byte msg[],
                                          size_t msg_len,
                                          RandomNumberGenerator& rng) = 0;

      virtual ~Encryption() {}
   };

/**
* Public key decryption interface
*/
class BOTAN_DLL Decryption
   {
   public:
      typedef PK_Spec_Private_Key Spec;

      virtual size_t max_input_bits() const = 0;

      virtual secure_vector<byte> decrypt(byte& valid_mask,
                                          const byte ciphertext[],
                                          size_t ciphertext_len) = 0;

      virtual ~Decryption() {}
   };

/**
* Public key signature verification interface
*/
class BOTAN_DLL Verification
   {
   public:
      typedef PK_Spec_Public_Key Spec;

      /*
      * Add more data to the message currently being signed
      * @param msg the message
      * @param msg_len the length of msg in bytes
      */
      virtual void update(const byte msg[], size_t msg_len) = 0;

      /*
      * Perform a signature operation
      * @param rng a random number generator
      */
      virtual bool is_valid_signature(const byte sig[], size_t sig_len) = 0;

      /**
      * Get the maximum message size in bits supported by this public key.
      * @return maximum message in bits
      */
      virtual size_t max_input_bits() const = 0;

      /**
      * Find out the number of message parts supported by this scheme.
      * @return number of message parts
      */
      virtual size_t message_parts() const { return 1; }

      /**
      * Find out the message part size supported by this scheme/key.
      * @return size of the message parts
      */
      virtual size_t message_part_size() const { return 0; }

      virtual ~Verification() {}
   };

/**
* Public key signature creation interface
*/
class BOTAN_DLL Signature
   {
   public:
      typedef PK_Spec_Private_Key Spec;

      /**
      * Find out the number of message parts supported by this scheme.
      * @return number of message parts
      */
      virtual size_t message_parts() const { return 1; }

      /**
      * Find out the message part size supported by this scheme/key.
      * @return size of the message parts
      */
      virtual size_t message_part_size() const { return 0; }

      /*
      * Add more data to the message currently being signed
      * @param msg the message
      * @param msg_len the length of msg in bytes
      */
      virtual void update(const byte msg[], size_t msg_len) = 0;

      /*
      * Perform a signature operation
      * @param rng a random number generator
      */
      virtual secure_vector<byte> sign(RandomNumberGenerator& rng) = 0;

      virtual ~Signature() {}
   };

/**
* A generic key agreement operation (eg DH or ECDH)
*/
class BOTAN_DLL Key_Agreement
   {
   public:
      typedef PK_Spec_Private_Key Spec;

      virtual secure_vector<byte> agree(size_t key_len,
                                        const byte other_key[], size_t other_key_len,
                                        const byte salt[], size_t salt_len) = 0;

      virtual ~Key_Agreement() {}
   };

/**
* KEM (key encapsulation)
*/
class BOTAN_DLL KEM_Encryption
   {
   public:
      typedef PK_Spec_Public_Key Spec;

      virtual void kem_encrypt(secure_vector<byte>& out_encapsulated_key,
                               secure_vector<byte>& out_shared_key,
                               size_t desired_shared_key_len,
                               Botan::RandomNumberGenerator& rng,
                               const uint8_t salt[],
                               size_t salt_len) = 0;

      virtual ~KEM_Encryption() {}
   };

class BOTAN_DLL KEM_Decryption
   {
   public:
      typedef PK_Spec_Private_Key Spec;

      virtual secure_vector<byte> kem_decrypt(const byte encap_key[],
                                              size_t len,
                                              size_t desired_shared_key_len,
                                              const uint8_t salt[],
                                              size_t salt_len) = 0;

      virtual ~KEM_Decryption() {}
   };

}

}


namespace Botan {

/**
* PKCS #8 General Exception
*/
struct BOTAN_DLL PKCS8_Exception : public Decoding_Error
   {
   explicit PKCS8_Exception(const std::string& error) :
      Decoding_Error("PKCS #8: " + error) {}
   };

/**
* This namespace contains functions for handling PKCS #8 private keys
*/
namespace PKCS8 {

/**
* BER encode a private key
* @param key the private key to encode
* @return BER encoded key
*/
BOTAN_DLL secure_vector<byte> BER_encode(const Private_Key& key);

/**
* Get a string containing a PEM encoded private key.
* @param key the key to encode
* @return encoded key
*/
BOTAN_DLL std::string PEM_encode(const Private_Key& key);

/**
* Encrypt a key using PKCS #8 encryption
* @param key the key to encode
* @param rng the rng to use
* @param pass the password to use for encryption
* @param msec number of milliseconds to run the password derivation
* @param pbe_algo the name of the desired password-based encryption
         algorithm; if empty ("") a reasonable (portable/secure)
         default will be chosen.
* @return encrypted key in binary BER form
*/
BOTAN_DLL std::vector<byte>
BER_encode(const Private_Key& key,
           RandomNumberGenerator& rng,
           const std::string& pass,
           std::chrono::milliseconds msec = std::chrono::milliseconds(300),
           const std::string& pbe_algo = "");

/**
* Get a string containing a PEM encoded private key, encrypting it with a
* password.
* @param key the key to encode
* @param rng the rng to use
* @param pass the password to use for encryption
* @param msec number of milliseconds to run the password derivation
* @param pbe_algo the name of the desired password-based encryption
         algorithm; if empty ("") a reasonable (portable/secure)
         default will be chosen.
* @return encrypted key in PEM form
*/
BOTAN_DLL std::string
PEM_encode(const Private_Key& key,
           RandomNumberGenerator& rng,
           const std::string& pass,
           std::chrono::milliseconds msec = std::chrono::milliseconds(300),
           const std::string& pbe_algo = "");

/**
* Load an encrypted key from a data source.
* @param source the data source providing the encoded key
* @param rng the rng to use
* @param get_passphrase a function that returns passphrases
* @return loaded private key object
*/
BOTAN_DLL Private_Key* load_key(DataSource& source,
                                RandomNumberGenerator& rng,
                                std::function<std::string ()> get_passphrase);

/** Load an encrypted key from a data source.
* @param source the data source providing the encoded key
* @param rng the rng to use
* @param pass the passphrase to decrypt the key
* @return loaded private key object
*/
BOTAN_DLL Private_Key* load_key(DataSource& source,
                                RandomNumberGenerator& rng,
                                const std::string& pass);

/** Load an unencrypted key from a data source.
* @param source the data source providing the encoded key
* @param rng the rng to use
* @return loaded private key object
*/
BOTAN_DLL Private_Key* load_key(DataSource& source,
                                RandomNumberGenerator& rng);

/**
* Load an encrypted key from a file.
* @param filename the path to the file containing the encoded key
* @param rng the rng to use
* @param get_passphrase a function that returns passphrases
* @return loaded private key object
*/
BOTAN_DLL Private_Key* load_key(const std::string& filename,
                                RandomNumberGenerator& rng,
                                std::function<std::string ()> get_passphrase);

/** Load an encrypted key from a file.
* @param filename the path to the file containing the encoded key
* @param rng the rng to use
* @param pass the passphrase to decrypt the key
* @return loaded private key object
*/
BOTAN_DLL Private_Key* load_key(const std::string& filename,
                                RandomNumberGenerator& rng,
                                const std::string& pass);

/** Load an unencrypted key from a file.
* @param filename the path to the file containing the encoded key
* @param rng the rng to use
* @return loaded private key object
*/
BOTAN_DLL Private_Key* load_key(const std::string& filename,
                                RandomNumberGenerator& rng);

/**
* Copy an existing encoded key object.
* @param key the key to copy
* @param rng the rng to use
* @return new copy of the key
*/
BOTAN_DLL Private_Key* copy_key(const Private_Key& key,
                                RandomNumberGenerator& rng);

}

}


namespace Botan {

/**
* PSSR (called EMSA4 in IEEE 1363 and in old versions of the library)
*/
class BOTAN_DLL PSSR final : public EMSA
   {
   public:

      /**
      * @param hash the hash object to use
      */
      explicit PSSR(HashFunction* hash);

      /**
      * @param hash the hash object to use
      * @param salt_size the size of the salt to use in bytes
      */
      PSSR(HashFunction* hash, size_t salt_size);

      static PSSR* make(const Spec& spec);
   private:
      void update(const byte input[], size_t length) override;

      secure_vector<byte> raw_data() override;

      secure_vector<byte> encoding_of(const secure_vector<byte>& msg,
                                      size_t output_bits,
                                      RandomNumberGenerator& rng) override;

      bool verify(const secure_vector<byte>& coded,
                  const secure_vector<byte>& raw,
                  size_t key_bits) override;

      size_t m_SALT_SIZE;
      std::unique_ptr<HashFunction> m_hash;
   };

}


namespace Botan {

/**
* The two types of signature format supported by Botan.
*/
enum Signature_Format { IEEE_1363, DER_SEQUENCE };

/**
* Public Key Encryptor
*/
class BOTAN_DLL PK_Encryptor
   {
   public:

      /**
      * Encrypt a message.
      * @param in the message as a byte array
      * @param length the length of the above byte array
      * @param rng the random number source to use
      * @return encrypted message
      */
      std::vector<byte> encrypt(const byte in[], size_t length,
                                 RandomNumberGenerator& rng) const
         {
         return enc(in, length, rng);
         }

      /**
      * Encrypt a message.
      * @param in the message
      * @param rng the random number source to use
      * @return encrypted message
      */
      template<typename Alloc>
      std::vector<byte> encrypt(const std::vector<byte, Alloc>& in,
                                RandomNumberGenerator& rng) const
         {
         return enc(in.data(), in.size(), rng);
         }

      /**
      * Return the maximum allowed message size in bytes.
      * @return maximum message size in bytes
      */
      virtual size_t maximum_input_size() const = 0;

      PK_Encryptor() {}
      virtual ~PK_Encryptor() {}

      PK_Encryptor(const PK_Encryptor&) = delete;

      PK_Encryptor& operator=(const PK_Encryptor&) = delete;

   private:
      virtual std::vector<byte> enc(const byte[], size_t,
                                    RandomNumberGenerator&) const = 0;
   };

/**
* Public Key Decryptor
*/
class BOTAN_DLL PK_Decryptor
   {
   public:
      /**
      * Decrypt a ciphertext, throwing an exception if the input
      * seems to be invalid (eg due to an accidental or malicious
      * error in the ciphertext).
      *
      * @param in the ciphertext as a byte array
      * @param length the length of the above byte array
      * @return decrypted message
      */
      secure_vector<byte> decrypt(const byte in[], size_t length) const;

      /**
      * Same as above, but taking a vector
      * @param in the ciphertext
      * @return decrypted message
      */
      template<typename Alloc>
      secure_vector<byte> decrypt(const std::vector<byte, Alloc>& in) const
         {
         return decrypt(in.data(), in.size());
         }

      /**
      * Decrypt a ciphertext. If the ciphertext is invalid (eg due to
      * invalid padding) or is not the expected length, instead
      * returns a random string of the expected length. Use to avoid
      * oracle attacks, especially against PKCS #1 v1.5 decryption.
      */
      secure_vector<byte>
      decrypt_or_random(const byte in[],
                        size_t length,
                        size_t expected_pt_len,
                        RandomNumberGenerator& rng) const;

      /**
      * Decrypt a ciphertext. If the ciphertext is invalid (eg due to
      * invalid padding) or is not the expected length, instead
      * returns a random string of the expected length. Use to avoid
      * oracle attacks, especially against PKCS #1 v1.5 decryption.
      *
      * Additionally checks (also in const time) that:
      *    contents[required_content_offsets[i]] == required_content_bytes[i]
      * for 0 <= i < required_contents
      *
      * Used for example in TLS, which encodes the client version in
      * the content bytes: if there is any timing variation the version
      * check can be used as an oracle to recover the key.
      */
      secure_vector<byte>
      decrypt_or_random(const byte in[],
                        size_t length,
                        size_t expected_pt_len,
                        RandomNumberGenerator& rng,
                        const byte required_content_bytes[],
                        const byte required_content_offsets[],
                        size_t required_contents) const;

      PK_Decryptor() {}
      virtual ~PK_Decryptor() = default;

      PK_Decryptor(const PK_Decryptor&) = delete;
      PK_Decryptor& operator=(const PK_Decryptor&) = delete;

   private:
      virtual secure_vector<byte> do_decrypt(byte& valid_mask,
                                             const byte in[], size_t in_len) const = 0;
   };

/**
* Public Key Signer. Use the sign_message() functions for small
* messages. Use multiple calls update() to process large messages and
* generate the signature by finally calling signature().
*/
class BOTAN_DLL PK_Signer
   {
   public:

      /**
      * Construct a PK Signer.
      * @param key the key to use inside this signer
      * @param emsa the EMSA to use
      * An example would be "EMSA1(SHA-224)".
      * @param format the signature format to use
      */
      PK_Signer(const Private_Key& key,
                const std::string& emsa,
                Signature_Format format = IEEE_1363,
                const std::string& provider = "");

      /**
      * Sign a message all in one go
      * @param in the message to sign as a byte array
      * @param length the length of the above byte array
      * @param rng the rng to use
      * @return signature
      */
      std::vector<byte> sign_message(const byte in[], size_t length,
                                     RandomNumberGenerator& rng)
         {
         this->update(in, length);
         return this->signature(rng);
         }

      /**
      * Sign a message.
      * @param in the message to sign
      * @param rng the rng to use
      * @return signature
      */
      std::vector<byte> sign_message(const std::vector<byte>& in,
                                     RandomNumberGenerator& rng)
         { return sign_message(in.data(), in.size(), rng); }

      std::vector<byte> sign_message(const secure_vector<byte>& in,
                                     RandomNumberGenerator& rng)
         { return sign_message(in.data(), in.size(), rng); }

      /**
      * Add a message part (single byte).
      * @param in the byte to add
      */
      void update(byte in) { update(&in, 1); }

      /**
      * Add a message part.
      * @param in the message part to add as a byte array
      * @param length the length of the above byte array
      */
      void update(const byte in[], size_t length);

      /**
      * Add a message part.
      * @param in the message part to add
      */
      void update(const std::vector<byte>& in) { update(in.data(), in.size()); }

      /**
      * Add a message part.
      * @param in the message part to add
      */
      void update(const std::string& in)
         {
         update(reinterpret_cast<const byte*>(in.data()), in.size());
         }

      /**
      * Get the signature of the so far processed message (provided by the
      * calls to update()).
      * @param rng the rng to use
      * @return signature of the total message
      */
      std::vector<byte> signature(RandomNumberGenerator& rng);

      /**
      * Set the output format of the signature.
      * @param format the signature format to use
      */
      void set_output_format(Signature_Format format) { m_sig_format = format; }
   private:
      std::unique_ptr<PK_Ops::Signature> m_op;
      Signature_Format m_sig_format;
   };

/**
* Public Key Verifier. Use the verify_message() functions for small
* messages. Use multiple calls update() to process large messages and
* verify the signature by finally calling check_signature().
*/
class BOTAN_DLL PK_Verifier
   {
   public:
      /**
      * Construct a PK Verifier.
      * @param pub_key the public key to verify against
      * @param emsa the EMSA to use (eg "EMSA3(SHA-1)")
      * @param format the signature format to use
      */
      PK_Verifier(const Public_Key& pub_key,
                  const std::string& emsa,
                  Signature_Format format = IEEE_1363,
                  const std::string& provider = "");

      /**
      * Verify a signature.
      * @param msg the message that the signature belongs to, as a byte array
      * @param msg_length the length of the above byte array msg
      * @param sig the signature as a byte array
      * @param sig_length the length of the above byte array sig
      * @return true if the signature is valid
      */
      bool verify_message(const byte msg[], size_t msg_length,
                          const byte sig[], size_t sig_length);
      /**
      * Verify a signature.
      * @param msg the message that the signature belongs to
      * @param sig the signature
      * @return true if the signature is valid
      */
      template<typename Alloc, typename Alloc2>
      bool verify_message(const std::vector<byte, Alloc>& msg,
                          const std::vector<byte, Alloc2>& sig)
         {
         return verify_message(msg.data(), msg.size(),
                               sig.data(), sig.size());
         }

      /**
      * Add a message part (single byte) of the message corresponding to the
      * signature to be verified.
      * @param in the byte to add
      */
      void update(byte in) { update(&in, 1); }

      /**
      * Add a message part of the message corresponding to the
      * signature to be verified.
      * @param msg_part the new message part as a byte array
      * @param length the length of the above byte array
      */
      void update(const byte msg_part[], size_t length);

      /**
      * Add a message part of the message corresponding to the
      * signature to be verified.
      * @param in the new message part
      */
      void update(const std::vector<byte>& in)
         { update(in.data(), in.size()); }

      /**
      * Add a message part of the message corresponding to the
      * signature to be verified.
      */
      void update(const std::string& in)
         {
         update(reinterpret_cast<const byte*>(in.data()), in.size());
         }

      /**
      * Check the signature of the buffered message, i.e. the one build
      * by successive calls to update.
      * @param sig the signature to be verified as a byte array
      * @param length the length of the above byte array
      * @return true if the signature is valid, false otherwise
      */
      bool check_signature(const byte sig[], size_t length);

      /**
      * Check the signature of the buffered message, i.e. the one build
      * by successive calls to update.
      * @param sig the signature to be verified
      * @return true if the signature is valid, false otherwise
      */
      template<typename Alloc>
      bool check_signature(const std::vector<byte, Alloc>& sig)
         {
         return check_signature(sig.data(), sig.size());
         }

      /**
      * Set the format of the signatures fed to this verifier.
      * @param format the signature format to use
      */
      void set_input_format(Signature_Format format);

   private:
      std::unique_ptr<PK_Ops::Verification> m_op;
      Signature_Format m_sig_format;
   };

/**
* Key used for key agreement
*/
class BOTAN_DLL PK_Key_Agreement
   {
   public:

      /**
      * Construct a PK Key Agreement.
      * @param key the key to use
      * @param kdf name of the KDF to use (or 'Raw' for no KDF)
      * @param provider the algo provider to use (or empty for default)
      */
      PK_Key_Agreement(const Private_Key& key,
                       const std::string& kdf,
                       const std::string& provider = "");

      /*
      * Perform Key Agreement Operation
      * @param key_len the desired key output size
      * @param in the other parties key
      * @param in_len the length of in in bytes
      * @param params extra derivation params
      * @param params_len the length of params in bytes
      */
      SymmetricKey derive_key(size_t key_len,
                              const byte in[],
                              size_t in_len,
                              const byte params[],
                              size_t params_len) const;

      /*
      * Perform Key Agreement Operation
      * @param key_len the desired key output size
      * @param in the other parties key
      * @param in_len the length of in in bytes
      * @param params extra derivation params
      * @param params_len the length of params in bytes
      */
      SymmetricKey derive_key(size_t key_len,
                              const std::vector<byte>& in,
                              const byte params[],
                              size_t params_len) const
         {
         return derive_key(key_len, in.data(), in.size(),
                           params, params_len);
         }

      /*
      * Perform Key Agreement Operation
      * @param key_len the desired key output size
      * @param in the other parties key
      * @param in_len the length of in in bytes
      * @param params extra derivation params
      */
      SymmetricKey derive_key(size_t key_len,
                              const byte in[], size_t in_len,
                              const std::string& params = "") const
         {
         return derive_key(key_len, in, in_len,
                           reinterpret_cast<const byte*>(params.data()),
                           params.length());
         }

      /*
      * Perform Key Agreement Operation
      * @param key_len the desired key output size
      * @param in the other parties key
      * @param params extra derivation params
      */
      SymmetricKey derive_key(size_t key_len,
                              const std::vector<byte>& in,
                              const std::string& params = "") const
         {
         return derive_key(key_len, in.data(), in.size(),
                           reinterpret_cast<const byte*>(params.data()),
                           params.length());
         }

   private:
      std::unique_ptr<PK_Ops::Key_Agreement> m_op;
   };

/**
* Encryption using a standard message recovery algorithm like RSA or
* ElGamal, paired with an encoding scheme like OAEP.
*/
class BOTAN_DLL PK_Encryptor_EME : public PK_Encryptor
   {
   public:
      size_t maximum_input_size() const override;

      /**
      * Construct an instance.
      * @param key the key to use inside the decryptor
      * @param padding the message encoding scheme to use (eg "OAEP(SHA-256)")
      */
      PK_Encryptor_EME(const Public_Key& key,
                       const std::string& padding,
                       const std::string& provider = "");
   private:
      std::vector<byte> enc(const byte[], size_t,
                             RandomNumberGenerator& rng) const override;

      std::unique_ptr<PK_Ops::Encryption> m_op;
   };

/**
* Decryption with an MR algorithm and an EME.
*/
class BOTAN_DLL PK_Decryptor_EME : public PK_Decryptor
   {
   public:
     /**
      * Construct an instance.
      * @param key the key to use inside the encryptor
      * @param eme the EME to use
      */
      PK_Decryptor_EME(const Private_Key& key,
                       const std::string& eme,
                       const std::string& provider = "");
   private:
      secure_vector<byte> do_decrypt(byte& valid_mask,
                                     const byte in[],
                                     size_t in_len) const override;

      std::unique_ptr<PK_Ops::Decryption> m_op;
   };

class BOTAN_DLL PK_KEM_Encryptor
   {
   public:
      PK_KEM_Encryptor(const Public_Key& key,
                       const std::string& kem_param = "",
                       const std::string& provider = "");

      void encrypt(secure_vector<byte>& out_encapsulated_key,
                   secure_vector<byte>& out_shared_key,
                   size_t desired_shared_key_len,
                   Botan::RandomNumberGenerator& rng,
                   const uint8_t salt[],
                   size_t salt_len);

      template<typename Alloc>
         void encrypt(secure_vector<byte>& out_encapsulated_key,
                      secure_vector<byte>& out_shared_key,
                      size_t desired_shared_key_len,
                      Botan::RandomNumberGenerator& rng,
                      const std::vector<uint8_t, Alloc>& salt)
         {
         this->encrypt(out_encapsulated_key,
                       out_shared_key,
                       desired_shared_key_len,
                       rng,
                       salt.data(), salt.size());
         }

      void encrypt(secure_vector<byte>& out_encapsulated_key,
                   secure_vector<byte>& out_shared_key,
                   size_t desired_shared_key_len,
                   Botan::RandomNumberGenerator& rng)
         {
         this->encrypt(out_encapsulated_key,
                       out_shared_key,
                       desired_shared_key_len,
                       rng,
                       nullptr,
                       0);
         }

   private:
      std::unique_ptr<PK_Ops::KEM_Encryption> m_op;
   };

class BOTAN_DLL PK_KEM_Decryptor
   {
   public:
      PK_KEM_Decryptor(const Private_Key& key,
                       const std::string& kem_param = "",
                       const std::string& provider = "");

      secure_vector<byte> decrypt(const byte encap_key[],
                                  size_t encap_key_len,
                                  size_t desired_shared_key_len,
                                  const uint8_t salt[],
                                  size_t salt_len);

      secure_vector<byte> decrypt(const byte encap_key[],
                                  size_t encap_key_len,
                                  size_t desired_shared_key_len)
         {
         return this->decrypt(encap_key, encap_key_len,
                              desired_shared_key_len,
                              nullptr, 0);
         }

      template<typename Alloc1, typename Alloc2>
         secure_vector<byte> decrypt(const std::vector<byte, Alloc1>& encap_key,
                                     size_t desired_shared_key_len,
                                     const std::vector<byte, Alloc2>& salt)
         {
         return this->decrypt(encap_key.data(), encap_key.size(),
                              desired_shared_key_len,
                              salt.data(), salt.size());
         }

   private:
      std::unique_ptr<PK_Ops::KEM_Decryption> m_op;
   };

}



namespace Botan {

/**
* RSA Public Key
*/
class BOTAN_DLL RSA_PublicKey : public virtual IF_Scheme_PublicKey
   {
   public:
      std::string algo_name() const override { return "RSA"; }

      RSA_PublicKey(const AlgorithmIdentifier& alg_id,
                    const secure_vector<byte>& key_bits) :
         IF_Scheme_PublicKey(alg_id, key_bits)
         {}

      /**
      * Create a RSA_PublicKey
      * @arg n the modulus
      * @arg e the exponent
      */
      RSA_PublicKey(const BigInt& n, const BigInt& e) :
         IF_Scheme_PublicKey(n, e)
         {}

   protected:
      RSA_PublicKey() {}
   };

/**
* RSA Private Key
*/
class BOTAN_DLL RSA_PrivateKey : public RSA_PublicKey,
                                 public IF_Scheme_PrivateKey
   {
   public:
      bool check_key(RandomNumberGenerator& rng, bool) const override;

      RSA_PrivateKey(const AlgorithmIdentifier& alg_id,
                     const secure_vector<byte>& key_bits,
                     RandomNumberGenerator& rng) :
         IF_Scheme_PrivateKey(rng, alg_id, key_bits) {}

      /**
      * Construct a private key from the specified parameters.
      * @param rng a random number generator
      * @param p the first prime
      * @param q the second prime
      * @param e the exponent
      * @param d if specified, this has to be d with
      * exp * d = 1 mod (p - 1, q - 1). Leave it as 0 if you wish to
      * the constructor to calculate it.
      * @param n if specified, this must be n = p * q. Leave it as 0
      * if you wish to the constructor to calculate it.
      */
      RSA_PrivateKey(RandomNumberGenerator& rng,
                     const BigInt& p, const BigInt& q,
                     const BigInt& e, const BigInt& d = 0,
                     const BigInt& n = 0) :
         IF_Scheme_PrivateKey(rng, p, q, e, d, n) {}

      /**
      * Create a new private key with the specified bit length
      * @param rng the random number generator to use
      * @param bits the desired bit length of the private key
      * @param exp the public exponent to be used
      */
      RSA_PrivateKey(RandomNumberGenerator& rng,
                     size_t bits, size_t exp = 65537);
   };

}


namespace Botan {

/**
* A queue that knows how to zeroize itself
*/
class BOTAN_DLL SecureQueue : public Fanout_Filter, public DataSource
   {
   public:
      std::string name() const override { return "Queue"; }

      void write(const byte[], size_t) override;

      size_t read(byte[], size_t) override;
      size_t peek(byte[], size_t, size_t = 0) const override;
      size_t get_bytes_read() const override;

      bool end_of_data() const override;

      bool empty() const;

      bool check_available(size_t n) override { return n <= size(); }

      /**
      * @return number of bytes available in the queue
      */
      size_t size() const;

      bool attachable() override { return false; }

      /**
      * SecureQueue assignment
      * @param other the queue to copy
      */
      SecureQueue& operator=(const SecureQueue& other);

      /**
      * SecureQueue default constructor (creates empty queue)
      */
      SecureQueue();

      /**
      * SecureQueue copy constructor
      * @param other the queue to copy
      */
      SecureQueue(const SecureQueue& other);

      ~SecureQueue() { destroy(); }

   private:
      void destroy();
      size_t m_bytes_read;
      class SecureQueueNode* m_head;
      class SecureQueueNode* m_tail;
   };

}


namespace Botan {

/**
* NIST's SHA-160
*/
class BOTAN_DLL SHA_160 : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "SHA-160"; }
      size_t output_length() const override { return 20; }
      HashFunction* clone() const override { return new SHA_160; }

      void clear() override;

      SHA_160() : MDx_HashFunction(64, true, true), m_digest(5), m_W(80)
         {
         clear();
         }
   protected:
      /**
      * Set a custom size for the W array. Normally 80, but some
      * subclasses need slightly more for best performance/internal
      * constraints
      * @param W_size how big to make W
      */
      explicit SHA_160(size_t W_size) :
         MDx_HashFunction(64, true, true), m_digest(5), m_W(W_size)
         {
         clear();
         }

      void compress_n(const byte[], size_t blocks) override;
      void copy_out(byte[]) override;

      /**
      * The digest value, exposed for use by subclasses (asm, SSE2)
      */
      secure_vector<u32bit> m_digest;

      /**
      * The message buffer, exposed for use by subclasses (asm, SSE2)
      */
      secure_vector<u32bit> m_W;
   };

}


namespace Botan {

/**
* SHA-224
*/
class BOTAN_DLL SHA_224 final : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "SHA-224"; }
      size_t output_length() const override { return 28; }
      HashFunction* clone() const override { return new SHA_224; }

      void clear() override;

      SHA_224() : MDx_HashFunction(64, true, true), m_digest(8)
         { clear(); }
   private:
      void compress_n(const byte[], size_t blocks) override;
      void copy_out(byte[]) override;

      secure_vector<u32bit> m_digest;
   };

/**
* SHA-256
*/
class BOTAN_DLL SHA_256 final : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "SHA-256"; }
      size_t output_length() const override { return 32; }
      HashFunction* clone() const override { return new SHA_256; }

      void clear() override;

      SHA_256() : MDx_HashFunction(64, true, true), m_digest(8)
         { clear(); }
   private:
      void compress_n(const byte[], size_t blocks) override;
      void copy_out(byte[]) override;

      secure_vector<u32bit> m_digest;
   };

}


namespace Botan {

/**
* SHA-384
*/
class BOTAN_DLL SHA_384 final : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "SHA-384"; }
      size_t output_length() const override { return 48; }
      HashFunction* clone() const override { return new SHA_384; }

      void clear() override;

      SHA_384() : MDx_HashFunction(128, true, true, 16), m_digest(8)
         { clear(); }
   private:
      void compress_n(const byte[], size_t blocks) override;
      void copy_out(byte[]) override;

      secure_vector<u64bit> m_digest;
   };

/**
* SHA-512
*/
class BOTAN_DLL SHA_512 final : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "SHA-512"; }
      size_t output_length() const override { return 64; }
      HashFunction* clone() const override { return new SHA_512; }

      void clear() override;

      SHA_512() : MDx_HashFunction(128, true, true, 16), m_digest(8)
         { clear(); }
   private:
      void compress_n(const byte[], size_t blocks) override;
      void copy_out(byte[]) override;

      secure_vector<u64bit> m_digest;
   };

/**
* SHA-512/256
*/
class BOTAN_DLL SHA_512_256 final : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "SHA-512-256"; }
      size_t output_length() const override { return 32; }
      HashFunction* clone() const override { return new SHA_512_256; }

      void clear() override;

      SHA_512_256() : MDx_HashFunction(128, true, true, 16), m_digest(8) { clear(); }
   private:
      void compress_n(const byte[], size_t blocks) override;
      void copy_out(byte[]) override;

      secure_vector<u64bit> m_digest;
   };

}


namespace Botan {

/**
* SRP6a Client side
* @param username the username we are attempting login for
* @param password the password we are attempting to use
* @param group_id specifies the shared SRP group
* @param hash_id specifies a secure hash function
* @param salt is the salt value sent by the server
* @param B is the server's public value
* @param rng is a random number generator
*
* @return (A,K) the client public key and the shared secret key
*/
std::pair<BigInt,SymmetricKey>
BOTAN_DLL srp6_client_agree(const std::string& username,
                            const std::string& password,
                            const std::string& group_id,
                            const std::string& hash_id,
                            const std::vector<byte>& salt,
                            const BigInt& B,
                            RandomNumberGenerator& rng);

/**
* Generate a new SRP-6 verifier
* @param identifier a username or other client identifier
* @param password the secret used to authenticate user
* @param salt a randomly chosen value, at least 128 bits long
* @param group_id specifies the shared SRP group
* @param hash_id specifies a secure hash function
*/
BigInt BOTAN_DLL generate_srp6_verifier(const std::string& identifier,
                                        const std::string& password,
                                        const std::vector<byte>& salt,
                                        const std::string& group_id,
                                        const std::string& hash_id);

/**
* Return the group id for this SRP param set, or else thrown an
* exception
* @param N the group modulus
* @param g the group generator
* @return group identifier
*/
std::string BOTAN_DLL srp6_group_identifier(const BigInt& N, const BigInt& g);

/**
* Represents a SRP-6a server session
*/
class BOTAN_DLL SRP6_Server_Session
   {
   public:
      /**
      * Server side step 1
      * @param v the verification value saved from client registration
      * @param group_id the SRP group id
      * @param hash_id the SRP hash in use
      * @param rng a random number generator
      * @return SRP-6 B value
      */
      BigInt step1(const BigInt& v,
                   const std::string& group_id,
                   const std::string& hash_id,
                   RandomNumberGenerator& rng);

      /**
      * Server side step 2
      * @param A the client's value
      * @return shared symmetric key
      */
      SymmetricKey step2(const BigInt& A);

   private:
      std::string m_hash_id;
      BigInt m_B, m_b, m_v, m_S, m_p;
      size_t m_p_bytes = 0;
   };

}


namespace Botan {

/**
* A GnuTLS compatible SRP6 authenticator file
*/
class BOTAN_DLL SRP6_Authenticator_File
   {
   public:
      /**
      * @param filename will be opened and processed as a SRP
      * authenticator file
      */
      explicit SRP6_Authenticator_File(const std::string& filename);

      bool lookup_user(const std::string& username,
                       BigInt& v,
                       std::vector<byte>& salt,
                       std::string& group_id) const;
   private:
      struct SRP6_Data
         {
         SRP6_Data() {}

         SRP6_Data(const BigInt& v_,
                   const std::vector<byte>& salt_,
                   const std::string& group_id_) :
            v(v_), salt(salt_), group_id(group_id_) {}

         // public member variable:
         BigInt v;

         // public member variable:
         std::vector<byte> salt;

         // public member variable:
         std::string group_id;
         };

      std::map<std::string, SRP6_Data> m_entries;
   };

}


namespace Botan {

class BOTAN_DLL Stream_Cipher_Mode : public Cipher_Mode
   {
   public:
      explicit Stream_Cipher_Mode(StreamCipher* cipher) : m_cipher(cipher) {}

      void update(secure_vector<byte>& buf, size_t offset) override
         {
         if(offset < buf.size())
            m_cipher->cipher1(&buf[offset], buf.size() - offset);
         }

      void finish(secure_vector<byte>& buf, size_t offset) override
         { return update(buf, offset); }

      size_t output_length(size_t input_length) const override { return input_length; }

      size_t update_granularity() const override { return 64; /* arbitrary */ }

      size_t minimum_final_size() const override { return 0; }

      size_t default_nonce_length() const override { return 0; }

      bool valid_nonce_length(size_t nonce_len) const override
         { return m_cipher->valid_iv_length(nonce_len); }

      Key_Length_Specification key_spec() const override { return m_cipher->key_spec(); }

      std::string name() const override { return m_cipher->name(); }

      void clear() override { return m_cipher->clear(); }

   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override
         {
         m_cipher->set_iv(nonce, nonce_len);
         return secure_vector<byte>();
         }

      void key_schedule(const byte key[], size_t length) override
         {
         m_cipher->set_key(key, length);
         }

      std::unique_ptr<StreamCipher> m_cipher;
   };

}


namespace Botan {

/**
* Return a shared reference to a global PRNG instance provided by the
* operating system. For instance might be instantiated by /dev/urandom
* or CryptGenRandom.
*/
BOTAN_DLL RandomNumberGenerator& system_rng();

/*
* Instantiatable reference to the system RNG.
*/
class BOTAN_DLL System_RNG : public RandomNumberGenerator
   {
   public:
      System_RNG() : m_rng(system_rng()) {}

      void randomize(Botan::byte out[], size_t len) override { m_rng.randomize(out, len); }

      bool is_seeded() const override { return m_rng.is_seeded(); }

      void clear() override { m_rng.clear(); }

      std::string name() const override { return m_rng.name(); }

      size_t reseed_with_sources(Entropy_Sources& srcs,
                                         size_t poll_bits,
                                         std::chrono::milliseconds poll_timeout) override
         {
         return m_rng.reseed_with_sources(srcs, poll_bits, poll_timeout);
         }

      void add_entropy(const byte in[], size_t len) override { m_rng.add_entropy(in, len); }
   private:
      Botan::RandomNumberGenerator& m_rng;
   };

}


namespace Botan {

/**
* Estimate work factor for discrete logarithm
* @param prime_group_size size of the group in bits
* @return estimated security level for this group
*/
size_t dl_work_factor(size_t prime_group_size);

/**
* Return the appropriate exponent size to use for a particular prime
* group. This is twice the size of the estimated cost of breaking the
* key using an index calculus attack; the assumption is that if an
* arbitrary discrete log on a group of size bits would take about 2^n
* effort, and thus using an exponent of size 2^(2*n) implies that all
* available attacks are about as easy (as e.g Pollard's kangaroo
* algorithm can compute the DL in sqrt(x) operations) while minimizing
* the exponent size for performance reasons.
*/
size_t dl_exponent_size(size_t prime_group_size);

/**
* Estimate work factor for integer factorization
* @param n_bits size of modulus in bits
* @return estimated security level for this modulus
*/
size_t if_work_factor(size_t n_bits);

/**
* Estimate work factor for EC discrete logarithm
* @param prime_group_size size of the group in bits
* @return estimated security level for this group
*/
size_t ecp_work_factor(size_t prime_group_size);

}


namespace Botan {

/**
* Distinguished Name
*/
class BOTAN_DLL X509_DN final : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;

      std::multimap<OID, std::string> get_attributes() const;
      std::vector<std::string> get_attribute(const std::string&) const;

      std::multimap<std::string, std::string> contents() const;

      void add_attribute(const std::string&, const std::string&);
      void add_attribute(const OID&, const std::string&);

      static std::string deref_info_field(const std::string&);

      std::vector<byte> get_bits() const;

      X509_DN();
      explicit X509_DN(const std::multimap<OID, std::string>&);
      explicit X509_DN(const std::multimap<std::string, std::string>&);
   private:
      std::multimap<OID, ASN1_String> m_dn_info;
      std::vector<byte> m_dn_bits;
   };

bool BOTAN_DLL operator==(const X509_DN&, const X509_DN&);
bool BOTAN_DLL operator!=(const X509_DN&, const X509_DN&);
bool BOTAN_DLL operator<(const X509_DN&, const X509_DN&);

BOTAN_DLL std::ostream& operator<<(std::ostream& out, const X509_DN& dn);
BOTAN_DLL std::istream& operator>>(std::istream& in, X509_DN& dn);

}


namespace Botan {

/**
* Zlib Compression
*/
class BOTAN_DLL Zlib_Compression final : public Stream_Compression
   {
   public:
      std::string name() const override { return "Zlib_Compression"; }
   private:
      Compression_Stream* make_stream(size_t level) const override;
   };

/**
* Zlib Decompression
*/
class BOTAN_DLL Zlib_Decompression final : public Stream_Decompression
   {
   public:
      std::string name() const override { return "Zlib_Decompression"; }
   private:
      Compression_Stream* make_stream() const override;
   };

/**
* Deflate Compression
*/
class BOTAN_DLL Deflate_Compression final : public Stream_Compression
   {
   public:
      std::string name() const override { return "Deflate_Compression"; }
   private:
      Compression_Stream* make_stream(size_t level) const override;
   };

/**
* Deflate Decompression
*/
class BOTAN_DLL Deflate_Decompression final : public Stream_Decompression
   {
   public:
      std::string name() const override { return "Deflate_Decompression"; }
   private:
      Compression_Stream* make_stream() const override;
   };

/**
* Gzip Compression
*/
class BOTAN_DLL Gzip_Compression final : public Stream_Compression
   {
   public:
      Gzip_Compression(byte os_code = 255) : m_os_code(os_code) {}

      std::string name() const override { return "Gzip_Compression"; }
   private:
      Compression_Stream* make_stream(size_t level) const override;
      const byte m_os_code;
   };

/**
* Gzip Decompression
*/
class BOTAN_DLL Gzip_Decompression final : public Stream_Decompression
   {
   public:
      std::string name() const override { return "Gzip_Decompression"; }
   private:
      Compression_Stream* make_stream() const override;
   };

}


#endif
