# Compiler
CC = riscv64-unknown-elf-gcc
# Store .c source file
SRCS_C = \
	pmp_util.c \
	pmp_debug.c \
	pmp_user.c

# Names for relocatable object files
OBJS += $(SRCS_C:.c=.o)

# Set default goal
.DEFAULT_GOAL := all

all: libpmp.so

libpmp.so:${OBJS}
	@${CC} -shared $^ -o libxxx.so
	@rm *.o
	
$(OBJS): %.o : %.c
	@${CC} -c -fPIC $< -o $@

.PHONY: clean
clean:
	@rm *.so
