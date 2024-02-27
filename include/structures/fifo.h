#ifndef __INCLUDE_STRUCTURES_FIFO_H__
#define __INCLUDE_STRUCTURES_FIFO_H__

#include <pthread.h>
#include <stddef.h>

struct ts_fifo {
    size_t m_readIndex;
    size_t m_writeIndex;
    size_t m_count;
    size_t m_size;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_readCondition;
    pthread_cond_t m_writeCondition;
    void **m_buffer;
};

int fifo_init(struct ts_fifo *p_fifo, size_t p_size);
int fifo_read(struct ts_fifo *p_fifo, void **p_buffer);
int fifo_write(struct ts_fifo *p_fifo, void *p_element);
int fifo_tryRead(struct ts_fifo *p_fifo, void **p_buffer);
int fifo_tryWrite(struct ts_fifo *p_fifo, void *p_element);
size_t fifo_getCount(struct ts_fifo *p_fifo);

#endif
