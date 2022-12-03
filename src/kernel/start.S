
#include "platform.h"

	# size of each hart's stack is 1024 bytes
	.equ	STACK_SIZE, 1024

	.global	_start

	.text
_start:
	# park harts with id != 0
	csrr	t0, mhartid		# read current hart id
	mv	tp, t0			# keep CPU's hartid in its tp for later usage.
	mv  tp, t1          # set t1 to sepreate the hart
	# bnez	t0, park		# if we're not on the hart 0
					# we park the hart
	# Setup stacks, the stack grows from bottom to top, so we put the
	# stack pointer to the very end of the stack range.
	slli	t0, t0, 10		# shift left the hart id by 1024
	la	sp, stacks + STACK_SIZE	# set the initial stack pointer
					# to the end of the first stack space
	add	sp, sp, t0		# move the current hart stack pointer
					# to its place in the stack space
	bnez t1, start_kernel   # if it's on hart 0
	addi t1, t1, -1
	bnez t1, start_kernel_hart1
    addi t1, t1, -1
    bnez t1, start_kernel_hart2
	j	park		# rest of hart all park

park:
	wfi
	j	park

stacks:
	.skip	STACK_SIZE * MAXNUM_CPU # allocate space for all the harts stacks

	.end				# End of file
