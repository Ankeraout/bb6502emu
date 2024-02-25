#include "component/ram/ram32k.h"

#include <string.h>

void ram32k_init(struct ts_ram32k *p_ram) {
    memset(p_ram->m_data, 0, 32768);
}

t_busData ram32k_read(struct ts_ram32k *p_ram, t_busAddress p_address) {
    return p_ram->m_data[p_address & 0x7fff];
}

void ram32k_write(
    struct ts_ram32k *p_ram,
    t_busAddress p_address,
    t_busData p_data
) {
    p_ram->m_data[p_address & 0x7fff] = p_data;
}
