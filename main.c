#include "asm.h"
#include "screen.h"
#include "init.h"
#include "pic.h"
#include "mem.h"

/* Method from interrupt.s */
extern void setup_idt();

/* Our generic Exception handler, that is bounced to from
 * any of the 32 first interrupt handlers in the IDT
 */
void k_exception_handler(unsigned int exception)
{
	set_cursor(0,0);
	putch('E');
	putch('X');
	putch('C');
	putch(' ');
	print_hex_char(exception);
}

/* Default keymap, just maps some symbols and digits */
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

/* Keyboard "driver", basically just an IRQ1 interrupt handler
 * Decodes the make/break codes via the keymap and prints the letters
 */
void keyboard_handler()
{
	unsigned char code = inb(0x60);
	if((code & 0x80) == 0)
	{
		// If there is a map, print the letter
		if(keymap[code] != -1)
			putch(keymap[code]);
	}
}

/* Generic IRQ handler, is bounced to from interrupt handlers 32-47 */
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

// Physical memory size in pages
extern void _end, _start;
long phys_memory_size = 0;
long kernel_size = 0;

/* Initialize kernel, doing these steps:
 *  - Set up GDT
 *  - Set up IDT
 *  - Remap PIC
 *  - Set up physical allocator
 *  - Make a page directory that maps the kernel 1:1, as well as other things
 *  - Set up paging
 */
void init(long *mb_data)
{
	cls();

	/* Check if memory size is present in mb_data and save it for our own use */
	if((mb_data[0] & 1) == 1)
	{
		phys_memory_size = (mb_data[2] + 1024) * 1024;
	}
	else
	{
		// XXX: Panic here
	}

	kernel_size = &_end - &_start;
	set_color(1);
	/*print_hex(phys_memory_size);
	puts(" bytes of physical memory");
	print_hex(kernel_size);
	puts(" bytes used for kernel");*/

	gdt_install();
	set_color(2);
	puts("Initialized GDT");

	setup_idt();
	set_color(3);
	puts("Set up IDT");

	pic_remap(32, 32+8);
	set_color(4);
	puts("Remapped PIC");

	set_color(5);
	init_palloc(phys_memory_size, kernel_size, mb_data);
	set_color(6);
	puts("Pagestack set up");

	set_color(7);
}

/* Kernel main */
void _main(void *mb_data, unsigned int mb_magic)
{
	init(mb_data);
	/* Spin on the halt, doing nothing whatsoever except handle interrupts */
	while(1)
	{
		hlt();
	}
}
