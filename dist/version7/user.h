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
typedef int pid_t;

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

#define errno geterrno()

#define MAX_MESSAGE_SIZE 1024

typedef struct message {
  pid_t from;
  unsigned int tag;
  size_t size;
  char data[MAX_MESSAGE_SIZE];
} message;

/* System calls */

pid_t getpid(void);
void exit(int status);
ssize_t write(int fd, const void *buf, size_t count);
ssize_t read(int fd, void *buf, size_t count);
int geterrno(void);
int brk(void *end_data_segment);
int send(pid_t to, unsigned int tag, const void *data, size_t size);
int receive(message *msg, int block);
int close(int fd);
int pipe(int filedes[2]);
int dup2(int oldfd, int newfd);
int kill(pid_t pid);

/* Memory allocation */

void *malloc(unsigned int nbytes);
void free(void *ptr);

#endif /* USER_H */
