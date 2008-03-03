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

_loader:
	cli
	; Save the magic number
	mov ecx, eax
	; here's the trick: we load a GDT with a base address
	; of 0x40000000 for the code (0x08) and data (0x10) segments
	lgdt [trickgdt]
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	; jump to the higher half kernel
	jmp 0x08:higherhalf

higherhalf:
; Set up a stack and bounce into kernel main
	mov esp, stack+STACKSIZE
	push ecx ; Multiboot magic number
	push ebx ; Multiboot info struct
	call _main
	jmp $

; Helper stub for setting the gdt and segments
gdt_set:
extern gp
	lgdt [gp]
	mov eax, 0x10
	mov ds, eax
	mov es, eax
	mov fs, eax
	mov gs, eax
	mov ss, eax
	jmp 0x08:_skip
_skip:
	ret

; This is a special setup section for the fake GDT that allows us to higher-half
; the kernel
section .setup
trickgdt:
	dw gdt_end - gdt - 1 ; size of the GDT
	dd gdt ; linear address of GDT

gdt:
	dd 0, 0							; null gate
	db 0xFF, 0xFF, 0, 0, 0, 10011010b, 11001111b, 0x40	; code selector 0x08: base 0x40000000, limit 0xFFFFFFFF, type 0x9A, granularity 0xCF
	db 0xFF, 0xFF, 0, 0, 0, 10010010b, 11001111b, 0x40	; data selector 0x10: base 0x40000000, limit 0xFFFFFFFF, type 0x92, granularity 0xCF

gdt_end:

; Space for our stack
section .bss
align 32
	resb STACKSIZE
stack:

