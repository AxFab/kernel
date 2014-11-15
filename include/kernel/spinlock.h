#ifndef KERNEL_SPINLOCK_H__
#define KERNEL_SPINLOCK_H__

#include <kernel/core.h>


typedef struct spinlock     spinlock_t;
struct spinlock {
  int32_t       key_;
  int           cpu_;
  const char*   where_;
};


void klock (spinlock_t* lock, const char* where);
int ktrylock (spinlock_t* lock, const char* where);
void kunlock (spinlock_t* lock) ;
int kislocked (spinlock_t* lock);
int klockcount ();


#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define __AT__  __FILE__ ":" TOSTRING(__LINE__)
#define klock(l,...)  klock(l,__AT__)
#define ktrylock(l,...)  ktrylock(l,__AT__)


#endif /* KERNEL_SPINLOCK_H__ */
