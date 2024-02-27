#include <errno.h>
#include <stdlib.h>

#include "structures/fifo.h"

int fifo_init(struct ts_fifo *p_fifo, size_t p_size) {
    p_fifo->m_buffer = malloc(sizeof(void *) * p_size);

    if(p_fifo->m_buffer == NULL) {
        return -ENOMEM;
    }

    int l_returnValue = pthread_cond_init(&p_fifo->m_readCondition, NULL);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    l_returnValue = pthread_cond_init(&p_fifo->m_writeCondition, NULL);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    l_returnValue = pthread_mutex_init(&p_fifo->m_mutex, NULL);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    p_fifo->m_readIndex = 0;
    p_fifo->m_writeIndex = 0;
    p_fifo->m_size = p_size;
    p_fifo->m_count = 0;

    return 0;
}

int fifo_read(struct ts_fifo *p_fifo, void **p_buffer) {
    int l_returnValue = pthread_mutex_lock(&p_fifo->m_mutex);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    if(p_fifo->m_count == 0) {
        l_returnValue =
            pthread_cond_wait(&p_fifo->m_readCondition, &p_fifo->m_mutex);

        if(l_returnValue != 0) {
            return l_returnValue;
        }
    }

    *p_buffer = p_fifo->m_buffer[p_fifo->m_readIndex++];

    if(p_fifo->m_readIndex == p_fifo->m_size) {
        p_fifo->m_readIndex = 0;
    }

    p_fifo->m_count--;

    pthread_mutex_unlock(&p_fifo->m_mutex);

    pthread_cond_signal(&p_fifo->m_writeCondition);

    return 0;
}

int fifo_write(struct ts_fifo *p_fifo, void *p_element) {
    int l_returnValue = pthread_mutex_lock(&p_fifo->m_mutex);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    if(p_fifo->m_count == p_fifo->m_size) {
        l_returnValue =
            pthread_cond_wait(&p_fifo->m_writeCondition, &p_fifo->m_mutex);

        if(l_returnValue != 0) {
            return l_returnValue;
        }
    }

    p_fifo->m_buffer[p_fifo->m_writeIndex++] = p_element;

    if(p_fifo->m_writeIndex == p_fifo->m_size) {
        p_fifo->m_writeIndex = 0;
    }

    p_fifo->m_count++;

    pthread_mutex_unlock(&p_fifo->m_mutex);

    pthread_cond_signal(&p_fifo->m_readCondition);

    return 0;
}

int fifo_tryRead(struct ts_fifo *p_fifo, void **p_buffer) {
    int l_returnValue = pthread_mutex_lock(&p_fifo->m_mutex);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    if(p_fifo->m_count == 0) {
        pthread_mutex_unlock(&p_fifo->m_mutex);
        return 0;
    }

    *p_buffer = p_fifo->m_buffer[p_fifo->m_readIndex++];

    if(p_fifo->m_readIndex == p_fifo->m_size) {
        p_fifo->m_readIndex = 0;
    }

    p_fifo->m_count--;

    pthread_mutex_unlock(&p_fifo->m_mutex);

    pthread_cond_signal(&p_fifo->m_writeCondition);

    return 1;
}

int fifo_tryWrite(struct ts_fifo *p_fifo, void *p_element) {
    int l_returnValue = pthread_mutex_lock(&p_fifo->m_mutex);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    if(p_fifo->m_count == p_fifo->m_size) {
        pthread_mutex_unlock(&p_fifo->m_mutex);
        return 0;
    }

    p_fifo->m_buffer[p_fifo->m_writeIndex++] = p_element;

    if(p_fifo->m_writeIndex == p_fifo->m_size) {
        p_fifo->m_writeIndex = 0;
    }

    p_fifo->m_count++;

    pthread_mutex_unlock(&p_fifo->m_mutex);

    pthread_cond_signal(&p_fifo->m_readCondition);

    return 1;
}

size_t fifo_getCount(struct ts_fifo *p_fifo) {
    return p_fifo->m_count;
}
