#ifndef __INCLUDE_COMPONENT_SERIAL_MOS6551_H__
#define __INCLUDE_COMPONENT_SERIAL_MOS6551_H__

#include <pthread.h>

#include "component/bus/bus.h"
#include "component/serial/serial.h"
#include "structures/fifo.h"

struct ts_mos6551 {
    struct ts_serial m_serial;
    struct ts_fifo m_fifoRead;
    struct ts_fifo m_fifoWrite;
    uint8_t m_regRxData;
    uint8_t m_regTxData;
    uint8_t m_regStatus;
    uint8_t m_regControl;
    uint8_t m_regCommand;
};

int mos6551_init(struct ts_mos6551 *p_serial);
t_busData mos6551_read(struct ts_mos6551 *p_serial, t_busAddress p_address);
void mos6551_write(
    struct ts_mos6551 *p_serial,
    t_busAddress p_address,
    t_busData p_data
);
void mos6551_cycle(struct ts_mos6551 *p_serial);

#endif
