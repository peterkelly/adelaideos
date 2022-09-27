/**
 * This file is part of AdelaideOS (http://adelaideos.sourceforge.net)
 * Original version developed by Peter Kelly <kellypmk@gmail.com>
 * The contents of this file are in the public domain.
 */

#include "kernel.h"

screenchar *screen = (screenchar*)VIDEO_MEMORY;
extern char kbdmap[128];
extern char kbdmap_shift[128];

unsigned int xpos = 0;
unsigned int ypos = 0;

unsigned int shift_pressed = 0;
unsigned int timer_ticks = 0;
extern process processes[MAX_PROCESSES];

/*==============================================================================
scroll

Move lines 1-24 up by one line, and clear the last line.
==============================================================================*/
static void scroll()
{
  unsigned int y;
  unsigned int x;

  /* Move all lines up one */
  for (y = 0; y < SCREEN_HEIGHT-1; y++)
    for (x = 0; x < SCREEN_WIDTH; x++) {
      screen[y*SCREEN_WIDTH+x] = screen[(y+1)*SCREEN_WIDTH+x];
  }

  /* Clear the last line */
  for (x = 0; x < SCREEN_WIDTH; x++)
    screen[y*SCREEN_WIDTH+x].c = ' ';
  ypos--;
}

/*==============================================================================
write_to_screen

Prints a series of characters to the screen, by writing directly to video
memory. The position of the cursor is updated accordingly. The screen is
scrolled if the text goes beyond the end of the last line.
==============================================================================*/
void write_to_screen(const char *data, unsigned int count)
{
  unsigned int i;
  for (i = 0; i < count; i++) {
    char c = data[i];

    if ('\n' == c) {
      /* newline */
      xpos = 0;
      ypos++;
    }
    else if ('\t' == c) {
      /* tab */
      int add = 8-(xpos%8);
      xpos += add;
    }
    else {
      /* normal character */
      screen[ypos*SCREEN_WIDTH+xpos].c = c;
      xpos++;
    }
    if (xpos >= SCREEN_WIDTH) {
      xpos = 0;
      ypos++;
    }
    if (ypos >= SCREEN_HEIGHT) {
      scroll();
    }
  }
  move_cursor(xpos,ypos);
}

/*==============================================================================
timer_handler

This function is called every time a timer interrupt occurs, which happens 50
times per second. You can implement any logic here that needs to happen at
regular intervals (e.g. a context switch to the next process).
==============================================================================*/
void timer_handler(regs *r)
{
  timer_ticks++;

  /* After 5 seconds, suspend the first process */
  if (5*TICKS_PER_SECOND == timer_ticks) {
    kprintf("Suspending process A\n");
    suspend_process(&processes[1]);
  }

  /* After 10 seconds, resume the first process */
  if (10*TICKS_PER_SECOND == timer_ticks) {
    kprintf("Resuming process A\n");
    resume_process(&processes[1]);
  }

  /* After 15 seconds, kill the second process */
  if (15*TICKS_PER_SECOND == timer_ticks) {
    kprintf("Killing process B\n");
    kill_process(&processes[2]);
  }

  /* After 20 seconds, kill the first process */
  if (20*TICKS_PER_SECOND == timer_ticks) {
    kprintf("Killing process A\n");
    kill_process(&processes[1]);
  }

  context_switch(r);
}

/*==============================================================================
keyboard_handler

This function is called every time a key is pressed or released. The lowest 7
bits of the scancode indicate which key it was, and the highest bit indicates
whether the event was a key press or key release.

The key values correspond to the entries in the kbdmap and kbdmap_shift arrays,
or for special keys (e.g. arrow keys) the KEY_* macros defined in constants.h
==============================================================================*/
void keyboard_handler(regs *r, unsigned char scancode)
{
  unsigned char key = scancode & 0x7F;

  if (scancode & 0x80) {
    /* key release */
    if (KEY_SHIFT == key)
      shift_pressed = 0;
  }
  else {
    /* key press */
    if (KEY_SHIFT == key) {
      shift_pressed = 1;
    }
    else {
      char c = shift_pressed ? kbdmap_shift[key] : kbdmap[key];
      write_to_screen(&c,1);
    }
  }

  context_switch(r);
}

/*==============================================================================
process_a

Simple test process. Just repeatedly prints out a string, reporting the number
of iterations done so far.
==============================================================================*/
void process_a()
{
  int iterations = 0;
  while (1) {
    int i;
    for (i = 0; i < 5000000; i++);
    printf("I am process A, iterations = %d\n",iterations);
    iterations++;
  }
}

/*==============================================================================
process_b

Simple test process. Just repeatedly prints out a string, reporting the number
of iterations done so far.
==============================================================================*/
void process_b()
{
  int iterations = 0;
  while (1) {
    int i;
    for (i = 0; i < 5000000; i++);
    printf("I am process B, iterations = %d\n",iterations);
    iterations++;
  }
}

/*==============================================================================
kernel_main

This is the first thing that executes when the kernel starts. Any initialisation
e.g. interrupt handlers must be done at the start of the function. This function
should never return.
==============================================================================*/
void kernel_main(void)
{
  setup_segmentation();
  setup_interrupts();

  /* Clear the screen */
  unsigned int x;
  unsigned int y;
  for (y = 0; y < SCREEN_HEIGHT; y++)
    for (x = 0; x < SCREEN_WIDTH; x++)
      screen[y*SCREEN_WIDTH+x].c = ' ';

  /* Place the cursor on line 0 to start with */
  move_cursor(xpos,ypos);

  kprintf("Welcome to AdelaideOS\n");

  /* Start two processes */
  start_process(process_a);
  start_process(process_b);

  /* Go in to user mode and enable interrupts */
  enter_user_mode();

  /* Loop indenitely... we should never return from this function */
  /* Pretty soon a context switch will occur, and the processor will jump
     out of this loop and start executing the first scheduled process */
  while (1);
}
