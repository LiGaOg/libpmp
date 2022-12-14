# Compiler
CC = riscv64-unknown-elf-gcc
CFLAGS = -nostdlib -fno-builtin -march=rv32ima -mabi=ilp32 -g -Wall
# Store .c source file
SRCS_C = \
	pmp_util.c \
	pmp_debug.c \
	pmp_exception.c \
	pmp_user.c

# Names for relocatable object files
OBJS += $(SRCS_C:.c=.o)

# Set default goal
.DEFAULT_GOAL := all

all: libpmp.a

libpmp.a:${OBJS}
	@ar rcs libpmp.a $^
	@rm *.o
	
$(OBJS): %.o : %.c
	@${CC} ${CFLAGS} -c $< -o $@
.PHONY: clean
clean:
	@rm *.a
