#ifndef KERN_MEM_H
#define KERN_MEM_H 1

#define PAGE_SIZE 4096

void align_to_page(long *address);
void inline push_page(void *address);
void init_pagestack(long phys_memory_size, long kernel_size);

#endif
