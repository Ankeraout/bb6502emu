#ifndef __INCLUDE_COMPONENT_BUS_BUS_H__
#define __INCLUDE_COMPONENT_BUS_BUS_H__

#include <stdint.h>

struct ts_bus;

typedef uint16_t t_busAddress;
typedef uint16_t t_busSize;

struct ts_bus {
    uint8_t (*m_read8)(struct ts_bus *p_bus, t_busAddress p_address);
    void (*m_write8)(
        struct ts_bus *p_bus,
        t_busAddress p_address,
        uint8_t p_data
    );
    void (*m_cycle)(struct ts_bus *p_bus);
};

static inline uint8_t busRead8(struct ts_bus *p_bus, t_busAddress p_address) {
    return p_bus->m_read8(p_bus, p_address);
}

static inline void busWrite8(
    struct ts_bus *p_bus,
    t_busAddress p_address,
    uint8_t p_data
) {
    p_bus->m_write8(p_bus, p_address, p_data);
}

static inline void busCycle(struct ts_bus *p_bus) {
    p_bus->m_cycle(p_bus);
}

#endif
