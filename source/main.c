/**
 * This file is part of AdelaideOS (http://adelaideos.sourceforge.net)
 * Original version developed by Peter Kelly <kellypmk@gmail.com>
 * The contents of this file are in the public domain.
 */

#include "kernel.h"
#ifdef USE_UNIXPROC
#include "filesystem.h"
#endif

screenchar *screen = (screenchar*)VIDEO_MEMORY;
extern char kbdmap[128];
extern char kbdmap_shift[128];

unsigned int xpos = 0;
unsigned int ypos = 0;

unsigned int shift_pressed = 0;
unsigned int timer_ticks = 0;
#ifdef USE_UNIXPROC

char *filesystem;
#endif
#ifdef USE_FILEDESC
pipe_buffer *input_pipe = NULL;
#endif
#ifdef USE_PROCESSES
extern process processes[MAX_PROCESSES];
#endif

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
#ifndef USE_LIBC
  if (0 == timer_ticks % (5*TICKS_PER_SECOND))
    write_to_screen("Hello from timer_handler\n",25);
#endif
#ifdef USE_PROCESSES
#ifndef USE_FILEDESC

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
#endif /* USE_PAGING */

  context_switch(r);
#endif /* USE_PROCESSES */
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
      #ifdef USE_FILEDESC
      if (input_pipe)
        write_to_pipe(input_pipe,&c,1);
      #else
      write_to_screen(&c,1);
      #endif
    }
  }
  #ifdef USE_PROCESSES

  context_switch(r);
  #endif
}
#ifdef USE_PROCESSES

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
    #ifdef USE_SYSCALLS
    printf("I am process A (pid %d), iterations = %d\n",getpid(),iterations);
    #else
    printf("I am process A, iterations = %d\n",iterations);
    #endif
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
    #ifdef USE_SYSCALLS
    printf("I am process B (pid %d), iterations = %d\n",getpid(),iterations);
    #else
    printf("I am process B, iterations = %d\n",iterations);
    #endif
    iterations++;
  }
}
#endif
#ifdef USE_FILEDESC

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
#endif
#ifdef USE_UNIXPROC

/*==============================================================================
launch_shell

The first process launched at kernel startup. All this does is simply execute
the shell. It is now the only process created via start_process; everything else
starts via fork.

The reason for using a string on the stack here is that execve checks to see if
the string is located within the process's address space. Since all processes
load their executable image from the filesystem, the checks performed by
valid_pointer have been changed in this version of the kernel to forbid
processes from passing in pointers residing in the kernel's memory, which would
be the case if we passed in a pointer to a string that was compiled directly
into the kernel's image.
==============================================================================*/
void launch_shell()
{
  char program[100];
  snprintf(program,100,"/bin/sh");
  execve(program,NULL,NULL);
  printf("/bin/sh: execve failed: %d\n",errno);
  exit(1);
}
#endif

/*==============================================================================
kernel_main

This is the first thing that executes when the kernel starts. Any initialisation
e.g. interrupt handlers must be done at the start of the function. This function
should never return.
==============================================================================*/
#ifdef USE_UNIXPROC
void kernel_main(multiboot *mb)
#else
void kernel_main(void)
#endif
{
  setup_segmentation();
  setup_interrupts();
  #ifdef USE_MALLOC
  kmalloc_init();
  #endif

  /* Clear the screen */
  unsigned int x;
  unsigned int y;
  for (y = 0; y < SCREEN_HEIGHT; y++)
    for (x = 0; x < SCREEN_WIDTH; x++)
      screen[y*SCREEN_WIDTH+x].c = ' ';

  #ifndef USE_LIBC
  /* Output to screen by writing directly to video memory */
  char *str = "Hello World";
  for (x = 0; str[x]; x++)
    screen[x].c = str[x];

  /* Place the cursor on line 1 to start with */
  ypos = 1;
  move_cursor(xpos,ypos);
  #else
  /* Place the cursor on line 0 to start with */
  move_cursor(xpos,ypos);

  kprintf("Welcome to AdelaideOS\n");
  #endif
  #ifdef USE_UNIXPROC

  assert(1 == mb->mods_count);
  assert(mb->mods_addr[0].mod_end < 2*MB);
  filesystem = (char*)mb->mods_addr[0].mod_start;
  /* Check here for the size of the RAM disk. Because we use a hard-coded
     value of 2MB for the start of the kernel's private data area, we can't
     safely work with filesystems that extend into this area. This is really
     just a hack to avoid the additional complexity of computing the right
     place to start the kernel and page memory regions, but suffices for our
     purposes. */
  if (mb->mods_addr[0].mod_end >= 2*MB)
    assert(!"Filesystem goes beyond 2Mb limit. Please use smaller filesystem.");

  #endif
  #ifdef USE_UNIXPROC

  pid_t pid = start_process(launch_shell);
  input_pipe = processes[pid].filedesc[STDIN_FILENO]->p;
  #else
  #ifdef USE_FILEDESC
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
  #else
  #ifdef USE_PROCESSES

  /* Start two processes */
  start_process(process_a);
  start_process(process_b);
  #endif
  #endif
  #endif

  #ifdef USE_PAGING
  /* Go in to user mode and enable interrupts */
  enter_user_mode();
  #else
  enable_interrupts();
  #endif

  /* Loop indenitely... we should never return from this function */
#ifdef USE_PROCESSES
  /* Pretty soon a context switch will occur, and the processor will jump
     out of this loop and start executing the first scheduled process */
#endif
  while (1);
}
