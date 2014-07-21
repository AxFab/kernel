#ifndef PAGES_H__
#define PAGES_H__

#include <kcore.h>

uintptr_t kPg_AllocPage (void);
void kPg_ReleasePage (uintptr_t page);

int kPg_PreSystem (void);
int kPg_AddRam (uint64_t base, uint64_t length);
int kPg_Initialize (void);

#endif /* PAGES_H__ */
