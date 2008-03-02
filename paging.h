#ifndef KERN_PAGING_H
#define KERN_PAGING_H 1

#define PDIR_PAGE_PRESENT (1<<0)
#define PDIR_RWABLE       (1<<1)
void init_paging();

#endif
