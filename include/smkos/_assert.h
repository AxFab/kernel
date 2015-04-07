#pragma once

/* Define some useful macros */
#define _Kb_ (1024)
#define _Mb_ (1024*_Kb_)
#define _Gb_ (1024*_Mb_)
#define _Tb_ (1024*_Gb_)
#define _Pb_ (1024*_Tb_)
#define _Eb_ (1024*_Pb_)

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
#else
#  define assert(e)   __assert_do(e,#e, __AT__)
#  define DEBUG(c)    do { c } while(0)
#  define assert_pathname(n)  \
    __assert_do(__assert_pathname(n), #n " isn't a valid pathname", __AT__);
void __assert_do(int as, const char *ex, const char *at);
bool __assert_pathname (const char *name);
#endif

