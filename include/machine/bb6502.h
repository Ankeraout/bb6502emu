#ifndef __INCLUDE_MACHINE_BB6502_H__
#define __INCLUDE_MACHINE_BB6502_H__

#include "component/bus/busBB6502.h"
#include "component/cpu/nmos6502.h"
#include "component/ram/ram32k.h"
#include "component/rom/rom32k.h"
#include "machine/machine.h"

struct ts_bb6502 {
    struct ts_machine m_machine;
    struct ts_busBB6502 m_bus;
    struct ts_nmos6502 m_cpu;
    struct ts_ram32k m_ram;
    struct ts_rom32k m_rom;
};

void bb6502_init(struct ts_bb6502 *p_machine, const void *p_rom, size_t p_size);

#endif
