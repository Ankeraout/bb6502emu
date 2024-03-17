#include <stdio.h>
#include <string.h>

#include "component/serial/serial.h"

static void *serialThreadStdioRx(void *p_arg);
static void *serialThreadStdioTx(void *p_arg);
static void *serialThreadNullTx(void *p_arg);
static int serialStartNull(struct ts_serial *p_serial);
static int serialStartStdio(struct ts_serial *p_serial);

int serialInit(struct ts_serial *p_serial, const char *p_mode) {
    pthread_mutexattr_t l_mutexAttr;

    int l_returnValue = pthread_mutexattr_init(&l_mutexAttr);
    pthread_mutexattr_settype(&l_mutexAttr, PTHREAD_MUTEX_RECURSIVE_NP);

    l_returnValue = pthread_mutex_init(&p_serial->m_mutex, &l_mutexAttr);

    pthread_mutexattr_destroy(&l_mutexAttr);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    l_returnValue = pthread_cond_init(&p_serial->m_txFullCondition, NULL);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    p_serial->m_rxBuffer = C_SERIAL_FLAG_NODATA;
    p_serial->m_txBuffer = C_SERIAL_FLAG_NODATA;

    if(p_mode == NULL) {
        return serialStartNull(p_serial);
    } else if(strcmp(p_mode, "stdio") == 0) {
        return serialStartStdio(p_serial);
    }

    pthread_mutex_destroy(&p_serial->m_mutex);

    return -1;
}

bool serialIsRxFull(struct ts_serial *p_serial) {
    return (p_serial->m_rxBuffer & C_SERIAL_FLAG_NODATA) == 0;
}

bool serialIsTxFull(struct ts_serial *p_serial) {
    return (p_serial->m_txBuffer & C_SERIAL_FLAG_NODATA) == 0;
}

int serialReadRx(struct ts_serial *p_serial) {
    pthread_mutex_lock(&p_serial->m_mutex);
    int l_returnValue = p_serial->m_rxBuffer;
    p_serial->m_rxBuffer = 0x80000000;
    pthread_mutex_unlock(&p_serial->m_mutex);

    return l_returnValue;
}

int serialReadTx(struct ts_serial *p_serial) {
    pthread_mutex_lock(&p_serial->m_mutex);
    int l_returnValue = p_serial->m_txBuffer;
    p_serial->m_txBuffer = 0x80000000;
    pthread_mutex_unlock(&p_serial->m_mutex);

    return l_returnValue;
}

int serialWriteRx(struct ts_serial *p_serial, uint8_t p_data) {
    int l_returnValue = 0;

    pthread_mutex_lock(&p_serial->m_mutex);

    if((p_serial->m_rxBuffer & C_SERIAL_FLAG_NODATA) != 0) {
        p_serial->m_rxBuffer = p_data;
    } else {
        p_serial->m_rxBuffer |= C_SERIAL_FLAG_OVERRUN;
        l_returnValue = 1;
        printf("serial: rx overrun (%c)\n", p_data);
    }

    pthread_mutex_unlock(&p_serial->m_mutex);
    pthread_cond_signal(&p_serial->m_txFullCondition);

    return l_returnValue;
}

int serialWriteTx(struct ts_serial *p_serial, uint8_t p_data) {
    int l_returnValue = 0;

    pthread_mutex_lock(&p_serial->m_mutex);

    if((p_serial->m_txBuffer & C_SERIAL_FLAG_NODATA) != 0) {
        p_serial->m_txBuffer = p_data;
    } else {
        p_serial->m_txBuffer |= C_SERIAL_FLAG_OVERRUN;
        l_returnValue = 1;
    }

    pthread_mutex_unlock(&p_serial->m_mutex);

    return l_returnValue;
}

static void *serialThreadStdioTx(void *p_arg) {
    struct ts_serial *l_serial = (struct ts_serial *)p_arg;
    
    pthread_mutex_lock(&l_serial->m_mutex);

    while(true) {
        if((l_serial->m_txBuffer & C_SERIAL_FLAG_NODATA) != 0) {
            pthread_cond_wait(&l_serial->m_txFullCondition, &l_serial->m_mutex);
        }

        putchar((uint8_t)serialReadTx(l_serial));

        fflush(stdout);
    }

    pthread_mutex_unlock(&l_serial->m_mutex);

    return NULL;
}

static void *serialThreadNullTx(void *p_arg) {
    struct ts_serial *l_serial = (struct ts_serial *)p_arg;
    
    pthread_mutex_lock(&l_serial->m_mutex);

    while(true) {
        if((l_serial->m_rxBuffer & C_SERIAL_FLAG_NODATA) != 0) {
            pthread_cond_wait(&l_serial->m_txFullCondition, &l_serial->m_mutex);
        }

        serialReadRx(l_serial);
    }

    pthread_mutex_unlock(&l_serial->m_mutex);

    return NULL;
}

static void *serialThreadStdioRx(void *p_arg) {
    struct ts_serial *l_serial = (struct ts_serial *)p_arg;

    while(true) {
        int l_value = getchar();

        if(l_value < 0) {
            return NULL;
        }

        pthread_mutex_lock(&l_serial->m_mutex);
        serialWriteRx(l_serial, l_value);
        pthread_mutex_unlock(&l_serial->m_mutex);
    }

    return NULL;
}

static int serialStartNull(struct ts_serial *p_serial) {
    pthread_t l_thread;

    return pthread_create(&l_thread, NULL, serialThreadNullTx, p_serial);
}

static int serialStartStdio(struct ts_serial *p_serial) {
    pthread_t l_thread;
    
    int l_returnValue =
        pthread_create(&l_thread, NULL, serialThreadStdioRx, p_serial);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    return pthread_create(&l_thread, NULL, serialThreadStdioTx, p_serial);
}
