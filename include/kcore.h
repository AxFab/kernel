#ifndef KCORE_H__
#define KCORE_H__

// Config AxLibC - Get stdlib basic version (no syscalls)
#ifndef __EX
#define __EX
#endif

// Standard includes
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <limits.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>

/* MISSING STDLIB */
int snprintf(char* s, size_t n, const char* format, ... );

typedef struct spinlock     spinlock_t;
struct spinlock {
  int32_t   key_;
  int       cpu_;
  int       why_;
};

/** ltime_t is an accurate time storage
 *  it hold the number of microsecond since 1st Jan 1900 */
typedef int64_t ltime_t;
ltime_t ltime (ltime_t* ptr);

/* END OF STDLIB */

// Configuration header
#include <kconfig.h>

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
// inodes.h
typedef struct kStat        kStat_t;
typedef struct kInode       kInode_t;
typedef struct kFsys        kFsys_t;
typedef struct kDevice      kDevice_t;
typedef struct kResxFile    kResxFile_t;
typedef struct kFileOp      kFileOp_t;
// memory.h
typedef struct kVma         kVma_t;
typedef struct kAddSpace    kAddSpace_t;
// tasks.h
typedef struct kProcess     kProcess_t;
typedef struct kTask        kTask_t;
typedef struct kAssembly    kAssembly_t;
typedef struct kSection     kSection_t;


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
void kstacktrace(uintptr_t MaxFrames);
void kdump (void* ptr, size_t lg);

// Print ------------------------------------------------
int kputc(int c);
int kprintf(const char* str, ...);
const char* kpsize (uintmax_t number);

// Alloc ------------------------------------------------
void* kalloc(size_t size);
void kfree(void* addr);
char* kcopystr(const char* str);

// Lock -------------------------------------------------
void klock (spinlock_t* lock, int why);
int ktrylock (spinlock_t* lock, int why);
void kunlock (spinlock_t* lock) ;
int kislocked (spinlock_t* lock);
int klockcount ();


// ======================================================
#define MIN(a,b)  ((a) <= (b) ? (a) : (b))
#define MAX(a,b)  ((a) >= (b) ? (a) : (b))

#ifndef __KERNEL
#define PAGE_SIZE 4096
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
#define THREAD_ID 0x1793

// ======================================================

#endif /* KCORE_H__ */
