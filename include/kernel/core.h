#ifndef KCORE_H__
#define KCORE_H__

// Config AxLibC - Get stdlib basic version (no syscalls)
#ifndef __EX
#define __EX
#endif

// Standard includes
#include <ax/core.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <limits.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <atomic.h>

/* --- MISSING STDLIB --- */
unsigned long long strtoull (const char * str, char ** endptr, int base);
int snprintf(char* s, size_t n, const char* format, ... );


#include <kernel/spinlock.h>
#include <kernel/list.h>

#define LOCK_INIT  {0, 0, NULL}

/** ltime_t is an accurate time storage
  * It hold the number of microsecond since 1st Jan 1970..
  * This type can hold up to "584 years" in nanosecond count, (signed). 
  * The counter start at Epoch, for a range from - 5041 BC - to - 8981 AD -
  */
typedef int64_t ltime_t;
ltime_t ltime (ltime_t* ptr);

#define TRUE    (!0)
#define FALSE   (0)

/* --- END OF STDLIB --- */

// Configuration header
#include <kernel/config.h>

// ======================================================
// Macro size -------------------------------------------
#define _Kb_      (1024)
#define _Mb_      (1024 * _Kb_)
#define _Gb_      (1024 * _Mb_)
#define _Tb_      (1024LL * _Gb_)
#define _Pb_      (1024LL * _Tb_)
#define _Eb_      (1024LL * _Eb_)

// Macro align ------------------------------------------
#define ALIGN_UP(v,a)      (((v)+(a-1))&(~(a-1)))
#define ALIGN_DW(v,a)      ((v)&(~(a-1)))

// Macro error ------------------------------------------
#define __noerror()     kseterrno(0,__FILE__,__LINE__, __func__)
#define __seterrno(e)   kseterrno(e,__FILE__,__LINE__, __func__)
#define __geterrno()    kgeterrno()
#define __nounused(a)  ((void)a)

// Macro
#define KALLOC(T)     ((T*)kalloc (sizeof(T)))
#define SIZEOF(T)     kprintf ("Sizeof " #T ": %x\n", sizeof(T))
#define NO_LOCK       assert(klockcount() == 0)
#define MOD_ENTER     NO_LOCK
#define MOD_LEAVE     NO_LOCK


// ======================================================
// Kernel types
typedef struct kCpuRegs     kCpuRegs_t;
typedef struct kTty         kTty_t;
typedef struct kUser        kUser_t;
// vfs.h
typedef struct kStat        kStat_t;
typedef struct kInode       kInode_t;
typedef struct kStream      kStream_t;
typedef struct kPage        kPage_t;
typedef struct kPipe        kPipe_t;
typedef struct kFifo        kFifo_t;
typedef struct kTerm        kTerm_t;

typedef struct kFifoPen     kFifoPen_t;
typedef struct kNTty        kNTty_t;
typedef struct kLine        kLine_t;
// memory.h
typedef struct kVma         kVma_t;
typedef struct kAddSpace    kAddSpace_t;
// tasks.h
typedef struct kProcess     kProcess_t;
typedef struct kTask        kTask_t;
typedef struct kAssembly    kAssembly_t;
typedef struct kSection     kSection_t;
typedef struct kBucket      kBucket_t;
typedef struct kDevice      kDevice_t;
typedef struct kEvent       kEvent_t;
typedef struct kWaiting     kWaiting_t;



// ======================================================
// Init -------------------------------------------------
// uint32_t getuid ();
// uint32_t getgid ();
// kMemSpace_t* getSpace();
void kinit ();

// Error ------------------------------------------------
int kseterrno(int err, const char* file, int line, const char* func);
int kgeterrno();
int kpanic(const char* str, ...);

// Debug ------------------------------------------------
void ksymreg (uintptr_t ptr, const char* sym);
const char* ksymbol (void* address);
void kstacktrace(uintptr_t MaxFrames);
void kdump (void* ptr, size_t lg);
void kregisters (kCpuRegs_t* regs);

// Print ------------------------------------------------
int kputc(int c);
int kprintf(const char* str, ...);
int kvprintf (const char* str, va_list ap);
const char* kpsize (uintmax_t number);

// Alloc ------------------------------------------------
void* kalloc(size_t size);
void kfree(void* addr);
char* kcopystr(const char* str);

// CPU -------------------------------------------------
int kcpu_state();

// ======================================================

#ifndef __KERNEL
#define PAGE_SIZE 4096
#define OPEN_MAX 12
#define O_STATMSK 0x101c00

#define kalloc(s) calloc(s,1)
#define kfree  free
#define kprintf printf
void* malloc (size_t);
void* calloc (size_t, size_t);
void free (void*);
int printf(const char*, ...);
#else

#define printf kprintf
#define malloc kalloc
#define free kfree
#endif


// ======================================================

struct kTty
{
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
  uint32_t* _ptr;
  uint32_t  _length;
};


#endif /* KCORE_H__ */
