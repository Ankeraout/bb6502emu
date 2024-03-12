#ifndef __INCLUDE_COMPONENT_RAM_RAM32K_H__
#define __INCLUDE_COMPONENT_RAM_RAM32K_H__

#include <stddef.h>
#include <stdint.h>

#include "component/bus/bus.h"

struct ts_ram32k {
    uint8_t m_data[32768];
};

void ram32k_init(struct ts_ram32k *p_ram);
uint8_t ram32k_read8(struct ts_ram32k *p_ram, t_busAddress p_address);
void ram32k_write8(
    struct ts_ram32k *p_ram,
    t_busAddress p_address,
    uint8_t p_data
);

#endif
