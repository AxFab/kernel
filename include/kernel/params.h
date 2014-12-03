#ifndef PARAMS_H__
#define PARAMS_H__

#include <kernel/config.h>

#define PARAM_KOBJECT(p,t)          (p != NULL)
#define PARAM_FILENAME(n)           (n != NULL && strnlen(n, MAX_STRING_LENGTH) < MAX_STRING_LENGTH)
#define PARAM_KERNEL_BUFFER(b,s,a)  (b != NULL && ((unsigned long)b & (a-1)) == 0)

#define PARAM_USER_BUFFER(m,b,l)    (b != NULL)
#define PARAM_USER_STRING(m,s,l)    (s != NULL && strnlen(s, MIN(MAX_STRING_LENGTH,l)) < MIN(MAX_STRING_LENGTH,l))


#define kprintf(n,s,...) do {                       \
          if (n) kprintf(n##_PFX s,##__VA_ARGS__);  \
        } while(0)

// On module, we should have mutex on drivers, in order to restore interupt.
#define MODULE_ENTER(i,d)   do {          \
          klock(d);                       \
          kunlock(i);                     \
        } while (0);
          // assert (klockcount() == 1);

#define MODULE_LEAVE(i,d)   do {          \
          klock(i);                       \
          kunlock(d);                     \
        } while (0);
          // assert (klockcount() == 1);


#endif /* PARAMS_H__ */
