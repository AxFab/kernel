#include <errno.h>
#include <stddef.h>
#ifndef _SKC_SPLOCK_H
# include <skc/splock.h>
#endif

#define __static_inline  static __inline

#ifndef _AT_
# define __TOSTR(s) #s
# define __STRING(s) __TOSTR(s)
# define _AT_ __FILE__ ":" __STRING(__LINE__)
#endif

#define sp_lock(lock)     sp_lock_(lock,1,_AT_)
#define sp_unlock(lock)   sp_unlock_(lock,1)
#define sp_trylock(lock)  sp_trylock_(lock,1,_AT_)

#define rdi_lock(lock)    rdi_lock_(lock,1,_AT_)
#define rdi_unlock(lock)  rdi_unlock_(lock,1)
#define rdi_trylock(lock) rdi_trylock_(lock,1,_AT_)
#define wri_lock(lock)    wri_lock_(lock,1,_AT_)
#define wri_unlock(lock)  wri_unlock_(lock,1)
#define wri_trylock(lock) wri_trylock_(lock,1,_AT_)
#define rdi_upgrade(lock) rdi_upgrade(lock,1, _AT_)

#define rd_lock(lock)     rd_lock_(lock,0,_AT_)
#define rd_unlock(lock)   rd_unlock_(lock,0)
#define rd_trylock(lock)  rd_trylock_(lock,0,_AT_)
#define wr_lock(lock)     wr_lock_(lock,0,_AT_)
#define wr_unlock(lock)   wr_unlock_(lock,0)
#define wr_trylock(lock)  wr_trylock_(lock,0,_AT_)
#define rd_upgrade(lock)  rd_upgrade(lock,0, _AT_)


// http://locklessinc.com/articles/locks/

__static_inline void sp_lock_(splock_t *lock, int irq, const char *at)
{
  if (irq)
    cpu_irq_off();
	for (;;) {
		if (!atomic_xchg(&lock->spin_, 1)) {
      lock->cpu_ = cpu_no();
      lock->at_ = at;
      return;
    }
    while (atomic_read(&lock->spin_)) 
      cpu_relax();
	}
}

__static_inline void sp_unlock_(splock_t *lock, int irq)
{
  lock->cpu_ = -1;
  lock->at_ = NULL;
  atomic_set(&lock->spin_, 0);
  if (irq)
    cpu_irq_on();
}

__static_inline int sp_trylock_(splock_t *lock, int irq, const char *at)
{
  if (irq)
    cpu_irq_off();
  if (atomic_xchg(&lock->spin_, 1) == 1) {
    if (irq)
      cpu_irq_on();
    return EBUSY;
  }
  
  lock->cpu_ = cpu_no();
  lock->at_ = at;
  return 0;
}

__static_inline int sp_locked(splock_t *lock)
{
  return (atomic_read(&lock->spin_) == 0) ? 0 : EBUSY;
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* Lock for write purpose */
__static_inline void wr_lock_(rwlock_t *lock, int irq, const char *at)
{
	sp_lock_(&lock->spl_, irq, at);
	
	while (atomic_read(&lock->readers_)) 
    cpu_relax();
}

__static_inline void wr_unlock_(rwlock_t *lock, const char *at)
{
  sp_unlock_(&lock->spl_, at);
}

__static_inline int wr_trylock_(rwlock_t *lock, int irq, const char *at)
{
  if (atomic_read(&lock->readers_))
    return EBUSY;
	
  if (sp_trylock_(&lock->spl_, irq, at)) 
    return EBUSY;
	
	if (atomic_read(&lock->readers_)) {
		sp_unlock_(&lock->spl_, irq);
		return EBUSY;
	}

	return 0;
}

__static_inline int wr_locked(rwlock_t *lock)
{
  return sp_locked(&lock->spl_);
}

__static_inline void rd_lock_(rwlock_t *lock, int irq, const char *at) 
{
	for (;;) {
		atomic_inc(&lock->readers_);
    if (!sp_locked(&lock->spl_)) 
      return;

		atomic_dec(&lock->readers_);
		while (sp_locked(&lock->spl_))
      cpu_relax();
	}
}

__static_inline void rd_unlock_(rwlock_t *lock, int irq)
{
	atomic_dec(&lock->readers_);
}

__static_inline int rd_trylock_(rwlock_t *lock, int irq, const char *at)
{
	atomic_inc(&lock->readers_);
  if (!sp_locked(&lock->spl_)) 
    return 0;
	
	atomic_dec(&lock->readers_);
	return EBUSY;
}

__static_inline int rd_locked(rwlock_t *lock)
{
  return atomic_read(&lock->readers_) > 0;
}

__static_inline int rd_upgrade_(rwlock_t *lock, int irq, const char *at)
{
  if (sp_trylock_(&lock->spl_, irq, at)) 
    return EBUSY;
	
	atomic_dec(&lock->readers_);
	while (atomic_read(&lock->readers_)) 
    cpu_relax();
	
	return 0;
}
