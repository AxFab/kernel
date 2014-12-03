#ifndef ATOMIC_H__
#define ATOMIC_H__

#include <stdint.h>


/*=============================================================================
     x86-64
=============================================================================*/
#if defined (__x86_64__)

  static inline void atomic_inc_i32 (volatile int32_t* ref)
  {
    __asm__ __volatile__ ("lock incl %0" : "=m"(*ref));
  }


  static inline void atomic_dec_i32 (volatile int32_t* ref)
  {
    __asm__ __volatile__ ("lock decl %0" : "=m"(*ref));
  }


  static inline int32_t atomic_xchg_i32 (volatile int32_t* ref, int32_t val)
  {
    register int rval = val;
    __asm__ __volatile__ ( "lock xchg %1,%0" : "=m" (*ref), "=r" (rval) : "1" (val));
    return rval;
  }


  static inline int32_t atomic_add_i32(volatile int32_t *ref, int32_t val)
  {
     asm volatile("lock xaddl %%eax, %2;"
                  :"=a" (val) :"a" (val), "m" (*ref) :"memory");
     return val;
  }

  static inline int32_t atomic_sub_i32(volatile int32_t *ref, int32_t val)
  {
    val = -val;
    asm volatile("lock xaddl %%eax, %2;"
                 :"=a" (val) :"a" (val), "m" (*ref) :"memory");
     return val;
  }

  static inline void atomic_inc_i64 (volatile int64_t* ref)
  {
    __asm__ __volatile__ ("lock incq %0" : "=m"(*ref));
  }


  static inline void atomic_dec_i64 (volatile int64_t* ref)
  {
    __asm__ __volatile__ ("lock decq %0" : "=m"(*ref));
  }


  static inline int64_t atomic_add_i64(volatile int64_t *ref, int64_t val)
  {
     asm volatile("lock xaddq %%rax, %2;"
                  :"=a" (val) :"a" (val), "m" (*ref) :"memory");
     return val;
  }

/*=============================================================================
     ia64
=============================================================================*/
#elif defined (__ia64__)

  static inline void atomic_inc_i32 (volatile int32_t* ref)
  {
    volatile int32_t tmp;
    __asm__ __volatile__ ("fetchadd4.rel %0=[%1],1"
                          : "=r"(tmp) : "r"(ref): "memory");
  }


  static inline void atomic_dec_i32 (volatile int32_t* ref)
  {
    volatile int32_t tmp;
    __asm__ __volatile__ ("fetchadd4.rel %0=[%1],-1"
                          : "=r"(tmp) : "r"(ref): "memory");
  }


  static inline int32_t atomic_xchg_i32 (volatile int32_t* ref, int32_t val)
  {
    volatile int32_t res = val;
    __asm__ __volatile__ (" xchg4 %0=[%1],%0 "
                          :"=r"(res): "r"(ref) : "memory");
    return res;
  }


  static inline int32_t atomic_add_i32(volatile int32_t *ref, int32_t val)
  {
    volatile int32_t tmp;
    __asm__ __volatile__ ("fetchadd4.rel %0=[%1],%2"
                          : "=r"(tmp) : "r"(ref), "i"(val) : "memory");
    return tmp + val;
  }

  static inline int32_t atomic_sub_i32(volatile int32_t *ref, int32_t val)
  {
    volatile int32_t tmp;
    val = -val;
    __asm__ __volatile__ ("fetchadd4.rel %0=[%1],%2"
                          : "=r"(tmp) : "r"(ref), "i"(val) : "memory");
    return tmp + val;
  }


  static inline void atomic_inc_i64 (volatile int64_t* ref)
  {
    volatile int32_t tmp;
    __asm__ __volatile__ ("fetchadd8.rel %0=[%1],1"
                          : "=r"(tmp) : "r"(ref): "memory");
  }


  static inline void atomic_dec_i64 (volatile int64_t* ref)
  {
    volatile int32_t tmp;
    __asm__ __volatile__ ("fetchadd8.rel %0=[%1],-1"
                          : "=r"(tmp) : "r"(ref): "memory");
  }


  static inline int64_t atomic_add_i64(volatile int64_t *ref, int64_t val)
  {
    volatile int32_t tmp;
    __asm__ __volatile__ ("fetchadd8.rel %0=[%1],%2"
                          : "=r"(tmp) : "r"(ref), "i"(val) : "memory");
    return tmp + val;
  }


/*=============================================================================
     Default
=============================================================================*/
#else

  static inline int32_t atomic_xchg_i32 (volatile int32_t* ref, int32_t val)
  {
    int32_t ex = *ref;
    *ref = val;
    return ex;
  }

  static inline void atomic_inc_i32 (volatile int32_t* ref)
  {
    ++(*ref);
  }


  static inline void atomic_dec_i32 (volatile int32_t* ref)
  {
    --(*ref);
  }

  static inline int32_t atomic_add_i32(volatile int32_t *ref, int32_t val)
  {
    return (*ref) += val;
  }

  static inline int32_t atomic_sub_i32(volatile int32_t *ref, int32_t val)
  {
    val = -val;
    return (*ref) += val;
  }

#endif

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

#ifdef __KERNEL

static inline void cli()
{
  __asm__ ("cli");
}

static inline void sti()
{
  __asm__ ("sti");
}

#else

static inline void cli() {}
static inline void sti() {}

#endif

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

#endif /* ATOMIC_H__ */
