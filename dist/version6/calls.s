/**
 * This file is part of AdelaideOS (http://adelaideos.sourceforge.net)
 * Original version developed by Peter Kelly <kellypmk@gmail.com>
 * The contents of this file are in the public domain.
 */

#include "constants.h"
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
syscall brk         SYSCALL_BRK
syscall send        SYSCALL_SEND
syscall receive     SYSCALL_RECEIVE
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
