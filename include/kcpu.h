#ifndef KCPU_H__
#define KCPU_H__

#include <kcore.h>

uint32_t _xchg_u32 (uint32_t* ref, uint32_t val);
int32_t _xchg_i32 (int32_t* ref, int32_t val);

void _inc_i32 (int32_t* ref);
void _dec_i32 (int32_t* ref);



void outb (uint16_t port, uint8_t value);
void outw (uint16_t port, uint16_t value);
uint8_t inb (uint16_t port);
uint16_t inw (uint16_t port);

void insl (uint16_t port, void* addr, uint32_t count);
void insw (uint16_t port, void* addr, uint32_t count);
void outsw (uint16_t port, void* addr, uint32_t count);


#endif /* KCPU_H__ */
