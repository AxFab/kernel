#ifndef KCORE_H__
#define KCORE_H__

// Config AxLibC - Get stdlib basic version (no syscalls)
#ifndef __EX
#define __EX
#endif

// Standard includes
#include <string.h>
#include <assert.h>
#include <stddef.h>
#include <errno.h>
#include <stdint.h>
#include <limits.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <atomic.h>

/* --- MISSING STDLIB --- */
unsigned long long strtoull (const char *str, char **endptr, int base);
int snprintf(char *s, size_t n, const char *format, ... );

// STDBOOL
typedef char bool;
#define true    ((bool)(!0))
#define false   ((bool)(0))


/* ===========================================================================
        Kernel types
=========================================================================== */
/* CORE ------------------------------------------------------------------- */
typedef struct spinlock     spinlock_t;
typedef struct llnode       llnode_t;
typedef struct llhead       llhead_t;
typedef struct aanode       aanode_t;
typedef struct aatree       aatree_t;
/// nanotime_t is an accurate time counter with nanoseconds since epoch.
typedef int64_t nanotime_t;

/* MEMORY ----------------------------------------------------------------- */
typedef struct kAddSpace    kAddSpace_t;
typedef struct kVma         kVma_t;

/* VFS -------------------------------------------------------------------- */
typedef struct kDevice      kDevice_t;
typedef struct kStat        kStat_t;
typedef struct kInode       kInode_t;
typedef struct kBucket      kBucket_t;

/* STREAM ----------------------------------------------------------------- */
typedef struct kStream      kStream_t;
typedef struct kFifo        kFifo_t;
typedef struct kTerm        kTerm_t;

/* TASK ------------------------------------------------------------------- */
typedef struct kSession     kSession_t;
typedef struct kUser        kUser_t;
typedef struct kProcess     kProcess_t;
typedef struct kThread      kThread_t;
typedef struct kAssembly    kAssembly_t;
typedef struct kSection     kSection_t;

/* ASYNC ------------------------------------------------------------------ */
typedef struct kEvent       kEvent_t;
typedef struct kWaiting     kWaiting_t;

/* OTHERS ----------------------------------------------------------------- */
typedef struct kCpuRegs     kCpuRegs_t;
typedef struct kNTty        kNTty_t;
typedef struct kLine        kLine_t;

/* ======================================================================== */
#define LOCK_INIT  {0, 0, NULL}
struct spinlock {
  int32_t       key_;
  int           cpu_;
  const char   *where_;
};

#define ANCHOR_INIT  {LOCK_INIT, NULL, NULL, 0}
// struct list_anchor
struct llhead {
  spinlock_t  lock_;
  llnode_t     *first_;
  llnode_t     *last_;
  int         count_;
};

/** Linked list node */
struct llnode {
  llnode_t     *prev_;
  llnode_t     *next_;
};

#define AAANODE_INIT  { NULL, NULL, 0, 0 }

/** AATree (self-balancing binary tree) node */
struct aanode {
  aanode_t  *left_;
  aanode_t  *right_;
  long  value_;
  int  level_;
};

#define AAATREE_INIT  { LOCK_INIT, NULL, NULL, NULL, 0, 0, 0 }

/** AATree (self-balancing binary tree) head */
struct aatree {
  spinlock_t  lock_;
  aanode_t   *root_;
  aanode_t   *last_;
  aanode_t   *deleted_;
  int         count_;
};

#define get_item(a,s,m)   ((a) == NULL ? NULL : ((s*)(((char*)(a)) - offsetof(s,m))))

/* ===========================================================================
        Kernel macros
=========================================================================== */
// Macro size ----------------------------------------------------------------
#define _Kb_      (1024)
#define _Mb_      (1024 * _Kb_)
#define _Gb_      (1024 * _Mb_)
#define _Tb_      (1024LL * _Gb_)
#define _Pb_      (1024LL * _Tb_)
#define _Eb_      (1024LL * _Eb_)

// Macro align ---------------------------------------------------------------
#define ALIGN_UP(v,a)      (((v)+(a-1))&(~(a-1)))
#define ALIGN_DW(v,a)      ((v)&(~(a-1)))

// Macro min-max -------------------------------------------------------------
// #if !defined(MIN) || !defined(MAX)
#define MIN(a,b)    ((a)<=(b)?(a):(b))
#define MAX(a,b)    ((a)>=(b)?(a):(b))
#define IS_PW2(v)   ((v) != 0 && ((v) & ((v)-1)) == 0)
// #endif

// Macro error ---------------------------------------------------------------
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define __AT__  __FILE__ ":" TOSTRING(__LINE__)
// #define __AT__  __FILE__ ":" TOSTRING(__LINE__) " on " __func__ "()"

#define __nounused(a)  ((void)a)




// Macro object --------------------------------------------------------------
#define KALLOC(T)     ((T*)kalloc (sizeof(T), 0))

#define SIZEOF(T)     kprintf ("Sizeof " #T ": %x\n", sizeof(T))

// Configuration header ------------------------------------------------------
#include <kernel/config.h>
#include <arch/define.h>


/* ===========================================================================
        Kernel runtime
=========================================================================== */
// Error ------------------------------------------------
int kseterrno(int err, const char *at);
int kgeterrno();
int kpanic(const char *str, ...);

// Print ---------------------------------------------------------------------
#define ktrace(s,...) kprintf("  [----] " s,__VA_ARGS__)
int kputc(int c);
int kprintf(const char *str, ...);
int kvprintf (const char *str, va_list ap);
const char *kpsize (uintmax_t number);

// Alloc ---------------------------------------------------------------------
void *kalloc(size_t size, int slab);
void kfree(void *addr);
char *kstrdup(const char *str);

// Miscallenous --------------------------------------------------------------
// nanotime_t ltime (nanotime_t* ptr);

// Debug ---------------------------------------------------------------------
const char *ksymbol (void *address);
void kstacktrace(uintptr_t max_frames);
void kdump (void *ptr, size_t lg);
void kregisters (kCpuRegs_t *regs);


#include <kernel/info.h>
#include <kernel/spinlock.h>
#include <kernel/list.h>
#include <kernel/aatree.h>



// /* --- END OF STDLIB --- */


// // ======================================================


#define __noerror()     __seterrno(0)
#define __seterrno(e)   __seterrno_(e,__AT__)

int *_geterrno();

static inline int __seterrno_(int err, const char *at)
{
  *_geterrno() = err;

  if (err)
    kprintf("er.] Error %d at %s: %s\n", err, at, strerror(err));

  return err;
}

static inline int __geterrno()
{
  return *_geterrno();
}



// ======================================================
// Init -------------------------------------------------
// uint32_t getuid ();
// uint32_t getgid ();
// kMemSpace_t* getSpace();
void kinit ();

// Debug ------------------------------------------------
void ksymreg (uintptr_t ptr, const char *sym);

// CPU -------------------------------------------------
int kcpu_state();
struct tm cpu_get_clock();
int cpu_ticks_interval ();
int cpu_ticks_delay();

// ======================================================


// #define OPEN_MAX 12
// #define O_STATMSK 0x101c00

// #define offsetof(s,m)   (size_t)&(((s *)0)->m)


// ======================================================

typedef struct  kTty kTty_t;
struct kTty {
  uint32_t  _color;
  uint32_t  _bkground;
  int       _cursorX;
  int       _cursorY;
  int       _mode; // TODO txtmode
  int       _column;
  int       _row;
  int       _width;
  int       _height;
  int       _depth;
  uint32_t *_ptr;
  uint32_t  _length;
};


#endif /* KCORE_H__ */
