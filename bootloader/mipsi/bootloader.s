##
# 
# @file boot.s
#
# @author Marcelo Ruaro (marcelo.ruaro@acad.pucrs.br)
# GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
# PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
#
# @brief Initializes the stack pointer and jumps to main(). Handles the syscall.
##

.section .init
.set noreorder
.align 2

.globl _start
.ent _start
_start:
   li    $sp, sp_addr   # Initializes stack pointer
   # SEND TASK ALLOCATED
   jal   main
   nop
   
   move 	$s0, $v0 # Save the program return status
try_exit:
   move  $a1, $s0 # Move to a1 the return status
   move  $a0, $0  # Move exit syscall request to a0 
   syscall        # Calls exit()
   nop
   beq	$v0, $0, try_exit # If not exited, retry
   nop
.end _start

.section .text
.set noreorder
.align   2

.globl system_call
.ent system_call
system_call:
   syscall
   nop
   jr	$ra
   nop
.end system_call

.globl  _has_priv        # _num is a global symbol, when it is defined
.data               # switch to read-write data section
.align 4
_has_priv:                   # declare the label 
    .long  0           # 4 bytes of initialized storage after the label
