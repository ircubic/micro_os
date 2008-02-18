global _loader
global gdt_set
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

; Set up a stack and bounce into kernel main
_loader:
	mov esp, stack+STACKSIZE
	push eax ; Multiboot magic number
	push ebx ; Multiboot info struct
	call _main
	jmp $

; Helper stub for setting the gdt and segments
; XXX: Should probably move elsewhere
gdt_set:
extern gp
	lgdt [gp]
	mov eax, 0x10
	mov ds, eax
	mov ss, eax
	ret

; Space for our stack
section .bss
align 32
stack:
	resb STACKSIZE
