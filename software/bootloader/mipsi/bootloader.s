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
   jal   main
   nop
   
   move  $a0, $0 
   syscall        # Calls exit()
   nop
.end _start
    
.section .text
.set noreorder
.align   2

.globl SystemCall
.ent SystemCall
SystemCall:
   syscall
   nop
   jr	$31
   nop
.end SystemCall
