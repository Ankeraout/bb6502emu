#include "component/bus/busBB6502.h"

static void busBB6502_cycle(struct ts_bus *p_bus);
static t_busData busBB6502_read(struct ts_bus *p_bus, t_busAddress p_address);
static void busBB6502_write(
    struct ts_bus *p_bus,
    t_busAddress p_address,
    t_busData p_data
);

void busBB6502_init(
    struct ts_busBB6502 *p_bus,
    struct ts_rom32k *p_rom,
    struct ts_ram32k *p_ram
) {
    p_bus->m_bus.m_cycle = busBB6502_cycle;
    p_bus->m_bus.m_read = busBB6502_read;
    p_bus->m_bus.m_write = busBB6502_write;

    p_bus->m_rom = p_rom;
    p_bus->m_ram = p_ram;
}

static void busBB6502_cycle(struct ts_bus *p_bus) {
    // Do nothing (yet)
    ((void)p_bus);
}


static t_busData busBB6502_read(struct ts_bus *p_bus, t_busAddress p_address) {
    struct ts_busBB6502 *l_bus = (struct ts_busBB6502 *)p_bus;
    
    if((p_address & (1 << 15)) == 0) {
        return ram32k_read(l_bus->m_ram, p_address);
    } else {
        return rom32k_read(l_bus->m_rom, p_address);
    }
}

static void busBB6502_write(
    struct ts_bus *p_bus,
    t_busAddress p_address,
    t_busData p_data
) {
    struct ts_busBB6502 *l_bus = (struct ts_busBB6502 *)p_bus;

    if((p_address & (1 << 15)) == 0) {
        ram32k_write(l_bus->m_ram, p_address, p_data);
    }
}
