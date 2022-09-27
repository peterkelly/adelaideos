/**
 * This file is part of AdelaideOS (http://adelaideos.sourceforge.net)
 * Original version developed by Peter Kelly <kellypmk@gmail.com>
 * The contents of this file are in the public domain.
 */

#include "kernel.h"

#define IDLE_STACK_SIZE 4096

char idle_stack[IDLE_STACK_SIZE];

/* Statically allocated space for processes. The main reason for using a static
   array is so that we can easily look up a process given its pid. */
process processes[MAX_PROCESSES];

/* The next process id to be used */
unsigned int next_pid = 0;

/* The process that is currently being executed by the CPU */
process *current_process = NULL;

/* We store two (doubly-linked) lists of processes: The  ready list contains all
   the processes that have work that can be done immediately, and are thus
   eligible for selection by the scheduler. The suspended list is those
   processes which are waiting for something to happen before they can continue,
   such as user input becoming availbale. */
processlist ready = { first: NULL, last: NULL };
processlist suspended = { first: NULL, last: NULL };

#define ALLOCATION_START 2*MB

/* Position at which the next chunk of allocated memory will start */
unsigned int next_alloc = ALLOCATION_START;

/*==============================================================================
allocate_memory

A very simple memory allocator. This simply hands out chunks of memory starting
from the physical address given by ALLOCATION_START. It does not keep track of
what memory has been allocated, other than the end of the last chunk, and thus
we can't free memory. This is just enough to get us going; later on we'll look
at more sophisticated allocation schemes.
==============================================================================*/
static unsigned int allocate_memory(unsigned int nbytes)
{
  unsigned int address = next_alloc;
  next_alloc += nbytes;
  return address;
}

/*==============================================================================
init_regs

Each process has a state associated with it, in the form of CPU registers.
Whenever a context switch occurs and this process becomes the current one, the
registers are loaded from memory into the CPU. When a process is first created,
we must initialise these to the appropriate values, so that the processor will
start off in the right state when the process receives its first time slice.
==============================================================================*/
void init_regs(regs *r, unsigned int stack_max, void (*start_addr)(void))
{
  /* Zero all registers */
  unsigned int pos;
  for (pos = 0; pos < sizeof(regs); pos++)
    ((char*)r)[pos] = 0;

  /* Set the segment registers and EFLAGS register. The processes use the user
     code and data segments, which means that they run in user mode (ring 3).
     This prevents them from executing privileged operations or accessing parts
     of memory that are set aside exclusively for use by the kernel. */
  r->gs = USER_DATA_SEGMENT|RING_3;
  r->fs = USER_DATA_SEGMENT|RING_3;
  r->es = USER_DATA_SEGMENT|RING_3;
  r->ds = USER_DATA_SEGMENT|RING_3;
  r->ss = USER_DATA_SEGMENT|RING_3;
  r->cs = USER_CODE_SEGMENT|RING_3;
  r->eflags = 0x202;

  /* Set the EIP (instruction pointer) register to the starting address of the
     process, and the ESP (stack pointer) register to the bottom of the stack.
     When the process starts executing, it will begin from the specified start
     address. */
  r->eip = (unsigned int)start_addr;
  r->useresp = stack_max;
}

/*==============================================================================
get_free_pid

Finds the lowest unused process identifier. This is used when creating a new
process, when we need to obtain a slot in the processes array to use to store
the data associated with the new process. This function only checks for pids
greater than 0, since the fork system call (implemented in later versions of
the kernel) treats a return value of 0 specially.

If there are no free slots, this function returns -1.
==============================================================================*/
pid_t get_free_pid(void)
{
  pid_t pid = 1;
  while ((pid < MAX_PROCESSES) && processes[pid].exists)
    pid++;
  if (pid >= MAX_PROCESSES)
    return -1;
  else
    return pid;
}

/*==============================================================================
start_process

Create a new process and add it to the list of ready processes. The first
parameter specifies the starting address, i.e. the point at which the CPU should
begin executing code when the process is first scheduled. In the simple case
where we do not have a way to load executable files from disk, this can just be
a pointer to a function defined in the kernel.

The process will not actually start immediately. This will only happen after it
gets scheduled as a result of a context switch. Since these happen 50 times per
second, the process will get its first time slice very shortly.
==============================================================================*/
pid_t start_process(void (*start_address)(void))
{
  /* Find a free process identifier */
  pid_t pid = get_free_pid();
  if (0 > pid)
    return -1;

  /* Initialise the process structure */
  process *proc = &processes[pid];
  memset(proc,0,sizeof(process));
  proc->pid = pid;
  proc->exists = 1;

  unsigned int memsize = 1*MB;
  proc->stack_start = allocate_memory(memsize);
  proc->stack_end = proc->stack_start+memsize;

  /* Initialise registers */
  init_regs(&proc->saved_regs,proc->stack_end,start_address);

  /* Add this process to the list of ready processes */
  proc->ready = 1;
  list_add(&ready,proc);
  return pid;
}

/*==============================================================================
kill_process

Stop a running process and removes it from memory. We can't actually free the
memory here, since we don't have a free function yet, but this is where you
would do it.
==============================================================================*/
void kill_process(process *proc)
{
  int current = (current_process == proc);
  if (current_process == proc) {
    current_process = NULL;
  }
  if (proc->ready) {
    list_remove(&ready,proc)
  }
  else {
    list_remove(&suspended,proc);
  }
  proc->exists = 0;
}

/*==============================================================================
suspend_process

Mark a processes as suspended, so that it will not be considered by the
scheduler. Processes are suspended whenever they need to wait for some external
event to occur, e.g. the user entering some input. Suspension can also be used
to have processes sleep for a specified period of time, although this requires
additional logic in the scheduler which we have not implemented.

This function must only be called with a process that is not already suspended.
==============================================================================*/
void suspend_process(process *proc)
{
  assert(proc->exists);
  assert(proc->ready);
  proc->ready = 0;
  list_remove(&ready,proc);
  list_add(&suspended,proc);
}

/*==============================================================================
resume_process

Mark a process as ready, so that it will be eligible for execution, and given
time slices by the scheduler. This is usually called when an event that a
process has been waiting on occurs, e.g. some user input becomes available.

This function must only be called with a process that is currently suspended.
==============================================================================*/
void resume_process(process *proc)
{
  assert(proc->exists);
  assert(!proc->ready);
  proc->ready = 1;
  list_remove(&suspended,proc);
  list_add(&ready,proc);
}

/*==============================================================================
context_switch

Switch to another process. This is called by the timer interrupt handler, which
is fired 50 times per second. The current process is suspended, another one is
chosen from the ready list, and then this is activated.

There are a few special situations we need to handle here. We need to check upon
entry if there is actually a process running - if not, then we don't need to
save its state. This can be the case if the kernel has just started, or if all
processes are suspended. Also, if there are no processes to switch to, we must
cause the processor to go into an idle loop until one or more processes become
ready.

Upon return, this function ensures that the values in the register state pointed
to by r are that of the process being switch to. There is some inefficiency
here, because the registers must be copied twice (once in the interrupt handler,
and once here). However this is slightly simpler than maintaining a separate
kernel stack for each process.
==============================================================================*/
void context_switch(regs *r)
{
  /* Save the stack pointer for the currently running process */
  if (current_process)
    memmove(&current_process->saved_regs,r,sizeof(regs));

  /* If the current process is no longer in the ready list (i.e. it has just
     been suspended), then we can't use the next pointer, since this will point
     to the next process in the suspended list instead of the ready list.
     Instead, act as if there is no current process. */
  if (!current_process->ready)
    current_process = NULL;

  /* Round-robin scheduling: Move to the next process in the ready list, or to
     the first item in the list if the current process was the last (or null) */
  if (current_process && current_process->next)
    current_process = current_process->next;
  else
    current_process = ready.first;

  if (current_process) {
    /* We have a new current process. Copy in the register state of this
       process, so that call_interrupt_handler (defined in start.s) will use
       these values to restore the processor state. */
    memmove(r,&current_process->saved_regs,sizeof(regs));
  }
  else {
    /* There are no processes ready to run. Set up a stack for the idle routine
       to use, and cause the interrupt handler to jump to the idle loop defined
       in start.s */
    char *end = idle_stack+IDLE_STACK_SIZE;
    init_regs(r,(unsigned int)end,idle);
  }
}
