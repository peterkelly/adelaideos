/**
 * This file is part of AdelaideOS (http://adelaideos.sourceforge.net)
 * Original version developed by Peter Kelly <kellypmk@gmail.com>
 * The contents of this file are in the public domain.
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H


/* Memory */
#define NULL                 0
#define MB                   1048576
#define KB                   1024
#define BUFSIZE              1024
#define PAGE_SIZE            4096
#define KERNEL_CODE_SEGMENT  0x08
#define KERNEL_DATA_SEGMENT  0x10
#define USER_CODE_SEGMENT    0x18
#define USER_DATA_SEGMENT    0x20
#define TSS_SEGMENT          0x28
#define MAX_PROCESSES        32
#define PROCESS_STACK_BASE   0x40000000 /* 1Gb */
#define PROCESS_STACK_SIZE   (64*KB)
#define KERNEL_MEM_BASE      (2*MB)
#define KERNEL_MEM_SIZEPOW2  22
#define KERNEL_MEM_SIZE      (4*MB) /* 2^KERNEL_MEM_SIZEPOW2 */
#define PAGE_START           (6*MB)
#define STDIN_FILENO         0
#define STDOUT_FILENO        1
#define STDERR_FILENO        2

/* Hardware info */
#define SCREEN_WIDTH         80
#define SCREEN_HEIGHT        25
#define VIDEO_MEMORY         0xB8000
#define TICKS_PER_SECOND     50
#define RING_0               0
#define RING_1               1
#define RING_2               2
#define RING_3               3

/* Special keys */
#define KEY_SHIFT            42
#define KEY_CTRL             29
#define KEY_ALT              56
#define KEY_ENTER            28
#define KEY_BACKSPACE        14
#define KEY_TAB              15

#define KEY_UP               72
#define KEY_DOWN             80
#define KEY_LEFT             75
#define KEY_RIGHT            77
#define KEY_CAPS             58
#define KEY_PGUP             73
#define KEY_PGDOWN           81

#define KEY_F1               59
#define KEY_F2               60
#define KEY_F3               61
#define KEY_F4               62
#define KEY_F5               63
#define KEY_F6               64
#define KEY_F7               65
#define KEY_F8               66
#define KEY_F9               67
#define KEY_F10              68
#define KEY_F11              87
#define KEY_F12              88

/* Interrupts */
#define MAX_EXCEPTION        31
#define INTERRUPT_TIMER      32
#define INTERRUPT_KEYBOARD   33
#define INTERRUPT_SYSCALL    48

/* System calls */
#define SYSCALL_GETPID       1
#define SYSCALL_EXIT         2
#define SYSCALL_WRITE        3
#define SYSCALL_READ         4
#define SYSCALL_GETERRNO     5
#define SYSCALL_KILL         20

/* errno values */
#define EBADF                2  /* Bad file descriptor */
#define EINVAL               3  /* Invalid argument */
#define ESRCH                4  /* No such process */
#define EPERM                5  /* Operation not permitted */
#define ENOENT               6  /* No such file or directory */
#define EMFILE               7  /* Too many open files */
#define EISDIR               8  /* Is a directory */
#define ENOTDIR              9  /* Not a directory */
#define ENOSYS               10 /* Function not implemented */
#define ENOMEM               11 /* Not enough space */
#define EFAULT               12 /* Bad address */
#define EAGAIN               13 /* Resource unavailable, try again */
#define ECHILD               14 /* No child processes */
#define ERRNO_MAX            14
#define ESUSPEND             1000

#endif /* CONSTANTS_H */
