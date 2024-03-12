#include "component/ram/ram32k.h"

#include <string.h>

void ram32k_init(struct ts_ram32k *p_ram) {
    memset(p_ram->m_data, 0, 32768);
}

uint8_t ram32k_read8(struct ts_ram32k *p_ram, t_busAddress p_address) {
    return p_ram->m_data[p_address & 0x7fff];
}

void ram32k_write8(
    struct ts_ram32k *p_ram,
    t_busAddress p_address,
    uint8_t p_data
) {
    p_ram->m_data[p_address & 0x7fff] = p_data;
}
