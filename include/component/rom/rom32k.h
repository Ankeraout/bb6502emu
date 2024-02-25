#ifndef __INCLUDE_COMPONENT_ROM_ROM32K_H__
#define __INCLUDE_COMPONENT_ROM_ROM32K_H__

#include <stddef.h>
#include <stdint.h>

#include "component/bus/bus.h"

struct ts_rom32k {
    uint8_t m_data[32768];
};

void rom32k_init(struct ts_rom32k *p_rom, const void *p_buffer, size_t p_size);
t_busData rom32k_read(struct ts_rom32k *p_rom, t_busAddress p_address);

#endif
