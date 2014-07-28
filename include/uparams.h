#ifndef UPARAMS_H__
#define UPARAMS_H__

#include <kcore.h>


int kUserParam_Buffer (kAddSpace_t* addp, void* base, size_t length);
int kUserParam_String (kAddSpace_t* addp, const char* str, int max);


#endif /* UPARAMS_H__ */
