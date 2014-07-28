#include <kernel/memory.h>
#include <kernel/scheduler.h>
#include <kernel/inodes.h>
#include <kernel/info.h>


// ---------------------------------------------------------------------------
/** Resolve a soft page fault by allocating a new page in designed context
 *  FIXME Get rule to know how many pages must be mapped
 */
int kpg_fill_stream (kVma_t* vma, uint32_t address, int rights)
{
  if (KLOG_PF) kprintf ("PF] stream at <%x> \n", address);
  size_t lg = PAGE_SIZE / vma->ino_->stat_.cblock_;
  size_t off = (vma->offset_ + (address - vma->base_)) / vma->ino_->stat_.cblock_;

  uint32_t page;
  int read;
  if (kfs_map (vma->ino_, off, &page, &read))
    return __geterrno ();

  kpg_resolve (address, TABLE_DIR_PRC, rights, PG_USER_RDWR, page);
  if (read)
    kfs_feed (vma->ino_, (void*)address, lg, off);

  if (KLOG_PF) kprintf ("PF] fill stream at <%x> \n", address);
  return __noerror();
}


// ---------------------------------------------------------------------------
/** Resolve a soft page fault by allocating a new page in designed context */
void kpg_sync_stream (kVma_t* vma, uint32_t address)
{
  // uint32_t address = vma->base_;
  // size_t lg = vma->limit_ - vma->base_;
  // for (;address < vma->limit_; address += PAGE_SIZE)

  if (KLOG_PF) kprintf ("PF] stream at <%x> \n", address);
  size_t lg = PAGE_SIZE / vma->ino_->stat_.cblock_;
  size_t off = (vma->offset_ + (address - vma->base_)) / vma->ino_->stat_.cblock_;
  kfs_sync (vma->ino_, (void*)address, lg, off);
  // kTty_HexDump (address, 0x40);
  if (KLOG_PF) kprintf ("PF] sync stream at <%x> \n", address);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

