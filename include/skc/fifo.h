#ifndef _SKC_FIFO_H
#define _SKC_FIFO_H 1

#include <cdefs/stddef.h>
#include <assert.h>

#ifndef MIN
# define MIN(a,b) ((a)<(b)?(a):(b))
# define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#define FP_NOBLOCK (1 << 0)
#define FP_EOL (1 << 1)
#define FP_WR (1 << 2)

typedef struct fifo fifo_t;


/* Instanciate a new FIFO */
fifo_t *fifo_init(void *buf, size_t lg);
/* Look for a specific bytes in consumable data */
size_t fifo_indexof(fifo_t *fifo, char ch) ;
/* Read some bytes from the FIFO */
size_t fifo_out(fifo_t *fifo, void *buf, size_t lg, int flags);
/* Write some bytes on the FIFO */
size_t fifo_in(fifo_t *fifo, const void *buf, size_t lg, int flags);
/* Reinitialize the queue */
void fifo_reset(fifo_t *fifo);

#endif  /* _SKC_FIFO_H */
