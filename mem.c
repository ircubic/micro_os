#include "mem.h"
#include "screen.h"

// Constants that define the size of the bitmap cells
#define CELL_BIT  5
#define CELL_SIZE (1<<CELL_BIT)

extern void _end, _phys_start;

/* Align an address to a page boundary */
void align_to_page(long *address)
{
	*address = (*address + PAGE_SIZE - 1) & ~(PAGE_SIZE-1);
}

/*******************************************************************************
 * KERNEL HEAP ALLOCATOR
 *
 * This is a stripped down version of malloc() for the kernel's use. It is
 * stripped down because there are a lot of assumptions and simplifications we
 * can make since we are Lords of Memory
 ******************************************************************************/



/*******************************************************************************
 * PHYSICAL MEMORY ALLOCATOR
 *
 * Uses a single bitmap to keep track of which parts of physical memory are
 * currently in use. The bitmap is made up of 32bit integers to facilitate quick
 * searching for free pages.
 *
 * We also keep track of where the first free page is, to speed things up even
 * more
 ******************************************************************************/

/* The page bitmap */
static long *page_bitmap = 0;

/* A few places to store statistical information about the bitmap */
static long bitmap_cells = 0;
static long bitmap_pages = 0;
static long num_pages = 0;

/* Marker of the earliest free cell of physical memory, start at 1MB */
static long earliest_free_cell = 0x100000 >> (PAGE_BIT+CELL_BIT);

/* Find an open page in the bitmap at the cell offset given.
 * Assumes that the given cell actually has an open page
 */
int get_page_in_cell(long cell)
{
	char curr_bit = 0;
	long bitmap_cell = page_bitmap[cell];
	for(; curr_bit < CELL_SIZE; curr_bit++)
	{
		if((bitmap_cell & (1<<curr_bit)) == 0)
		{
			/* Set page to reserved */
			page_bitmap[cell] |= (1<<curr_bit);
			// Keep track of which cell in the bitmap has the first free page
			if(page_bitmap[cell] == ~0)
				earliest_free_cell = cell + 1;
			else
				earliest_free_cell = cell;

			return (cell<<CELL_BIT) + curr_bit;
		}
	}

	// XXX: what else can we do? :/
	return 0;
}

/* Get a free page of physical memory
 *
 * Walks through the cells of the bitmap, starting at the cell with the earliest
 * recorded free page, checks if they have all bits set (i.e. equal to ~0).
 * Then walk through the individual bits to find the free page, and convert the
 * index to a physical memory access
 */
void *get_page()
{
	long curr_cell = earliest_free_cell;
	void *address = (void*)0;

	/* First loop through on a cell-level, looking for cells with free pages,
	 * i.e. any cell that does not have all its bits set
	 */
	for(;curr_cell < bitmap_cells; curr_cell++)
	{
		if(page_bitmap[curr_cell] != ~0l)
		{
			break;
		}
	}

	/* Find the open page and convert it to an address */
	long page = get_page_in_cell(curr_cell);
	address = (void*)(page << PAGE_BIT);

	print_hex((long)address);
	puts(" allocated");

	return address;
}

/* Free the page pointed to by the page_addr by converting it to a cell/bit pair
 * in the bitmap and setting that to be open. Also updates the earliest free
 * cell if needed.
 *
 * TODO: For now is lenient and allows the address to be in the middle of a
 * page, but should probably be more strict about it later, as it is most likely
 * indicative of an error somewhere
 */
void free_page(void* page_addr)
{
	long page_offset = (long)page_addr >> PAGE_BIT;
	long cell = page_offset >> CELL_BIT;
	long bit = page_offset & (CELL_SIZE - 1);

	page_bitmap[cell] &= ~(1<<bit);

	if(cell < earliest_free_cell)
		earliest_free_cell = cell;


	print_hex((long)page_addr);
	puts(" freed");

}

/* Reserve or free the page of physical memory given by cell[bit] */
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

/* Reserve or free a page of physical memory, just a simple shorthand */
void pmem_set_page(long page, char free)
{
	pmem_set_bit(page >> CELL_BIT, page & (CELL_BIT-1), free);
}


/* Reserve or free an entire cell of physical memory */
void pmem_set_cell(long cell, char free)
{
	if(free)
	{
		num_pages += CELL_SIZE;
		page_bitmap[cell] = 0;
	}
	else
	{
		num_pages -= CELL_SIZE;
		page_bitmap[cell] = ~0;
	}
}

/* Reserve or free an area of physical memory in the bitmap */
void pmem_set_area(long offset, long len, char free)
{
	// Convert offset and len to pages, and do some sanity checking
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
	long curr_cell = offset >> CELL_BIT;
	long curr_bit = (offset & (CELL_SIZE-1));

	// Fill up the starting bits of the first partial cell, if any
	if(curr_bit != 0)
	{
		// Modify len so that we can easily extract trailing bits later
		len -= CELL_SIZE - curr_bit;
		for(;curr_bit < CELL_SIZE; curr_bit++)
			pmem_set_bit(curr_cell, curr_bit, free);

		curr_bit = 0;
		curr_cell++;
	}

	// Fill full 32bit cells
	for(; len >= CELL_SIZE; len -= CELL_SIZE)
	{
		pmem_set_cell(curr_cell, free);
		curr_cell++;
	}

	// Fill the trailing bits
	for(curr_bit = 0; curr_bit < len; curr_bit++)
		pmem_set_bit(curr_cell, curr_bit, free);

}

/* Parse the memory map given by GRUB and free up usable memory areas in the
 * bitmap
 */
void parse_memory_map(long *mb_data)
{
	// Is there a memory map?
	if((mb_data[0] & 1<<6))
	{
		/* Point to the memory map */
		long mmap_length = mb_data[11];
		void *mmap_end = (void*)mb_data[12] + mmap_length;
		mem_map_t *map_ptr = (mem_map_t *)mb_data[12];

		// This is just for sake of reporting at the end
		long usable_mem_size = 0;
		long full_mem_size = 0;

		for(;(void*)map_ptr < mmap_end;)
		{
			// Type 1 means usable memory
			if(map_ptr->type == 1)
			{
				pmem_set_area(map_ptr->addr_low, map_ptr->length_low, 1);
				usable_mem_size += map_ptr->length_low;
			}

			full_mem_size += map_ptr->length_low;

			// Move ahead size + sizeof(size) bytes, as the size of size is not
			// included in size. Yes, I did that on purpose.
			map_ptr = ((void*)map_ptr) + map_ptr->size + sizeof(map_ptr->size);
		}

		// Print utterly useless status information
		print_hex(usable_mem_size);
		putch(' ');
		putch('/');
		putch(' ');
		print_hex(full_mem_size);
		puts(" B of usable memory");
	}
	else
	{
		// XXX: Panic should be done here
		puts("No memory map info");
	}
}

/* Initialize the physical memory allocator
 * Set up the bitmap to fill the available physical memory space,
 * and reserve space for the memory areas given by GRUB as reserved, as well as
 * kernel space.
 */
void init_palloc(long phys_memory_size, long kernel_size, long* mb_data)
{
	// Bitmap starts right after the end of the kernel code
	page_bitmap = &_end;

	/* Reserve memory for kernel and page stack in physical memory */

	// How many pages is there in total?
	long reserve = (phys_memory_size / PAGE_SIZE);
	bitmap_pages = reserve;
	num_pages = reserve;
	
	// Get the amount of cells the bitmap will contain, aligned to full cells
	reserve = (reserve >> CELL_BIT) + ((reserve & (CELL_SIZE-1)) != 0);
	bitmap_cells = reserve;

	// Make into bytes and add the size of the kernel to get the reserved kernel
	// space
	reserve = kernel_size + (reserve*4);
	align_to_page(&reserve);

	print_hex(reserve);
	puts(" bytes of reserved memory");

	// Start by marking every bit of memory as taken
	pmem_set_area(0, phys_memory_size, 0);

	// Parse the memory map to re-open the usable memory space
	parse_memory_map(mb_data);

	// Then reserve kernel space
	pmem_set_area((long)&_phys_start, reserve, 0);

	print_hex(num_pages);
	puts(" pages available for allocation");

	/* Print memory map */
	puts("Memory map");
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

