/**
 * This file is part of AdelaideOS (http://adelaideos.sourceforge.net)
 * Original version developed by Peter Kelly <kellypmk@gmail.com>
 * The contents of this file are in the public domain.
 */

#include "constants.h"

#ifdef USE_LIBC
#ifndef USER_H
#define USER_H

#define assert(e) ((void) ((e) ? 0 : __assert (#e, __FUNCTION__)))
void __assert(const char *str, const char *function);

/* Typedefs */
typedef int ssize_t;
typedef unsigned int size_t;
#ifdef USE_PROCESSES
typedef int pid_t;
#endif
#ifdef USE_UNIXPROC

/* Filesystem data structures */
struct stat {
  unsigned int st_mode;
  unsigned int st_uid;
  unsigned int st_gid;
  unsigned int st_size;
  unsigned int st_mtime;
};

/* Mode bits as used by Linux */
#define S_IFMT     0170000
#define S_IFREG    0100000
#define S_IFDIR    0040000
#define S_IRWXU    00700
#define S_IRUSR    00400
#define S_IWUSR    00200
#define S_IXUSR    00100
#define S_IRWXG    00070
#define S_IRGRP    00040
#define S_IWGRP    00020
#define S_IXGRP    00010
#define S_IRWXO    00007
#define S_IROTH    00004
#define S_IWOTH    00002
#define S_IXOTH    00001

/* Structures used by opendir/readdir/closedir */

struct dirent {
  unsigned int d_ino;
  char d_name[NAME_MAX+1];
};

typedef struct DIR {
  int fd;
  struct dirent ent;
} DIR;
#endif
#ifdef USE_LIBC

/* Standard C library functions - libc.c */

#define va_list unsigned int
#define va_start(_ap,_fmt) _ap = ((unsigned int)&_fmt)+sizeof(_fmt);
#define va_arg(_ap,_type) \
  ({ _type *_v = (_type*)_ap; _ap += sizeof(_type); *_v; })
#define va_copy(_dest,_src) { (_dest) = (_src); }
#define va_end(_ap)

#ifdef USE_UNIXPROC
char *strerror(int errnum);
void perror(const char *s);
#endif
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
#ifdef USE_UNIXPROC
DIR *opendir(const char *filename); /* actually a libc function */
struct dirent *readdir(DIR *dirp); /* actually a libc function */
int closedir(DIR *dirp); /* actually a libc function */
#endif
#endif /* USE_LIBC */
#ifdef USE_SYSCALLS

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
#ifdef USE_MALLOC
int brk(void *end_data_segment);
int send(pid_t to, unsigned int tag, const void *data, size_t size);
int receive(message *msg, int block);
#endif
#ifdef USE_FILEDESC
int close(int fd);
int pipe(int filedes[2]);
int dup2(int oldfd, int newfd);
#endif
#ifdef USE_UNIXPROC
pid_t fork(void);
int execve(const char *filename, char *const argv [], char *const envp[]);
pid_t waitpid(pid_t pid, int *status, int options);
int stat(const char *path, struct stat *buf);
int open(const char *pathname, int flags);
int getdent(int fd, struct dirent *entry);
int chdir(const char *path);
char *getcwd(char *buf, size_t size);
#endif
int kill(pid_t pid);
#endif /* USE_SYSCALLS */
#ifdef USE_MALLOC

/* Memory allocation */

void *malloc(unsigned int nbytes);
void free(void *ptr);
#endif /* USE_MALLOC */

#endif /* USER_H */
#endif /* USE_LIBC */
