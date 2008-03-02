#ifndef KERNEL_ASM_H
#define KERNEL_ASM_H 1

/* Small ASM stubs to enable the use of I/O ports in C */
static inline void outb(unsigned short port, unsigned char val)
{
	asm volatile("outb %0, %1"::"a"(val), "Nd"(port));
}

static inline void outw(unsigned short port, unsigned short val)
{
	asm volatile("outw %0, %1"::"a"(val), "Nd"(port));
}

static inline void outl(unsigned short port, unsigned long val)
{
	asm volatile("outd %0, %1"::"a"(val), "Nd"(port));
}

static inline unsigned char inb(unsigned short port)
{
	unsigned char ret;
	asm volatile("inb %1, %0":"=a"(ret):"Nd"(port));
	return ret;
}

static inline unsigned short inw(unsigned short port)
{
	unsigned short ret;
	asm volatile("inw %1, %0":"=a"(ret):"Nd"(port));
	return ret;
}

static inline unsigned long inl(unsigned short port)
{
	unsigned long ret;
	asm volatile("inl %1, %0":"=a"(ret):"Nd"(port));
	return ret;
}

/* Access to the HLT instruction for sleepin' */
static inline void hlt()
{
	asm("hlt");
}

static inline void cli()
{
	asm("cli");
}

static inline void sti()
{
	asm("sti");
}

#endif
