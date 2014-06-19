#include <memory.h>

typedef struct fakeInode fakeInode_t;
struct fakeInode {
  char* name_;
  int readers_;
  size_t length_;
  spinlock_t lock_;
};

fakeInode_t* getInode (const char* name, size_t length)
{
  fakeInode_t* ino = (fakeInode_t*)kalloc (sizeof(fakeInode_t));
  ino->name_ = kcopystr (name);
  ino->length_ = length;
  return ino;
}


int kmmap (kAddSpace_t* addressSpace, void* add, fakeInode_t* ino, size_t length, off_t offset, int flags)
{
  kVma_t area = { flags, (uintptr_t)add, (uintptr_t)add + length, NULL, NULL, (kInode_t*)ino, offset};
  return kVma_MMap (addressSpace, &area) ? 0 : -1;
}

int kFs_Open(kInode_t* ino)
{
  fakeInode_t* f = (fakeInode_t*)ino;
  klock (&f->lock_);
  f->readers_++;
  kunlock (&f->lock_);
  return __noerror();
}

int kFs_Close(kInode_t* ino)
{
  fakeInode_t* f = (fakeInode_t*)ino;
  klock (&f->lock_);
  assert (f->readers_ > 0);
  f->readers_--;

  if (f->readers_ == 0) {
    free (f->name_);
    kunlock (&f->lock_);
    free (f);
    return __noerror();
  }

  kunlock (&f->lock_);
  return __noerror();
}

int main ()
{
  kAddSpace_t* addressSpace = kVma_New (STACK_DEFAULT);

  kVma_t area = { VMA_READ, 0, 2 * _Kb_, NULL, NULL, NULL, 0};
  kVma_MMap (addressSpace, &area);

  fakeInode_t* iLs = getInode ("ls", 0);
  fakeInode_t* iLib = getInode ("libaxc.so", 0);

  // We read the file - kernel asm loader read the first 8Kb of the file
  kmmap(addressSpace, NULL, iLs, 8 * _Kb_, 0, VMA_SHARED);

  // It use the library libaxc.so - kernel asm loader read the first 8Kb of the file
  kmmap(addressSpace, NULL, iLib, 8 * _Kb_, 0, VMA_SHARED);
  kVma_Display(addressSpace);

  // The program is ready...
  // Create the Heap
  kmmap(addressSpace, HEAP_START, NULL, 256 * _Mb_, 0, VMA_HEAP | VMA_READ | VMA_WRITE | VMA_GROWSUP);

  // We load the source code    (idea, we add VMA_FIXED, that say this page can be shared as FIXED address)
  //   CODE - The code page can't be shared, as it contains modified re-location and/or dynamic link
  //   DATA - The data page is marked as copy-on-write (the may_shared will request a page copy first)
  kmmap(addressSpace, (void*)(8 * _Mb_), iLs, 4 * _Kb_, 0x1000, VMA_READ | VMA_EXEC | VMA_CODE);
  kmmap(addressSpace, (void*)(8 * _Mb_ + 0x1000), iLs, 4 * _Kb_, 0x2000, VMA_MAYSHARED | VMA_MAYWRITE | VMA_READ | VMA_DATA);
  kmmap(addressSpace, NULL, iLib, 4 * _Kb_, 0x1000, VMA_READ | VMA_EXEC | VMA_CODE);
  kmmap(addressSpace, NULL, iLib, 4 * _Kb_, 0x2000, VMA_MAYSHARED | VMA_MAYWRITE | VMA_READ | VMA_DATA);

  // HERE WE EDIT THE ProcedureLinkageTable (or we let it like this...(it will call interupt resolveSymbol !?))
  kVma_Display(addressSpace);
  int k = 0;

  do {
    k = kVma_GrowUp (addressSpace, HEAP_START, 16 * _Mb_);
  } while (!k);

  kVma_Display(addressSpace);
  kVma_Destroy (addressSpace);
  NO_LOCK;
  return 0;
}
