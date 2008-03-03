#ifndef KERN_PAGING_H
#define KERN_PAGING_H 1

#define PDIR_PAGE_PRESENT (1<<0)
#define PDIR_RWABLE       (1<<1)
void init_paging(void *kernel_start, unsigned long kernel_length);
unsigned long *kernel_page_dir;
extern long *page_base;

#endif
