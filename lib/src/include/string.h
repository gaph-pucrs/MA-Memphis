#pragma once

#include <stddef.h>

void *memcpy(void *destionation, const void *source, size_t num);
char *strcpy(char *destination, const char *source);

void *memset(void * ptr, int value, size_t num);
size_t strlen(const char *str);
