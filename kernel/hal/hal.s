##
# MA-Memphis
# @file hal.s
#
# @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
# GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
# PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
#
# @date September 2019
#
# @brief Hardware abstraction layer for OS bootloader, syscalls, interrupts and
# context switching
##

.section .init
.align 4

.globl _start
_start:
	# .option push
    # .option norelax
    # la gp, __global_pointer$
    # .option pop

	li		sp,sp_addr	# Stack to top

	# Configure system status MPP=0, MPIE=0, MIE=0
	csrw	mstatus, zero

	# Enable MEI when unmasked (if MIE=1)
	li		t0, 0x800
	csrw	mie, t0

	# Clear pending interrupts
	csrw	mip, zero

	# Disable S-Mode int/exc handling
	# Handle them in M-Mode
	csrw	mideleg, zero
	csrw	medeleg, zero

	# Configure Syscall
	la		t0,vector_entry		# Load the vector_entry address to t0
	csrw	mtvec,t0			# Write vector_entry address to mtvec
								# Last bit is 0, means DIRECT mode

	# Configure addressing
	csrw	0x7C0,zero			# Clear offset addressing

	jal		main
$L1:
	j		$L1


.section .text
.align 4

vector_entry:				# Set to mtvec.BASE and DIRECT
	j		save_ctx

interrupt_handler:
	# All interrupts are handled by this function

	#JUMP TO INTERRUPT SERVICE ROUTINE WITH ARG
	li		t0,0x20000000	# HW Address base
	lw		t1,0x20(t0)     # IRQ_STATUS
	lw		t2,0x10(t0)     # IRQ_MASK
	and		a0,t1,t2		# Function arg
	jal		os_isr
	# The C function calls hal_run_task

exception_handler:
	# ALL synchronous exceptions are heandled by this function, not only ecall.
	# WARNING: ALL exceptions will be handled as ecall for now.

	# # Save t0 to mscratch to let kernel use it
	# csrw	mscratch,t0

	# csrr	t0,mcause			# Load exception cause to t0
	# andi	t0,t0,0xFFFFFFF7	# Clear bit 3 (ecall)
	# beqz	t0,is_ecall			# If cause is ecall, jump to routine

	# csrr	t0,mscratch			# Else load back t0 and return to program
	# mret						# IMPORTANT: it does not handles exceptions
# is_call:
	# jumps to system calls handler
	jal		os_syscall
	
	# Save Syscall return to task TCB
	lw		t0,current		# Load "current" to t0
	sw		a0,40(t0)		# Save a0 (syscall return)

	# verifies if scheduling is needed
	lw		t0,schedule_after_syscall
	beqz	t0,system_service_restore	# If not, restore the program ctx

	# Else, schedules the next ready task
	jal		sched_run
system_service_restore:
	# Restores the context of the scheduled task and runs it
	lw		a0,current		# Load TCB pointer to function argument
	jal		hal_run_task

save_ctx:
	# mscratch CSR is reserved to OS
	# Save t0 to mscratch to let kernel use it
	csrw	mscratch,t0
	lw		t0,current		# Load "current" to t0

	# Save registers
	#sw		zero,0(t0)
	sw		ra,4(t0)
	sw		sp,8(t0)
	sw		gp,12(t0)
	sw		tp,16(t0)
	#sw		t0,20(t0)
	sw		t1,24(t0)
	sw		t2,28(t0)
	sw		s0,32(t0)
	sw		s1,36(t0)
	sw		a0,40(t0)
	sw		a1,44(t0)
	sw		a2,48(t0)
	sw		a3,52(t0)
	sw		a4,56(t0)
	sw		a5,60(t0)
	sw		a6,64(t0)
	sw		a7,68(t0)
	sw		s2,72(t0)
	sw		s3,76(t0)
	sw		s4,80(t0)
	sw		s5,84(t0)
	sw		s6,88(t0)
	sw		s7,92(t0)
	sw		s8,96(t0)
    sw		s9,100(t0)
	sw		s10,104(t0)
	sw		s11,108(t0)
	sw		t3,112(t0)
	sw		t4,116(t0)
	sw		t5,120(t0)
	sw		t6,124(t0)

	csrr	t1,mepc		# Load mepc
	sw		t1,128(t0)	# Save mepc

	csrr	t2,0x7C0	# MRAR
	sw		t2,132(t0)	# Save mrar

	csrr	t3,mscratch	# Load t0 to t3 now that t3 has been saved
	sw		t3,20(t0)	# Save t0 that is loaded into t3 to its reg address
	
	li		sp,sp_addr	# Restore stack pointer to top

	csrr	t4,mcause			# Load mcause to t4
	li 		t5,0x80000000		# Bit 31 (interrupt)
	bltu	t4, t5, exception_handler	# If 0 (exception) continue to ecall
	j 		interrupt_handler		# Else (interrupt) continue to isr

.globl hal_run_task
hal_run_task:
	# a0 has the TCB pointer

	lw		t0,128(a0)	# mepc
	csrw	mepc,t0		# Restore mepc

	lw		t1,132(a0)	# offset
	csrw	0x7C0,t1	# Restore mrar

	# Load registers
	#lw		zero,0(a0)
	lw		ra,4(a0)
	lw		sp,8(a0)
	lw		gp,12(a0)
	lw		tp,16(a0)
	lw		t0,20(a0)
	lw		t1,24(a0)
	lw		t2,28(a0)
	lw		s0,32(a0)
	lw		s1,36(a0)
	#lw		a0,40(a0)
	lw		a1,44(a0)
	lw		a2,48(a0)
	lw		a3,52(a0)
	lw		a4,56(a0)
	lw		a5,60(a0)
	lw		a6,64(a0)
	lw		a7,68(a0)
	lw		s2,72(a0)
	lw		s3,76(a0)
	lw		s4,80(a0)
	lw		s5,84(a0)
	lw		s6,88(a0)
	lw		s7,92(a0)
	lw		s8,96(a0)
    lw		s9,100(a0)
	lw		s10,104(a0)
	lw		s11,108(a0)
	lw		t3,112(a0)
	lw		t4,116(a0)
	lw		t5,120(a0)
	lw		t6,124(a0)

	# Finally loads a0
	lw		a0,40(a0)

	mret	# Return to task enabling interrupt for M-Mode

.globl hal_disable_interrupts
hal_disable_interrupts:
	li		t0, 0x8			# MIE
	csrc	mstatus,t0		# Clear MIE
	ret

.globl system_call
system_call:
   ret

.section .rodata		# Constants
.align 4
.globl _has_priv	# Set available for 'extern'
_has_priv: .4byte 1
