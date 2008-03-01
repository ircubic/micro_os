#include "mem.h"
#include "screen.h"

extern void _end, _start;

static long bitmap_cells = 0;
static long bitmap_pages = 0;
static long num_pages = 0;
static long earliest_free_cell = 16*1024*1024/(1<<PAGE_BIT)/32;
static long *page_bitmap = 0;

void align_to_page(long *address)
{
	/* If we're in the middle of a page, move to next page and mask off the 4K
	 * part at the end;
	 */
	if(*address & 0xFFF)
		*address = (*address + 0x1000) & ~0xFFF;
}

void pmem_set_bit(long cell, char bit, char free)
{
	if(!free)
	{
		page_bitmap[cell] |= (1<<bit);
		num_pages--;
	}
	else
	{
		page_bitmap[cell] &= ~(1<<bit);
		num_pages++;
	}
}

void pmem_set_area(long offset, long len, char free)
{
	// Convert to pages
	offset >>= PAGE_BIT;
	if(offset >= bitmap_pages)
		return;

	align_to_page(&len);
	len >>= PAGE_BIT;
	if((offset + len) > bitmap_pages)
	{
		len = bitmap_pages - offset - 1;
		puts("TOO BIG");
	}

	// Initialize cell and bit offset
	long cur_cell = offset >> 5;
	long cur_bit = (offset & 0x1F);

	// Fill up the starting bits of the first partial cell, if any
	if(cur_bit != 0)
	{
		// Modify len so that we can easily extract trailing bits later
		len -= 32 - cur_bit;
		for(;cur_bit < 32; cur_bit++)
		{
			pmem_set_bit(cur_cell, cur_bit, free);
		}
		cur_bit = 0;
		cur_cell++;
	}

	// Fill full 32bit cells
	for(; len >= 32; len -= 32)
	{
		if(free)
		{
			num_pages += 32;
			page_bitmap[cur_cell] = 0;
		}
		else
		{
			num_pages -= 32;
			page_bitmap[cur_cell] = ~0;
		}
		cur_cell++;
	}

	// Fill the trailing bits
	for(cur_bit = 0; cur_bit < len; cur_bit++)
	{
		pmem_set_bit(cur_cell, cur_bit, free);
	}

}

void parse_memory_map(long *mb_data)
{
	if((mb_data[0] & 1<<6))
	{
		long usable_mem_size = 0;
		long full_mem_size = 0;
		long mmap_length = mb_data[11];
		void *mmap_end = (void*)mb_data[12] + mmap_length;
		mem_map_t *map_ptr = (mem_map_t *)mb_data[12];

		for(;(void*)map_ptr < mmap_end;)
		{
			/*
			print_hex(map_ptr->addr_low);
			putch(' ');
			print_hex(map_ptr->addr_high);
			putch(' ');
			print_hex(map_ptr->length_low);
			putch(' ');
			print_hex(map_ptr->length_high);
			putch(' ');
			print_hex(map_ptr->type);
			puts("");
			*/

			if(map_ptr->type == 1)
			{
				pmem_set_area(map_ptr->addr_low, map_ptr->length_low, 1);
				usable_mem_size += map_ptr->length_low;
			}
			full_mem_size += map_ptr->length_low;
			map_ptr = ((void*)map_ptr) + map_ptr->size + 4;
		}
		print_hex(usable_mem_size);
		putch(' ');
		putch('/');
		putch(' ');
		print_hex(full_mem_size);
		puts(" B of usable memory");
	}
	else
	{
		puts("No memory map info");
	}
}


void *get_page()
{
	long curr_cell = earliest_free_cell;
	long curr_bit = 0;
	void *address = (void*)0;

	for(;curr_cell < bitmap_cells; curr_cell++)
	{
		/* If this cell has at least one free page */
		if(page_bitmap[curr_cell] != ~0l)
		{
			break;
		}
	}

	long cell = page_bitmap[curr_cell];
	for(;curr_bit < 32; curr_bit++)
	{
		if((cell & (1<<curr_bit)) == 0)
		{
			address = (void*)(((curr_cell*32) + curr_bit) * PAGE_SIZE);
			page_bitmap[curr_cell] |= (1<<curr_bit);
			if(page_bitmap[curr_cell] == ~0)
				curr_cell++;
			earliest_free_cell = curr_cell;
			print_hex((long)address);
			puts(" returned");
			return address;
		}
	}
	return 0;
}

void free_page(void* page_addr)
{
	long page_offset = (long)page_addr >> PAGE_BIT;
	long cell = page_offset >> 5;
	long bit = page_offset & 0x1F;

	page_bitmap[cell] &= ~(1<<bit);

	if(cell < earliest_free_cell)
		earliest_free_cell = cell;


	print_hex((long)page_addr);
	puts(" freed");

}

void init_palloc(long phys_memory_size, long kernel_size, long* mb_data)
{
	page_bitmap = &_end;

	// Reserve memory for kernel and page stack in physical memory

	// How many pages is there in total?
	long reserve = (phys_memory_size / PAGE_SIZE);
	bitmap_pages = reserve;
	num_pages = reserve;
	
	// Turn into the amount of 32bit integers the bitmap will take
	reserve = (reserve >> 5) + ((reserve & 0x1F) != 0);
	bitmap_cells = reserve;

	// Plus the size of the kernel
	reserve = kernel_size + (reserve*4);
	align_to_page(&reserve);

	print_hex(reserve);
	puts(" bytes of reserved memory");

	pmem_set_area(0, phys_memory_size, 0);

	parse_memory_map(mb_data);

	pmem_set_area((long)&_start, reserve, 0);
	print_hex(num_pages);
	puts(" pages available for allocation");

	int i = 0;
	for(; i < bitmap_cells; i++)
	{
		if(page_bitmap[i])
		{
			print_hex(i);
			putch(' ');
			print_hex(page_bitmap[i]);
			puts("");
		}
	}
}

