
#ifndef BOTAN_AMALGAMATION_INTERNAL_H__
#define BOTAN_AMALGAMATION_INTERNAL_H__

#include <algorithm>
#include <condition_variable>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>


#if defined(_MSC_VER) && (_MSC_VER <= 1800)

   #define BOTAN_WORKAROUND_GH_321
   #define NOMINMAX 1
   #define WIN32_LEAN_AND_MEAN 1
   #include <windows.h>

#endif

namespace Botan {

#if defined(BOTAN_WORKAROUND_GH_321)

class WinCS_Mutex
   {
   public:
      WinCS_Mutex()
         {
         ::InitializeCriticalSection(&m_cs);
         }

      ~WinCS_Mutex()
         {
         ::DeleteCriticalSection(&m_cs);
         }

      void lock()
         {
         ::EnterCriticalSection(&m_cs);
         }

      void unlock()
         {
         ::LeaveCriticalSection(&m_cs);
         }

   private:
      CRITICAL_SECTION m_cs;
   };

#endif

template<typename T>
class Algo_Registry
   {
   public:
      typedef typename T::Spec Spec;

      typedef std::function<T* (const Spec&)> maker_fn;

      static Algo_Registry<T>& global_registry()
         {
         static Algo_Registry<T> g_registry;
         return g_registry;
         }

      void add(const std::string& name, const std::string& provider, maker_fn fn, byte pref)
         {
         std::lock_guard<mutex> lock(m_mutex);
         if(!m_algo_info[name].add_provider(provider, fn, pref))
            throw Exception("Duplicated registration of " + name + "/" + provider);
         }

      std::vector<std::string> providers_of(const Spec& spec)
         {
         std::lock_guard<mutex> lock(m_mutex);
         auto i = m_algo_info.find(spec.algo_name());
         if(i != m_algo_info.end())
            return i->second.providers();
         return std::vector<std::string>();
         }

      void set_provider_preference(const Spec& spec, const std::string& provider, byte pref)
         {
         std::lock_guard<mutex> lock(m_mutex);
         auto i = m_algo_info.find(spec.algo_name());
         if(i != m_algo_info.end())
            i->second.set_pref(provider, pref);
         }

      T* make(const Spec& spec, const std::string& provider = "")
         {
         const std::vector<maker_fn> makers = get_makers(spec, provider);

         try
            {
            for(auto&& maker : makers)
               {
               if(T* t = maker(spec))
                  return t;
               }
            }
         catch(std::exception& e)
            {
            throw Lookup_Error("Creating '" + spec.as_string() + "' failed: " + e.what());
            }

         return nullptr;
         }

      class Add
         {
         public:
            Add(const std::string& basename, maker_fn fn, const std::string& provider, byte pref)
               {
               Algo_Registry<T>::global_registry().add(basename, provider, fn, pref);
               }

            Add(bool cond, const std::string& basename, maker_fn fn, const std::string& provider, byte pref)
               {
               if(cond)
                  Algo_Registry<T>::global_registry().add(basename, provider, fn, pref);
               }
         };

   private:

#if defined(BOTAN_WORKAROUND_GH_321)
      using mutex = WinCS_Mutex;
#else
      using mutex = std::mutex;
#endif

      Algo_Registry()  { }

      std::vector<maker_fn> get_makers(const Spec& spec, const std::string& provider)
         {
         std::lock_guard<mutex> lock(m_mutex);
         return m_algo_info[spec.algo_name()].get_makers(provider);
         }

      struct Algo_Info
         {
         public:
            bool add_provider(const std::string& provider, maker_fn fn, byte pref)
               {
               if(m_maker_fns.count(provider) > 0)
                  return false;

               m_maker_fns[provider] = fn;
               m_prefs.insert(std::make_pair(pref, provider));
               return true;
               }

            std::vector<std::string> providers() const
               {
               std::vector<std::string> v;
               for(auto&& k : m_prefs)
                  v.push_back(k.second);
               return v;
               }

            void set_pref(const std::string& provider, byte pref)
               {
               auto i = m_prefs.begin();
               while(i != m_prefs.end())
                  {
                  if(i->second == provider)
                     i = m_prefs.erase(i);
                  else
                     ++i;
                  }
               m_prefs.insert(std::make_pair(pref, provider));
               }

            std::vector<maker_fn> get_makers(const std::string& req_provider)
               {
               std::vector<maker_fn> r;

               if(!req_provider.empty())
                  {
                  // find one explicit provider requested by user or fail
                  auto i = m_maker_fns.find(req_provider);
                  if(i != m_maker_fns.end())
                     r.push_back(i->second);
                  }
               else
                  {
                  for(auto&& pref : m_prefs)
                     r.push_back(m_maker_fns[pref.second]);
                  }

               return r;
               }
         private:
            std::multimap<byte, std::string, std::greater<byte>> m_prefs;
            std::unordered_map<std::string, maker_fn> m_maker_fns;
         };

      mutex m_mutex;
      std::unordered_map<std::string, Algo_Info> m_algo_info;
   };

template<typename T> T*
make_a(const typename T::Spec& spec, const std::string& provider = "")
   {
   return Algo_Registry<T>::global_registry().make(spec, provider);
   }

template<typename T> std::vector<std::string> providers_of(const typename T::Spec& spec)
   {
   return Algo_Registry<T>::global_registry().providers_of(spec);
   }

template<typename T> T*
make_new_T(const typename Algo_Registry<T>::Spec& spec)
   {
   if(spec.arg_count() == 0)
      return new T;
   return nullptr;
   }

template<typename T, size_t DEF_VAL> T*
make_new_T_1len(const typename Algo_Registry<T>::Spec& spec)
   {
   return new T(spec.arg_as_integer(0, DEF_VAL));
   }

template<typename T, size_t DEF1, size_t DEF2> T*
make_new_T_2len(const typename Algo_Registry<T>::Spec& spec)
   {
   return new T(spec.arg_as_integer(0, DEF1), spec.arg_as_integer(1, DEF2));
   }

template<typename T> T*
make_new_T_1str(const typename Algo_Registry<T>::Spec& spec, const std::string& def)
   {
   return new T(spec.arg(0, def));
   }

template<typename T> T*
make_new_T_1str_req(const typename Algo_Registry<T>::Spec& spec)
   {
   return new T(spec.arg(0));
   }

template<typename T, typename X> T*
make_new_T_1X(const typename Algo_Registry<T>::Spec& spec)
   {
   std::unique_ptr<X> x(Algo_Registry<X>::global_registry().make(Botan::SCAN_Name(spec.arg(0))));
   if(!x)
      throw Exception(spec.arg(0));
   return new T(x.release());
   }

#define BOTAN_REGISTER_TYPE(T, type, name, maker, provider, pref)        \
   namespace { Algo_Registry<T>::Add g_ ## type ## _reg(name, maker, provider, pref); } \
   BOTAN_FORCE_SEMICOLON

#define BOTAN_REGISTER_TYPE_COND(cond, T, type, name, maker, provider, pref) \
   namespace { Algo_Registry<T>::Add g_ ## type ## _reg(cond, name, maker, provider, pref); } \
   BOTAN_FORCE_SEMICOLON

#define BOTAN_DEFAULT_ALGORITHM_PRIO 100
#define BOTAN_SIMD_ALGORITHM_PRIO    110

#define BOTAN_REGISTER_NAMED_T(T, name, type, maker)                 \
   BOTAN_REGISTER_TYPE(T, type, name, maker, "base", BOTAN_DEFAULT_ALGORITHM_PRIO)

#define BOTAN_REGISTER_T(T, type, maker)                                \
   BOTAN_REGISTER_TYPE(T, type, #type, maker, "base", BOTAN_DEFAULT_ALGORITHM_PRIO)

#define BOTAN_REGISTER_T_NOARGS(T, type) \
   BOTAN_REGISTER_TYPE(T, type, #type, make_new_T<type>, "base", BOTAN_DEFAULT_ALGORITHM_PRIO)
#define BOTAN_REGISTER_T_1LEN(T, type, def) \
   BOTAN_REGISTER_TYPE(T, type, #type, (make_new_T_1len<type,def>), "base", BOTAN_DEFAULT_ALGORITHM_PRIO)

#define BOTAN_REGISTER_NAMED_T_NOARGS(T, type, name, provider) \
   BOTAN_REGISTER_TYPE(T, type, name, make_new_T<type>, provider, BOTAN_DEFAULT_ALGORITHM_PRIO)
#define BOTAN_COND_REGISTER_NAMED_T_NOARGS(cond, T, type, name, provider, pref) \
   BOTAN_REGISTER_TYPE_COND(cond, T, type, name, make_new_T<type>, provider, pref)

#define BOTAN_REGISTER_NAMED_T_2LEN(T, type, name, provider, len1, len2) \
   BOTAN_REGISTER_TYPE(T, type, name, (make_new_T_2len<type,len1,len2>), provider, BOTAN_DEFAULT_ALGORITHM_PRIO)

// TODO move elsewhere:
#define BOTAN_REGISTER_TRANSFORM(name, maker) BOTAN_REGISTER_T(Transform, name, maker)
#define BOTAN_REGISTER_TRANSFORM_NOARGS(name) BOTAN_REGISTER_T_NOARGS(Transform, name)

}


namespace Botan {

/**
* Power of 2 test. T should be an unsigned integer type
* @param arg an integer value
* @return true iff arg is 2^n for some n > 0
*/
template<typename T>
inline bool is_power_of_2(T arg)
   {
   return ((arg != 0 && arg != 1) && ((arg & (arg-1)) == 0));
   }

/**
* Return the index of the highest set bit
* T is an unsigned integer type
* @param n an integer value
* @return index of the highest set bit in n
*/
template<typename T>
inline size_t high_bit(T n)
   {
   for(size_t i = 8*sizeof(T); i > 0; --i)
      if((n >> (i - 1)) & 0x01)
         return i;
   return 0;
   }

/**
* Return the index of the lowest set bit
* T is an unsigned integer type
* @param n an integer value
* @return index of the lowest set bit in n
*/
template<typename T>
inline size_t low_bit(T n)
   {
   for(size_t i = 0; i != 8*sizeof(T); ++i)
      if((n >> i) & 0x01)
         return (i + 1);
   return 0;
   }

/**
* Return the number of significant bytes in n
* @param n an integer value
* @return number of significant bytes in n
*/
template<typename T>
inline size_t significant_bytes(T n)
   {
   for(size_t i = 0; i != sizeof(T); ++i)
      if(get_byte(i, n))
         return sizeof(T)-i;
   return 0;
   }

/**
* Compute Hamming weights
* @param n an integer value
* @return number of bits in n set to 1
*/
template<typename T>
inline size_t hamming_weight(T n)
   {
   const byte NIBBLE_WEIGHTS[] = {
      0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

   size_t weight = 0;
   for(size_t i = 0; i != 2*sizeof(T); ++i)
      weight += NIBBLE_WEIGHTS[(n >> (4*i)) & 0x0F];
   return weight;
   }

/**
* Count the trailing zero bits in n
* @param n an integer value
* @return maximum x st 2^x divides n
*/
template<typename T>
inline size_t ctz(T n)
   {
   for(size_t i = 0; i != 8*sizeof(T); ++i)
      if((n >> i) & 0x01)
         return i;
   return 8*sizeof(T);
   }

template<typename T>
size_t ceil_log2(T x)
   {
   if(x >> (sizeof(T)*8-1))
      return sizeof(T)*8;

   size_t result = 0;
   T compare = 1;

   while(compare < x)
      {
      compare <<= 1;
      result++;
      }

   return result;
   }

}


namespace Botan {

/*
* Allocation Size Tracking Helper for Zlib/Bzlib/LZMA
*/
class Compression_Alloc_Info
   {
   public:
      template<typename T>
      static void* malloc(void* self, T n, T size)
         {
         return static_cast<Compression_Alloc_Info*>(self)->do_malloc(n, size);
         }

      static void free(void* self, void* ptr)
         {
         static_cast<Compression_Alloc_Info*>(self)->do_free(ptr);
         }

   private:
      void* do_malloc(size_t n, size_t size);
      void do_free(void* ptr);

      std::unordered_map<void*, size_t> m_current_allocs;
   };

/**
* Wrapper for Zlib/Bzlib/LZMA stream types
*/
template<typename Stream, typename ByteType>
class Zlib_Style_Stream : public Compression_Stream
   {
   public:
      void next_in(byte* b, size_t len) override
         {
         m_stream.next_in = reinterpret_cast<ByteType*>(b);
         m_stream.avail_in = len;
         }

      void next_out(byte* b, size_t len) override
         {
         m_stream.next_out = reinterpret_cast<ByteType*>(b);
         m_stream.avail_out = len;
         }

      size_t avail_in() const override { return m_stream.avail_in; }

      size_t avail_out() const override { return m_stream.avail_out; }

      Zlib_Style_Stream()
         {
         clear_mem(&m_stream, 1);
         m_allocs.reset(new Compression_Alloc_Info);
         }

      ~Zlib_Style_Stream()
         {
         clear_mem(&m_stream, 1);
         m_allocs.reset();
         }

   protected:
      typedef Stream stream_t;

      stream_t* streamp() { return &m_stream; }

      Compression_Alloc_Info* alloc() { return m_allocs.get(); }
   private:
      stream_t m_stream;
      std::unique_ptr<Compression_Alloc_Info> m_allocs;
   };

#define BOTAN_REGISTER_COMPRESSION(C, D) \
   BOTAN_REGISTER_T_1LEN(Transform, C, 9); \
   BOTAN_REGISTER_T_NOARGS(Transform, D)

}


#if defined(BOTAN_HAS_VALGRIND)
  #include <valgrind/memcheck.h>
#endif

namespace Botan {

namespace CT {

/**
* Use valgrind to mark the contents of memory as being undefined.
* Valgrind will accept operations which manipulate undefined values,
* but will warn if an undefined value is used to decided a conditional
* jump or a load/store address. So if we poison all of our inputs we
* can confirm that the operations in question are truly const time
* when compiled by whatever compiler is in use.
*
* Even better, the VALGRIND_MAKE_MEM_* macros work even when the
* program is not run under valgrind (though with a few cycles of
* overhead, which is unfortunate in final binaries as these
* annotations tend to be used in fairly important loops).
*
* This approach was first used in ctgrind (https://github.com/agl/ctgrind)
* but calling the valgrind mecheck API directly works just as well and
* doesn't require a custom patched valgrind.
*/
template<typename T>
inline void poison(const T* p, size_t n)
   {
#if defined(BOTAN_HAS_VALGRIND)
   VALGRIND_MAKE_MEM_UNDEFINED(p, n * sizeof(T));
#else
   BOTAN_UNUSED(p);
   BOTAN_UNUSED(n);
#endif
   }

template<typename T>
inline void unpoison(const T* p, size_t n)
   {
#if defined(BOTAN_HAS_VALGRIND)
   VALGRIND_MAKE_MEM_DEFINED(p, n * sizeof(T));
#else
   BOTAN_UNUSED(p);
   BOTAN_UNUSED(n);
#endif
   }

template<typename T>
inline void unpoison(T& p)
   {
#if defined(BOTAN_HAS_VALGRIND)
   VALGRIND_MAKE_MEM_DEFINED(&p, sizeof(T));
#else
   BOTAN_UNUSED(p);
#endif
   }

/*
* T should be an unsigned machine integer type
* Expand to a mask used for other operations
* @param in an integer
* @return If n is zero, returns zero. Otherwise
* returns a T with all bits set for use as a mask with
* select.
*/
template<typename T>
inline T expand_mask(T x)
   {
   T r = x;
   // First fold r down to a single bit
   for(size_t i = 1; i != sizeof(T)*8; i *= 2)
      r |= r >> i;
   r &= 1;
   r = ~(r - 1);
   return r;
   }

template<typename T>
inline T select(T mask, T from0, T from1)
   {
   return (from0 & mask) | (from1 & ~mask);
   }

template<typename PredT, typename ValT>
inline ValT val_or_zero(PredT pred_val, ValT val)
   {
   return select(CT::expand_mask<ValT>(pred_val), val, static_cast<ValT>(0));
   }

template<typename T>
inline T is_zero(T x)
   {
   return ~expand_mask(x);
   }

template<typename T>
inline T is_equal(T x, T y)
   {
   return is_zero(x ^ y);
   }

template<typename T>
inline T is_less(T x, T y)
   {
   /*
   This expands to a constant time sequence with GCC 5.2.0 on x86-64
   but something more complicated may be needed for portable const time.
   */
   return expand_mask<T>(x < y);
   }

template<typename T>
inline void conditional_copy_mem(T value,
                                 T* to,
                                 const T* from0,
                                 const T* from1,
                                 size_t elems)
   {
   const T mask = CT::expand_mask(value);

   for(size_t i = 0; i != elems; ++i)
      {
      to[i] = CT::select(mask, from0[i], from1[i]);
      }
   }

template<typename T>
inline void cond_zero_mem(T cond,
                          T* array,
                          size_t elems)
   {
   const T mask = CT::expand_mask(cond);
   const T zero(0);

   for(size_t i = 0; i != elems; ++i)
      {
      array[i] = CT::select(mask, zero, array[i]);
      }
   }

template<typename T>
inline T expand_top_bit(T a)
   {
   return expand_mask<T>(a >> (sizeof(T)*8-1));
   }

template<typename T>
inline T max(T a, T b)
   {
   const T a_larger = b - a; // negative if a is larger
   return select(expand_top_bit(a), a, b);
   }

template<typename T>
inline T min(T a, T b)
   {
   const T a_larger = b - a; // negative if a is larger
   return select(expand_top_bit(b), b, a);
   }

inline secure_vector<uint8_t> strip_leading_zeros(const uint8_t in[], size_t length)
   {
   size_t leading_zeros = 0;

   uint8_t only_zeros = 0xFF;

   for(size_t i = 0; i != length; ++i)
      {
      only_zeros &= CT::is_zero(in[i]);
      leading_zeros += CT::select<uint8_t>(only_zeros, 1, 0);
      }

   return secure_vector<byte>(in + leading_zeros, in + length);
   }

inline secure_vector<byte> strip_leading_zeros(const secure_vector<uint8_t>& in)
   {
   return strip_leading_zeros(in.data(), in.size());
   }

}

}


namespace Botan {

/**
* Entropy source using SecRandomCopyBytes from Darwin's Security.framework
*/
class Darwin_SecRandom final : public Entropy_Source
   {
   public:
      std::string name() const override { return "darwin_secrandom"; }

      void poll(Entropy_Accumulator& accum) override;
   };

}


namespace Botan {

/**
* Fixed Window Exponentiator
*/
class Fixed_Window_Exponentiator : public Modular_Exponentiator
   {
   public:
      void set_exponent(const BigInt&) override;
      void set_base(const BigInt&) override;
      BigInt execute() const override;

      Modular_Exponentiator* copy() const override
         { return new Fixed_Window_Exponentiator(*this); }

      Fixed_Window_Exponentiator(const BigInt&, Power_Mod::Usage_Hints);
   private:
      Modular_Reducer m_reducer;
      BigInt m_exp;
      size_t m_window_bits;
      std::vector<BigInt> m_g;
      Power_Mod::Usage_Hints m_hints;
   };

/**
* Montgomery Exponentiator
*/
class Montgomery_Exponentiator : public Modular_Exponentiator
   {
   public:
      void set_exponent(const BigInt&) override;
      void set_base(const BigInt&) override;
      BigInt execute() const override;

      Modular_Exponentiator* copy() const override
         { return new Montgomery_Exponentiator(*this); }

      Montgomery_Exponentiator(const BigInt&, Power_Mod::Usage_Hints);
   private:
      BigInt m_exp, m_modulus, m_R_mod, m_R2_mod;
      word m_mod_prime;
      size_t m_mod_words, m_exp_bits, m_window_bits;
      Power_Mod::Usage_Hints m_hints;
      std::vector<BigInt> m_g;
   };

}


namespace Botan {

class donna128
   {
   public:
      donna128(u64bit ll = 0, u64bit hh = 0) { l = ll; h = hh; }

      donna128(const donna128&) = default;
      donna128& operator=(const donna128&) = default;

      friend donna128 operator>>(const donna128& x, size_t shift)
         {
         donna128 z = x;
         const u64bit carry = z.h << (64 - shift);
         z.h = (z.h >> shift);
         z.l = (z.l >> shift) | carry;
         return z;
         }

      friend donna128 operator<<(const donna128& x, size_t shift)
         {
         donna128 z = x;
         const u64bit carry = z.l >> (64 - shift);
         z.l = (z.l << shift);
         z.h = (z.h << shift) | carry;
         return z;
         }

      friend u64bit operator&(const donna128& x, u64bit mask)
         {
         return x.l & mask;
         }

      u64bit operator&=(u64bit mask)
         {
         h = 0;
         l &= mask;
         return l;
         }

      donna128& operator+=(const donna128& x)
         {
         l += x.l;
         h += (l < x.l);
         h += x.h;
         return *this;
         }

      donna128& operator+=(u64bit x)
         {
         l += x;
         h += (l < x);
         return *this;
         }

      u64bit lo() const { return l; }
      u64bit hi() const { return h; }
   private:
      u64bit h = 0, l = 0;
   };

inline donna128 operator*(const donna128& x, u64bit y)
   {
   BOTAN_ASSERT(x.hi() == 0, "High 64 bits of donna128 set to zero during multiply");

   u64bit lo = 0, hi = 0;
   mul64x64_128(x.lo(), y, &lo, &hi);
   return donna128(lo, hi);
   }

inline donna128 operator+(const donna128& x, const donna128& y)
   {
   donna128 z = x;
   z += y;
   return z;
   }

inline donna128 operator+(const donna128& x, u64bit y)
   {
   donna128 z = x;
   z += y;
   return z;
   }

inline donna128 operator|(const donna128& x, const donna128& y)
   {
   return donna128(x.lo() | y.lo(), x.hi() | y.hi());
   }

inline u64bit carry_shift(const donna128& a, size_t shift)
   {
   return (a >> shift).lo();
   }

inline u64bit combine_lower(const donna128& a, size_t s1,
                            const donna128& b, size_t s2)
   {
   donna128 z = (a >> s1) | (b << s2);
   return z.lo();
   }

#if defined(BOTAN_TARGET_HAS_NATIVE_UINT128)
inline u64bit carry_shift(const uint128_t a, size_t shift)
   {
   return static_cast<u64bit>(a >> shift);
   }

inline u64bit combine_lower(const uint128_t a, size_t s1,
                            const uint128_t b, size_t s2)
   {
   return static_cast<u64bit>((a >> s1) | (b << s2));
   }
#endif

}


namespace Botan {

BOTAN_DLL std::vector<std::string> get_files_recursive(const std::string& dir);

}


namespace Botan {

/**
* Entropy source using high resolution timers
*
* @note Any results from timers are marked as not contributing entropy
* to the poll, as a local attacker could observe them directly.
*/
class High_Resolution_Timestamp final : public Entropy_Source
   {
   public:
      std::string name() const override { return "timestamp"; }
      void poll(Entropy_Accumulator& accum) override;
   };

}


namespace Botan {

/**
* Round up
* @param n a non-negative integer
* @param align_to the alignment boundary
* @return n rounded up to a multiple of align_to
*/
inline size_t round_up(size_t n, size_t align_to)
   {
   BOTAN_ASSERT(align_to != 0, "align_to must not be 0");

   if(n % align_to)
      n += align_to - (n % align_to);
   return n;
   }

/**
* Round down
* @param n an integer
* @param align_to the alignment boundary
* @return n rounded down to a multiple of align_to
*/
template<typename T>
inline T round_down(T n, T align_to)
   {
   if(align_to == 0)
      return n;

   return (n - (n % align_to));
   }

/**
* Clamp
*/
inline size_t clamp(size_t n, size_t lower_bound, size_t upper_bound)
   {
   if(n < lower_bound)
      return lower_bound;
   if(n > upper_bound)
      return upper_bound;
   return n;
   }

}


namespace Botan {

template<typename T>
T* make_block_cipher_mode(const Transform::Spec& spec)
   {
   if(std::unique_ptr<BlockCipher> bc = BlockCipher::create(spec.arg(0)))
      return new T(bc.release());
   return nullptr;
   }

template<typename T, size_t LEN1>
T* make_block_cipher_mode_len(const Transform::Spec& spec)
   {
   if(std::unique_ptr<BlockCipher> bc = BlockCipher::create(spec.arg(0)))
      {
      const size_t len1 = spec.arg_as_integer(1, LEN1);
      return new T(bc.release(), len1);
      }

   return nullptr;
   }

template<typename T, size_t LEN1, size_t LEN2>
T* make_block_cipher_mode_len2(const Transform::Spec& spec)
   {
   if(std::unique_ptr<BlockCipher> bc = BlockCipher::create(spec.arg(0)))
      {
      const size_t len1 = spec.arg_as_integer(1, LEN1);
      const size_t len2 = spec.arg_as_integer(2, LEN2);
      return new T(bc.release(), len1, len2);
      }

   return nullptr;
   }

#define BOTAN_REGISTER_BLOCK_CIPHER_MODE(E, D)                          \
   BOTAN_REGISTER_NAMED_T(Transform, #E, E, make_block_cipher_mode<E>); \
   BOTAN_REGISTER_NAMED_T(Transform, #D, D, make_block_cipher_mode<D>)

#define BOTAN_REGISTER_BLOCK_CIPHER_MODE_LEN(E, D, LEN)                          \
   BOTAN_REGISTER_NAMED_T(Transform, #E, E, (make_block_cipher_mode_len<E, LEN>)); \
   BOTAN_REGISTER_NAMED_T(Transform, #D, D, (make_block_cipher_mode_len<D, LEN>))

#define BOTAN_REGISTER_BLOCK_CIPHER_MODE_LEN2(E, D, LEN1, LEN2)                          \
   BOTAN_REGISTER_NAMED_T(Transform, #E, E, (make_block_cipher_mode_len2<E, LEN1, LEN2>)); \
   BOTAN_REGISTER_NAMED_T(Transform, #D, D, (make_block_cipher_mode_len2<D, LEN1, LEN2>))

}


#if (BOTAN_MP_WORD_BITS != 32)
   #error The mp_x86_32 module requires that BOTAN_MP_WORD_BITS == 32
#endif

namespace Botan {

/*
* Helper Macros for x86 Assembly
*/
#define ASM(x) x "\n\t"

/*
* Word Multiply
*/
inline word word_madd2(word a, word b, word* c)
   {
   asm(
      ASM("mull %[b]")
      ASM("addl %[c],%[a]")
      ASM("adcl $0,%[carry]")

      : [a]"=a"(a), [b]"=rm"(b), [carry]"=&d"(*c)
      : "0"(a), "1"(b), [c]"g"(*c) : "cc");

   return a;
   }

/*
* Word Multiply/Add
*/
inline word word_madd3(word a, word b, word c, word* d)
   {
   asm(
      ASM("mull %[b]")

      ASM("addl %[c],%[a]")
      ASM("adcl $0,%[carry]")

      ASM("addl %[d],%[a]")
      ASM("adcl $0,%[carry]")

      : [a]"=a"(a), [b]"=rm"(b), [carry]"=&d"(*d)
      : "0"(a), "1"(b), [c]"g"(c), [d]"g"(*d) : "cc");

   return a;
   }

}


namespace Botan {

/*
* Helper Macros for x86 Assembly
*/
#ifndef ASM
  #define ASM(x) x "\n\t"
#endif

#define ADDSUB2_OP(OPERATION, INDEX)                     \
        ASM("movl 4*" #INDEX "(%[y]), %[carry]")         \
        ASM(OPERATION " %[carry], 4*" #INDEX "(%[x])")   \

#define ADDSUB3_OP(OPERATION, INDEX)                     \
        ASM("movl 4*" #INDEX "(%[x]), %[carry]")         \
        ASM(OPERATION " 4*" #INDEX "(%[y]), %[carry]")   \
        ASM("movl %[carry], 4*" #INDEX "(%[z])")         \

#define LINMUL_OP(WRITE_TO, INDEX)                       \
        ASM("movl 4*" #INDEX "(%[x]),%%eax")             \
        ASM("mull %[y]")                                 \
        ASM("addl %[carry],%%eax")                       \
        ASM("adcl $0,%%edx")                             \
        ASM("movl %%edx,%[carry]")                       \
        ASM("movl %%eax, 4*" #INDEX "(%[" WRITE_TO "])")

#define MULADD_OP(IGNORED, INDEX)                        \
        ASM("movl 4*" #INDEX "(%[x]),%%eax")             \
        ASM("mull %[y]")                                 \
        ASM("addl %[carry],%%eax")                       \
        ASM("adcl $0,%%edx")                             \
        ASM("addl 4*" #INDEX "(%[z]),%%eax")             \
        ASM("adcl $0,%%edx")                             \
        ASM("movl %%edx,%[carry]")                       \
        ASM("movl %%eax, 4*" #INDEX " (%[z])")

#define DO_8_TIMES(MACRO, ARG) \
        MACRO(ARG, 0) \
        MACRO(ARG, 1) \
        MACRO(ARG, 2) \
        MACRO(ARG, 3) \
        MACRO(ARG, 4) \
        MACRO(ARG, 5) \
        MACRO(ARG, 6) \
        MACRO(ARG, 7)

#define ADD_OR_SUBTRACT(CORE_CODE)     \
        ASM("rorl %[carry]")           \
        CORE_CODE                      \
        ASM("sbbl %[carry],%[carry]")  \
        ASM("negl %[carry]")

/*
* Word Addition
*/
inline word word_add(word x, word y, word* carry)
   {
   asm(
      ADD_OR_SUBTRACT(ASM("adcl %[y],%[x]"))
      : [x]"=r"(x), [carry]"=r"(*carry)
      : "0"(x), [y]"rm"(y), "1"(*carry)
      : "cc");
   return x;
   }

/*
* Eight Word Block Addition, Two Argument
*/
inline word word8_add2(word x[8], const word y[8], word carry)
   {
   asm(
      ADD_OR_SUBTRACT(DO_8_TIMES(ADDSUB2_OP, "adcl"))
      : [carry]"=r"(carry)
      : [x]"r"(x), [y]"r"(y), "0"(carry)
      : "cc", "memory");
   return carry;
   }

/*
* Eight Word Block Addition, Three Argument
*/
inline word word8_add3(word z[8], const word x[8], const word y[8], word carry)
   {
   asm(
      ADD_OR_SUBTRACT(DO_8_TIMES(ADDSUB3_OP, "adcl"))
      : [carry]"=r"(carry)
      : [x]"r"(x), [y]"r"(y), [z]"r"(z), "0"(carry)
      : "cc", "memory");
   return carry;
   }

/*
* Word Subtraction
*/
inline word word_sub(word x, word y, word* carry)
   {
   asm(
      ADD_OR_SUBTRACT(ASM("sbbl %[y],%[x]"))
      : [x]"=r"(x), [carry]"=r"(*carry)
      : "0"(x), [y]"rm"(y), "1"(*carry)
      : "cc");
   return x;
   }

/*
* Eight Word Block Subtraction, Two Argument
*/
inline word word8_sub2(word x[8], const word y[8], word carry)
   {
   asm(
      ADD_OR_SUBTRACT(DO_8_TIMES(ADDSUB2_OP, "sbbl"))
      : [carry]"=r"(carry)
      : [x]"r"(x), [y]"r"(y), "0"(carry)
      : "cc", "memory");
   return carry;
   }

/*
* Eight Word Block Subtraction, Two Argument
*/
inline word word8_sub2_rev(word x[8], const word y[8], word carry)
   {
   asm(
      ADD_OR_SUBTRACT(DO_8_TIMES(ADDSUB3_OP, "sbbl"))
      : [carry]"=r"(carry)
      : [x]"r"(y), [y]"r"(x), [z]"r"(x), "0"(carry)
      : "cc", "memory");
   return carry;
   }

/*
* Eight Word Block Subtraction, Three Argument
*/
inline word word8_sub3(word z[8], const word x[8], const word y[8], word carry)
   {
   asm(
      ADD_OR_SUBTRACT(DO_8_TIMES(ADDSUB3_OP, "sbbl"))
      : [carry]"=r"(carry)
      : [x]"r"(x), [y]"r"(y), [z]"r"(z), "0"(carry)
      : "cc", "memory");
   return carry;
   }

/*
* Eight Word Block Linear Multiplication
*/
inline word word8_linmul2(word x[8], word y, word carry)
   {
   asm(
      DO_8_TIMES(LINMUL_OP, "x")
      : [carry]"=r"(carry)
      : [x]"r"(x), [y]"rm"(y), "0"(carry)
      : "cc", "%eax", "%edx");
   return carry;
   }

/*
* Eight Word Block Linear Multiplication
*/
inline word word8_linmul3(word z[8], const word x[8], word y, word carry)
   {
   asm(
      DO_8_TIMES(LINMUL_OP, "z")
      : [carry]"=r"(carry)
      : [z]"r"(z), [x]"r"(x), [y]"rm"(y), "0"(carry)
      : "cc", "%eax", "%edx");
   return carry;
   }

/*
* Eight Word Block Multiply/Add
*/
inline word word8_madd3(word z[8], const word x[8], word y, word carry)
   {
   asm(
      DO_8_TIMES(MULADD_OP, "")
      : [carry]"=r"(carry)
      : [z]"r"(z), [x]"r"(x), [y]"rm"(y), "0"(carry)
      : "cc", "%eax", "%edx");
   return carry;
   }

/*
* Multiply-Add Accumulator
*/
inline void word3_muladd(word* w2, word* w1, word* w0, word x, word y)
   {
   asm(
      ASM("mull %[y]")

      ASM("addl %[x],%[w0]")
      ASM("adcl %[y],%[w1]")
      ASM("adcl $0,%[w2]")

      : [w0]"=r"(*w0), [w1]"=r"(*w1), [w2]"=r"(*w2)
      : [x]"a"(x), [y]"d"(y), "0"(*w0), "1"(*w1), "2"(*w2)
      : "cc");
   }

/*
* Multiply-Add Accumulator
*/
inline void word3_muladd_2(word* w2, word* w1, word* w0, word x, word y)
   {
   asm(
      ASM("mull %[y]")

      ASM("addl %[x],%[w0]")
      ASM("adcl %[y],%[w1]")
      ASM("adcl $0,%[w2]")

      ASM("addl %[x],%[w0]")
      ASM("adcl %[y],%[w1]")
      ASM("adcl $0,%[w2]")

      : [w0]"=r"(*w0), [w1]"=r"(*w1), [w2]"=r"(*w2)
      : [x]"a"(x), [y]"d"(y), "0"(*w0), "1"(*w1), "2"(*w2)
      : "cc");
   }

}


namespace Botan {

/*
* The size of the word type, in bits
*/
const size_t MP_WORD_BITS = BOTAN_MP_WORD_BITS;

/*
* If cond == 0, does nothing.
* If cond > 0, swaps x[0:size] with y[0:size]
* Runs in constant time
*/
BOTAN_DLL
void bigint_cnd_swap(word cnd, word x[], word y[], size_t size);

/*
* If cond > 0 adds x[0:size] to y[0:size] and returns carry
* Runs in constant time
*/
BOTAN_DLL
word bigint_cnd_add(word cnd, word x[], const word y[], size_t size);

/*
* If cond > 0 subs x[0:size] to y[0:size] and returns borrow
* Runs in constant time
*/
BOTAN_DLL
word bigint_cnd_sub(word cnd, word x[], const word y[], size_t size);

/*
* 2s complement absolute value
* If cond > 0 sets x to ~x + 1
* Runs in constant time
*/
BOTAN_DLL
void bigint_cnd_abs(word cnd, word x[], size_t size);

/**
* Two operand addition
* @param x the first operand (and output)
* @param x_size size of x
* @param y the second operand
* @param y_size size of y (must be >= x_size)
*/
void bigint_add2(word x[], size_t x_size,
                 const word y[], size_t y_size);

/**
* Three operand addition
*/
void bigint_add3(word z[],
                 const word x[], size_t x_size,
                 const word y[], size_t y_size);

/**
* Two operand addition with carry out
*/
word bigint_add2_nc(word x[], size_t x_size, const word y[], size_t y_size);

/**
* Three operand addition with carry out
*/
word bigint_add3_nc(word z[],
                    const word x[], size_t x_size,
                    const word y[], size_t y_size);

/**
* Two operand subtraction
*/
word bigint_sub2(word x[], size_t x_size,
                 const word y[], size_t y_size);

/**
* Two operand subtraction, x = y - x; assumes y >= x
*/
void bigint_sub2_rev(word x[], const word y[], size_t y_size);

/**
* Three operand subtraction
*/
word bigint_sub3(word z[],
                 const word x[], size_t x_size,
                 const word y[], size_t y_size);

/*
* Shift Operations
*/
void bigint_shl1(word x[], size_t x_size,
                 size_t word_shift, size_t bit_shift);

void bigint_shr1(word x[], size_t x_size,
                 size_t word_shift, size_t bit_shift);

void bigint_shl2(word y[], const word x[], size_t x_size,
                 size_t word_shift, size_t bit_shift);

void bigint_shr2(word y[], const word x[], size_t x_size,
                 size_t word_shift, size_t bit_shift);

/*
* Linear Multiply
*/
void bigint_linmul2(word x[], size_t x_size, word y);
void bigint_linmul3(word z[], const word x[], size_t x_size, word y);

/**
* Montgomery Reduction
* @param z integer to reduce, of size exactly 2*(p_size+1).
           Output is in the first p_size+1 words, higher
           words are set to zero.
* @param p modulus
* @param p_size size of p
* @param p_dash Montgomery value
* @param workspace array of at least 2*(p_size+1) words
*/
void bigint_monty_redc(word z[],
                       const word p[], size_t p_size,
                       word p_dash,
                       word workspace[]);

/*
* Montgomery Multiplication
*/
void bigint_monty_mul(word z[], size_t z_size,
                      const word x[], size_t x_size, size_t x_sw,
                      const word y[], size_t y_size, size_t y_sw,
                      const word p[], size_t p_size, word p_dash,
                      word workspace[]);

/*
* Montgomery Squaring
*/
void bigint_monty_sqr(word z[], size_t z_size,
                      const word x[], size_t x_size, size_t x_sw,
                      const word p[], size_t p_size, word p_dash,
                      word workspace[]);

/**
* Compare x and y
*/
s32bit bigint_cmp(const word x[], size_t x_size,
                  const word y[], size_t y_size);

/**
* Compute ((n1<<bits) + n0) / d
*/
word bigint_divop(word n1, word n0, word d);

/**
* Compute ((n1<<bits) + n0) % d
*/
word bigint_modop(word n1, word n0, word d);

/*
* Comba Multiplication / Squaring
*/
void bigint_comba_mul4(word z[8], const word x[4], const word y[4]);
void bigint_comba_mul6(word z[12], const word x[6], const word y[6]);
void bigint_comba_mul8(word z[16], const word x[8], const word y[8]);
void bigint_comba_mul9(word z[18], const word x[9], const word y[9]);
void bigint_comba_mul16(word z[32], const word x[16], const word y[16]);

void bigint_comba_sqr4(word out[8], const word in[4]);
void bigint_comba_sqr6(word out[12], const word in[6]);
void bigint_comba_sqr8(word out[16], const word in[8]);
void bigint_comba_sqr9(word out[18], const word in[9]);
void bigint_comba_sqr16(word out[32], const word in[16]);

/*
* High Level Multiplication/Squaring Interfaces
*/
void bigint_mul(word z[], size_t z_size, word workspace[],
                const word x[], size_t x_size, size_t x_sw,
                const word y[], size_t y_size, size_t y_sw);

void bigint_sqr(word z[], size_t z_size, word workspace[],
                const word x[], size_t x_size, size_t x_sw);

}


namespace Botan {

namespace OS {

/**
* Returns the OS assigned process ID, if available. Otherwise returns 0.
*/
uint32_t get_process_id();

/**
* Returns the value of the hardware cycle counter, if available.
* Returns 0 if not available. On Windows uses QueryPerformanceCounter.
* On other platforms reads the native cycle counter directly.
* The epoch and update rate are arbitrary and may not be constant
* (depending on the hardware).
*/
uint64_t get_processor_timestamp();

/**
* Returns the value of the system clock with best resolution available,
* normalized to nanoseconds resolution.
*/
uint64_t get_system_timestamp_ns();

/*
* Returns the maximum amount of memory (in bytes) we could/should
* hyptothetically allocate. Reads "BOTAN_MLOCK_POOL_SIZE" from
* environment which can be set to zero.
*/
size_t get_memory_locking_limit();

/*
* Request so many bytes of page-aligned RAM locked into memory using
* mlock, VirtualLock, or similar. Returns null on failure. The memory
* returned is zeroed. Free it with free_locked_pages.
*/
void* allocate_locked_pages(size_t length);

/*
* Free memory allocated by allocate_locked_pages
*/
void free_locked_pages(void* ptr, size_t length);

}

}


namespace Botan {

/**
* Container of output buffers for Pipe
*/
class Output_Buffers
   {
   public:
      size_t read(byte[], size_t, Pipe::message_id);
      size_t peek(byte[], size_t, size_t, Pipe::message_id) const;
      size_t get_bytes_read(Pipe::message_id) const;
      size_t remaining(Pipe::message_id) const;

      void add(class SecureQueue*);
      void retire();

      Pipe::message_id message_count() const;

      Output_Buffers();
      ~Output_Buffers();
   private:
      class SecureQueue* get(Pipe::message_id) const;

      std::deque<SecureQueue*> m_buffers;
      Pipe::message_id m_offset;
   };

}


namespace Botan {

Public_Key* make_public_key(const AlgorithmIdentifier& alg_id,
                            const secure_vector<byte>& key_bits);

Private_Key* make_private_key(const AlgorithmIdentifier& alg_id,
                              const secure_vector<byte>& key_bits,
                              RandomNumberGenerator& rng);

}


namespace Botan {

namespace PK_Ops {

class Encryption_with_EME : public Encryption
   {
   public:
      size_t max_input_bits() const override;

      secure_vector<byte> encrypt(const byte msg[], size_t msg_len,
                                  RandomNumberGenerator& rng) override;

      ~Encryption_with_EME();
   protected:
      explicit Encryption_with_EME(const std::string& eme);
   private:
      virtual size_t max_raw_input_bits() const = 0;

      virtual secure_vector<byte> raw_encrypt(const byte msg[], size_t len,
                                              RandomNumberGenerator& rng) = 0;
      std::unique_ptr<EME> m_eme;
   };

class Decryption_with_EME : public Decryption
   {
   public:
      size_t max_input_bits() const override;

      secure_vector<byte> decrypt(byte& valid_mask,
                                  const byte msg[], size_t msg_len) override;

      ~Decryption_with_EME();
   protected:
      explicit Decryption_with_EME(const std::string& eme);
   private:
      virtual size_t max_raw_input_bits() const = 0;
      virtual secure_vector<byte> raw_decrypt(const byte msg[], size_t len) = 0;
      std::unique_ptr<EME> m_eme;
   };

class Verification_with_EMSA : public Verification
   {
   public:
      void update(const byte msg[], size_t msg_len) override;
      bool is_valid_signature(const byte sig[], size_t sig_len) override;

      bool do_check(const secure_vector<byte>& msg,
                    const byte sig[], size_t sig_len);

   protected:

      explicit Verification_with_EMSA(const std::string& emsa);
      ~Verification_with_EMSA();

      /**
      * @return boolean specifying if this key type supports message
      * recovery and thus if you need to call verify() or verify_mr()
      */
      virtual bool with_recovery() const = 0;

      /*
      * Perform a signature check operation
      * @param msg the message
      * @param msg_len the length of msg in bytes
      * @param sig the signature
      * @param sig_len the length of sig in bytes
      * @returns if signature is a valid one for message
      */
      virtual bool verify(const byte[], size_t,
                          const byte[], size_t)
         {
         throw Invalid_State("Message recovery required");
         }

      /*
      * Perform a signature operation (with message recovery)
      * Only call this if with_recovery() returns true
      * @param msg the message
      * @param msg_len the length of msg in bytes
      * @returns recovered message
      */
      virtual secure_vector<byte> verify_mr(const byte[], size_t)
         {
         throw Invalid_State("Message recovery not supported");
         }

   private:
      std::unique_ptr<EMSA> m_emsa;
   };

class Signature_with_EMSA : public Signature
   {
   public:
      void update(const byte msg[], size_t msg_len) override;

      secure_vector<byte> sign(RandomNumberGenerator& rng) override;
   protected:
      explicit Signature_with_EMSA(const std::string& emsa);
      ~Signature_with_EMSA();
   private:

      /**
      * Get the maximum message size in bits supported by this public key.
      * @return maximum message in bits
      */
      virtual size_t max_input_bits() const = 0;

      bool self_test_signature(const std::vector<byte>& msg,
                               const std::vector<byte>& sig) const;

      virtual secure_vector<byte> raw_sign(const byte msg[], size_t msg_len,
                                           RandomNumberGenerator& rng) = 0;

      std::unique_ptr<EMSA> m_emsa;
   };

class Key_Agreement_with_KDF : public Key_Agreement
   {
   public:
      secure_vector<byte> agree(size_t key_len,
                                const byte other_key[], size_t other_key_len,
                                const byte salt[], size_t salt_len) override;

   protected:
      explicit Key_Agreement_with_KDF(const std::string& kdf);
      ~Key_Agreement_with_KDF();
   private:
      virtual secure_vector<byte> raw_agree(const byte w[], size_t w_len) = 0;
      std::unique_ptr<KDF> m_kdf;
   };

class KEM_Encryption_with_KDF : public KEM_Encryption
   {
   public:
      void kem_encrypt(secure_vector<byte>& out_encapsulated_key,
                       secure_vector<byte>& out_shared_key,
                       size_t desired_shared_key_len,
                       Botan::RandomNumberGenerator& rng,
                       const uint8_t salt[],
                       size_t salt_len) override;

   protected:
      virtual void raw_kem_encrypt(secure_vector<byte>& out_encapsulated_key,
                                   secure_vector<byte>& raw_shared_key,
                                   Botan::RandomNumberGenerator& rng) = 0;

      explicit KEM_Encryption_with_KDF(const std::string& kdf);
      ~KEM_Encryption_with_KDF();
   private:
      std::unique_ptr<KDF> m_kdf;
   };

class KEM_Decryption_with_KDF : public KEM_Decryption
   {
   public:
      secure_vector<byte> kem_decrypt(const byte encap_key[],
                                      size_t len,
                                      size_t desired_shared_key_len,
                                      const uint8_t salt[],
                                      size_t salt_len) override;

   protected:
      virtual secure_vector<byte>
      raw_kem_decrypt(const byte encap_key[], size_t len) = 0;

      explicit KEM_Decryption_with_KDF(const std::string& kdf);
      ~KEM_Decryption_with_KDF();
   private:
      std::unique_ptr<KDF> m_kdf;
   };

}

}


namespace Botan {

template<typename OP, typename T>
OP* make_pk_op(const typename T::Spec& spec)
   {
   if(auto* key = dynamic_cast<const typename T::Key_Type*>(&spec.key()))
      return new T(*key, spec.padding());
   return nullptr;
   }

#define BOTAN_REGISTER_PK_OP(T, NAME, TYPE) BOTAN_REGISTER_NAMED_T(T, NAME, TYPE, (make_pk_op<T, TYPE>))

#define BOTAN_REGISTER_PK_ENCRYPTION_OP(NAME, TYPE) BOTAN_REGISTER_PK_OP(PK_Ops::Encryption, NAME, TYPE)
#define BOTAN_REGISTER_PK_DECRYPTION_OP(NAME, TYPE) BOTAN_REGISTER_PK_OP(PK_Ops::Decryption, NAME, TYPE)
#define BOTAN_REGISTER_PK_SIGNATURE_OP(NAME, TYPE) BOTAN_REGISTER_PK_OP(PK_Ops::Signature, NAME, TYPE)
#define BOTAN_REGISTER_PK_VERIFY_OP(NAME, TYPE) BOTAN_REGISTER_PK_OP(PK_Ops::Verification, NAME, TYPE)
#define BOTAN_REGISTER_PK_KEY_AGREE_OP(NAME, TYPE) BOTAN_REGISTER_PK_OP(PK_Ops::Key_Agreement, NAME, TYPE)

#define BOTAN_REGISTER_PK_KEM_ENCRYPTION_OP(NAME, TYPE) BOTAN_REGISTER_PK_OP(PK_Ops::KEM_Encryption, NAME, TYPE)
#define BOTAN_REGISTER_PK_KEM_DECRYPTION_OP(NAME, TYPE) BOTAN_REGISTER_PK_OP(PK_Ops::KEM_Decryption, NAME, TYPE)

}


namespace Botan {

template<typename T>
inline void prefetch_readonly(const T* addr, size_t length)
   {
#if defined(__GNUG__)
   const size_t Ts_per_cache_line = CPUID::cache_line_size() / sizeof(T);

   for(size_t i = 0; i <= length; i += Ts_per_cache_line)
      __builtin_prefetch(addr + i, 0);
#endif
   }

template<typename T>
inline void prefetch_readwrite(const T* addr, size_t length)
   {
#if defined(__GNUG__)
   const size_t Ts_per_cache_line = CPUID::cache_line_size() / sizeof(T);

   for(size_t i = 0; i <= length; i += Ts_per_cache_line)
      __builtin_prefetch(addr + i, 1);
#endif
   }

}


namespace Botan {

class Semaphore
   {
   public:
      explicit Semaphore(int value = 0) : m_value(value), m_wakeups(0) {}

      void acquire();

      void release(size_t n = 1);

   private:
      int m_value;
      int m_wakeups;
      std::mutex m_mutex;
      std::condition_variable m_cond;
   };

}


namespace Botan {

inline std::vector<byte> to_byte_vector(const std::string& s)
   {
   return std::vector<byte>(s.cbegin(), s.cend());
   }

inline std::string to_string(const secure_vector<byte> &bytes)
   {
   return std::string(bytes.cbegin(), bytes.cend());
   }

/**
* Return the keys of a map as a std::set
*/
template<typename K, typename V>
std::set<K> map_keys_as_set(const std::map<K, V>& kv)
   {
   std::set<K> s;
   for(auto&& i : kv)
      {
      s.insert(i.first);
      }
   return s;
   }

/*
* Searching through a std::map
* @param mapping the map to search
* @param key is what to look for
* @param null_result is the value to return if key is not in mapping
* @return mapping[key] or null_result
*/
template<typename K, typename V>
inline V search_map(const std::map<K, V>& mapping,
                    const K& key,
                    const V& null_result = V())
   {
   auto i = mapping.find(key);
   if(i == mapping.end())
      return null_result;
   return i->second;
   }

template<typename K, typename V, typename R>
inline R search_map(const std::map<K, V>& mapping, const K& key,
                    const R& null_result, const R& found_result)
   {
   auto i = mapping.find(key);
   if(i == mapping.end())
      return null_result;
   return found_result;
   }

/*
* Insert a key/value pair into a multimap
*/
template<typename K, typename V>
void multimap_insert(std::multimap<K, V>& multimap,
                     const K& key, const V& value)
   {
#if defined(BOTAN_BUILD_COMPILER_IS_SUN_STUDIO)
   // Work around a strange bug in Sun Studio
   multimap.insert(std::make_pair<const K, V>(key, value));
#else
   multimap.insert(std::make_pair(key, value));
#endif
   }

/**
* Existence check for values
*/
template<typename T>
bool value_exists(const std::vector<T>& vec,
                  const T& val)
   {
   for(size_t i = 0; i != vec.size(); ++i)
      if(vec[i] == val)
         return true;
   return false;
   }

template<typename T, typename Pred>
void map_remove_if(Pred pred, T& assoc)
   {
   auto i = assoc.begin();
   while(i != assoc.end())
      {
      if(pred(i->first))
         assoc.erase(i++);
      else
         i++;
      }
   }

}


#endif
