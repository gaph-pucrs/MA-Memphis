## MEMPHIS VERSION - 8.0 - support for RT applications
##
## Distribution:  June 2016
##
## Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
##
## Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
##
## Brief description: Initializes the stack pointer and jumps to main(). Handles the syscall.



        .section .init
        .align  2
        .globl  _start
        .ent    _start
_start:
   .set noreorder

   li $sp,sp_addr # new initialization

   jal   main
   nop
   
   move $4,$0   
   syscall 
   nop
   
$L1:
   j $L1
   nop

        .end _start
    
###################################################
.section .text

   .globl SystemCall
   .ent SystemCall
SystemCall:
   .set	noreorder
   
   syscall 
   nop
   jr	$31
   nop
   
   .set reorder
   .end SystemCall


