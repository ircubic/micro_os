#include "init.h"

/* Code to initialize machine
 * So far it's only used to set up segments in the GDT, but later
 * a lot of the init code will be here
 *
 * The segments we set up are 4GB flat
 */

// A single GDT entry
struct gdt_entry
{
	unsigned short limit_low;
	unsigned short base_low;
	unsigned char base_middle;
	unsigned char access;
	unsigned char granularity;
	unsigned char base_high;
} __attribute__((packed));

// The GDTR data
struct gdt_ptr
{
	unsigned short limit;
	unsigned long base;
} __attribute__((packed));

// We use three segments
struct gdt_entry gdt[3];
struct gdt_ptr gp;

// This is in loader.s
extern void gdt_set();

/* Set a single entry in the GDT, we hate the format :/
 * Params:
 *  num    - The entry to set
 *  base   - The address the segment starts with
 *  limit  - The size of the segment
 *  access - Access bits
 *  gran   - Granularity of segment (set in upper nibble)
 */
void gdt_set_entry(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
{
	gdt[num].base_low = (base & 0xFFFF);
	gdt[num].base_middle = (base >> 16) & 0xFF;
	gdt[num].base_high = (base >> 24) & 0xFF;

	gdt[num].limit_low = (limit & 0xFFFF);
	gdt[num].granularity = ((limit >> 16) & 0x0F);

	gdt[num].granularity |= (gran & 0xF0);
	gdt[num].access = access;
}

/* Set up and install our three segments */
void gdt_install()
{
	gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
	gp.base = (unsigned long)&gdt;

	gdt_set_entry(0, 0, 0, 0, 0);
	gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
	gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

	gdt_set();
}
