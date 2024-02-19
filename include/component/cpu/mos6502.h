#ifndef __INCLUDE_COMPONENT_CPU_MOS6502_H__
#define __INCLUDE_COMPONENT_CPU_MOS6502_H__

#include <stdbool.h>
#include <stdint.h>

#include "component/bus/bus.h"
#include "component/cpu/cpu.h"

struct ts_mos6502 {
    struct ts_cpu m_cpu;
    struct ts_bus *m_bus;
    uint8_t m_regA;
    uint8_t m_regX;
    uint8_t m_regY;
    uint8_t m_regSP;
    uint16_t m_regPC;
    bool m_flagN;
    bool m_flagV;
    bool m_flagD;
    bool m_flagI;
    bool m_flagZ;
    bool m_flagC;
    bool m_flagNMI;
    bool m_flagIRQ;
    bool m_pendingNMI;
};

void mos6502_init(struct ts_mos6502 *p_mos6502, struct ts_bus *p_bus);
void mos6502_reset(struct ts_mos6502 *p_mos6502);
void mos6502_setNMI(struct ts_mos6502 *p_mos6502, bool p_nmi);
void mos6502_setIRQ(struct ts_mos6502 *p_mos6502, bool p_irq);

#endif
