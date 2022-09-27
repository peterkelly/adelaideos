/**
 * This file is part of AdelaideOS (http://adelaideos.sourceforge.net)
 * Original version developed by Peter Kelly <kellypmk@gmail.com>
 * The contents of this file are in the public domain.
 */

#include "constants.h"

.section .text
.globl start
.globl interrupt_handlers
.globl idle
.globl set_gdt
.globl set_tss
.globl idt_load
.globl fpustate
.globl enable_interrupts

.globl ih_stack

start:
  mov $sys_stack,%esp
  push %ebx # multiboot header
  call kernel_main

.align 4
mboot:
  .set MULTIBOOT_PAGE_ALIGN, 1<<0
  .set MULTIBOOT_MEMORY_INFO, 1<<1
  .set MULTIBOOT_AOUT_KLUDGE, 1<<16
  .set MULTIBOOT_HEADER_MAGIC, 0x1BADB002
  .set MULTIBOOT_HEADER_FLAGS, MULTIBOOT_PAGE_ALIGN|MULTIBOOT_MEMORY_INFO|MULTIBOOT_AOUT_KLUDGE
  .set MULTIBOOT_CHECKSUM, -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

  .int MULTIBOOT_HEADER_MAGIC
  .int MULTIBOOT_HEADER_FLAGS
  .int MULTIBOOT_CHECKSUM

  .int mboot
  .int code
  .int bss
  .int end
  .int start

idle:
  jmp idle

set_gdt:
  mov 4(%esp),%eax
  lgdt (%eax)
  mov $0x10,%ax
  mov %eax,%ds
  mov %eax,%es
  mov %eax,%fs
  mov %eax,%gs
  mov %eax,%ss
  ljmp $0x8, $(set_gdt_end)
set_gdt_end:
  ret

set_tss:
  mov 4(%esp),%eax
  ltr %ax
  ret

idt_load:
  lidt (idtp)
  ret

interrupt_handlers:
  .int isr0,  isr1,  isr2,  isr3,  isr4,  isr5,  isr6,  isr7
  .int isr8,  isr9,  isr10, isr11, isr12, isr13, isr14, isr15
  .int isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
  .int isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
  .int isr32, isr33, isr34, isr35, isr36, isr37, isr38, isr39
  .int isr40, isr41, isr42, isr43, isr44, isr45, isr46, isr47
  .int isr48

.macro isrs_noparam num
.globl isr\num
isr\num:
  cli
  pushl $0x0
  pushl $\num
  jmp call_interrupt_handler
.endm

.macro isrs_param num
.globl isr\num
isr\num:
  cli
  pushl $\num
  jmp call_interrupt_handler
.endm

# Exceptions
isrs_noparam 0         # Divide By Zero
isrs_noparam 1         # Debug
isrs_noparam 2         # Non Maskable Interrupt
isrs_noparam 3         # Int 3
isrs_noparam 4         # INTO
isrs_noparam 5         # Out of Bounds
isrs_noparam 6         # Invalid Opcode
isrs_noparam 7         # Coprocessor Not Available
isrs_param   8         # Double Fault
isrs_noparam 9         # Coprocessor Segment Overrun
isrs_param   10        # Bad TSS
isrs_param   11        # Segment Not Present
isrs_param   12        # Stack Fault
isrs_param   13        # General Protection Fault
isrs_param   14        # Page Fault
isrs_noparam 15        # Reserved
isrs_noparam 16        # Floating Point
isrs_noparam 17        # Alignment Check
isrs_noparam 18        # Machine Check
isrs_noparam 19        # Reserved
isrs_noparam 20        # Reserved
isrs_noparam 21        # Reserved
isrs_noparam 22        # Reserved
isrs_noparam 23        # Reserved
isrs_noparam 24        # Reserved
isrs_noparam 25        # Reserved
isrs_noparam 26        # Reserved
isrs_noparam 27        # Reserved
isrs_noparam 28        # Reserved
isrs_noparam 29        # Reserved
isrs_noparam 30        # Reserved
isrs_noparam 31        # Reserved

# IRQs
isrs_noparam 32        # IRQ 0
isrs_noparam 33        # IRQ 1
isrs_noparam 34        # IRQ 2
isrs_noparam 35        # IRQ 3
isrs_noparam 36        # IRQ 4
isrs_noparam 37        # IRQ 5
isrs_noparam 38        # IRQ 6
isrs_noparam 39        # IRQ 7
isrs_noparam 40        # IRQ 8
isrs_noparam 41        # IRQ 9
isrs_noparam 42        # IRQ 10
isrs_noparam 43        # IRQ 11
isrs_noparam 44        # IRQ 12
isrs_noparam 45        # IRQ 13
isrs_noparam 46        # IRQ 14
isrs_noparam 47        # IRQ 15

# Other interrupts
isrs_noparam 48        # System call

call_interrupt_handler:
  # Save the state of all CPU registers
  pusha
  push %ds
  push %es
  push %fs
  push %gs
  fsave fpustate
  subl $108,%esp

  # Change the segment registers to those used for kernel mode
  mov $0x10,%ax
  mov %ax,%ds
  mov %ax,%es
  mov %ax,%fs
  mov %ax,%gs
  mov %esp,%eax

  # Call interrupt_handler()
  push %eax
  mov $interrupt_handler,%eax
  call *%eax
  pop %eax

  # Restore register state
  frstor fpustate
  addl $108,%esp
  pop %gs
  pop %fs
  pop %es
  pop %ds
  popa
  add $8,%esp
  iret
enable_interrupts:
  sti
  ret

.globl inb
inb:
  push %edx
  movl 8(%esp),%edx
  movl $0,%eax
  inb %dx,%al
  pop %edx
  ret

.globl outb
outb:
  push %edx
  movl 12(%esp),%eax
  movl 8(%esp),%edx
  outb %al,%dx
  pop %edx
  ret

.section .bss
  .lcomm fpustate,108
  .lcomm sys_stack_top,65536
  .lcomm sys_stack,0
  .lcomm ih_stack_top,65536
  .lcomm ih_stack,0
