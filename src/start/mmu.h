#ifndef MMU_H__
#define MMU_H__

#include <kcore.h>

typedef unsigned int mpage_t;

// mpage_t kMmu_GetPage ();
// void kMmu_ReleasePage (mpage_t page);
void kMmu_Build ();
void kMmu_Initialize (void);
void kMmu_SetRAM (unsigned long long base, unsigned long long length);
void kMmu_Ready (unsigned long long max) ;



#endif /* MMU_H__ */
