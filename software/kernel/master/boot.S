##------------------------------------------------------------------------------------------------
##
##  DISTRIBUTED MEMPHIS -  8.0
##
##  Research group: GAPH-PUCRS    -    contact   fernando.moraes@pucrs.br
##
##  Distribution:  September 2013
##
##  Source name:  boot.S
##
##  Brief description: Initializes the stack pointer and jumps to main().
##
##------------------------------------------------------------------------------------------------

        .section .init    
        .set noat
        .align  2
        .globl  _start
        .ent    _start
_start:
        .set noreorder
		
		li $sp,sp_addr
        jal     main
        nop
$L1:
        j $L1
        nop
        
        .end _start
        
