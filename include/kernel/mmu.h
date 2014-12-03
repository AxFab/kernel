#ifndef KERNEL_MMU_H__
#define KERNEL_MMU_H__

#define AD_USERSP 1
#define AD_UNUSED -1
#define AD_KERNEL 0



#define  MMU_KERNEL (1 << 0)
#define  MMU_READ   (1 << 1)
#define  MMU_WRITE  (1 << 2)
#define  MMU_EXEC   (1 << 3)

#define  MMU_DIR_KERNEL (1 << 4)
#define  MMU_DIR_READ   (1 << 5)
#define  MMU_DIR_WRITE  (1 << 6)
#define  MMU_DIR_EXEC   (1 << 7)

#define  MMU_THREAD     (1 << 8)
#define  MMU_PROCESS    (1 << 9)
#define  MMU_SYSTEM     (1 << 10)

#define PF_PROT (1<<0)
#define PF_WRITE (1<<1)
#define PF_USER (1<<2)
#define PF_RSVD (1<<3)
#define PF_INSTR (1<<4) 


#define SIGSEV 13

void mmu_prolog (); 

/** Function to inform paging module that some RAM can be used by the system. */
void mmu_ram (uint64_t base, uint64_t length);
/** */
int mmu_init ();


/** Allocat a single page for the system and return it's physical address */
page_t mmu_newpage();
/** Mark a single physique page, returned by mmu_newpage, as available again */
void mmu_release(page_t page);

void mmu_stat ();

page_t mmu_new_dir ();

int mmu_resolve (void* address, page_t page, int access, bool zero);

void* mmu_temporary (page_t* page);

void mmu_reset_stack ();

// ---------------------------------------------------------------------------
static inline int mmu_addspace(void* address)
{
  size_t add = (size_t)address;
  if (add >= MMU_USERSP_BASE && add < MMU_USERSP_LIMIT)
    return AD_USERSP;

  if (add >= MMU_KHEAP_BASE && add < MMU_KHEAP_LIMIT)
    return AD_KERNEL;
  
  if (add >= MMU_KERNEL_BASE && add < MMU_KERNEL_LIMIT)
    return AD_KERNEL;

  return AD_UNUSED;
}


#endif /* KERNEL_MMU_H__ */
