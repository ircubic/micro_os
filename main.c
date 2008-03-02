#include "asm.h"
#include "screen.h"
#include "init.h"
#include "pic.h"
#include "mem.h"
#include "paging.h"

/* Macro for fixing up a memory address that is not segmented */
#define DE_SEGMENT(x) ((void*)x - 0x40000000)

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
extern void _end, _start, _phys_start;
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
	/* Screen needs to work while we're in our trick segment */
	screen_set_base((void*)DE_SEGMENT(0xB8000));
	cls();

	/* Fix up MultiBoot data so that it can be used in our trick segmentation */
	mb_data = (long*) DE_SEGMENT(mb_data);
	mb_data[12] = (long) DE_SEGMENT(mb_data[12]);

	/* Check if memory size is present in mb_data and save it for our own use */
	if((mb_data[0] & 1) == 1)
	{
		phys_memory_size = (mb_data[2] + 1024) * 1024;
	}
	else
	{
		puts("OH FUCK");
	}


	kernel_size = &_end - &_start;

	set_color(2);
	kernel_size = init_palloc(phys_memory_size, kernel_size, mb_data);
	puts("Pagestack set up");

	set_color(3);
	setup_idt();
	puts("Set up IDT");

	set_color(4);
	init_paging(&_phys_start, kernel_size);
	puts("Paging");

	set_color(5);
	gdt_install();
	print_hex(kernel_page_dir[0]);
	puts(" is the first MB!");
	print_hex(page_base[256]);
	puts(" is also the first MB");
	screen_set_base((void*)0xB8000);
	puts("Initialized GDT");


	pic_remap(32, 32+8);
	set_color(6);
	puts("Remapped PIC");
	sti();

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
