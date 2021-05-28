#pragma once

#include <stdarg.h>

int printf(const char *format, ...);
int vprintf(const char *format, va_list arg);
int puts(const char *str);
