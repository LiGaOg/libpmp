
#include "platform.h"

	# size of each hart's stack is 1024 bytes
	.equ	STACK_SIZE, 1024

	.global	_start
	.global _save_context
	.global _restore_context

	.text
_start:
	# park harts with id != 0
	csrr	t0, mhartid		# read current hart id
	mv	tp, t0			# keep CPU's hartid in its tp for later usage.
	bnez	t0, park		# if we're not on the hart 0
					# we park the hart
	# Setup stacks, the stack grows from bottom to top, so we put the
	# stack pointer to the very end of the stack range.
	slli	t0, t0, 10		# shift left the hart id by 1024
	la	sp, stacks + STACK_SIZE	# set the initial stack pointer
					# to the end of the first stack space
	add	sp, sp, t0		# move the current hart stack pointer
					# to its place in the stack space

	j	start_kernel		# hart 0 jump to c

park:
	wfi
	j	park

_save_context:
	
	addi sp, sp, -120
	# save context

	sw x1, 0(sp)
	sw x3, 4(sp)
	sw x4, 8(sp)
	sw x5, 12(sp)
	sw x6, 16(sp)
	sw x7, 20(sp)
	sw x8, 24(sp)
	sw x9, 28(sp)
	sw x10, 32(sp)
	sw x11, 36(sp)
	sw x12, 40(sp)
	sw x13, 44(sp)
	sw x14, 48(sp)
	sw x15, 52(sp)
	sw x16, 56(sp)
	sw x17, 60(sp)
	sw x18, 64(sp)
	sw x19, 68(sp)
	sw x20, 72(sp)
	sw x21, 76(sp)
	sw x22, 80(sp)
	sw x23, 84(sp)
	sw x24, 88(sp)
	sw x25, 92(sp)
	sw x26, 96(sp)
	sw x27, 100(sp)
	sw x28, 104(sp)
	sw x29, 108(sp)
	sw x30, 112(sp)
	sw x31, 116(sp)

	call pmp_exception_handler

	
	lw x1, 0(sp)
	lw x3, 4(sp)
	lw x4, 8(sp)
	lw x5, 12(sp)
	lw x6, 16(sp)
	lw x7, 20(sp)
	lw x8, 24(sp)
	lw x9, 28(sp)
	lw x10, 32(sp)
	lw x11, 36(sp)
	lw x12, 40(sp)
	lw x13, 44(sp)
	lw x14, 48(sp)
	lw x15, 52(sp)
	lw x16, 56(sp)
	lw x17, 60(sp)
	lw x18, 64(sp)
	lw x19, 68(sp)
	lw x20, 72(sp)
	lw x21, 76(sp)
	lw x22, 80(sp)
	lw x23, 84(sp)
	lw x24, 88(sp)
	lw x25, 92(sp)
	lw x26, 96(sp)
	lw x27, 100(sp)
	lw x28, 104(sp)
	lw x29, 108(sp)
	lw x30, 112(sp)
	lw x31, 116(sp)
	addi sp, sp, 120

	mret

stacks:
	.skip	STACK_SIZE * MAXNUM_CPU # allocate space for all the harts stacks

	.end				# End of file
