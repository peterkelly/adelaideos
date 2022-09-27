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
pipe_buffer *input_pipe = NULL;
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
      if (input_pipe)
        write_to_pipe(input_pipe,&c,1);
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
    printf("I am process A (pid %d), iterations = %d\n",getpid(),iterations);
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
    printf("I am process B (pid %d), iterations = %d\n",getpid(),iterations);
    iterations++;
  }
}

/*==============================================================================
uppercase

A filter which converts all of the data passed to its standard input (fd 0) to
uppercase, and then sends the result to its standard output (fd 1).
==============================================================================*/
void uppercase()
{
  while (1) {
    char buf[BUFSIZE+1];
    int r;
    while (0 < (r = read(0,buf,BUFSIZE))) {
      int i;
      for (i = 0; i < r; i++) {
        if (('a' <= buf[i]) && ('z' >= buf[i]))
          buf[i] += 'A'-'a';
      }
      write(STDOUT_FILENO,buf,r);
    }
  }
  exit(0);
}

/*==============================================================================
number_lines

A filter which prints the line number next to each line. This works in a similar
manner to uppercase; it reads data from standard input, manipulates it, and then
writes the modified data to standard output.
==============================================================================*/
void number_lines()
{
  while (1) {
    char buf[BUFSIZE+1];
    int r;
    int lineno = 0;
    printf("%3d ",lineno);
    while (0 < (r = read(0,buf,BUFSIZE))) {
      int i;
      for (i = 0; i < r; i++) {
        write(STDOUT_FILENO,&buf[i],1);
        if ('\n' == buf[i]) {
          lineno++;
          printf("%3d ",lineno);
        }
      }
    }
  }
  exit(0);
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
  kmalloc_init();

  /* Clear the screen */
  unsigned int x;
  unsigned int y;
  for (y = 0; y < SCREEN_HEIGHT; y++)
    for (x = 0; x < SCREEN_WIDTH; x++)
      screen[y*SCREEN_WIDTH+x].c = ' ';

  /* Place the cursor on line 0 to start with */
  move_cursor(xpos,ypos);

  kprintf("Welcome to AdelaideOS\n");
  kprintf("Pipe test; enter some input below:\n");

  /* Start two processes */
  pid_t pid1 = start_process(uppercase);
  pid_t pid2 = start_process(number_lines);

  /* Send characters entered via the keyboard to the input of process 1 */
  input_pipe = processes[pid1].filedesc[STDIN_FILENO]->p;

  /* Send the output of process 1 to the input of process 2 */
  pipe_buffer *p1p2pipe = new_pipe();
  processes[pid1].filedesc[STDOUT_FILENO] = new_pipe_writer(p1p2pipe);
  processes[pid2].filedesc[STDIN_FILENO] = new_pipe_reader(p1p2pipe);

  /* Process 2 will write to the screen (which is set up as the default within
     start_process) */

  /* Go in to user mode and enable interrupts */
  enter_user_mode();

  /* Loop indenitely... we should never return from this function */
  /* Pretty soon a context switch will occur, and the processor will jump
     out of this loop and start executing the first scheduled process */
  while (1);
}
