#include <string.h>

#include "common.h"
#include "component/cpu/nmos6502.h"

static void nmos6502_step(struct ts_cpu *p_cpu);
static void nmos6502_reset(struct ts_cpu *p_cpu);
static inline uint8_t nmos6502_fetch8(struct ts_nmos6502 *p_cpu);
static inline uint16_t nmos6502_fetch16(struct ts_nmos6502 *p_cpu);

void nmos6502_init(struct ts_nmos6502 *p_nmos6502, struct ts_bus *p_bus) {
    memset(p_nmos6502, 0, sizeof(struct ts_nmos6502));

    p_nmos6502->m_bus = p_bus;
    p_nmos6502->m_cpu.m_step = nmos6502_step;
    p_nmos6502->m_cpu.m_reset = nmos6502_reset;

    nmos6502_reset(&p_nmos6502->m_cpu);
}

void nmos6502_setNMI(struct ts_nmos6502 *p_nmos6502, bool p_nmi) {
    p_nmos6502->m_pendingNMI |= !p_nmos6502->m_flagNMI && p_nmi;
    p_nmos6502->m_flagNMI = p_nmi;
}

void nmos6502_setIRQ(struct ts_nmos6502 *p_nmos6502, bool p_irq) {
    p_nmos6502->m_flagIRQ = p_irq;
}

static void nmos6502_step(struct ts_cpu *p_cpu) {
    M_UNUSED_PARAMETER(p_cpu);
}

static void nmos6502_reset(struct ts_cpu *p_cpu) {
    struct ts_nmos6502 *l_nmos6502 = (struct ts_nmos6502 *)p_cpu;

    l_nmos6502->m_regSP = 0xff;
    l_nmos6502->m_regPC = l_nmos6502->m_bus->m_read(l_nmos6502->m_bus, 0x01fd);
    l_nmos6502->m_regPC |=
        l_nmos6502->m_bus->m_read(l_nmos6502->m_bus, 0x1fc) << 8;
    l_nmos6502->m_flagI = true;
    l_nmos6502->m_flagD = false;
}

static inline uint8_t nmos6502_fetch8(struct ts_nmos6502 *p_cpu) {
    return p_cpu->m_bus->m_read(p_cpu->m_bus, p_cpu->m_regPC++);
}

static inline uint16_t nmos6502_fetch16(struct ts_nmos6502 *p_cpu) {
    uint8_t l_low = nmos6502_fetch8(p_cpu);
    uint8_t l_high = nmos6502_fetch8(p_cpu);

    return (l_high << 8) | l_low;
}
