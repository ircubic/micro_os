# testOS Makefile
# Copyright (c) 2005 Daniel E. Bruce

# Programs
CC = gcc
LD = ld
ASM = nasm

# Flags
CFLAGS = -O -Wall -Werror -fno-builtin -nostdlib -nostartfiles -nodefaultlibs
ASMFLAGS = -f elf
LDFLAGS = 

# Object list for kernel
ASM_OBJS = loader.o interrupt.o
C_OBJS = main.o init.o screen.o pic.o mem.o
OBJS = $(ASM_OBJS) $(C_OBJS)

# Other misc. vars
LNKSCR = linker.ld
OUTPUT = kernel.bin

default: $(OUTPUT)

# Links the kernel binary
$(OUTPUT): $(ASM_OBJS) $(C_OBJS)
	$(LD) -T $(LNKSCR) -o kernel.bin $(OBJS)

# Assembles .asm files into .o files =)
%.o: %.s
	$(ASM) $(ASMFLAGS) -o $@ $<

# Creates C files :o
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(OUTPUT)
