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

        .text    
        .set noat
        .align  2
        .globl  entry
        .ent    entry
entry:
        .set noreorder
		
		li $sp,sp_addr
        jal     main
        nop
$L1:
        j $L1
        nop
        
        .end entry
        
