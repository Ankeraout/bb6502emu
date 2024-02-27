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
    struct ts_ram32k *p_ram,
    struct ts_mos6551 *p_serial
) {
    p_bus->m_bus.m_cycle = busBB6502_cycle;
    p_bus->m_bus.m_read = busBB6502_read;
    p_bus->m_bus.m_write = busBB6502_write;

    p_bus->m_rom = p_rom;
    p_bus->m_ram = p_ram;
    p_bus->m_serial = p_serial;
}

static void busBB6502_cycle(struct ts_bus *p_bus) {
    struct ts_busBB6502 *l_bus = (struct ts_busBB6502 *)p_bus;

    mos6551_cycle(l_bus->m_serial);
}

static t_busData busBB6502_read(struct ts_bus *p_bus, t_busAddress p_address) {
    struct ts_busBB6502 *l_bus = (struct ts_busBB6502 *)p_bus;

    uint8_t l_returnValue = 0xff;
    
    if((p_address & (1 << 15)) == 0) {
        if((p_address & (1 << 14)) == 0) {
            l_returnValue = ram32k_read(l_bus->m_ram, p_address);
        } else if((p_address & 0xfffc) == 0x5000) {
            l_returnValue = mos6551_read(l_bus->m_serial, p_address);
        }
    } else {
        l_returnValue = rom32k_read(l_bus->m_rom, p_address);
    }

    busBB6502_cycle(p_bus);

    return l_returnValue;
}

static void busBB6502_write(
    struct ts_bus *p_bus,
    t_busAddress p_address,
    t_busData p_data
) {
    struct ts_busBB6502 *l_bus = (struct ts_busBB6502 *)p_bus;

    if((p_address & (1 << 15)) == 0) {
        if((p_address & (1 << 14)) == 0) {
            ram32k_write(l_bus->m_ram, p_address, p_data);
        } else if((p_address & 0xfffc) == 0x5000) {
            mos6551_write(l_bus->m_serial, p_address, p_data);
        }
    }

    busBB6502_cycle(p_bus);
}
