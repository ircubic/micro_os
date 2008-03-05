global setup_idt
extern k_exception_handler
extern k_irq_handler

; This code sets up our standard IDT for launching into
; the kernel.

section .text
align 4

; Generic interrupt handler macro
; Params:
;  1: Interrupt number
;  2: Parameter to pass to handler function
;  3: Handler function
%macro intr_handler 3
_intr_%1:
; This is for interrupts with error codes
%if (%1 == 8) || (%1 >= 10 && %1 <= 14) || (%1 == 17)
	; Errorcode in eax, eax to esp
	xchg eax, [esp]
	; Emulate pusha
	push ebx
	push ecx
	push edx
	push ebp
	push esi
	push edi
%else
	pusha
%endif
	push DWORD %2
	call %3
	add esp, 4
%if (%1 == 8) || (%1 >= 10 && %1 <= 14) || (%1 == 17)
	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop eax
%else
	popa 
%endif
	iret

%endmacro

; Create 32 Exception handlers, with numeric offset 0
%assign i 0
%rep 32
	intr_handler i, i, k_exception_handler
%assign i i+1
%endrep

; Create 16 IRQ handlers, with numeric offset 1
%rep 16
	intr_handler i, i-32, k_irq_handler
%assign i i+1
%endrep

; Set an IDT entry
; Params:
;  1: IDT index
;  2: Interrupt handler
;  3: Segment
;  4: Attributes
%macro set_idt_entry 4
	lea eax, [%2]
	mov WORD [idt + (%1 * 8)], ax
	shr eax, 16
	mov WORD [idt + (%1 * 8) + 2], %3
	mov BYTE [idt + (%1 * 8) + 4], 0
	mov BYTE [idt + (%1 * 8) + 5], %4
	mov WORD [idt + (%1 * 8) + 6], ax
%endmacro

; Setup an IDT table with 32 Exception handlers and 16 IRQ handlers
; They are all marked as present interrupt gates
; All of them point to code segment.
setup_idt:
	cli
%assign i 0
%rep 32+16
	set_idt_entry i, _intr_ %+ i, 0x8, 0x8E
%assign i i+1
%endrep

	mov WORD [idtr], idt_end - idt - 1
	; The addition is because when the IDT is setup, we're still using the trick GDT,
	; and the IDTR needs to contain the physical address
	mov DWORD [idtr+2], idt + 0x40000000 
	lidt [idtr] 
	sti
	ret

; Make room for our IDT
align 4
idt:
 TIMES ((32 + 16) * 8) db 0
idt_end:

; IDTR data
idtr:
 dw 0
 dd 0
