##
# 
# @file hal.s
#
# @author Fernando Gehm Moraes (fernando.moraes@pucrs.br)
# GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
# PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
#
# @brief Hardware abstraction layer for OS bootloader, syscalls, interrupts and
# context switching
##

.section .init
.align 2
.set noat
.set noreorder

.globl _start
.ent _start
_start:
	li		$sp, sp_addr
	jal		main
	nop

$L1:
	j		$L1
	nop
.end _start

.org 0x3c
hal_isr_entry:
	j		hal_isr
	nop

# Address 0x44
hal_syscall_entry:
	j		hal_syscall
	nop

.section .text
.align 2
.set noat
.set noreorder
hal_isr:
	# $k0 and k1 are kernel registers (reserved for OS)
	
	#sw		$0, needTaskScheduling

	# Load pointer to the current scheduled task. First variables are registers
	lw		$k1, current
	#nop

	# Save registers
	sw		$v0, 0($k1)
	sw		$v1, 4($k1)
	sw		$a0, 8($k1)
	sw		$a1, 12($k1)
	sw		$a2, 16($k1)
	sw		$a3, 20($k1)
	sw		$t0, 24($k1)
	sw		$t1, 28($k1)
	sw		$t2, 32($k1)
	sw		$t3, 36($k1)
	sw		$t4, 40($k1)
	sw		$t5, 44($k1)
	sw		$t6, 48($k1)
	sw		$t7, 52($k1)
	sw		$s0, 56($k1)
	sw		$s1, 60($k1)
	sw		$s2, 64($k1)
	sw		$s3, 68($k1)
	sw		$s4, 72($k1)
	sw		$s5, 76($k1)
	sw		$s6, 80($k1)
	sw		$s7, 84($k1)
	sw		$t8, 88($k1)
	sw		$t9, 92($k1)
	#sw		$gp, 96($k1)	# We are not using Global Pointer
	sw		$sp, 100($k1)
	sw		$fp, 104($k1)
	sw		$ra, 108($k1)

	mfhi	$k0				# Load hi to k0
	sw		$k0, 112($k1)	# Save hi
	mflo	$k0				# Load lo to k0
	sw		$k0, 116($k1)	# Save lo
	mfc0	$k0, $14		# Load epc ($14) to k0
	sw		$k0, 120($k1)	# Save pc

	lui		$a1, 0x2000		# Loads value 0x20000000 (memory mapped registers address) to a1
	li		$sp, sp_addr	# Loads the kernel stack pointer base
	lw		$a0, 0x20($a1)	# Load IRQ_STATUS to a0
	lw		$a2, 0x10($a1)	# Load IRQ_MASK to a2
	#nop
	
	and		$a0, $a0, $a2	# a0 = a0 & a2
	jal		os_isr
	nop

hal_syscall:
	# $k0 and k1 are kernel registers (reserved for OS)
	lw		$k1, current
	#nop

	# Save minimum context
	sw		$a0, 8($k1)
	sw		$a1, 12($k1)
	sw		$a2, 16($k1)
	sw		$a3, 20($k1)

	#sw		$gp, 96($k1)
	sw		$sp, 100($k1)
	sw		$ra, 108($k1)

	mfc0	$k0, $14		# Load epc to k0
	sw		$k0, 120($k1)	# Save epc

	li		$sp, sp_addr	# Loads the kernel stack pointer base
	jal		os_syscall
	nop

	# Verifies if should schedule
	lb		$k0, schedule_after_syscall
	#nop
	beqz	$k0, hal_restore_minimum	# If should not schedule, restore the minimum context saved
	nop

	#lw		$k1, current	# I guess the k1 cannot be modified by C
	#nop
	sw		$v0, 0($k1)        #  $v0  
	sw		$v1, 4($k1)        #  $v1
	# No need to save temporary registers. The syscall has been CALLED.
	#sw		$t0, 24($k1)
	#sw		$t1, 28($k1)
	#sw		$t2, 32($k1)
	#sw		$t3, 36($k1)
	#sw		$t4, 40($k1)
	#sw		$t5, 44($k1)
	#sw		$t6, 48($k1)
	#sw		$t7, 52($k1)
	sw		$s0, 56($k1)
	sw		$s1, 60($k1)
	sw		$s2, 64($k1)
	sw		$s3, 68($k1)
	sw		$s4, 72($k1)
	sw		$s5, 76($k1)
	sw		$s6, 80($k1)
	sw		$s7, 84($k1)
	# More temporary registers.
	#sw		$t8, 88($k1)
	#sw		$t9, 92($k1)
	sw		$fp, 104($k1)
	# Hi and lo. No need to save. Syscall has benn CALLED.
	#mfhi	$k0
	#sw		$k0, 112($k1)
	#mflo	$k0
	#sw		$k0, 116($k1)

	# Run the task scheduler
	jal		sched_run
	nop

	# Runs the scheduled task
	lw		$a0, current
	#nop
	jal hal_run_task
	nop

hal_restore_minimum:
	# Current is in k1... I guess
	#lw		$k1, current
	#nop

	#lw		$gp, 96($k1)
	lw		$sp, 100($k1)
	lw		$ra, 108($k1)

	lw		$k0, 120($k1)	# Load PC to k0
    lw		$k1, 124($k1)	# Load offset for paging setup

	li		$at, 0x1
	mtc0	$k1, $10
	jr		$k0				# Jumps to pc
	mtc0	$at, $12		# Enable interrupt

.globl hal_disable_interrupts
.ent hal_disable_interrupts
hal_disable_interrupts:
	jr      $ra			# Return to caller
	mtc0    $zero, $12	# Store coprocessor 0, register 12 the value of 0
.end hal_disable_interrupts

.globl hal_run_task
.ent hal_run_task
hal_run_task:
	move    $k1, $a0
	lw      $v0, 0($k1)
	lw      $v1, 4($k1)
	lw      $a0, 8($k1)
	lw      $a1, 12($k1)
	lw      $a2, 16($k1)
	lw      $a3, 20($k1)
	lw      $t0, 24($k1)
	lw      $t1, 28($k1)
	lw      $t2, 32($k1)
	lw      $t3, 36($k1)
	lw      $t4, 40($k1)
	lw      $t5, 44($k1)
	lw      $t6, 48($k1)
	lw      $t7, 52($k1)
	lw      $s0, 56($k1)
	lw      $s1, 60($k1)
	lw      $s2, 64($k1)
	lw      $s3, 68($k1)
	lw      $s4, 72($k1)
	lw      $s5, 76($k1)
	lw      $s6, 80($k1)
	lw      $s7, 84($k1)
	lw      $t8, 88($k1)
	lw      $t9, 92($k1)
	#lw		$gp, 96($k1)
	lw      $sp, 100($k1)
	lw      $fp, 104($k1)
	lw      $ra, 108($k1)

	lw      $k0, 112($k1)    # $hi
	mthi    $k0
	lw      $k0, 116($k1)    # $lo
	mtlo    $k0

	lw      $k0, 120($k1)    # loads pc of the task that will run
	lw      $k1, 124($k1)    # loads offset of the task (for paging setup)
	li      $at, 0x1

	mtc0    $k1, $10
	jr      $k0             # jumps to pc
	mtc0    $at, $12        # enables interrupts
.end hal_run_task

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
    .long  1           # 4 bytes of initialized storage after the label


