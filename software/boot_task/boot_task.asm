## MEMPHIS VERSION - 8.0 - support for RT applications
##
## Distribution:  June 2016
##
## Created by: Marcelo Ruaro - contact: marcelo.ruaro@acad.pucrs.br
##
## Research group: GAPH-PUCRS   -  contact:  fernando.moraes@pucrs.br
##
## Brief description: Initializes the stack pointer and jumps to main(). Handles the syscall.



        .text
        .align  2
        .globl  entry
        .ent    entry
entry:
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

        .end entry
  
###################################################

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


