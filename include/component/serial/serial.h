#ifndef __INCLUDE_COMPONENT_SERIAL_SERIAL_H__
#define __INCLUDE_COMPONENT_SERIAL_SERIAL_H__

#include <stddef.h>
#include <stdint.h>

struct ts_serial {
    int (*m_read)(struct ts_serial *p_serial, uint8_t *p_buffer, size_t p_size);
    int (*m_write)(
        struct ts_serial *p_serial,
        const uint8_t *p_buffer,
        size_t p_size
    );
};

#endif
