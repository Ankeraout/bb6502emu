#ifndef __INCLUDE_COMPONENT_BUS_BUSBB6502_H__
#define __INCLUDE_COMPONENT_BUS_BUSBB6502_H__

#include "component/bus/bus.h"
#include "component/ram/ram32k.h"
#include "component/rom/rom32k.h"
#include "component/serial/mos6551.h"

struct ts_busBB6502 {
    struct ts_bus m_bus;
    struct ts_rom32k *m_rom;
    struct ts_ram32k *m_ram;
    struct ts_mos6551 *m_serial;
};

void busBB6502_init(
    struct ts_busBB6502 *p_bus,
    struct ts_rom32k *p_rom,
    struct ts_ram32k *p_ram,
    struct ts_mos6551 *p_serial
);

#endif
