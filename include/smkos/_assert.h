#pragma once

#include <smkos/_types.h>

/* Define some useful macros */
#define _Kb_ (1024L)
#define _Mb_ (1024L*_Kb_)
#define _Gb_ (1024LL*_Mb_)
#define _Tb_ (1024LL*_Gb_)
#define _Pb_ (1024LL*_Tb_)
#define _Eb_ (1024LL*_Pb_)

#define ALIGN_UP(v,a)      (((v)+(a-1))&(~(a-1)))
#define ALIGN_DW(v,a)      ((v)&(~(a-1)))

#define MIN(a,b)    ((a)<=(b)?(a):(b))
#define MAX(a,b)    ((a)>=(b)?(a):(b))
#define POW2(v)   ((v) != 0 && ((v) & ((v)-1)) == 0)

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define __AT__  __FILE__ ":" TOSTRING(__LINE__)

#if 0
#  define assert(e) ((void)0)
#  define DEBUG(c)  ((void)0)
#elif 0
#  define assert(e)   ((e) ? ((void)0) : __assert_fail(#e, __AT__))
#  define DEBUG(c)    do { c } while(0)
void __assert_fail(const char *ex, const char *at);
#else
#  define assert(e)   __assert_do(e,#e, __AT__)
#  define DEBUG(c)    do { c } while(0)
void __assert_do(int as, const char *ex, const char *at);
#endif


int cpu_no();
#define kCpuNo cpu_no()

#define KLOG_TRACE "[info] "
#define NO_MMU 1

enum KDR_TYPE {
  KDR_NONE = 0,
  KDR_FS,
  KDR_BK,
};

