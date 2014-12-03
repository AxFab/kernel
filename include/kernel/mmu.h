#ifndef KERNEL_MMU_H__
#define KERNEL_MMU_H__

#define AD_USERSP 1
#define AD_UNUSED -1
#define AD_KERNEL 0



#define VMA_WRITE         0x002 ///< Pages can be written to
#define VMA_EXEC          0x001 ///< Pages can be executed
#define VMA_READ          0x004 ///< Pages can be read from

#define VMA_KERNEL       0x10000

#define VMA_SHARED        0x008 ///< Page are shared
#define VMA_MAYREAD       0x010 ///< The VMA_READ flag can be set
#define VMA_MAYWRITE      0x020 ///< The VMA_WRITE flag can be set
#define VMA_MAYEXEC       0x040 ///< The VMA_EXEC flag can be set
#define VMA_MAYSHARED     0x080 ///< The VMA_SHARED flag can be set
#define VMA_GROWSDOWN     0x100 ///< The area can grow downward
#define VMA_GROWSUP       0x200 ///< The area can grow upward

#define VMA_SHM           0x400 ///< The area is used for shared memory
#define VMA_FILE          0x800 ///< The area map an executable file
#define VMA_HEAP          0x1000 ///< The area map a process heap
#define VMA_STACK         0x2000 ///< The area map a thread stack

#define VMA_CODE          0x4000
#define VMA_DATA          0x8000


#define VMA_ACCESS      (VMA_WRITE | VMA_EXEC | VMA_READ)
#define VMA_MMU         (VMA_ACCESS | VMA_KERNEL)
#define VMA_ASSEMBLY    (VMA_CODE | VMA_DATA)
#define VMA_TYPE        (VMA_SHM | VMA_FILE | VMA_HEAP | VMA_STACK)


#define PF_PROT   (1<<0)
#define PF_WRITE  (1<<1)
#define PF_USER   (1<<2)
#define PF_RSVD   (1<<3)
#define PF_INSTR  (1<<4) 


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

int mmu_resolve (size_t address, page_t page, int access, bool zero);

void* mmu_temporary (page_t* page);

void mmu_reset_stack ();

// ---------------------------------------------------------------------------
static inline int mmu_addspace(size_t address)
{
  if (address >= MMU_USERSP_BASE && address < MMU_USERSP_LIMIT)
    return AD_USERSP;

  if (address >= MMU_KHEAP_BASE && address < MMU_KHEAP_LIMIT)
    return AD_KERNEL;
  
  if (address >= MMU_KERNEL_BASE && address < MMU_KERNEL_LIMIT)
    return AD_KERNEL;

  return AD_UNUSED;
}


#endif /* KERNEL_MMU_H__ */
