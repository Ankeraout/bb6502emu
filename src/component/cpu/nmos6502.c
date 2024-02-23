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
static inline uint16_t nmos6502_getAddressRelative(struct ts_nmos6502 *p_cpu);
static inline uint16_t nmos6502_getAddressIndirectYIndexed(
    struct ts_nmos6502 *p_cpu
);
static inline uint16_t nmos6502_getAddressZeroPageXIndexed(
    struct ts_nmos6502 *p_cpu
);
static inline uint16_t nmos6502_getAddressAbsoluteYIndexed(
    struct ts_nmos6502 *p_cpu
);
static inline uint16_t nmos6502_getAddressAbsoluteXIndexed(
    struct ts_nmos6502 *p_cpu
);
static inline void nmos6502_opcodeOra(
    struct ts_nmos6502 *p_cpu,
    uint8_t p_operand
);
static inline uint8_t nmos6502_opcodeAsl(
    struct ts_nmos6502 *p_cpu,
    uint8_t p_operand
);
static inline void nmos6502_opcodeBranch(
    struct ts_nmos6502 *p_cpu,
    bool p_condition
);
static inline void nmos6502_opcodeAnd(
    struct ts_nmos6502 *p_cpu,
    uint8_t p_operand
);
static inline void nmos6502_opcodeBit(
    struct ts_nmos6502 *p_cpu,
    uint8_t p_operand
);
static inline uint8_t nmos6502_opcodeRol(
    struct ts_nmos6502 *p_cpu,
    uint8_t p_operand
);
static inline void nmos6502_opcodeEor(
    struct ts_nmos6502 *p_cpu,
    uint8_t p_operand
);
static inline uint8_t nmos6502_opcodeLsr(
    struct ts_nmos6502 *p_cpu,
    uint8_t p_operand
);
static inline void nmos6502_opcodeAdc(
    struct ts_nmos6502 *p_cpu,
    uint8_t p_operand
);
static inline uint8_t nmos6502_opcodeRor(
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
            l_tmpAddress = nmos6502_getAddressZeroPage(l_cpu);
            nmos6502_opcodeOra(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
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
            nmos6502_opcodeOra(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x0e: // ASL Absolute
            l_tmpAddress = nmos6502_getAddressAbsolute(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = nmos6502_opcodeAsl(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x10: // BPL Relative
            nmos6502_opcodeBranch(l_cpu, l_cpu->m_flagN);
            break;

        case 0x11: // ORA Indirect, Y-indexed
            l_tmpAddress = nmos6502_getAddressIndirectYIndexed(l_cpu);
            nmos6502_opcodeOra(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x15: // ORA Zero-page, X-indexed
            l_tmpAddress = nmos6502_getAddressZeroPageXIndexed(l_cpu);
            nmos6502_opcodeOra(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x16: // ASL Zero-page, X-indexed
            l_tmpAddress = nmos6502_getAddressZeroPageXIndexed(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = nmos6502_opcodeAsl(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x18: // CLC Implied
            l_cpu->m_flagC = false;
            break;
        
        case 0x19: // ORA Absolute, Y-indexed
            l_tmpAddress = nmos6502_getAddressAbsoluteYIndexed(l_cpu);
            nmos6502_opcodeOra(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x1d: // ORA Absolute, X-indexed
            l_tmpAddress = nmos6502_getAddressAbsoluteXIndexed(l_cpu);
            nmos6502_opcodeOra(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x1e: // ASL Absolute, X-indexed
            l_tmpAddress = nmos6502_getAddressAbsoluteXIndexed(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = nmos6502_opcodeAsl(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x20: // AND X-indexed, indirect
            l_tmpAddress = nmos6502_getAddressXIndexedIndirect(l_cpu);
            nmos6502_opcodeAnd(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x24: // BIT Zero-page
            l_tmpAddress = nmos6502_getAddressZeroPage(l_cpu);
            nmos6502_opcodeBit(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x25: // AND Zero-page
            l_tmpAddress = nmos6502_getAddressZeroPage(l_cpu);
            nmos6502_opcodeAnd(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x26: // ROL Zero-page
            l_tmpAddress = nmos6502_getAddressZeroPage(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = nmos6502_opcodeRol(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x28: // PLP
            nmos6502_setP(l_cpu, nmos6502_pop8(l_cpu));
            break;

        case 0x29: // AND Immediate
            nmos6502_opcodeAnd(l_cpu, nmos6502_fetch8(l_cpu));
            break;

        case 0x2a: // ROL A
            l_cpu->m_regA = nmos6502_opcodeRol(l_cpu, l_cpu->m_regA);
            break;

        case 0x2c: // BIT Absolute
            l_tmpAddress = nmos6502_getAddressAbsolute(l_cpu);
            nmos6502_opcodeBit(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x2d: // AND Absolute
            l_tmpAddress = nmos6502_getAddressAbsolute(l_cpu);
            nmos6502_opcodeAnd(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x2e: // ROL Absolute
            l_tmpAddress = nmos6502_getAddressAbsolute(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = nmos6502_opcodeRol(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x30: // BMI Relative
            nmos6502_opcodeBranch(l_cpu, l_cpu->m_flagN);
            break;

        case 0x31: // AND Indirect, Y-indexed
            l_tmpAddress = nmos6502_getAddressIndirectYIndexed(l_cpu);
            nmos6502_opcodeAnd(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x35: // AND Zero-page, X-indexed
            l_tmpAddress = nmos6502_getAddressZeroPageXIndexed(l_cpu);
            nmos6502_opcodeAnd(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x36: // ROL Zero-page, X-indexed
            l_tmpAddress = nmos6502_getAddressZeroPageXIndexed(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = nmos6502_opcodeRol(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x38: // SEC Implied
            l_cpu->m_flagC = true;
            break;

        case 0x39: // AND Absolute, Y-indexed
            l_tmpAddress = nmos6502_getAddressAbsoluteYIndexed(l_cpu);
            nmos6502_opcodeAnd(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x3d: // AND Absolute, X-indexed
            l_tmpAddress = nmos6502_getAddressAbsoluteXIndexed(l_cpu);
            nmos6502_opcodeAnd(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x3e: // ROL Absolute, X-indexed
            l_tmpAddress = nmos6502_getAddressAbsoluteXIndexed(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = nmos6502_opcodeRol(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x40: // RTI Implied
            nmos6502_setP(l_cpu, nmos6502_pop8(l_cpu));
            l_cpu->m_regPC = nmos6502_pop16(l_cpu);
            break;

        case 0x41: // EOR X-indexed, indirect
            l_tmpAddress = nmos6502_getAddressXIndexedIndirect(l_cpu);
            nmos6502_opcodeEor(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x45: // EOR Zero-page
            l_tmpAddress = nmos6502_getAddressZeroPage(l_cpu);
            nmos6502_opcodeEor(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x46: // LSR Zero-page
            l_tmpAddress = nmos6502_getAddressZeroPage(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = nmos6502_opcodeLsr(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x48: // PHA Implied
            nmos6502_push8(l_cpu, l_cpu->m_regA);
            break;

        case 0x49: // EOR Immediate
            nmos6502_opcodeEor(l_cpu, nmos6502_fetch8(l_cpu));
            break;

        case 0x4a: // LSR A
            l_cpu->m_regA = nmos6502_opcodeLsr(l_cpu, l_cpu->m_regA);
            break;

        case 0x50: // BVC Relative
            nmos6502_opcodeBranch(l_cpu, !l_cpu->m_flagV);
            break;

        case 0x51: // EOR Indirect, Y-indexed
            l_tmpAddress = nmos6502_getAddressIndirectYIndexed(l_cpu);
            nmos6502_opcodeEor(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x55: // EOR Zero-page, X-indexed
            l_tmpAddress = nmos6502_getAddressZeroPageXIndexed(l_cpu);
            nmos6502_opcodeEor(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x56: // LSR Zero-page, X-indexed
            l_tmpAddress = nmos6502_getAddressZeroPageXIndexed(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = nmos6502_opcodeLsr(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x58: // CLI
            l_cpu->m_flagI = false;
            break;

        case 0x59: // EOR Absolute, Y-indexed
            l_tmpAddress = nmos6502_getAddressAbsoluteYIndexed(l_cpu);
            nmos6502_opcodeEor(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x5d: // EOR Absolute, X-indexed
            l_tmpAddress = nmos6502_getAddressAbsoluteXIndexed(l_cpu);
            nmos6502_opcodeEor(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x5e: // LSR Absolute, X-indexed
            l_tmpAddress = nmos6502_getAddressAbsoluteXIndexed(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = nmos6502_opcodeLsr(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x60: // RTS Implied
            l_cpu->m_regPC = nmos6502_pop16(l_cpu);
            break;

        case 0x61: // ADC X-indexed, indirect
            l_tmpAddress = nmos6502_getAddressXIndexedIndirect(l_cpu);
            nmos6502_opcodeAdc(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x65: // ADC Zero-page
            l_tmpAddress = nmos6502_getAddressZeroPage(l_cpu);
            nmos6502_opcodeAdc(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x66: // ROR Zero-page
            l_tmpAddress = nmos6502_getAddressZeroPage(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = nmos6502_opcodeRor(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x68: // PLA Implied
            l_cpu->m_regA = nmos6502_pop8(l_cpu);
            break;

        case 0x69: // ADC Immediate
            nmos6502_opcodeAdc(l_cpu, nmos6502_fetch8(l_cpu));
            break;

        case 0x6a: // ROR A
            l_cpu->m_regA = nmos6502_opcodeRor(l_cpu, l_cpu->m_regA);
            break;

        case 0x6c: // JMP Indirect
            l_cpu->m_regPC = nmos6502_read16(l_cpu, nmos6502_fetch16(l_cpu));
            break;

        case 0x6d: // ADC Absolute
            l_tmpAddress = nmos6502_getAddressAbsolute(l_cpu);
            nmos6502_opcodeAdc(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x6e: // ROR Absolute
            l_tmpAddress = nmos6502_getAddressZeroPage(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = nmos6502_opcodeRor(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;
        
        case 0x70: // BVS Relative
            nmos6502_opcodeBranch(l_cpu, l_cpu->m_flagV);
            break;

        case 0x71: // ADC Indirect, Y-indexed
            l_tmpAddress = nmos6502_getAddressIndirectYIndexed(l_cpu);
            nmos6502_opcodeAdc(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x75: // ADC Zero-page, X-indexed
            l_tmpAddress = nmos6502_getAddressZeroPageXIndexed(l_cpu);
            nmos6502_opcodeAdc(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x76: // ROR Zero-page, X-indexed
            l_tmpAddress = nmos6502_getAddressZeroPageXIndexed(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = nmos6502_opcodeRor(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x78: // SEI
            l_cpu->m_flagI = true;
            break;

        case 0x79: // ADC Absolute, Y-indexed
            l_tmpAddress = nmos6502_getAddressAbsoluteYIndexed(l_cpu);
            nmos6502_opcodeAdc(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x7d: // ADC Absolute, X-indexed
            l_tmpAddress = nmos6502_getAddressAbsoluteXIndexed(l_cpu);
            nmos6502_opcodeAdc(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            break;

        case 0x7e: // ROR Absolute, X-indexed
            l_tmpAddress = nmos6502_getAddressAbsoluteXIndexed(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = nmos6502_opcodeRor(l_cpu, l_tmpData);
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
    uint16_t l_address = (nmos6502_fetch8(p_cpu) + p_cpu->m_regX) & 0x00ff;

    uint8_t l_low = busRead(p_cpu->m_bus, l_address);
    uint8_t l_high = busRead(p_cpu->m_bus, (l_address + 1) & 0x00ff);

    return (l_high << 8) | l_low;
}

static inline uint16_t nmos6502_getAddressZeroPage(struct ts_nmos6502 *p_cpu) {
    return nmos6502_fetch8(p_cpu);
}

static inline uint16_t nmos6502_getAddressAbsolute(struct ts_nmos6502 *p_cpu) {
    return nmos6502_fetch16(p_cpu);
}

static inline uint16_t nmos6502_getAddressRelative(struct ts_nmos6502 *p_cpu) {
    int8_t l_offset = nmos6502_fetch8(p_cpu);
    uint16_t l_finalAddress = p_cpu->m_regPC + l_offset;

    if((p_cpu->m_regPC >> 8) != (l_finalAddress >> 8)) {
        busCycle(p_cpu->m_bus);
    }

    return l_finalAddress;
}

static inline uint16_t nmos6502_getAddressIndirectYIndexed(
    struct ts_nmos6502 *p_cpu
) {
    uint16_t l_address = nmos6502_fetch8(p_cpu);

    uint8_t l_low = busRead(p_cpu->m_bus, l_address);
    uint8_t l_high = busRead(p_cpu->m_bus, (l_address + 1) & 0x00ff);

    l_address = (l_high << 8) | l_low;

    return l_address + p_cpu->m_regY;
}

static inline uint16_t nmos6502_getAddressZeroPageXIndexed(
    struct ts_nmos6502 *p_cpu
) {
    return (nmos6502_fetch8(p_cpu) + p_cpu->m_regX) & 0x00ff;
}

static inline uint16_t nmos6502_getAddressAbsoluteYIndexed(
    struct ts_nmos6502 *p_cpu
) {
    uint16_t l_address = nmos6502_fetch16(p_cpu);
    uint16_t l_finalAddress = l_address + p_cpu->m_regY;

    if((l_address >> 8) != (l_finalAddress >> 8)) {
        busCycle(p_cpu->m_bus);
    }

    return l_finalAddress;
}

static inline uint16_t nmos6502_getAddressAbsoluteXIndexed(
    struct ts_nmos6502 *p_cpu
) {
    uint16_t l_address = nmos6502_fetch16(p_cpu);
    uint16_t l_finalAddress = l_address + p_cpu->m_regX;

    if((l_address >> 8) != (l_finalAddress >> 8)) {
        busCycle(p_cpu->m_bus);
    }

    return l_finalAddress;
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

static inline void nmos6502_opcodeBranch(
    struct ts_nmos6502 *p_cpu,
    bool p_condition
) {
    if(p_condition) {
        p_cpu->m_regPC += (int8_t)nmos6502_fetch8(p_cpu);
    }

    // TODO: delay if page change?
    // TODO: delay if condition is true?
}

static inline void nmos6502_opcodeAnd(
    struct ts_nmos6502 *p_cpu,
    uint8_t p_operand
) {
    p_cpu->m_regA &= p_operand;
    nmos6502_setFlagsLogical(p_cpu, p_cpu->m_regA);
}

static inline void nmos6502_opcodeBit(
    struct ts_nmos6502 *p_cpu,
    uint8_t p_operand
) {
    uint8_t l_result = p_cpu->m_regA & p_operand;

    p_cpu->m_flagN = (p_operand & (1 << 7)) != 0;
    p_cpu->m_flagZ = l_result == 0;
    p_cpu->m_flagV = (p_operand & (1 << 6)) != 0;
}

static inline uint8_t nmos6502_opcodeRol(
    struct ts_nmos6502 *p_cpu,
    uint8_t p_operand
) {
    bool l_tmpFlag = p_cpu->m_flagC;
    p_cpu->m_flagC = (p_operand & (1 << 7)) != 0;
    p_operand <<= 1;

    if(l_tmpFlag) {
        p_operand |= 1;
    }

    nmos6502_setFlagsLogical(p_cpu, p_operand);

    return p_operand;
}

static inline void nmos6502_opcodeEor(
    struct ts_nmos6502 *p_cpu,
    uint8_t p_operand
) {
    p_cpu->m_regA ^= p_operand;
    nmos6502_setFlagsLogical(p_cpu, p_cpu->m_regA);
}

static inline uint8_t nmos6502_opcodeLsr(
    struct ts_nmos6502 *p_cpu,
    uint8_t p_operand
) {
    p_cpu->m_flagC = (p_operand & (1 << 0)) != 0;
    p_operand >>= 1;
    nmos6502_setFlagsLogical(p_cpu, p_operand);

    return p_operand;
}

static inline void nmos6502_opcodeAdc(
    struct ts_nmos6502 *p_cpu,
    uint8_t p_operand
) {
    uint8_t l_tmp = p_cpu->m_regA;

    p_cpu->m_regA += p_operand;
    
    if(p_cpu->m_flagC) {
        p_cpu->m_regA++;
    }

    nmos6502_setFlagsLogical(p_cpu, p_cpu->m_regA);
    p_cpu->m_flagV =
        (((l_tmp ^ p_operand) | (l_tmp ^ p_cpu->m_regA)) & (1 << 7)) != 0;
    p_cpu->m_flagC =
        l_tmp + p_operand + (p_cpu->m_flagC ? 1 : 0) >= 0x100;
}

static inline uint8_t nmos6502_opcodeRor(
    struct ts_nmos6502 *p_cpu,
    uint8_t p_operand
) {
    bool l_tmpFlag = p_cpu->m_flagC;
    p_cpu->m_flagC = (p_operand & (1 << 0)) != 0;
    p_operand >>= 1;

    if(l_tmpFlag) {
        p_operand |= 1 << 7;
    }

    nmos6502_setFlagsLogical(p_cpu, p_operand);

    return p_operand;
}
