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
C_OBJS = main.o init.o screen.o pic.o mem.o paging.o
OBJS = $(ASM_OBJS) $(C_OBJS)

# Other misc. vars
LNKSCR = linker.ld
OUTPUT = kernel.bin
TARGET_DIR = ../bin
IMAGE = $(TARGET_DIR)/a.img

default: $(OUTPUT) make_tags
	cp kernel.bin $(TARGET_DIR)

make_tags: 
	ctags -R .

image: default
	mcopy -o -i $(IMAGE) kernel.bin ::/


# Links the kernel binary
$(OUTPUT): $(LNKSCR) $(ASM_OBJS) $(C_OBJS)
	$(LD) -T $(LNKSCR) -o kernel.bin $(OBJS)

# General statement for assembly files
%.o: %.s
	$(ASM) $(ASMFLAGS) -o $@ $<

# General rule for C files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(OUTPUT) $(TARGET_DIR)/kernel.bin tags
