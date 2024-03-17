#ifndef __COMPONENT_SERIAL_SERIAL_H__
#define __COMPONENT_SERIAL_SERIAL_H__

#include <stdbool.h>
#include <stdint.h>

#include <pthread.h>

#include "commandline.h"

#define C_SERIAL_FLAG_NODATA 0x80000000
#define C_SERIAL_FLAG_OVERRUN 0x40000000

struct ts_serial {
    pthread_cond_t m_txFullCondition;
    pthread_mutex_t m_mutex;
    int m_rxBuffer;
    int m_txBuffer;
};

int serialInit(struct ts_serial *p_serial, const char *p_mode);
bool serialIsRxFull(struct ts_serial *p_serial);
bool serialIsTxFull(struct ts_serial *p_serial);
int serialReadRx(struct ts_serial *p_serial);
int serialReadTx(struct ts_serial *p_serial);
int serialWriteRx(struct ts_serial *p_serial, uint8_t p_data);
int serialWriteTx(struct ts_serial *p_serial, uint8_t p_data);

#endif
