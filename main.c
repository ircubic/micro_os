#include "screen.h"
#include "init.h"
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

void k_irq_handler(unsigned int irq)
{
	set_cursor(0, 0);
	putch('I');
	putch('R');
	putch('Q');
	putch(' ');
	print_hex_char(irq);
}
void _main(void *mb_data, unsigned int mb_magic)
{
	cls();
	putch('A');
	puts("");
	gdt_install();
	putch('B');
	puts("");
	setup_idt();
	puts("");
	putch('C');
	puts("");
	__asm("int $4");
}
