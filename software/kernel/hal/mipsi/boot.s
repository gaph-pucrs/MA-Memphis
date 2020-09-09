.globl hal_disable_interrupts
hal_disable_interrupts:
	jr      $ra			# Return to caller
	mtc0    $zero, $12	# Store coprocessor 0, register 12 the value of 0

.globl hal_run_task
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
	# lw      $gp, 96($k1)
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
