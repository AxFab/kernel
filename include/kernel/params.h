#ifndef PARAMS_H__
#define PARAMS_H__

#include <kernel/config.h>

#define PARAM_KOBJECT(p,t)          1
#define PARAM_KERNEL_BUFFER(b,s,a)  1


#define kprintf(n,s,...)    kprintf(s,##__VA_ARGS__)


// On module, we should have mutex on drivers, in order to restore interupt.
#define MODULE_ENTER(i,d)   do {          \
          klock(d);                       \
          kunlock(d);                     \
          assert (klockcount() == 1); \
        } while (0);

#define MODULE_LEAVE(i,d)   do {          \
          klock(i);                       \
          kunlock(d);                     \
          assert (klockcount() == 1); \
        } while (0);


#endif /* PARAMS_H__ */
