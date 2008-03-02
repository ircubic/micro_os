#include "paging.h"
#include "mem.h"
#include "screen.h"

#define SEGMENT_OFFSET 0x40000000

unsigned long kernel_page_dir[1024] __attribute__((aligned(4096)));
unsigned long first_page_table[1024] __attribute__((aligned(4096)));


void init_paging()
{
	/* Physical address of the page dir and page table*/
	void *pagedir_p = (void*)kernel_page_dir + SEGMENT_OFFSET;
	void *page_table_p = (void*)first_page_table + SEGMENT_OFFSET;

	int page = 0;
	/* Set up page table and clear page directory */
	for(; page < 1023; page++)
	{
		first_page_table[page] = (page * 4096) | 0x3;
		kernel_page_dir[page] = 0;
	}

	/* Identity map first 4MB, as well as mapping them into upper 3GB */
	kernel_page_dir[0] = (unsigned long)page_table_p | 0x3;
	kernel_page_dir[768] = (unsigned long)page_table_p | 0x3;

	asm volatile("mov %0, %%cr3;"
				 "mov %%cr0, %%eax;"
				 "orl $0x80000000, %%eax;"
				 "mov %%eax, %%cr0;" ::"r"(pagedir_p));
}
