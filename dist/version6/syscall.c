/**
 * This file is part of AdelaideOS (http://adelaideos.sourceforge.net)
 * Original version developed by Peter Kelly <kellypmk@gmail.com>
 * The contents of this file are in the public domain.
 */

#include "kernel.h"


extern process *current_process;
process processes[MAX_PROCESSES];

/*==============================================================================
valid_pointer

Checks to see if a pointer supplied to a system call is valid, i.e. resides
within the current process's user-accessible address space. This is a security
measure to stop processes from trying to subvert the restrictions of user mode
by tricking the kernel into reading from or writing to an area of memory that
the process would not normally have access to.

Any system call which has an invalid pointer supplied to it is supposed to
return -EFAULT.
==============================================================================*/
int valid_pointer(const void *ptr, unsigned int size)
{
  unsigned int start_address = (unsigned int)ptr;
  unsigned int end_address = start_address+size;

  if (0 == size)
    return 1;

  if (end_address < start_address)
    return 0;

  /* Within stack segment? */
  if ((start_address >= current_process->stack_start) &&
      (end_address <= current_process->stack_end))
    return 1;

  /* Within data segment? */
  if ((start_address >= current_process->data_start) &&
      (end_address <= current_process->data_end))
    return 1;

  /* Within kernel's data area? (e.g. a static string) */
  if ((start_address >= 1*MB) && (end_address <= 2*MB))
    return 1;

  /* Pointer is invalid */
  return 0;
}

/*==============================================================================
valid_string

Similar to valid_pointer. In the case of strings, we can simply check for a
particular length, since they are just arrays of characters terminated by '\0'.
So instead we have to scan through the string, checking that each part is valid
until we encounter the NULL terminator.
==============================================================================*/
int valid_string(const char *str)
{
  unsigned int len = 0;
  while (valid_pointer(str,len+1)) {
    if ('\0' == str[len])
      return 1;
    len++;
  }
  return 0;
}

/*==============================================================================
syscall_getpid

Returns the identifier of the calling process
==============================================================================*/
static pid_t syscall_getpid()
{
  return current_process->pid;
}

/*==============================================================================
syscall_exit

Terminates the calling process.
==============================================================================*/
static int syscall_exit(int status)
{
  disable_paging();
  kill_process(current_process);
  return -ESUSPEND;
}

/*==============================================================================
syscall_write

Writes the specified data to screen. This simply calls the write_to_screen,
function, which processes can no longer call directly since the paging
permissions set in start_process prevent them from directly writing to video
memory.
==============================================================================*/
static ssize_t syscall_write(int fd, const void *buf, size_t count)
{
  if (!valid_pointer(buf,count))
    return -EFAULT;
  write_to_screen(buf,count);
  return count;
}

/*==============================================================================
syscall_read

Read some data from a file descriptor. This is just a placeholder at this stage,
since file descriptors are not yet supported.
==============================================================================*/
static ssize_t syscall_read(int fd, void *buf, size_t count)
{
  if (!valid_pointer(buf,count))
    return -EFAULT;
  return -EBADF;
}

/*==============================================================================
syscall_geterrno

Retrieve the error code of the last system call. This is called whenever a
process access errno, which is defined in user.h as a call to this function.

Some systems take an alternative approach by making errno a global variable
within the process. We use a system call instead, since global variables require
each process to have a private text segment, which is not the case for most
versions of our kernel.
==============================================================================*/
static pid_t syscall_geterrno(void)
{
  return current_process->last_errno;
}

/*==============================================================================
syscall_brk

Called by a process when it wishes to extend the size of its data segment. The
memory allocation scheme used by processes in our kernel is based on a fixed
heap size, and thus each process calls brk only once, right at the beginning of
its execution. More advanced memory allocation schemes would allow the heap to
be extended if more memory is needed, and would call this function whenever they
need more pages mapped into their data segment for use by malloc.
==============================================================================*/
static int syscall_brk(void *end_data_segment)
{
  unsigned int oldend = current_process->data_end;
  unsigned int newend = (unsigned int)end_data_segment;

  /* Don't need to do anything if we already have enough memory */
  if (newend <= oldend)
    return 0;

  /* Round up to a multiple of PAGE_SIZE */
  if (0 != newend%PAGE_SIZE)
    newend = ((newend/PAGE_SIZE)+1)*PAGE_SIZE;

  disable_paging();
  map_new_pages(current_process->pdir,oldend,(newend-oldend)/PAGE_SIZE);
  enable_paging(current_process->pdir);
  current_process->data_end = newend;
  return 0;
}

int syscall_kill(pid_t pid)
{
  int r = 0;

  if ((0 > pid) || (MAX_PROCESSES <= pid) || !processes[pid].exists)
    return -ESRCH;

  if (pid == current_process->pid)
    r = -ESUSPEND; /* force context switch */

  kill_process(&processes[pid]);
  return r;
}

/*==============================================================================
syscall

Main dispatch routine for system calls. This is called from within
interrupt_handler whenever interrupt 48 (INTERRUPT_SYSCALL) is raised by a
process. It works out the location in memory of the arguments to the system
call, and then based on which call was requested, invokes the appropriate
function. Any new system calls that are added to the kernel should have an entry
in the switch statement which calls them.
==============================================================================*/
void syscall(regs *r)
{
  /* Get system call no. from the EAX register, which is set by the code for
     each system call within calls.s */
  unsigned int call_no = r->eax;

  /* Find out where the top of the process's stack is in memory. The arguments
     to this call are 4 bytes below this in the stack (right underneath the
     return address). In our kernel, all parameters to system calls are 32 bits
     wide, so we can just treat this address as the start of an array of
     integers. Where necessary these may be cast to pointers. */
  unsigned int useresp = r->useresp;
  int *args = (int*)(useresp+4);

  int res = -1;
  process *old_current = current_process;

  assert(current_process);
  current_process->in_syscall = call_no;

  /* Dispatch to the appropriate handler function */
  switch (call_no) {
  case SYSCALL_GETPID:
    res = syscall_getpid();
    break;
  case SYSCALL_EXIT:
    res = syscall_exit(args[0]);
    break;
  case SYSCALL_WRITE:
    res = syscall_write(args[0],(const void*)args[1],args[2]);
    break;
  case SYSCALL_READ:
    res = syscall_read(args[0],(char*)args[1],args[2]);
    break;
  case SYSCALL_GETERRNO:
    res = syscall_geterrno();
    break;
  case SYSCALL_BRK:
    res = syscall_brk((void*)args[0]);
    break;
  case SYSCALL_KILL:
    res = syscall_kill(args[0]);
    break;
  default:
    kprintf("Warning: Call to unimplemented system call %d\n",call_no);
    res = -ENOSYS;
    break;
  }

  /* Store the errno value, in case the process subsequently calls geterrno() */
  if ((SYSCALL_GETERRNO != call_no) && (-ESUSPEND != res) &&
      (NULL != current_process)) {
    if (res < 0) {
      current_process->last_errno = -res;
      res = -1;
    }
    else {
      current_process->last_errno = 0;
    }
  }

  /* -ESUSPEND means the process was suspended or killed. This means we can't
     just switch back to it directly, since the process is not in the ready
     queue. Instead, we perform a context switch, which will go to another
     running process, or an idle loop if there are no other processes ready. */
  if (-ESUSPEND == res) {
    context_switch(r);
  }
  else {
    /* System call has completed */
    old_current->in_syscall = 0;
    r->eax = res;

    /* We might have changed to a different process; if this is the case,
       perform a context switch */
    if (old_current != current_process)
      context_switch(r);
  }
}
