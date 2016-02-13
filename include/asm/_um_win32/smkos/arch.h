
#pragma once
#include <stdint.h>
#include <smkos/compiler.h>

typedef uint32_t page_t;

void outb (uint16_t port, uint8_t value);
void outw (uint16_t port, uint16_t value);
void outl (uint16_t port, uint32_t value);
uint8_t inb (uint16_t port);
uint16_t inw (uint16_t port);
uint32_t inl (uint16_t port);
void insb (uint16_t port, void *addr, int count);
void insw (uint16_t port, void *addr, int count);
void insl (uint16_t port, void *addr, int count);
void outsb (uint8_t port, void *addr, int count);
void outsw (uint16_t port, void *addr, int count);
void outsl (uint32_t port, void *addr, int count);
