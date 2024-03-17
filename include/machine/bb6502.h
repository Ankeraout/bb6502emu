#ifndef __INCLUDE_MACHINE_BB6502_H__
#define __INCLUDE_MACHINE_BB6502_H__

#include "commandline.h"
#include "component/bus/busBB6502.h"
#include "component/cpu/nmos6502.h"
#include "component/ram/ram32k.h"
#include "component/rom/rom32k.h"
#include "component/serial/mos6551.h"
#include "machine/machine.h"

struct ts_bb6502 {
    struct ts_machine m_machine;
    struct ts_busBB6502 m_bus;
    struct ts_nmos6502 m_cpu;
    struct ts_ram32k m_ram;
    struct ts_rom32k m_rom;
    struct ts_mos6551 m_mos6551;
    struct ts_serial m_serial;
};

struct ts_bb6502 *bb6502_init(
    struct ts_commandLineOptions *p_commandLineOptions
);

#endif
