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

.equ MMR_CLOCK_HOLD, 0x20000090
.equ INTR_MASK, 0x80000000
.equ MMR_ADDR, 0x20000000

.equ MAX_REGISTERS, 30
.equ PC_ADDR, MAX_REGISTERS*4
.equ OFF_ADDR, (MAX_REGISTERS+1)*4

.equ mrar, 0x7C0

.section .init
.align 4

.globl _start
_start:
	.option push
    .option norelax
    la gp, __global_pointer$
    .option pop

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
	csrw	mrar, zero			# Clear offset addressing

	jal		main

	la		t0, idle
	csrw	mepc, t0			# Load idle task to mepc

	la		a0, MMR_CLOCK_HOLD	# Load the CLOCK_HOLD address to a0
	mret						# Let the control to mepc

idle:
	sw		zero, 0(a0)
	j		idle


.section .text
.align 4

vector_entry:					# Set to mtvec.BASE and DIRECT	
	# Swap task sp with kernel sp
	csrrw	sp, mscratch, sp

	# Save gp and t0 to stack
	addi	sp, sp, -8
	sw		t0, 0(sp)
	sw		gp, 4(sp)

	# Load gp from kernel stack. Needed before loading current which is a global variable
	lw		gp, 8(sp)

	# Load "current" to s0
	lw		t0, current

	# If scheduled task was 'idle', no need to save minimum context
	beqz	t0, entry_check

	# Else, if running task, save some registers to let the kernel use
	addi	sp, sp, -8
	sw		t2, 0(sp)
	sw		t1, 4(sp)
	
	# Check if it was ecall
entry_check:
	csrr	t1, mcause					# Load mcause
	li 		t2, INTR_MASK				# Bit 31 (interrupt)
	bltu	t1, t2, exception_handler	# If 0 (exception) go to ecall

	# Else: continue to isr
interrupt_handler:

	# If scheduled task was 'idle', no need to save remaining context
	beqz	t0, isr_entry

	# Else, an interruption breaks the task control
	# So it is needed to save the caller-registers, because the callee-registers 
	# are saved by the ABI
	# To ease the migration process, save ALL, and not just callee registers
	sw		 ra,  0(t0)
	sw		 s0, 24(t0)
	sw		 s1, 28(t0)
	sw		 a0, 32(t0)
	sw		 a1, 36(t0)
	sw		 a2, 40(t0)
	sw		 a3, 44(t0)
	sw		 a4, 48(t0)
	sw		 a5, 52(t0)
	sw		 a6, 56(t0)
	sw		 a7, 60(t0)
	sw		 s2, 64(t0)
	sw		 s3, 68(t0)
	sw		 s4, 72(t0)
	sw		 s5, 76(t0)
	sw		 s6, 80(t0)
	sw		 s7, 84(t0)
	sw		 s8, 88(t0)
    sw		 s9, 92(t0)
	sw		s10, 96(t0)
	sw		s11,100(t0)
	sw		 t3,104(t0)
	sw		 t4,108(t0)
	sw		 t5,112(t0)
	sw		 t6,116(t0)

	# t2, t1, t0, and gp are in stack. Load them and save to current.
	lw		 t2,  0(sp)	# Task t2
	sw		 t2, 20(t0)
	lw		 t2,  4(sp)	# Task t1
	sw		 t2, 16(t0)
	lw		 t2,  8(sp)	# Task t0
	sw		 t2, 12(t0)
	lw		 t2, 12(sp) # Task gp
	sw		 t2,  8(t0)
	addi	 sp, sp, 20	# Pop stack. 1 extra word for popped kernel gp.

	csrr	 t2, mscratch	# Load the sp that is in the mscratch
	sw		 t2,  4(t0)		# Save sp to current

	csrr	 t2, mepc			# Load the PC
	sw		 t2, PC_ADDR(t0)	# Save the PC to current

	mv		 s0, t0		# Save current to use later

isr_entry:
	# JUMP TO INTERRUPT SERVICE ROUTINE WITH ARGS
	li		t0, MMR_ADDR	 # HW Address base
	lw		t1, 0x20(t0)     # IRQ_STATUS
	lw		t2, 0x10(t0)     # IRQ_MASK
	and		a0, t1, t2		 # Function arg
	jal		os_isr
	# The function returned the scheduled task pointer in a0

	# Save kernel context
	addi	sp, sp, -4
	sw		gp, 0(sp)		# Save gp to stack -- it will not be touched
	csrw	mscratch, sp	# Save sp to mscratch -- it will not be used anymore

	# If the idle task was scheduled, no need to restore the context
	beqz	a0, idle_entry

	# Else if the same task that was interrupted is scheduled, restore only needed
	beq		a0, s0, restore_minimum

	# Else if a new task was scheduled, restore a bit more of the context
	lw		 s1, 28(a0)
	lw		 s2, 64(a0)
	lw		 s3, 68(a0)
	lw		 s4, 72(a0)
	lw		 s5, 76(a0)
	lw		 s6, 80(a0)
	lw		 s7, 84(a0)
	lw		 s8, 88(a0)
    lw		 s9, 92(a0)
	lw		s10, 96(a0)
	lw		s11,100(a0)

	# Load epc from scheduled task
	lw		t0, PC_ADDR(a0)
	csrw	mepc, t0

	# Load offset from scheduled task
	lw		t0, OFF_ADDR(a0)
	csrw	mrar, t0

	# Continue to restore the remaining context
restore_minimum:
	lw		 ra,  0(a0)
	lw		 sp,  4(a0)
	lw		 gp,  8(a0)
	lw		 t0, 12(a0)
	lw		 t1, 16(a0)
	lw		 t2, 20(a0)
	lw		 s0, 24(a0)	# Only restore s0 because we used to store current.
	lw		 a1, 36(a0)
	lw		 a2, 40(a0)
	lw		 a3, 44(a0)
	lw		 a4, 48(a0)
	lw		 a5, 52(a0)
	lw		 a6, 56(a0)
	lw		 a7, 60(a0)
	lw		 t3,104(a0)
	lw		 t4,108(a0)
	lw		 t5,112(a0)
	lw		 t6,116(a0)
	# No need to restore remaining s registers. ABI takes care of this.

	lw		 a0, 32(a0)

	mret

exception_handler:
	# Save s0 in stack
	addi	sp, sp, -4
	sw		s0, 0(sp)

	mv		s0, t0		# Save current to use later

	# @todo Check t1 (mcause) for exceptions and at least print them

	jal		os_syscall

	lw		t0, current	# Load current tcb to t0

	# Update kernel gp in stack. Needed?
	sw		 gp, 20(sp)
	
	beq		t0, s0, ecall_return	# If scheduled the same TCB, simply return

	# Otherwise it is needed to save the previous task (s0) context
	# As syscall is CALLED, only save the callee-registers

	mv		 t1, s0			# Keep previous in t1 (no need to save t1)
	
	csrr	 t2, mscratch	# Task sp is in mscratch
	sw		 t2,  4(t1)
	
	lw		 t2, 16(t1)		# Task gp is in stack
	sw		 t2,  8(t1)

	lw		 t2,  0(sp)		# Task s0 is in stack
	sw		 t2, 24(t1)

	sw		 s1, 28(t1)
	sw		 s2, 64(t1)
	sw		 s3, 68(t1)
	sw		 s4, 72(t1)
	sw		 s5, 76(t1)
	sw		 s6, 80(t1)
	sw		 s7, 84(t1)
	sw		 s8, 88(t1)
    sw		 s9, 92(t1)
	sw		s10, 96(t1)
	sw		s11,100(t1)

	csrr	 t2, mepc		# Task pc is in mepc
	sw		 t2, PC_ADDR(t1)

	addi	 sp, sp, 20		# Pop stack values from saved context

	csrw	mscratch, sp	# Save kernel sp

	# If the idle task was scheduled, no need to restore the context
	beqz	t0, idle_entry

	# Otherwise, restore context of scheduled task
	# We don't know if the scheduled task stopped on a call or intr, so restore everything
	lw		 t1, PC_ADDR(t0)
	csrw	 mepc, t1			# Load task PC

	lw		 t1, OFF_ADDR(t0)
	csrw	 mrar, t1			# Load task offset

	lw		 ra,  0(t0)
	lw		 sp,  4(t0)
	lw		 gp,  8(t0)

	lw		 t1, 16(t0)
	lw		 t2, 20(t0)
	lw		 s0, 24(t0)
	lw		 s1, 28(t0)
	lw		 a0, 32(t0)
	lw		 a1, 36(t0)
	lw		 a2, 40(t0)
	lw		 a3, 44(t0)
	lw		 a4, 48(t0)
	lw		 a5, 52(t0)
	lw		 a6, 56(t0)
	lw		 a7, 60(t0)
	lw		 s2, 64(t0)
	lw		 s3, 68(t0)
	lw		 s4, 72(t0)
	lw		 s5, 76(t0)
	lw		 s6, 80(t0)
	lw		 s7, 84(t0)
	lw		 s8, 88(t0)
    lw		 s9, 92(t0)
	lw		s10, 96(t0)
	lw		s11,100(t0)
	lw		 t3,104(t0)
	lw		 t4,108(t0)
	lw		 t5,112(t0)
	lw		 t6,116(t0)

	lw		 t0, 12(t0)
	
	mret

ecall_return:
	# Restore minimum context from stack
	lw		 s0,  0(sp)
	lw		 gp, 16(sp)
	addi	 sp, sp, 20	# Pop stack and keep the kernel gp allocated

	csrrw	 sp, mscratch, sp	# Swap back kernel sp with task sp

	mret	

idle_entry:
	# Set the return pointer to the idle 'task'
	la		t0, idle
	csrw	mepc, t0

	# Ensure mrar has the kernel offset
	csrw	mrar, zero

	la		a0, MMR_CLOCK_HOLD

	mret

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
