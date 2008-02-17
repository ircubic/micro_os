global _loader
extern _main

section .text
align 4
MAGIC    equ 0x1BADB002       ; Multiboot Magic
FLAGS    equ (1<<1) | 1       ; Multiboot flags - 4K align and memory info
CHECKSUM equ -(MAGIC + FLAGS) ; Checksum for Multiboot

; Multiboot Header
	dd MAGIC 
	dd FLAGS
	dd CHECKSUM

STACKSIZE equ 0x4000

_loader:
	mov esp, stack+STACKSIZE
	cli ; XXX: No interrupts = no triple fault = <3
	push eax ; Multiboot magic number
	push ebx ; Multiboot info struct
	call _main
	hlt

section .bss
align 32
stack:
	resb STACKSIZE