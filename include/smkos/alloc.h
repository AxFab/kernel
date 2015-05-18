#pragma once
#include <stddef.h>
void *malloc_(size_t);
void *valloc_(size_t);
void free_(void *);
void alloc_init(size_t base, size_t length);