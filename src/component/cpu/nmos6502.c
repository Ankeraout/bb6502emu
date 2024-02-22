#include <string.h>

#include "common.h"
#include "component/cpu/nmos6502.h"

static void nmos6502_step(struct ts_cpu *p_cpu);
static void nmos6502_reset(struct ts_cpu *p_cpu);
static inline uint8_t nmos6502_fetch8(struct ts_nmos6502 *p_cpu);
static inline uint16_t nmos6502_fetch16(struct ts_nmos6502 *p_cpu);
static inline void nmos6502_push8(struct ts_nmos6502 *p_cpu, uint8_t p_value);
static inline uint8_t nmos6502_pop8(struct ts_nmos6502 *p_cpu);
static inline void nmos6502_push16(struct ts_nmos6502 *p_cpu, uint16_t p_value);
static inline uint16_t nmos6502_pop16(struct ts_nmos6502 *p_cpu);
static inline uint16_t nmos6502_read16(
    struct ts_nmos6502 *p_cpu,
    uint16_t p_address
);
static inline void nmos6502_interrupt(
    struct ts_nmos6502 *p_cpu,
    uint16_t p_fetchVector,
    bool p_flagB
);
static inline void nmos6502_checkInterrupts(struct ts_nmos6502 *p_cpu);
static inline void nmos6502_setFlagsLogical(
    struct ts_nmos6502 *p_cpu,
    uint8_t p_value
);
static inline uint8_t nmos6502_getP(struct ts_nmos6502 *p_cpu);
static inline void nmos6502_setP(struct ts_nmos6502 *p_cpu, uint8_t p_value);
static inline uint16_t nmos6502_getAddressXIndexedIndirect(
    struct ts_nmos6502 *p_cpu
);
static inline uint16_t nmos6502_getAddressZeroPage(struct ts_nmos6502 *p_cpu);
static inline uint16_t nmos6502_getAddressAbsolute(struct ts_nmos6502 *p_cpu);
static inline void nmos6502_opcodeOra(
    struct ts_nmos6502 *p_cpu,
    uint8_t p_operand
);
static inline uint8_t nmos6502_opcodeAsl(
    struct ts_nmos6502 *p_cpu,
    uint8_t p_operand
);

void nmos6502_init(struct ts_nmos6502 *p_nmos6502, struct ts_bus *p_bus) {
    memset(p_nmos6502, 0, sizeof(struct ts_nmos6502));

    p_nmos6502->m_bus = p_bus;
    p_nmos6502->m_cpu.m_step = nmos6502_step;
    p_nmos6502->m_cpu.m_reset = nmos6502_reset;

    nmos6502_reset(&p_nmos6502->m_cpu);
}

void nmos6502_setNMI(struct ts_nmos6502 *p_nmos6502, bool p_nmi) {
    p_nmos6502->m_pendingNMI |= !p_nmos6502->m_flagNMI && p_nmi;
    p_nmos6502->m_flagNMI |= p_nmi;
}

void nmos6502_setIRQ(struct ts_nmos6502 *p_nmos6502, bool p_irq) {
    p_nmos6502->m_flagIRQ = p_irq;
}

static void nmos6502_step(struct ts_cpu *p_cpu) {
    M_UNUSED_PARAMETER(p_cpu);

    struct ts_nmos6502 *l_cpu = (struct ts_nmos6502 *)p_cpu;

    uint8_t l_opcode = nmos6502_fetch8(l_cpu);

    uint16_t l_tmpAddress;
    uint8_t l_tmpData;

    switch(l_opcode) {
        case 0x00: // BRK
            nmos6502_interrupt(l_cpu, 0xfffe, true);
            break;

        case 0x01: // ORA X-indexed, indirect
            nmos6502_opcodeOra(
                l_cpu,
                nmos6502_getAddressXIndexedIndirect(l_cpu)
            );
            break;

        case 0x05: // ORA Zero-page
            nmos6502_opcodeOra(l_cpu, nmos6502_getAddressZeroPage(l_cpu));
            break;

        case 0x06: // ASL Zero-page
            l_tmpAddress = nmos6502_getAddressZeroPage(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = nmos6502_opcodeAsl(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x08: // PHP Implied
            nmos6502_push8(l_cpu, nmos6502_getP(l_cpu));
            break;

        case 0x09: // ORA Immediate
            nmos6502_opcodeOra(l_cpu, nmos6502_fetch8(l_cpu));
            break;

        case 0x0a: // ASL A
            l_cpu->m_regA = nmos6502_opcodeAsl(l_cpu, l_cpu->m_regA);
            break;

        case 0x0d: // ORA Absolute
            l_tmpAddress = nmos6502_getAddressAbsolute(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = nmos6502_opcodeAsl(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x0e: // ASL Absolute
            l_tmpAddress = nmos6502_getAddressAbsolute(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = nmos6502_opcodeAsl(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;
    }
}

static void nmos6502_reset(struct ts_cpu *p_cpu) {
    struct ts_nmos6502 *l_nmos6502 = (struct ts_nmos6502 *)p_cpu;

    l_nmos6502->m_regSP = 0xff;
    l_nmos6502->m_regPC = nmos6502_read16(l_nmos6502, 0xfffc);
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

static inline void nmos6502_push8(struct ts_nmos6502 *p_cpu, uint8_t p_value) {
    busWrite(p_cpu->m_bus, p_cpu->m_regSP-- | 0x100, p_value);
}

static inline uint8_t nmos6502_pop8(struct ts_nmos6502 *p_cpu) {
    return busRead(p_cpu->m_bus, ++p_cpu->m_regSP | 0x100);
}

static inline void nmos6502_push16(struct ts_nmos6502 *p_cpu, uint16_t p_value) {
    nmos6502_push8(p_cpu, p_value >> 8);
    nmos6502_push8(p_cpu, p_value);
}

static inline uint16_t nmos6502_pop16(struct ts_nmos6502 *p_cpu) {
    uint8_t l_low = nmos6502_pop8(p_cpu);
    uint8_t l_high = nmos6502_pop8(p_cpu);

    return (l_high << 8) | l_low;
}

static inline uint16_t nmos6502_read16(
    struct ts_nmos6502 *p_cpu,
    uint16_t p_address
) {
    uint8_t l_low = busRead(p_cpu->m_bus, p_address);
    uint8_t l_high = busRead(p_cpu->m_bus, p_address + 1);

    return (l_high << 8) | l_low;
}

static inline void nmos6502_interrupt(
    struct ts_nmos6502 *p_cpu,
    uint16_t p_fetchVector,
    bool p_flagB
) {
    nmos6502_push16(p_cpu, p_cpu->m_regPC);
    nmos6502_push8(p_cpu, nmos6502_getP(p_cpu) & (p_flagB ? 0xff : 0xef));

    // Load vector into PC
    p_cpu->m_regPC = nmos6502_read16(p_cpu, p_fetchVector);

    // Disable interrupts
    p_cpu->m_flagI = true;
}

static inline void nmos6502_checkInterrupts(struct ts_nmos6502 *p_cpu) {
    if(p_cpu->m_pendingNMI) {
        nmos6502_interrupt(p_cpu, 0xfffa, false);
    } else if(!p_cpu->m_flagI && p_cpu->m_flagIRQ) {
        nmos6502_interrupt(p_cpu, 0xfffe, false);
    }
}

static inline void nmos6502_setFlagsLogical(
    struct ts_nmos6502 *p_cpu,
    uint8_t p_value
) {
    p_cpu->m_flagN = (p_value & (1 << 7)) != 0;
    p_cpu->m_flagZ = p_value == 0;
}

static inline uint8_t nmos6502_getP(struct ts_nmos6502 *p_cpu) {
    uint8_t l_p = 0x30
        | (p_cpu->m_flagN ? 1 << 7 : 0)
        | (p_cpu->m_flagV ? 1 << 6 : 0)
        | (p_cpu->m_flagD ? 1 << 3 : 0)
        | (p_cpu->m_flagI ? 1 << 2 : 0)
        | (p_cpu->m_flagZ ? 1 << 1 : 0)
        | (p_cpu->m_flagC ? 1 << 0 : 0);

    return l_p;
}

static inline void nmos6502_setP(struct ts_nmos6502 *p_cpu, uint8_t p_value) {
    p_cpu->m_flagN = (p_value & (1 << 7)) != 0;
    p_cpu->m_flagV = (p_value & (1 << 6)) != 0;
    p_cpu->m_flagD = (p_value & (1 << 3)) != 0;
    p_cpu->m_flagI = (p_value & (1 << 2)) != 0;
    p_cpu->m_flagZ = (p_value & (1 << 1)) != 0;
    p_cpu->m_flagC = (p_value & (1 << 0)) != 0;
}

static inline uint16_t nmos6502_getAddressXIndexedIndirect(
    struct ts_nmos6502 *p_cpu
) {
    return nmos6502_read16(p_cpu, nmos6502_fetch8(p_cpu) + p_cpu->m_regX);
}

static inline uint16_t nmos6502_getAddressZeroPage(struct ts_nmos6502 *p_cpu) {
    return nmos6502_fetch8(p_cpu);
}

static inline uint16_t nmos6502_getAddressAbsolute(struct ts_nmos6502 *p_cpu) {
    return nmos6502_fetch16(p_cpu);
}

static inline void nmos6502_opcodeOra(
    struct ts_nmos6502 *p_cpu,
    uint8_t p_operand
) {
    p_cpu->m_regA |= p_operand;
    nmos6502_setFlagsLogical(p_cpu, p_cpu->m_regA);
}

static inline uint8_t nmos6502_opcodeAsl(
    struct ts_nmos6502 *p_cpu,
    uint8_t p_operand
) {
    p_cpu->m_flagC = (p_operand & (1 << 7)) != 0;
    p_operand <<= 1;
    nmos6502_setFlagsLogical(p_cpu, p_operand);

    return p_operand;
}
