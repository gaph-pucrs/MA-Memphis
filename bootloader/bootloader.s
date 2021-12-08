##
# @file bootloader.s
#
# @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
# GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
# PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
#
# @date September 2019
#
# @brief Initializes the stack pointer and jumps to main(). Handles the syscall.
##

.section .init	# Start of ".init" section (means bootloader)

.globl _start  	# Exports entry symbol
_start:
	.option push
    .option norelax
    la gp, __global_pointer$
    .option pop
	
	li sp, sp_addr	# Stack pointer address passed on makefile. Loads the page size to the sp.

	# JAL: Jump and Link
   	# Copies the address of the next instruction into the register ra
	jal main		# Execute main
   
	# Execute exit() until success
	mv s1, a0			# Save return value to s1
try_exit:
	mv a0, zero			# First argument is "0" exit
	mv a1, s1			# Second argument is the return value of main()
	ecall				# Calls the syscall(exit, return value)
	beqz a0, try_exit	# If exit() returned non-zero, retry

.section .text

.globl SystemCall	# "registers that a global SystemCall function exists to C code"
SystemCall:
	ecall			# Syscall address = set by kernel
	ret 			# Returns from syscall. $ra has the return address of the callee.

.section .rodata	# switch to read-only data section

# Temporary workaround for privilege check
# This is just a check, if this value is modify the security is not compromised,
# but the functionality may fail
.globl _has_priv	# Set the variable as global
_has_priv:			# Declare the variable
   .long  0			# 4 bytes of initialized storage after the label
