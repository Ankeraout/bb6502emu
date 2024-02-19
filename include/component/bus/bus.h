#ifndef __INCLUDE_COMPONENT_BUS_BUS_H__
#define __INCLUDE_COMPONENT_BUS_BUS_H__

#include <stdint.h>

struct ts_bus;

typedef uint16_t t_busAddress;
typedef uint16_t t_busSize;
typedef uint8_t t_busData;

struct ts_bus {
    t_busData (*m_read)(struct ts_bus *p_bus, t_busAddress p_address);
    void (*m_write)(
        struct ts_bus *p_bus,
        t_busAddress p_address,
        t_busData p_data
    );
    void (*m_cycle)(struct ts_bus *p_bus);
};

static inline t_busData busRead(struct ts_bus *p_bus, t_busAddress p_address) {
    return p_bus->m_read(p_bus, p_address);
}

static inline void busWrite(
    struct ts_bus *p_bus,
    t_busAddress p_address,
    t_busData p_data
) {
    p_bus->m_write(p_bus, p_address, p_data);
}

static inline void busCycle(struct ts_bus *p_bus) {
    p_bus->m_cycle(p_bus);
}

#endif
