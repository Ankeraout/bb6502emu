#ifndef __COMPONENT_SERIAL_MOS6551_H__
#define __COMPONENT_SERIAL_MOS6551_H__

#include <stdint.h>

#include "component/bus/bus.h"
#include "component/serial/serial.h"

struct ts_mos6551 {
    struct ts_serial m_serial;

    // MOS6551 registers
    uint8_t m_regTxBuffer;
    uint8_t m_regRxBuffer;
    uint8_t m_regStatus;
    uint8_t m_regCommand;
    uint8_t m_regControl;
};

int mos6551_init(struct ts_mos6551 *p_mos6551);
t_busData mos6551_read(struct ts_mos6551 *p_mos6551, t_busAddress p_address);
void mos6551_write(
    struct ts_mos6551 *p_mos6551,
    t_busAddress p_address,
    t_busData p_data
);

#endif
