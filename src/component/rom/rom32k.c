#include "component/rom/rom32k.h"

#include <string.h>

void rom32k_init(struct ts_rom32k *p_rom, const void *p_buffer, size_t p_size) {
    size_t l_copySize = 32768;

    if(p_size < l_copySize) {
        l_copySize = p_size;
    }

    memcpy(p_rom->m_data, p_buffer, l_copySize);
    memset(&p_rom->m_data[l_copySize], 0, 32768 - l_copySize);
}

t_busData rom32k_read(struct ts_rom32k *p_rom, t_busAddress p_address) {
    return p_rom->m_data[p_address & 0x7fff];
}
