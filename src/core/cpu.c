#include <kcore.h>

int32_t _xchg_i32 (int32_t* ref, int32_t val)
{
  int32_t ex = *ref;
  *ref = val;
  return ex;
}

void _inc_i32 (int32_t* ref)
{
  (*ref)++;
}

void _dec_i32 (int32_t* ref)
{
  (*ref)--;
}


