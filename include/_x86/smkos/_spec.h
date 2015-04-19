#pragma once

#include <stddef.h>

int strcmpi (const char* str1, const char* str2);
int snprintf(char* buf, size_t lg, const char* fmt, ...);

#define exit_() for(;;)

void kwrite(const char* m);

