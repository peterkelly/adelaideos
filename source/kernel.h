/**
 * This file is part of AdelaideOS (http://adelaideos.sourceforge.net)
 * Original version developed by Peter Kelly <kellypmk@gmail.com>
 * The contents of this file are in the public domain.
 */

#ifndef KERNEL_H
#define KERNEL_H

#include "constants.h"
#ifdef USE_LIBC
#include "user.h"
#endif
#ifdef USE_UNIXPROC
#include "filesystem.h"
#endif
#ifdef USE_MALLOC
#include "buddy.h"
#endif

/* Basic system data structures */

typedef struct {
  char c;
  unsigned char fg : 4;
  unsigned char bg : 4;
} __attribute__ ((__packed__)) screenchar;

typedef struct {
  unsigned int fstate[27];
  unsigned int gs, fs, es, ds;
  unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
  unsigned int int_no, err_code;
  unsigned int eip, cs, eflags, useresp, ss;
} __attribute__ ((packed)) regs;

/* main.c */
void timer_handler(regs *r);
void keyboard_handler(regs *r, unsigned char scancode);
void write_to_screen(const char *data, unsigned int count);

/* segmentation.c */
void setup_segmentation(void);

/* interrupts.c */
void fatal(const char *str);
void setup_interrupts(void);
void move_cursor(int x, int y);
#ifdef USE_PAGING

/* page.c */

#define PAGE_ADDRESS_MASK     0xFFFFF000
#define PAGE_USER             0x4
#define PAGE_SUPERVISOR       0
#define PAGE_READ_WRITE       0x2
#define PAGE_READ_ONLY        0
#define PAGE_PRESENT          0x1

typedef unsigned int *page_dir;
typedef unsigned int *page_table;

void *alloc_page(void);
void free_page(void *page);
void map_page(page_dir pdir, unsigned int logical, unsigned int physical,
              unsigned int access, unsigned int readwrite);
#ifdef USE_UNIXPROC
int lookup_page(page_dir pdir, unsigned int logical, unsigned int *phys);
#endif
void unmap_and_free_page(page_dir pdir, unsigned int logical);
void identity_map(page_dir pdir, unsigned int start, unsigned int end,
                  unsigned int access, unsigned int readwrite);
void map_new_pages(page_dir pdir, unsigned int base, unsigned int npages);
void free_page_dir(page_dir pdir);
#endif

/* start.s */
void set_gdt(void *gp);
void set_tss(unsigned int tss_seg);
void idle(void);
unsigned char inb(unsigned int port);
void outb(unsigned int port, unsigned int data);
#ifdef USE_PAGING
void enter_user_mode(void);
void enable_paging(page_dir pdir);
void disable_paging(void);
unsigned int getcr2(void);
#else
void enable_interrupts(void);
#endif
#ifdef USE_SYSCALLS
int in_user_mode(void);
#endif

#ifdef USE_FILEDESC

/* pipe.c */

typedef struct pipe_buffer {
  unsigned int reading;
  unsigned int writing;
  unsigned int alloc;
  unsigned int len;
  pid_t readpid;
  char *data;
} pipe_buffer;

pipe_buffer *new_pipe(void);
void write_to_pipe(pipe_buffer *p, const void *buf, size_t count);

#define FH_NONE          0
#define FH_SCREEN        1
#define FH_PIPE_WRITER   2
#define FH_PIPE_READER   3
#ifdef USE_ANSWERS
#define FH_FILE          4
#define FH_DIR           5

#define OPEN_DIRECTORY   0xFFFF
#endif

typedef struct filehandle filehandle;

typedef ssize_t write_fun(filehandle *fh, const void *buf, size_t count);
typedef ssize_t read_fun(filehandle *fh, void *buf, size_t count);
typedef void destroy_fun(filehandle *fh);

struct filehandle {
  int type;
  int refcount;
  write_fun *write;
  read_fun *read;
  destroy_fun *destroy;
  pipe_buffer *p; /* for pipes */
  #ifdef USE_ANSWERS
  struct directory_entry *entry;
  unsigned int pos; /* for files */
  unsigned int entryno; /* for directories */
  #endif
};

/* pipe.c */

filehandle *new_pipe_writer(pipe_buffer *p);
filehandle *new_pipe_reader(pipe_buffer *p);
#endif /* USE_FILEDESC */
#ifdef USE_PROCESSES

/* Macros for manipulating doubly-linked lists (used by the process list) */

#define list_add(_ll,_obj) {               \
    if ((_ll)->first) {                    \
      (_obj)->next = (_ll)->first;         \
      (_ll)->first->prev = (_obj);         \
      (_ll)->first = (_obj);               \
    }                                      \
    else {                                 \
      (_ll)->first = (_ll)->last = (_obj); \
    }                                      \
  }

#define list_remove(_ll,_obj) {            \
    if ((_ll)->first == (_obj))            \
      (_ll)->first = (_obj)->next;         \
    if ((_ll)->last == (_obj))             \
      (_ll)->last = (_obj)->prev;          \
    if ((_obj)->next)                      \
      (_obj)->next->prev = (_obj)->prev;   \
    if ((_obj)->prev)                      \
      (_obj)->prev->next = (_obj)->next;   \
    (_obj)->next = NULL;                   \
    (_obj)->prev = NULL;                   \
  }

/* process.c */

typedef struct process {
  pid_t pid;            /* process identifier/index into process array */
  int exists;           /* whether or not this process slot is used */
  regs saved_regs;      /* saved state for a non-active process */
  int ready;            /* is this process read to execute? */
  struct process *prev; /* Pointers for ready/suspended lists */
  struct process *next;
  #ifdef USE_PAGING
  page_dir pdir;        /* page directory */
  #endif
  #ifdef USE_SYSCALLS
  int in_syscall;       /* is this process currently executing a system call? */
  int last_errno;
  #endif
  #ifdef USE_FILEDESC
  filehandle *filedesc[MAX_FDS];
  #endif
  #ifdef USE_ANSWERS
  char cwd[PATH_MAX];
  #endif
  unsigned int stack_start;
  unsigned int stack_end;
  #ifdef USE_MALLOC
  unsigned int data_start;
  unsigned int data_end;
  #endif
  #ifdef USE_UNIXPROC
  unsigned int text_start;
  unsigned int text_end;
  pid_t parent_pid;
  int exit_status;
  int exited;
  pid_t waiting_on;
  #endif
  #ifdef USE_ANSWERS
  message *mailbox;
  int mailbox_size;
  int mailbox_alloc;
  int receive_blocked;
  #endif
} process;

typedef struct {
  process *first;
  process *last;
} processlist;

pid_t next_free_pid(void);
void init_regs(regs *r, unsigned int stack_max, void (*start_addr)(void));
pid_t get_free_pid(void);
pid_t start_process(void (*start_address)(void));
void kill_process(process *proc);
void suspend_process(process *proc);
void resume_process(process *proc);
void context_switch(regs *r);
#endif /* USE_PROCESSES */
#ifdef USE_SYSCALLS

/* syscall.c */

int valid_pointer(const void *ptr, unsigned int size);
int valid_string(const char *str);
void syscall(regs *r);
#endif
#ifdef USE_MALLOC

/* buddy.c */

void kmalloc_init(void);
void *kmalloc(unsigned int nbytes);
void kfree(void *ptr);
#endif
#ifdef USE_FILEDESC

/* filedesc.c */

filehandle *new_screen_handle(void);
void close_filehandle(filehandle *fh);
#endif

#endif /* KERNEL_H */
