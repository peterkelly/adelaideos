/**
 * This file is part of AdelaideOS (http://adelaideos.sourceforge.net)
 * Original version developed by Peter Kelly <kellypmk@gmail.com>
 * The contents of this file are in the public domain.
 */

#include "constants.h"
#ifdef USE_UNIXPROC
.section .text

.globl start
start:
#ifdef USE_MALLOC
  call init_userspace_malloc
#endif
  call main
#ifdef USE_SYSCALLS
  pushl %eax
  pushl $0
  mov $SYSCALL_EXIT,%eax
  int $INTERRUPT_SYSCALL
#endif
idle:
  jmp idle

.globl __assert
__assert:
  jmp user_mode_assert

.globl write_to_screen
write_to_screen:
  ret
#endif
