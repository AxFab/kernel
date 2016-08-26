
#ifndef _SMKOS_KLIB_H
#define _SMKOS_KLIB_H 1

#include <limits.h>
#include <stdint.h>
#include <cdefs/stddef.h>
#include <sys/types.h>
#include <time.h>
#include <assert.h>
#include <errno.h>

typedef uint32_t page_t;

void *kalloc(size_t lg);
void kfree(void *);

int kprintf(const char *, ...);
void kpanic(const char *, ...);

void __seterrno(int err);

#define CSL_RED "\033[31m"
#define CSL_GREEN "\033[32m"
#define CSL_YELLOW "\033[33m"
#define CSL_BLUE "\033[34m"
#define CSL_MAGENTA "\033[35m"
#define CSL_CYAN "\033[36m"
#define CSL_GRAY "\033[37m"
#define CSL_RESET "\033[0m"

#endif  /* _SMKOS_KLIB_H */