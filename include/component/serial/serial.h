#ifndef __COMPONENT_SERIAL_SERIAL_H__
#define __COMPONENT_SERIAL_SERIAL_H__

#include <stdbool.h>
#include <stdint.h>

struct ts_serial {
    int (*m_send)(struct ts_serial *p_serial, uint8_t p_data);
    int (*m_receive)(struct ts_serial *p_serial);
};

#endif
