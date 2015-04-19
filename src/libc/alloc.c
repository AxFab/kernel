#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <smkos/spinlock.h>
#include <smkos/llist.h>

#define __axlog kprintf
void __axlog(const char*msg, ...);


/* Private Macros ---------------------------------------------------------- */
#define ALLOC_MAX_CHUNK     0x30000000
#define ALLOC_MIN_CHUNK     sizeof(struct SMK_HeapChunk)
#define ALLOC_CHUNK_HEAD    (sizeof (uint32_t)*4)
#define alloc_chunk(p)      ((struct SMK_HeapChunk*)((uint32_t)(p) - ALLOC_CHUNK_HEAD))

#define ALLOC_CHECK         0x01
#define ALLOC_ISUSED        0x01
#define ALLOC_CORRUPTED     0x02
#define ALLOC_PARANOID      0x04

#define ALIGN(v,a)      (((v)+(a-1))&(~(a-1)))
#define _LOCK(h)        ((void)(0))
#define _UNLOCK(h)        ((void)(0))

/* Structures definitions -------------------------------------------------- */

struct SMK_HeapChunk {
    uint32_t prev_size;
    uint32_t reserved;
    uint32_t chunk_size;
    uint32_t is_used;
    union {
        // If the block is empty
        struct {
            struct SMK_HeapChunk* prev_chunk;
            struct SMK_HeapChunk* next_chunk;
        };
        // If the block is used
        uint32_t data[4];
    };
};


struct SMK_HeapArea {
    struct SMK_HeapChunk* start;
    struct SMK_HeapChunk* free_list;
    size_t max;
    size_t available;
    size_t begin_;
    size_t end_;
    char lock;
    long flags;
    struct spinlock lock_;
    struct llnode node_;
};


struct llhead gHeapArea;
struct SMK_HeapArea gArea;


/* Functions --------------------------------------------------------------- */
#define valloc(n) memalign_r(heap, PAGE_SIZE, (n))
#define pvalloc(n) memalign_r(heap, PAGE_SIZE, ALIGN(n, PAGE_SIZE))



/* ----------------------------------------------------------------------- */
/** Append a memory block on the list of free blocks. */
static void alloc_add_to_free (struct SMK_HeapArea* heap, struct SMK_HeapChunk* chunk)
{
    struct SMK_HeapChunk* curs = heap->free_list;
    chunk->is_used = 0; //  &= ~ALLOC_ISUSED;

    // In case there is no available blocks
    if ( !curs ) {
        heap->free_list = chunk;
        chunk->prev_chunk = NULL;
        chunk->next_chunk = NULL;
        return;
    }

    // In case the free block is the smallest of all
    if ( curs->chunk_size > chunk->chunk_size ) {
        heap->free_list = chunk;
        chunk->prev_chunk = NULL;
        chunk->next_chunk = curs;
        curs->prev_chunk = chunk;
        return;
    }

    // Browse the list until we found a bigger block or the end of the list
    while ( curs->next_chunk ) {
        if ( curs->next_chunk->chunk_size > chunk->chunk_size ) {
            break;
        }

        curs = curs->next_chunk;
    }

    if ( curs->next_chunk ) {
        curs->next_chunk->prev_chunk = chunk;
    }

    chunk->prev_chunk = curs;
    chunk->next_chunk = curs->next_chunk;
    curs->next_chunk = chunk;
}


/* ----------------------------------------------------------------------- */
/** Remove a memory block of the list of free blocks. */
static void alloc_rem_of_free (struct SMK_HeapArea* heap, struct SMK_HeapChunk* chunk)
{
    if ( chunk->next_chunk ) {
        chunk->next_chunk->prev_chunk = chunk->prev_chunk;
    }

    if ( chunk->prev_chunk ) {
        chunk->prev_chunk->next_chunk = chunk->next_chunk;
    } else {
        heap->free_list = chunk->next_chunk;
    }

    chunk->prev_chunk = (struct SMK_HeapChunk*)0xcccccccc;
    chunk->next_chunk = (struct SMK_HeapChunk*)0xcccccccc;
    chunk->is_used |= ALLOC_ISUSED;
}

/* ----------------------------------------------------------------------- */
/**
 * @brief retrace the heap to detect memory corruption
 * @return zero if the heap is not corrupted, non zero is errors have been
 * detected.
 *
 * Browse the list of block on address order check consistency and count
 * the number of blocks. Compare that to the free list and check list
 * consistency.
 */
int memcorrupt_r (struct SMK_HeapArea* heap)
{
  int err = 0;
  int free_chunks = 0;
  int total_chunks = 0;
  uintmax_t lsize = 0;
  struct SMK_HeapChunk* chunk = heap->free_list;
  struct SMK_HeapChunk* prev = NULL;
  while (chunk != NULL) {
    free_chunks++;
    if (chunk->is_used) {
      __axlog ("Free chunk at 0x%x mark as used\n", chunk);
      err++;
    }
    if (chunk->prev_chunk != prev) {
      __axlog ("Free Chunk at 0x%x isn't link to previous\n", chunk);
      err++;
    }
    if (chunk->chunk_size < lsize) {
      __axlog ("Free chunk at 0x%x is smaller than previous ones\n", chunk);
      err++;
    }
    lsize = chunk->chunk_size;
    prev = chunk;
    chunk = chunk->next_chunk;
  }

  chunk = heap->start;
  lsize = 0;
  while ((size_t)chunk < heap->max) {
    total_chunks++;
    if (chunk->is_used) {
      /*
      if (chunk->next_chunk != (struct SMK_HeapChunk*)0xcccccccc)
          err++;
      if (chunk->prev_chunk != (struct SMK_HeapChunk*)0xcccccccc)
          err++; */
    } else {
      free_chunks--;
      if ((uint32_t)chunk->next_chunk & 7)
        err++;
      if ((uint32_t)chunk->prev_chunk & 7)
        err++;
    }

    if (chunk->prev_size != lsize) {
      __axlog ("Wrong prev size mark at 0x%x\n", chunk);
      err++;
    }
    lsize = chunk->chunk_size;
    chunk = (struct SMK_HeapChunk*)((size_t)chunk + (size_t)chunk->chunk_size);
  }

  if (free_chunks != 0) {
    __axlog ("Free chunks not referenced\n", chunk);
    err++;
  }
  if ((size_t)chunk != heap->max) {
    __axlog ("Incomplete chunk map\n", chunk);
    err++;
  }

  return (err);
}

/* ----------------------------------------------------------------------- */
/** Initialize a heap segment structure info. */
void meminit_r(struct SMK_HeapArea* heap, void* base, size_t length)
{
  heap->flags |= ALLOC_PARANOID;
  heap->start = ( struct SMK_HeapChunk* ) ALIGN ( (uintptr_t)base, ALLOC_MIN_CHUNK );
  heap->begin_ = (size_t)base;
  heap->end_ = (size_t)base + length;
  heap->free_list = NULL;
  heap->available = (size_t)heap->start - (size_t)base;
  heap->available = length - heap->available;
  heap->available = heap->available & ~(ALLOC_MIN_CHUNK - 1);
  heap->max = (size_t)heap->start + heap->available;
  heap->start->prev_size = 0;
  heap->start->chunk_size = heap->available;
  heap->flags |= ALLOC_CHECK;
  heap->lock = 0;
  alloc_add_to_free (heap, heap->start);
}


/* ----------------------------------------------------------------------- */
/**
 * @brief allocate dynamic memory
 * @return The malloc() and calloc() functions return a pointer to the
 * allocated memory that is suitably aligned for any kind of variable. On
 * error, these functions return NULL. NULL may also be returned by a
 * successful call to malloc() with a size of zero, or by a successful call to
 * calloc() with nmemb or size equal to zero.
 *
 * The malloc() function allocates size bytes and returns a pointer to the
 * allocated memory. The memory is not initialized. If size is 0, then
 * malloc() returns either NULL, or a unique pointer value that can later be
 * successfully passed to free().
 *
 * @see malloc, free, calloc, realloc
 * @see brk, mmap, alloca, malloc_get_state, malloc_info, malloc_trim,
 * malloc_usable_size, mallopt, mcheck, mtrace, posix_memalign
 * @note By default, the kernel follows an optimistic memory allocation
 * strategy. This means that when malloc() returns non-NULL there is no
 * guarantee that the memory really is available. In case it turns out that
 * the system is out of memory, one or more processes will be killed.
 */
void* malloc_r(struct SMK_HeapArea* heap, size_t size)
{
  struct SMK_HeapChunk* chunk = heap->free_list;
  struct SMK_HeapChunk* split = NULL;
  struct SMK_HeapChunk* prev = NULL;
  struct SMK_HeapChunk* next = NULL;
  size_t lsize = 0;

  _LOCK(heap);

  if (heap->flags & ALLOC_PARANOID) {
    if (memcorrupt_r(heap)) {
      __axlog ("HEAP IS CORRUPTED #1\n");
    }
  }

  // We align the length requested
  if (size > ALLOC_MAX_CHUNK - ALLOC_MIN_CHUNK) {
    _UNLOCK(heap);
    return NULL;
  }
  size = ALIGN (size + ALLOC_CHUNK_HEAD, ALLOC_MIN_CHUNK );
  if (size < ALLOC_MIN_CHUNK) size = ALLOC_MIN_CHUNK;

  // Browse the free chunk list
  while ( chunk ) {

    // If we ask for heap corruption checks
    if (heap->flags & ALLOC_CHECK) {
      if (chunk->is_used)
        heap->flags |= ALLOC_CORRUPTED;
      if (chunk->prev_chunk != prev)
        heap->flags |= ALLOC_CORRUPTED;
      if (chunk->chunk_size < lsize)
        heap->flags |= ALLOC_CORRUPTED;
      if (heap->flags & ALLOC_CORRUPTED) {
        _UNLOCK(heap);
        return NULL;
      }

      lsize = chunk->chunk_size;
      prev = chunk;
    }

    if ( chunk->chunk_size >= size) {
      alloc_rem_of_free ( heap, chunk );
      if (chunk->chunk_size >= size + ALLOC_MIN_CHUNK ) {
        // If the size is enough for a new block
        split = ( struct SMK_HeapChunk* ) ((( size_t ) chunk ) + size);
        split->chunk_size = chunk->chunk_size - size;
        split->prev_size = size;
        next =  (struct SMK_HeapChunk*)((size_t)split + (size_t)split->chunk_size);
        if ((size_t)next < heap->max)
          next->prev_size = split->chunk_size;
        else if ((heap->flags & ALLOC_CHECK) && (size_t)next != heap->max) {
          heap->flags |= ALLOC_CORRUPTED;
          _UNLOCK(heap);
          return NULL;
        }
        alloc_add_to_free ( heap, split );
        chunk->chunk_size = size;
        chunk->is_used |= ALLOC_ISUSED;
      }

      heap->available -= size;
      _UNLOCK(heap);

      if (heap->flags & ALLOC_PARANOID) {
        if (memcorrupt_r(heap)) {
          __axlog ("HEAP IS CORRUPTED #2\n");
        }
      }

      // __axlog ("MALLOc RETURN 0x%x or 0x%x\n", chunk->data, &chunk->prev_chunk);
      return (chunk->data);
    }

    chunk = chunk->next_chunk;
  }

  _UNLOCK(heap);
  errno = ENOMEM;

  if (heap->flags & ALLOC_PARANOID) {
    if (memcorrupt_r(heap)) {
      __axlog ("HEAP IS CORRUPTED #3\n");
    }
  }

  return NULL;
}


/* ----------------------------------------------------------------------- */
/**
 * @brief free dynamic memory
 * @return The free() function returns no value.
 *
 * The free() function frees the memory space pointed to by ptr, which must
 * have been returned by a previous call to malloc(), calloc() or realloc().
 * Otherwise, or if free(ptr) has already been called before, undefined
 * behavior occurs. If ptr is NULL, no operation is performed.
 *
 * @see malloc, free, calloc, realloc
 * @see brk, mmap, alloca, malloc_get_state, malloc_info, malloc_trim,
 * malloc_usable_size, mallopt, mcheck, mtrace, posix_memalign
 */
void free_r(struct SMK_HeapArea* heap, void* ptr)
{
  struct SMK_HeapChunk* chunk = alloc_chunk(ptr);
  struct SMK_HeapChunk* prev = NULL;
  struct SMK_HeapChunk* next = NULL;

  if (heap->flags & ALLOC_PARANOID) {
    if (memcorrupt_r(heap)) {
      __axlog ("HEAP IS CORRUPTED #4\n");
    }
  }

  if ((size_t)chunk < (size_t)heap->start || (size_t)chunk > heap->max) {
    return;
  }

  _LOCK(heap);
  prev = (struct SMK_HeapChunk*)((size_t)chunk - (size_t)chunk->prev_size);
  next = (struct SMK_HeapChunk*)((size_t)chunk + (size_t)chunk->chunk_size);

  // If we ask for heap corruption checks
  if (heap->flags & ALLOC_CHECK) {
    if (!chunk->is_used)
      heap->flags |= ALLOC_CORRUPTED;
    if (prev != chunk && prev->chunk_size != chunk->prev_size)
      heap->flags |= ALLOC_CORRUPTED;
    if ((size_t)next < heap->max && next->prev_size != chunk->chunk_size)
      heap->flags |= ALLOC_CORRUPTED;
    if (heap->flags & ALLOC_CORRUPTED) {
      _UNLOCK(heap);

      if (heap->flags & ALLOC_PARANOID) {
        if (memcorrupt_r(heap)) {
          __axlog ("HEAP IS CORRUPTED #5\n");
        }
      }

      return;
    }
  }

  // Increase available memory
  heap->available += chunk->chunk_size;

  // If the previous chunk is unused collapse
  if (prev != chunk && !prev->is_used) {
    alloc_rem_of_free(heap, prev);
    // prev->is_used = FALSE;
    prev->chunk_size += chunk->chunk_size;
    if ((size_t)next < heap->max)
      next->prev_size = prev->chunk_size;
    chunk = prev;
  }

  // If the next chunk is unused collapse
  if ((size_t)next < heap->max && !next->is_used) {
    alloc_rem_of_free(heap, next);
    // next->is_used = FALSE;
    chunk->chunk_size += next->chunk_size;
    next = (struct SMK_HeapChunk*)((size_t)chunk + (size_t)chunk->chunk_size);
    if ((size_t)next < heap->max)
      next->prev_size = chunk->chunk_size;
  }

  // Freed the chunk
  alloc_add_to_free (heap, chunk);

  if (heap->flags & ALLOC_PARANOID) {
    if (memcorrupt_r(heap)) {
      __axlog ("HEAP IS CORRUPTED #6\n");
    }
  }

  _UNLOCK(heap);
}

/**
 * @brief allocate aligned memory
 * @return aligned_alloc(), memalign(), valloc(), and pvalloc() return a
 * pointer to the allocated memory, or NULL if the request fails.
 *
 * The obsolete function memalign() allocates size bytes and returns a pointer
 * to the allocated memory. The memory address will be a multiple of alignment,
 * which must be a power of two.
 *
 * @throw EINVAL - The alignment argument was not a power of two, or was not a
 * multiple of sizeof(void *).
 * @throw ENOMEM - There was insufficient memory to fulfill the allocation
 * request.
 * @see posix_memalign, aligned_alloc, memalign, valloc, pvalloc
 * @see brk, mmap, alloca, malloc_get_state, malloc_info, malloc_trim,
 * malloc_usable_size, mallopt, mcheck, mtrace, posix_memalign
 */
#if 0
void* memalign_r(struct SMK_HeapArea* heap, size_t alignment, size_t size)
{
    xHeapChunk_t* chunk;
    int* ptr, *aptr;

    if (!bpw2(alignment)) {
        // TODO errno = EINVAL
        return NULL;
    }

    if (alignment <= 16)
        return (malloc_r(heap, size));

    ptr = (int*)malloc_r(heap, size + alignment);
    if (((size_t)ptr & (alignment - 1)) == 0) {
        // Is already aligned
        chunk = alloc_chunk(ptr);
    //  chunk->chunk_size = ;
    }

    aptr = (int*)ALIGN((uintptr_t)ptr, alignment);



    return (ptr);
}
#endif




/* ----------------------------------------------------------------------- */
void* malloc_(size_t length)
{
  void* ptr;
  struct SMK_HeapArea *area;
  ll_for_each (&gHeapArea, area, struct SMK_HeapArea, node_) {
    ptr = malloc_r(area, length);
    if (ptr)
      return ptr;
  }

  return NULL;
}


/* ----------------------------------------------------------------------- */
void free_(void* ptr)
{
  struct SMK_HeapArea *area;
  ll_for_each (&gHeapArea, area, struct SMK_HeapArea, node_) {
    if (area->begin_ >= (size_t)ptr && area->end_ < (size_t)ptr) {
      free_r (area, ptr);
      return;
    }
  }
}


/* ----------------------------------------------------------------------- */
void alloc_init(size_t base, size_t length)
{
  struct SMK_HeapArea *area = &gArea;
  if (gHeapArea.count_ != 0) 
    area = malloc_(sizeof(struct SMK_HeapArea));

  meminit_r(area, base, length);
  ll_push_back(&gHeapArea, & area->node_);
}


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
