/**
 * This file is part of AdelaideOS (http://adelaideos.sourceforge.net)
 * Original version developed by Peter Kelly <kellypmk@gmail.com>
 * The contents of this file are in the public domain.
 */

#include "constants.h"
#ifdef USE_SYSCALLS
.section .text

.macro syscall name num
.globl \name
\name:
  mov $\num,%eax
  int $INTERRUPT_SYSCALL
  ret
.endm

syscall getpid      SYSCALL_GETPID
syscall exit        SYSCALL_EXIT
syscall write       SYSCALL_WRITE
syscall read        SYSCALL_READ
syscall geterrno    SYSCALL_GETERRNO
#ifdef USE_MALLOC
syscall brk         SYSCALL_BRK
syscall send        SYSCALL_SEND
syscall receive     SYSCALL_RECEIVE
#endif
#ifdef USE_FILEDESC
syscall close       SYSCALL_CLOSE
syscall pipe        SYSCALL_PIPE
syscall dup2        SYSCALL_DUP2
#endif
#ifdef USE_UNIXPROC
syscall stat        SYSCALL_STAT
syscall open        SYSCALL_OPEN
syscall getdent     SYSCALL_GETDENT
syscall chdir       SYSCALL_CHDIR
syscall getcwd      SYSCALL_GETCWD
#endif
#ifdef USE_UNIXPROC
syscall fork        SYSCALL_FORK
syscall execve      SYSCALL_EXECVE
syscall waitpid     SYSCALL_WAITPID
#endif
syscall kill        SYSCALL_KILL

.globl in_user_mode
in_user_mode:
  movw %ds,%ax
  cmpw $0x23,%ax
  jz in_user_mode_true
  mov $0,%eax
  ret
in_user_mode_true:
  mov $1,%eax
  ret
#endif
