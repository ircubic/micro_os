#include "mem.h"
#include "screen.h"
extern void _end, _start;
static long page_num = 0;
static void **page_stack = 0;
static void **page_stack_top = 0;

void align_to_page(long *address)
{
	/* If we're in the middle of a page, move to next page and mask off the 4K
	 * part at the end;
	 */
	if(*address & 0xFFF)
		*address = (*address + 0x1000) & ~0xFFF;
}

inline void push_page(void *address)
{
	page_stack_top++;
	page_num++;
	*page_stack_top = address;
}

inline void *pop_page()
{
	void *page = *page_stack_top;
	page_stack_top--;
	page_num--;
	return page;
}

inline void *top_page()
{
	return *page_stack_top;
}

long num_pages()
{
	return page_num;
}

void init_pagestack(long phys_memory_size, long kernel_size)
{
	page_stack = &_end;
	page_stack_top = page_stack - 1;
	// Reserve memory for kernel and page stack in physical memory

	// We need one 32bit integer for every page
	long reserve = (phys_memory_size / PAGE_SIZE) * 4;
	// Plus the size of the kernel
	reserve += kernel_size;
	align_to_page(&reserve);
	print_hex(reserve);
	puts(" bytes of reserved memory");

	// First page contains the last 4K of PM
	void *address = (void *)phys_memory_size - PAGE_SIZE;
	void *kernel_end = &_end + reserve;
	// Now start pushing pages on the page stack in reverse order, up to kernel
	for(;address >= (void *)kernel_end; address -= PAGE_SIZE)
	{
		push_page(address);
	}

	// Then from kernel start to 1MB barrier
	address = &_start - PAGE_SIZE;
	for(;address >= (void *)0x100000; address -= PAGE_SIZE)
	{
		push_page(address);
	}
	print_hex(num_pages());
	puts(" pages available for allocation");
}

