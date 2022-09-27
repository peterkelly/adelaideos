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


#endif /* KERNEL_H */
