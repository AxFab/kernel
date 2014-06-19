#ifndef KCPU_H__
#define KCPU_H__

#include <kcore.h>

uint32_t _xchg_u32 (uint32_t* ref, uint32_t val);
int32_t _xchg_i32 (int32_t* ref, int32_t val);

void _inc_i32 (int32_t* ref);
void _dec_i32 (int32_t* ref);

#endif /* KCPU_H__ */
