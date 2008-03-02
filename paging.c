#include "paging.h"
#include "mem.h"
#include "screen.h"

#define SEGMENT_OFFSET 0x40000000

extern void _start, _end, _phys_start;
long *page_base = (long*)0xFFC00000;


/* Get the page table at the given page directory index, creating it if it
 * doesn't exist
 */
void *get_page_table(long pdir_index)
{
	long *page_table = (long*)kernel_page_dir[pdir_index];
	if(page_table == 0)
	{
		page_table = get_page();
		kernel_page_dir[pdir_index] = (long)page_table;
		long page = 0;
		for(; page < 1024; page++)
			page_table[page] = 0;
	}
	return page_table;
}

/* Map the physical memory starting at pm_start to the virtual address vm_start,
 * for length bytes
 *
 * Assumes that the physical memory is already allocated
 */
void map_area(void *pm_start, void *vm_start, long length)
{
	/* First find the page directory entries we will be using */
	long pdir_entry = (long)vm_start >> 22;
	long pdir_end = ((long)vm_start + length) >> 22;
	long *page_table = 0;
	void *pm_ptr = (void*)((long)pm_start & (PAGE_SIZE-1));

	/* Amount of pages we need to map */
	length = (length + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

	print_hex(length);
	puts(" pages mapped");

	/* Walk through page directory entries */
	for(;pdir_entry <= pdir_end; pdir_entry++)
	{
		page_table = get_page_table(pdir_entry);

		/* Find the index into the page table */
		long page = ((long)pm_ptr >> 12) & 0x3FF;

		/* Map as many pages as we need to */
		for(; page < 1024 && length > 0; length -= PAGE_SIZE, page++, pm_ptr += PAGE_SIZE)
		{
			page_table[page] = (long)pm_ptr | 0x3;
		}
	}
}

void init_paging(void *kernel_start, unsigned long kernel_length)
{
	/* Allocate page directory and two page tables */
	void *pagedir_p = get_page();
	void *page_table_p = get_page();
	print_hex((long)pagedir_p);
	puts(" page dir");

	/* Set the addresses to be the higher-half addresses */
	kernel_page_dir = pagedir_p - SEGMENT_OFFSET;
	unsigned long *first_page_table = page_table_p - SEGMENT_OFFSET;

	int page = 0;

	/* Set up page table and clear page directory */
	for(; page < 1023; page++)
	{
		first_page_table[page] = (page * 4096) | 0x3;
		kernel_page_dir[page] = 0;
	}

	/* Identity map first 4MB */
	kernel_page_dir[0] = (unsigned long)page_table_p | 0x3;
	/* As well as mapping them into the 3GB range */
	kernel_page_dir[768] = (unsigned long)page_table_p | 0x3;

	/* Map page directory into itself as the top of the address space */
	kernel_page_dir[1023] = (unsigned long)pagedir_p | 0x3;

	asm volatile("mov %0, %%cr3;"
				 "mov %%cr0, %%eax;"
				 "orl $0x80000000, %%eax;"
				 "mov %%eax, %%cr0;"
				 ::"r"(pagedir_p));
	kernel_page_dir = (unsigned long*)0xFFFFF000;
}
