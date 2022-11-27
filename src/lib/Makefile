CC = riscv64-unknown-elf-gcc
CFLAGS = -nostdlib -fno-builtin -march=rv32ima -mabi=ilp32 -g -Wall
SRCS_C = \
	 pmp_heap.c \
	 pmp_user.c \

OBJS += $(SRCS_C:.c=.o)

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
