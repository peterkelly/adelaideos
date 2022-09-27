/**
 * This file is part of AdelaideOS (http://adelaideos.sourceforge.net)
 * Original version developed by Peter Kelly <kellypmk@gmail.com>
 * The contents of this file are in the public domain.
 */

#include "constants.h"

#ifndef USER_H
#define USER_H

#define assert(e) ((void) ((e) ? 0 : __assert (#e, __FUNCTION__)))
void __assert(const char *str, const char *function);

/* Typedefs */
typedef int ssize_t;
typedef unsigned int size_t;

/* Standard C library functions - libc.c */

#define va_list unsigned int
#define va_start(_ap,_fmt) _ap = ((unsigned int)&_fmt)+sizeof(_fmt);
#define va_arg(_ap,_type) \
  ({ _type *_v = (_type*)_ap; _ap += sizeof(_type); *_v; })
#define va_copy(_dest,_src) { (_dest) = (_src); }
#define va_end(_ap)

void *memset(void *b, int c, size_t len);
void *memmove(void *dst, const void *src, size_t len);
void *memcpy(void *dst, const void *src, size_t len);
size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);
int snprintf(char *str, size_t size, const char *format, ...);
int kprintf(const char *format, ...);
int printf(const char *format, ...);
void user_mode_assert(const char *str, const char *function);

#endif /* USER_H */
