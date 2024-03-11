#ifndef __INCLUDE_CORE_CPU_H__
#define __INCLUDE_CORE_CPU_H__

#include <stdbool.h>
#include <stdint.h>

#include "component/bus/bus.h"
#include "component/cpu/cpu.h"

struct ts_lr35902 {
    struct ts_cpu m_cpu;
    struct ts_bus *m_bus;
    uint8_t m_regA;
    uint8_t m_regB;
    uint8_t m_regC;
    uint8_t m_regD;
    uint8_t m_regE;
    uint8_t m_regH;
    uint8_t m_regL;
    uint16_t m_regSP;
    uint16_t m_regPC;
    bool m_flagZ;
    bool m_flagN;
    bool m_flagH;
    bool m_flagC;
    bool m_interruptsEnabled;
};

void lr35902_init(struct ts_lr35902 *p_lr35902, struct ts_bus *p_bus);

#endif
