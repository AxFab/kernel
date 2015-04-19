#pragma once

#include <stddef.h>

int strcmpi (const char* str1, const char* str2);
int snprintf(char* buf, size_t lg, const char* fmt, ...);
char* strdup(const char* str);

#define kwrite(m) fputs(m, stderr)
#define exit_() exit (EXIT_FAILURE)

void *malloc_(size_t size);
void free_(void *addr);

#define time time
#define clock clock
