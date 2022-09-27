/**
 * This file is part of AdelaideOS (http://adelaideos.sourceforge.net)
 * Original version developed by Peter Kelly <kellypmk@gmail.com>
 * The contents of this file are in the public domain.
 */

#ifndef KERNEL_H
#define KERNEL_H

#include "constants.h"
#include "user.h"

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

/* start.s */
void set_gdt(void *gp);
void set_tss(unsigned int tss_seg);
void idle(void);
unsigned char inb(unsigned int port);
void outb(unsigned int port, unsigned int data);
void enable_interrupts(void);


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
  unsigned int stack_start;
  unsigned int stack_end;
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

#endif /* KERNEL_H */
