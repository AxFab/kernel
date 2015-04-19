#pragma once
#include <smkos/kernel.h>
#include <smkos/_compiler.h>


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


// /* ----------------------------------------------------------------------- */
// static inline void outb (uint16_t port, uint8_t value) 
// {
//   asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port) );
// }


// /* ----------------------------------------------------------------------- */
// static inline void outw (uint16_t port, uint16_t value)
// {
//   asm volatile ("outw %0, %1" : : "a"(value), "Nd"(port) );
// }


// /* ----------------------------------------------------------------------- */
// static inline void outl (uint16_t port, uint32_t value)
// {
//   asm volatile ("outl %0, %1" : : "a"(value), "Nd"(port) );
// }


// /* ----------------------------------------------------------------------- */
// static inline uint8_t inb (uint16_t port)
// {
//   uint8_t ret;
//   asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port) );
//   return ret;
// }


// /* ----------------------------------------------------------------------- */
// static inline uint16_t inw (uint16_t port)
// {
//   uint16_t ret;
//   asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port) );
//   return ret;
// }


// /* ----------------------------------------------------------------------- */
// static inline uint32_t inl (uint16_t port)
// {
//   uint32_t ret;
//   asm volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port) );
//   return ret;
// }


// /* ----------------------------------------------------------------------- */
// static inline void insb (uint8_t port, void *addr, int count)
// {
//   uint8_t* buf = (uint8_t*)addr;
//   while (count--) {
//     *buf = inb(port);
//     ++buf;
//   }
// }


// /* ----------------------------------------------------------------------- */
// static inline void insw (uint16_t port, void *addr, int count)
// {
//   uint16_t* buf = (uint16_t*)addr;
//   while (count--) {
//     *buf = inw(port);
//     ++buf;
//   }
// }


// /* ----------------------------------------------------------------------- */
// static inline void insl (uint16_t port, void *addr, int count) 
// {
//   uint32_t* buf = (uint32_t*)addr;
//   while (count--) {
//     *buf = inl(port);
//     ++buf;
//   }
// }


// /* ----------------------------------------------------------------------- */
// static inline void outsb (uint8_t port, void *addr, int count)
// {
//   uint8_t* buf = (uint8_t*)addr;
//   while (count--) {
//     outb(*buf, port);
//     ++buf;
//   }
// }


// /* ----------------------------------------------------------------------- */
// static inline void outsw (uint16_t port, void *addr, int count)
// {
//   uint16_t* buf = (uint16_t*)addr;
//   while (count--) {
//     outw(*buf, port);
//     ++buf;
//   }
// }


// /* ----------------------------------------------------------------------- */
// static inline void outsl (uint32_t port, void *addr, int count)
// {
//   uint32_t* buf = (uint32_t*)addr;
//   while (count--) {
//     outl(*buf, port);
//     ++buf;
//   }
// }


// /* ----------------------------------------------------------------------- */
// /* ----------------------------------------------------------------------- */
