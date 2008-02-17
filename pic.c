#include "pic.h"
#include "asm.h"

void pic_remap(unsigned int int_off1, unsigned int int_off2)
{
	unsigned char mask1, mask2;
	asm("cli");
	mask1 = inb(PIC_MASTER_DATA);
	mask2 = inb(PIC_SLAVE_DATA);

	/* INIT master */
	outb(PIC_MASTER_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
	/* master starts at off1 */
	outb(PIC_MASTER_DATA, int_off1);
	/* Master has slave at IN2 */
	outb(PIC_MASTER_DATA, 1<<2);
	/* Set 8086 mode */
	outb(PIC_MASTER_DATA, PIC_ICW4_8086);

	/* INIT slave */
	outb(PIC_SLAVE_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
	/* slave starts at off2 */
	outb(PIC_SLAVE_DATA, int_off2);
	/* Slave connects to IN2 of master */
	outb(PIC_SLAVE_DATA, 2);
	/* Set 8086 mode */
	outb(PIC_SLAVE_DATA, PIC_ICW4_8086);

	/* Restore masks */
	outb(PIC_MASTER_DATA, mask1);
	outb(PIC_SLAVE_DATA, mask2);
	asm("sti");
}

void pic_signal_eoi(unsigned int irq)
{
	/* Signal EOI for IRQ */
	if(irq >= 8)
		outb(PIC_SLAVE_COMMAND, PIC_EOI);
	outb(PIC_MASTER_COMMAND, PIC_EOI);
}
