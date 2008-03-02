#ifndef KERN_MEM_H
#define KERN_MEM_H 1

// Constants for defining page size
#define PAGE_BIT  12
#define PAGE_SIZE (1<<PAGE_BIT)

void align_to_page(long *address);
void *get_page();
void free_page(void *address);
void init_palloc(long phys_memory_size, long kernel_size, long *mb_data);

typedef struct {
	long size;
	long addr_low;
	long addr_high;
	long length_low;
	long length_high;
	long type;
} mem_map_t;

#endif
