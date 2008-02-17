#include "asm.h"
#include "screen.h"
#include "init.h"
#include "pic.h"
extern void setup_idt();
extern void fsck();
extern const char* idt;

void k_exception_handler(unsigned int exception)
{
	set_cursor(0,0);
	putch('E');
	putch('X');
	putch('C');
	putch(' ');
	print_hex_char(exception);
}

const char keymap[128] = {
	  0,  -1, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',  -1,  -1,
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', -1, 'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'','`',  -1, '\\','Z', 'X', 'C', 'V',
	'B', 'N', 'M', ',', '.', '/',  -1,  -1,  -1, ' ',  -1,  -1,  -1,  -1,  -1,  -1,
	 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  
	 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  
	 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  
	 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  
};

void keyboard_handler()
{
	unsigned char code = inb(0x60);
	set_cursor(0,0);
	putch('K');
	putch('B');
	putch('D');
	putch(' ');
	if((code & 0x80) == 0)
	{
		if(keymap[code] != -1)
			putch(keymap[code]);
		else
			print_hex_char(code);
	}
}

void k_irq_handler(unsigned int irq)
{
	if(irq == 1)
	{
		keyboard_handler();
	}
	else if(irq > 1)
	{
		set_cursor(0, 0);
		putch('I');
		putch('R');
		putch('Q');
		putch(' ');
		print_hex_char(irq);
	}
	pic_signal_eoi(irq);
}
void _main(void *mb_data, unsigned int mb_magic)
{
	cls();
	gdt_install();
	puts("GDT");
	setup_idt();
	puts("IDT");
	pic_remap(32, 32+8);
	puts("PIC");
}
