##
# @file hal.S
#
# @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
# GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
# PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
#
# @date May 2022
#
# @brief Handles the syscall entry point.
##

.section .text
.align 4

.globl system_call	# "registers that a global SystemCall function exists to C code"
system_call:
	# If a function gets called here, it can touch ra, so save it in stack
	addi	sp, sp, -4
	sw		ra, 0(sp)
	ecall			# Syscall address = set by kernel
	lw		ra, 0(sp)
	addi	sp, sp, 4
	ret 			# Returns from syscall. $ra has the return address of the callee.
