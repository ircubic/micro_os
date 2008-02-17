global setup_idt
extern k_exception_handler
extern k_irq_handler

section .text
align 4

%macro intr_handler 3
_intr_%1:
	pusha
	push %2
	call %3
	pop eax
	popa 
	iret
%endmacro

%assign i 0
%rep 32
	intr_handler i, i, k_exception_handler
%assign i i+1
%endrep

%rep 16
	intr_handler i, i-32, k_irq_handler
%assign i i+1
%endrep

%macro set_idt_entry 4
	lea eax, [%2]
	mov WORD [idt + (%1 * 8)], ax
	shr eax, 16
	mov WORD [idt + (%1 * 8) + 2], %3
	mov BYTE [idt + (%1 * 8) + 4], 0
	mov BYTE [idt + (%1 * 8) + 5], %4
	mov WORD [idt + (%1 * 8) + 6], ax
%endmacro

setup_idt:

%assign i 0
%rep 32+16
	set_idt_entry i, _intr_ %+ i, 0x8, 0x8F
%assign i i+1
%endrep

	mov WORD [idtr], idt_end - idt - 1
	mov DWORD [idtr+2], idt
	lidt [idtr]
	ret

align 4
idt:
 TIMES ((32 + 16) * 8) db 0
idt_end:

idtr:
 dw 0
 dd 0
