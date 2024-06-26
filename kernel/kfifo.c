#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/kfifo.h>
#include <mm/mm.h>

struct kfifo_hdr {
    uint16_t recsize;
};

void kfifo_init(struct kfifo *fifo, void *data, size_t esize, size_t size)
{
    fifo->start = 0;
    fifo->end = 0;
    fifo->count = 0;
    fifo->data = data;
    fifo->esize = esize;
    fifo->size = size;

    /* Initialize kfifo as byte stream mode if esize is 1 */
    if (fifo->esize > 1) {
        /* Structured content with header (esize > 1) */
        fifo->header_size = sizeof(struct kfifo_hdr);
        fifo->payload_size = sizeof(struct kfifo_hdr) + esize;
    } else {
        /* Byte stream without header (esize == 1) */
        fifo->header_size = 0;
        fifo->payload_size = 1;
    }
}

struct kfifo *kfifo_alloc(size_t esize, size_t size)
{
    /* Allocate new kfifo object */
    struct kfifo *fifo = kmalloc(sizeof(struct kfifo));
    if (!fifo)
        return NULL; /* Allocation failed */
    kfifo_init(fifo, NULL, esize, size);

    /* Allocate buffer space for the kfifo */
    uint8_t *fifo_data = kmalloc(fifo->payload_size * size);
    if (!fifo_data)
        return NULL; /* Allocation failed  */
    fifo->data = fifo_data;

    /* Return the allocated kfifo object */
    return fifo;
}

void kfifo_free(struct kfifo *fifo)
{
    kfree(fifo->data);
    kfree(fifo);
}

static int kfifo_increase(struct kfifo *fifo, int ptr)
{
    ptr++;
    if (ptr >= fifo->size)
        ptr = 0;
    return ptr;
}

void kfifo_in(struct kfifo *fifo, const void *buf, size_t n)
{
    char *data_start;
    char *dest;

    if (kfifo_is_full(fifo)) {
        /* The FIFO is full, overwrite the oldest data and
         * shift the pointer to the next position */
        data_start = (char *) ((uintptr_t) fifo->data +
                               fifo->start * fifo->payload_size);
        dest = (char *) ((uintptr_t) data_start + fifo->header_size);
        memcpy(dest, buf, n);

        /* Update the start position */
        fifo->start = kfifo_increase(fifo, fifo->start);
    } else {
        /* Append data at the end of the FIFO as it still has free space */
        data_start =
            (char *) ((uintptr_t) fifo->data + fifo->end * fifo->payload_size);
        dest = (char *) ((uintptr_t) data_start + fifo->header_size);
        memcpy(dest, buf, n);

        /* Update the data count */
        fifo->count++;
    }

    /* Write the record size field if the FIFO is configured
     * as structured content mode */
    if (fifo->esize > 1)
        *(uint16_t *) data_start = n; /* recsize field */

    /* Update the end position */
    fifo->end = kfifo_increase(fifo, fifo->end);
}

void kfifo_out(struct kfifo *fifo, void *buf, size_t n)
{
    /* Return if no data to read */
    if (fifo->count <= 0)
        return;

    /* Copy the data from the FIFO */
    char *src = (char *) ((uintptr_t) fifo->data + fifo->header_size +
                          fifo->start * fifo->payload_size);
    memcpy(buf, src, n);

    /* Update FIFO information */
    fifo->start = kfifo_increase(fifo, fifo->start);
    fifo->count--;
}

void kfifo_out_peek(struct kfifo *fifo, void *data, size_t n)
{
    /* Return if no data to read */
    if (fifo->count <= 0)
        return;

    /* Calculate the start address of the next data */
    char *data_start =
        (char *) ((uintptr_t) fifo->data + fifo->start * fifo->payload_size);

    /* Copy the data from the fifo */
    if (fifo->esize > 1) {
        /* Structured content mode */
        char *src =
            (char *) ((uintptr_t) data_start + sizeof(struct kfifo_hdr));
        memcpy(data, src, n);
    } else {
        /* Byte stream mode */
        *(char *) data = *data_start;
    }
}

void kfifo_dma_in_prepare(struct kfifo *fifo, char **data_ptr)
{
    if (kfifo_is_full(fifo)) {
        /* FIFO is full, return the address of the oldest data to overwrite */
        *data_ptr = (char *) ((uintptr_t) fifo->data + fifo->header_size +
                              fifo->start * fifo->payload_size);
    } else {
        /* Return the next free space of the fifo */
        *data_ptr = (char *) ((uintptr_t) fifo->data + fifo->header_size +
                              fifo->end * fifo->payload_size);
    }
}

void kfifo_dma_in_finish(struct kfifo *fifo, size_t n)
{
    if (kfifo_is_full(fifo)) {
        /* Update the start pointer as the oldest data is overwritten */
        fifo->start = kfifo_increase(fifo, fifo->start);
    } else {
        /* Increase the element count in the FIFO */
        fifo->count++;
    }

    /* Update the record size */
    uint16_t *recsize = (uint16_t *) ((uintptr_t) fifo->data +
                                      fifo->start * fifo->payload_size);
    *recsize = n;

    /* Update as the data is written via DMA */
    fifo->end = kfifo_increase(fifo, fifo->end);
}

void kfifo_dma_out_prepare(struct kfifo *fifo, char **data_ptr, size_t *n)
{
    char *data_start =
        (char *) ((uintptr_t) fifo->data + fifo->start * fifo->payload_size);
    uint16_t *recsize = (uint16_t *) data_start;

    /* Return the address and size of the next data to read */
    *data_ptr = (char *) ((uintptr_t) data_start + fifo->header_size);
    *n = *recsize;
}

void kfifo_dma_out_finish(struct kfifo *fifo)
{
    /* Update the information as the data is read via DMA */
    fifo->start = kfifo_increase(fifo, fifo->start);
    fifo->count--;
}

size_t kfifo_peek_len(struct kfifo *fifo)
{
    /* kfifo_peek_len() is not supported under the
     * byte stream mode */
    if (fifo->esize <= 1)
        return 0;

    /* Read and return the recsize */
    uint16_t *recsize = (uint16_t *) ((uintptr_t) fifo->data +
                                      fifo->start * fifo->payload_size);
    return *recsize;
}

void kfifo_put(struct kfifo *fifo, void *data)
{
    kfifo_in(fifo, data, fifo->esize);
}

void kfifo_get(struct kfifo *fifo, void *data)
{
    kfifo_out(fifo, data, fifo->esize);
}

void kfifo_peek(struct kfifo *fifo, void *data)
{
    kfifo_out_peek(fifo, data, fifo->esize);
}

void kfifo_skip(struct kfifo *fifo)
{
    if (fifo->count <= 0)
        return;

    fifo->start = kfifo_increase(fifo, fifo->start);
    fifo->count--;
}

size_t kfifo_avail(struct kfifo *fifo)
{
    return fifo->size - fifo->count;
}

size_t kfifo_len(struct kfifo *fifo)
{
    return fifo->count;
}

size_t kfifo_esize(struct kfifo *fifo)
{
    return fifo->esize;
}

size_t kfifo_size(struct kfifo *fifo)
{
    return fifo->size;
}

size_t kfifo_header_size(void)
{
    return sizeof(struct kfifo_hdr);
}

bool kfifo_is_empty(struct kfifo *fifo)
{
    return fifo->count == 0;
}

bool kfifo_is_full(struct kfifo *fifo)
{
    return fifo->count == fifo->size;
}
