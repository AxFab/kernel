#pragma once

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

#define XOR_32_TO_8(v)  (((v) & 0xff) ^ (((v) >> 8) & 0xff) ^ (((v) >> 16) & 0xff) ^ (((v) >> 24) & 0xff))

#if 0
#  define assert(e) ((void)0)
#  define DEBUG(c)  ((void)0)
#else
#  define assert(e)       __assert_do((int)(e),#e, __AT__)
#  define assert_msg(e,m)   __assert_do((int)(e),m, __AT__)
#  define DEBUG(c)    do { c } while(0)
void __assert_do(int as, const char *ex, const char *at);
#endif


/* Interface to get cpu_no() -- should precced kernel.h */
int cpu_no();
#define kCpuNo cpu_no()

