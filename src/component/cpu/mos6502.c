#include <string.h>

#include "component/cpu/mos6502.h"

static void mos6502_step(struct ts_cpu *p_cpu);
static inline void mos6502_checkInterrupts(struct ts_mos6502 *p_cpu);
static inline void mos6502_interrupt(
    struct ts_mos6502 *p_cpu,
    uint16_t p_fetchVector,
    bool p_flagB
);
static inline uint8_t mos6502_getP(struct ts_mos6502 *p_cpu);
static inline void mos6502_setP(struct ts_mos6502 *p_cpu, uint8_t p_value);
static inline uint8_t mos6502_fetch8(struct ts_mos6502 *p_cpu);
static inline uint16_t mos6502_fetch16(struct ts_mos6502 *p_cpu);
static inline void mos6502_setFlagsLogical(
    struct ts_mos6502 *p_cpu,
    uint8_t p_value
);
static inline uint16_t mos6502_read16(
    struct ts_mos6502 *p_cpu,
    uint16_t p_address
);
static inline uint16_t mos6502_getIndexedIndirect(struct ts_mos6502 *p_cpu);
static inline uint16_t mos6502_getIndirectIndexed(struct ts_mos6502 *p_cpu);
static inline void mos6502_pushPC(struct ts_mos6502 *p_cpu);
static inline void mos6502_popPC(struct ts_mos6502 *p_cpu);
static inline void mos6502_ora(struct ts_mos6502 *p_cpu, uint8_t p_operand);
static inline uint8_t mos6502_asl(struct ts_mos6502 *p_cpu, uint8_t p_operand);
static inline void mos6502_and(struct ts_mos6502 *p_cpu, uint8_t p_operand);
static inline void mos6502_bit(struct ts_mos6502 *p_cpu, uint8_t p_operand);
static inline uint8_t mos6502_rol(struct ts_mos6502 *p_cpu, uint8_t p_operand);
static inline void mos6502_eor(struct ts_mos6502 *p_cpu, uint8_t p_operand);
static inline uint8_t mos6502_lsr(struct ts_mos6502 *p_cpu, uint8_t p_operand);
static inline void mos6502_adc(struct ts_mos6502 *p_cpu, uint8_t p_operand);
static inline uint8_t mos6502_ror(struct ts_mos6502 *p_cpu, uint8_t p_operand);
static inline void mos6502_cmp(
    struct ts_mos6502 *p_cpu,
    uint8_t p_operand1,
    uint8_t p_operand2
);
static inline uint8_t mos6502_dec(struct ts_mos6502 *p_cpu, uint8_t p_operand);
static inline uint8_t mos6502_inc(struct ts_mos6502 *p_cpu, uint8_t p_operand);
static inline void mos6502_sbc(struct ts_mos6502 *p_cpu, uint8_t p_operand);

void mos6502_init(struct ts_mos6502 *p_mos6502, struct ts_bus *p_bus) {
    memset(p_mos6502, 0, sizeof(struct ts_mos6502));

    p_mos6502->m_bus = p_bus;
    p_mos6502->m_cpu.m_step = mos6502_step;

    mos6502_reset(p_mos6502);
}

void mos6502_reset(struct ts_mos6502 *p_mos6502) {
    p_mos6502->m_regSP = 0xff;
    p_mos6502->m_regPC = p_mos6502->m_bus->m_read(p_mos6502->m_bus, 0x01fd);
    p_mos6502->m_regPC |=
        p_mos6502->m_bus->m_read(p_mos6502->m_bus, 0x1fc) << 8;
    p_mos6502->m_flagI = true;
    p_mos6502->m_flagD = false;
}

void mos6502_setNMI(struct ts_mos6502 *p_mos6502, bool p_nmi) {
    p_mos6502->m_pendingNMI |= !p_mos6502->m_flagNMI && p_nmi;
    p_mos6502->m_flagNMI = p_nmi;
}

void mos6502_setIRQ(struct ts_mos6502 *p_mos6502, bool p_irq) {
    p_mos6502->m_flagIRQ = p_irq;
}

static void mos6502_step(struct ts_cpu *p_cpu) {
    struct ts_mos6502 *l_cpu = (struct ts_mos6502 *)p_cpu;

    mos6502_checkInterrupts(l_cpu);

    uint8_t l_opcode = mos6502_fetch8(l_cpu);
    uint16_t l_tmpAddress;
    uint8_t l_tmpData;

    switch(l_opcode) {
        case 0x00: // BRK
            mos6502_interrupt(l_cpu, 0xfffe, true);
            break;

        case 0x01: // ORA indexed indirect
            mos6502_ora(
                l_cpu,
                busRead(
                    l_cpu->m_bus,
                    mos6502_getIndexedIndirect(l_cpu)
                )
            );
            break;

        case 0x05: // ORA zero-page
            mos6502_ora(
                l_cpu,
                busRead(l_cpu->m_bus, mos6502_fetch8(l_cpu))
            );
            break;

        case 0x06: // ASL zero-page
            l_tmpAddress = mos6502_fetch8(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = mos6502_asl(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x08: // PHP
            busWrite(
                l_cpu->m_bus,
                l_cpu->m_regSP-- | 0x100, 
                mos6502_getP(l_cpu)
            );
            break;

        case 0x09: // ORA immediate
            mos6502_ora(l_cpu, mos6502_fetch8(l_cpu));
            break;

        case 0x0a: // ASL A
            l_cpu->m_regA = mos6502_asl(l_cpu, l_cpu->m_regA);
            break;

        case 0x0d: // ORA absolute
            mos6502_ora(l_cpu, busRead(l_cpu->m_bus, mos6502_fetch16(l_cpu)));
            break;

        case 0x0e: // ASL absolute
            l_tmpAddress = mos6502_fetch16(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = mos6502_asl(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x10: // BPL relative
            if(!l_cpu->m_flagN) {
                l_cpu->m_regPC += (int8_t)mos6502_fetch8(l_cpu);
            }
            break;

        case 0x11: // ORA indirect indexed
            mos6502_ora(
                l_cpu,
                busRead(
                    l_cpu->m_bus,
                    mos6502_getIndirectIndexed(l_cpu)
                )
            );
            break;

        case 0x15: // ORA X-indexed zero-page
            mos6502_ora(
                l_cpu,
                busRead(
                    l_cpu->m_bus,
                    mos6502_fetch8(l_cpu) + l_cpu->m_regX
                )
            );
            break;

        case 0x16: // ASL X-indexed zero-page
            l_tmpAddress = mos6502_fetch8(l_cpu) + l_cpu->m_regX;
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = mos6502_asl(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x18: // CLC
            l_cpu->m_flagC = false;
            break;

        case 0x19: // ORA absolute, Y-indexed
            mos6502_ora(
                l_cpu,
                busRead(
                    l_cpu->m_bus,
                    mos6502_fetch16(l_cpu) + l_cpu->m_regY
                )
            );
            break;

        case 0x1d: // ORA absolute, X-indexed
            mos6502_ora(
                l_cpu,
                busRead(
                    l_cpu->m_bus,
                    mos6502_fetch16(l_cpu) + l_cpu->m_regX
                )
            );
            break;

        case 0x1e: // ASL absolute, X-indexed
            l_tmpAddress = mos6502_fetch16(l_cpu) + l_cpu->m_regX;
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = mos6502_asl(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x20: // JSR absolute
            mos6502_pushPC(l_cpu);
            l_cpu->m_regPC = mos6502_fetch16(l_cpu);
            break;

        case 0x21: // AND indexed indirect
            mos6502_and(
                l_cpu,
                busRead(
                    l_cpu->m_bus,
                    mos6502_getIndexedIndirect(l_cpu)
                )
            );
            break;

        case 0x24: // BIT zero-page
            mos6502_bit(l_cpu, busRead(l_cpu->m_bus, mos6502_fetch8(l_cpu)));
            break;

        case 0x25: // AND zero-page
            mos6502_and(l_cpu, busRead(l_cpu->m_bus, mos6502_fetch8(l_cpu)));
            break;

        case 0x26: // ROL zero-page
            l_tmpAddress = mos6502_fetch8(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = mos6502_rol(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x28: // PLP
            mos6502_setP(
                l_cpu,
                busRead(l_cpu->m_bus, ++l_cpu->m_regSP | 0x100)
            );

            break;

        case 0x29: // AND immediate
            mos6502_and(l_cpu, mos6502_fetch8(l_cpu));
            break;

        case 0x2a: // ROL A
            l_cpu->m_regA = mos6502_rol(l_cpu, l_cpu->m_regA);
            break;

        case 0x2c: // BIT absolute
            mos6502_bit(l_cpu, busRead(l_cpu->m_bus, mos6502_fetch16(l_cpu)));
            break;

        case 0x2d: // AND absolute
            mos6502_and(l_cpu, busRead(l_cpu->m_bus, mos6502_fetch16(l_cpu)));
            break;

        case 0x2e: // ROL absolute
            l_tmpAddress = mos6502_fetch16(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = mos6502_rol(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x30: // BMI relative
            if(l_cpu->m_flagN) {
                l_cpu->m_regPC += (int8_t)mos6502_fetch8(l_cpu);
            }
            break;

        case 0x31: // AND indirect indexed
            mos6502_and(
                l_cpu,
                busRead(
                    l_cpu->m_bus,
                    mos6502_getIndirectIndexed(l_cpu)
                )
            );
            break;

        case 0x35: // AND X-indexed zero-page
            mos6502_and(
                l_cpu,
                busRead(
                    l_cpu->m_bus,
                    mos6502_fetch8(l_cpu) + l_cpu->m_regX
                )
            );
            break;

        case 0x36: // ROL X-indexed zero-page
            l_tmpAddress = mos6502_fetch8(l_cpu) + l_cpu->m_regX;
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = mos6502_rol(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x38: // SEC
            l_cpu->m_flagC = true;
            break;

        case 0x39: // AND absolute, Y-indexed
            mos6502_and(
                l_cpu,
                busRead(
                    l_cpu->m_bus,
                    mos6502_fetch16(l_cpu) + l_cpu->m_regY
                )
            );
            break;

        case 0x3d: // AND absolute, X-indexed
            mos6502_and(
                l_cpu,
                busRead(
                    l_cpu->m_bus,
                    mos6502_fetch16(l_cpu) + l_cpu->m_regX
                )
            );
            break;

        case 0x3e: // ROL absolute, X-indexed
            l_tmpAddress = mos6502_fetch16(l_cpu) + l_cpu->m_regX;
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = mos6502_rol(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x40: // RTI
            mos6502_setP(l_cpu, busRead(l_cpu->m_bus, ++l_cpu->m_regSP | 0x100));
            mos6502_popPC(l_cpu);
            break;

        case 0x41: // EOR indexed indirect
            mos6502_eor(
                l_cpu,
                busRead(
                    l_cpu->m_bus,
                    mos6502_getIndexedIndirect(l_cpu)
                )
            );
            break;

        case 0x45: // EOR zero-page
            mos6502_eor(l_cpu, busRead(l_cpu->m_bus, mos6502_fetch8(l_cpu)));
            break;

        case 0x46: // LSR zero-page
            l_tmpAddress = mos6502_fetch8(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = mos6502_lsr(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x48: // PHA
            busWrite(l_cpu->m_bus, l_cpu->m_regSP-- | 0x100, l_cpu->m_regA);
            break;

        case 0x49: // EOR immediate
            mos6502_eor(l_cpu, mos6502_fetch8(l_cpu));
            break;

        case 0x4a: // LSR A
            mos6502_lsr(l_cpu, l_cpu->m_regA);
            break;

        case 0x4c: // JMP absolute
            l_cpu->m_regPC = mos6502_fetch16(l_cpu);
            break;

        case 0x4d: // EOR absolute
            mos6502_eor(l_cpu, busRead(l_cpu->m_bus, mos6502_fetch16(l_cpu)));
            break;

        case 0x4e: // LSR absolute
            l_tmpAddress = mos6502_fetch16(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = mos6502_lsr(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x50: // BVC relative
            if(!l_cpu->m_flagV) {
                l_cpu->m_regPC += (int8_t)mos6502_fetch8(l_cpu);
            }

            break;
        
        case 0x51: // EOR indirect indexed
            mos6502_eor(
                l_cpu,
                busRead(
                    l_cpu->m_bus,
                    mos6502_getIndirectIndexed(l_cpu)
                )
            );
            break;

        case 0x55: // EOR X-indexed zero-page
            mos6502_eor(
                l_cpu,
                busRead(
                    l_cpu->m_bus,
                    mos6502_fetch8(l_cpu) + l_cpu->m_regX
                )
            );
            break;

        case 0x56: // LSR X-indexed zero-page
            l_tmpAddress = mos6502_fetch8(l_cpu) + l_cpu->m_regX;
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = mos6502_lsr(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x58: // CLI
            l_cpu->m_flagI = false;
            break;

        case 0x59: // EOR absolute, Y-indexed
            mos6502_eor(
                l_cpu,
                busRead(
                    l_cpu->m_bus,
                    mos6502_fetch16(l_cpu) + l_cpu->m_regY
                )
            );
            break;

        case 0x5d: // EOR absolute, X-indexed
            mos6502_eor(
                l_cpu,
                busRead(
                    l_cpu->m_bus,
                    mos6502_fetch16(l_cpu) + l_cpu->m_regX
                )
            );
            break;

        case 0x5e: // LSR absolute, X-indexed
            l_tmpAddress = mos6502_fetch16(l_cpu) + l_cpu->m_regX;
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = mos6502_lsr(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x60: // RTS
            mos6502_popPC(l_cpu);
            break;

        case 0x61: // ADC indexed indirect
            mos6502_adc(
                l_cpu,
                busRead(
                    l_cpu->m_bus,
                    mos6502_getIndexedIndirect(l_cpu)
                )
            );
            break;

        case 0x65: // ADC zero-page
            mos6502_adc(l_cpu, busRead(l_cpu->m_bus, mos6502_fetch8(l_cpu)));
            break;

        case 0x66: // ROR zero-page
            l_tmpAddress = mos6502_fetch8(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = mos6502_ror(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x68: // PLA
            l_cpu->m_regA = busRead(l_cpu->m_bus, ++l_cpu->m_regSP | 0x100);
            break;

        case 0x69: // ADC immediate
            mos6502_adc(l_cpu, mos6502_fetch8(l_cpu));
            break;

        case 0x6a: // ROR A
            mos6502_ror(l_cpu, l_cpu->m_regA);
            break;

        case 0x6c: // JMP indirect
            l_cpu->m_regPC = mos6502_read16(l_cpu, mos6502_fetch16(l_cpu));
            break;

        case 0x6d: // ADC absolute
            mos6502_adc(l_cpu, busRead(l_cpu->m_bus, mos6502_fetch16(l_cpu)));
            break;

        case 0x6e: // ROR absolute
            l_tmpAddress = mos6502_fetch16(l_cpu);
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = mos6502_ror(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x70: // BVS relative
            if(l_cpu->m_flagV) {
                l_cpu->m_regPC += (int8_t)mos6502_fetch8(l_cpu);
            }

            break;

        case 0x71: // ADC indirect indexed
            mos6502_adc(
                l_cpu,
                busRead(
                    l_cpu->m_bus,
                    mos6502_getIndirectIndexed(l_cpu)
                )
            );
            break;

        case 0x75: // ADC X-indexed zero-page
            mos6502_adc(
                l_cpu,
                busRead(
                    l_cpu->m_bus,
                    mos6502_fetch8(l_cpu) + l_cpu->m_regX
                )
            );
            break;

        case 0x76: // ROR X-indexed zero-page
            l_tmpAddress = mos6502_fetch8(l_cpu) + l_cpu->m_regX;
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = mos6502_ror(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x78: // SEI
            l_cpu->m_flagI = true;
            break;

        case 0x79: // ADC absolute, Y-indexed
            mos6502_adc(
                l_cpu,
                busRead(
                    l_cpu->m_bus,
                    mos6502_fetch16(l_cpu) + l_cpu->m_regY
                )
            );
            break;

        case 0x7d: // ADC absolute, X-indexed
            mos6502_adc(
                l_cpu,
                busRead(
                    l_cpu->m_bus,
                    mos6502_fetch16(l_cpu) + l_cpu->m_regX
                )
            );
            break;

        case 0x7e: // ROR absolute, X-indexed
            l_tmpAddress = mos6502_fetch16(l_cpu) + l_cpu->m_regX;
            l_tmpData = busRead(l_cpu->m_bus, l_tmpAddress);
            l_tmpData = mos6502_ror(l_cpu, l_tmpData);
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x81: // STA indexed indirect
            l_cpu->m_bus->m_write(
                l_cpu->m_bus,
                mos6502_getIndexedIndirect(l_cpu),
                l_cpu->m_regA
            );
            break;

        case 0x84: // STY zero-page
            l_cpu->m_bus->m_write(
                l_cpu->m_bus,
                mos6502_fetch8(l_cpu),
                l_cpu->m_regY
            );
            break;

        case 0x85: // STA zero-page
            l_cpu->m_bus->m_write(
                l_cpu->m_bus,
                mos6502_fetch8(l_cpu),
                l_cpu->m_regA
            );
            break;

        case 0x86: // STX zero-page
            busWrite(l_cpu->m_bus, mos6502_fetch8(l_cpu), l_cpu->m_regX);
            break;

        case 0x88: // DEY
            l_cpu->m_regY--;
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regY);
            break;

        case 0x8a: // TXA
            l_cpu->m_regA = l_cpu->m_regX;
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regA);
            break;

        case 0x8c: // STY absolute
            busWrite(l_cpu->m_bus, mos6502_fetch16(l_cpu), l_cpu->m_regY);
            break;

        case 0x8d: // STA absolute
            busWrite(l_cpu->m_bus, mos6502_fetch16(l_cpu), l_cpu->m_regA);
            break;

        case 0x8e: // STX absolute
            busWrite(l_cpu->m_bus, mos6502_fetch16(l_cpu), l_cpu->m_regX);
            break;

        case 0x90: // BCC relative
            if(!l_cpu->m_flagC) {
                l_cpu->m_regPC += (int8_t)mos6502_fetch8(l_cpu);
            }

            break;

        case 0x91: // STA indirect indexed
            busWrite(
                l_cpu->m_bus,
                mos6502_getIndirectIndexed(l_cpu),
                l_cpu->m_regA
            );
            break;

        case 0x94: // STY X-indexed zero-page
            busWrite(
                l_cpu->m_bus,
                mos6502_fetch8(l_cpu) + l_cpu->m_regX,
                l_cpu->m_regY
            );
            break;

        case 0x95: // STA X-indexed zero-page
            busWrite(
                l_cpu->m_bus,
                mos6502_fetch8(l_cpu) + l_cpu->m_regX,
                l_cpu->m_regA
            );
            break;

        case 0x96: // STX Y-indexed zero-page
            busWrite(
                l_cpu->m_bus,
                mos6502_fetch8(l_cpu) + l_cpu->m_regY,
                l_cpu->m_regX
            );
            break;

        case 0x98: // TYA
            l_cpu->m_regA = l_cpu->m_regY;
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regA);
            break;

        case 0x99: // STA absolute Y-indexed
            busWrite(
                l_cpu->m_bus,
                mos6502_fetch16(l_cpu) + l_cpu->m_regY,
                l_cpu->m_regA
            );
            break;

        case 0x9a: // TXS
            l_cpu->m_regSP = l_cpu->m_regX;
            break;

        case 0x9d: // STA absolute X-indexed
            busWrite(
                l_cpu->m_bus,
                mos6502_fetch16(l_cpu) + l_cpu->m_regX,
                l_cpu->m_regA
            );
            break;

        case 0xa0: // LDY immediate
            l_cpu->m_regY = mos6502_fetch8(l_cpu);
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regY);
            break;

        case 0xa1: // LDA indexed indirect
            l_cpu->m_regA = busRead(
                l_cpu->m_bus,
                mos6502_getIndexedIndirect(l_cpu)
            );
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regA);
            break;

        case 0xa2: // LDX immediate
            l_cpu->m_regX = mos6502_fetch8(l_cpu);
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regX);
            break;

        case 0xa4: // LDY zero-page
            l_cpu->m_regY = busRead(l_cpu->m_bus, mos6502_fetch8(l_cpu));
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regY);
            break;

        case 0xa5: // LDA zero-page
            l_cpu->m_regA = busRead(l_cpu->m_bus, mos6502_fetch8(l_cpu));
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regA);
            break;

        case 0xa6: // LDX zero-page
            l_cpu->m_regX = busRead(l_cpu->m_bus, mos6502_fetch8(l_cpu));
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regX);
            break;

        case 0xa8: // TAY
            l_cpu->m_regY = l_cpu->m_regA;
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regY);
            break;

        case 0xa9: // LDA immediate
            l_cpu->m_regA = mos6502_fetch8(l_cpu);
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regA);
            break;

        case 0xaa: // TAX
            l_cpu->m_regX = l_cpu->m_regA;
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regX);
            break;

        case 0xac: // LDY absolute
            l_cpu->m_regY = busRead(l_cpu->m_bus, mos6502_fetch16(l_cpu));
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regY);
            break;

        case 0xad: // LDA absolute
            l_cpu->m_regA = busRead(l_cpu->m_bus, mos6502_fetch16(l_cpu));
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regA);
            break;

        case 0xae: // LDX absolute
            l_cpu->m_regX = busRead(l_cpu->m_bus, mos6502_fetch16(l_cpu));
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regX);
            break;

        case 0xb0: // BCS relative
            if(l_cpu->m_flagC) {
                l_cpu->m_regPC += (int8_t)mos6502_fetch8(l_cpu);
            }

            break;

        case 0xb1: // LDA indirect indexed
            l_cpu->m_regA = busRead(
                l_cpu->m_bus,
                mos6502_getIndirectIndexed(l_cpu)
            );

            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regA);
            break;

        case 0xb4: // LDY X-indexed zero-page
            l_cpu->m_regY = busRead(
                l_cpu->m_bus,
                mos6502_fetch8(l_cpu) + l_cpu->m_regX
            );
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regY);
            break;

        case 0xb5: // LDA X-indexed zero-page
            l_cpu->m_regA = busRead(
                l_cpu->m_bus,
                mos6502_fetch8(l_cpu) + l_cpu->m_regX
            );
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regA);
            break;

        case 0xb6: // LDX Y-indexed zero-page
            l_cpu->m_regX = busRead(
                l_cpu->m_bus,
                mos6502_fetch8(l_cpu) + l_cpu->m_regY
            );
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regX);
            break;

        case 0xb8: // CLV
            l_cpu->m_flagV = false;
            break;

        case 0xb9: // LDA absolute Y-indexed
            l_cpu->m_regA = busRead(
                l_cpu->m_bus,
                mos6502_fetch16(l_cpu) + l_cpu->m_regY
            );
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regA);
            break;

        case 0xba: // TSX
            l_cpu->m_regX = l_cpu->m_regSP;
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regX);
            break;

        case 0xbc: // LDY absolute X-indexed
            l_cpu->m_regY = busRead(
                l_cpu->m_bus,
                mos6502_fetch16(l_cpu) + l_cpu->m_regX
            );
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regY);
            break;

        case 0xbd: // LDA absolute X-indexed
            l_cpu->m_regA = busRead(
                l_cpu->m_bus,
                mos6502_fetch16(l_cpu) + l_cpu->m_regX
            );
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regA);
            break;

        case 0xbe: // LDX absolute Y-indexed
            l_cpu->m_regX = busRead(
                l_cpu->m_bus,
                mos6502_fetch16(l_cpu) + l_cpu->m_regY
            );
            mos6502_setFlagsLogical(l_cpu, l_cpu->m_regX);
            break;

        case 0xc0: // CPY immediate
            mos6502_cmp(l_cpu, l_cpu->m_regY, mos6502_fetch8(l_cpu));
            break;

        case 0xc1: // CMP indexed indirect
            mos6502_cmp(
                l_cpu,
                l_cpu->m_regA,
                mos6502_getIndexedIndirect(l_cpu)
            );
            break;

        case 0xc4: // CPY zero-page
            mos6502_cmp(
                l_cpu,
                l_cpu->m_regY,
                busRead(l_cpu->m_bus, mos6502_fetch8(l_cpu))
            );
            break;

        case 0xc5: // CMP zero-page
            mos6502_cmp(
                l_cpu,
                l_cpu->m_regA,
                busRead(l_cpu->m_bus, mos6502_fetch8(l_cpu))
            );
            break;

        case 0xc6: // DEC zero-page
            l_tmpAddress = mos6502_fetch8(l_cpu);
            l_tmpData = mos6502_dec(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0xc8: // INY
            l_cpu->m_regY = mos6502_inc(l_cpu, l_cpu->m_regY);
            break;

        case 0xc9: // CMP immediate
            mos6502_cmp(l_cpu, l_cpu->m_regA, mos6502_fetch8(l_cpu));
            break;

        case 0xca: // DEX
            l_cpu->m_regX = mos6502_dec(l_cpu, l_cpu->m_regX);
            break;

        case 0xcc: // CPY absolute
            mos6502_cmp(
                l_cpu,
                l_cpu->m_regY, 
                mos6502_read16(l_cpu, mos6502_fetch16(l_cpu))
            );
            break;

        case 0xcd: // CMP absolute
            mos6502_cmp(
                l_cpu,
                l_cpu->m_regA, 
                mos6502_read16(l_cpu, mos6502_fetch16(l_cpu))
            );
            break;

        case 0xce: // DEC absolute
            l_tmpAddress = mos6502_fetch16(l_cpu);
            l_tmpData = mos6502_dec(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0xd0: // BNE rel
            if(!l_cpu->m_flagZ) {
                l_cpu->m_regPC += (int8_t)mos6502_fetch8(l_cpu);
            }

            break;

        case 0xd1: // CMP indirect indexed
            mos6502_cmp(
                l_cpu,
                l_cpu->m_regA,
                busRead(l_cpu->m_bus, mos6502_getIndirectIndexed(l_cpu))
            );
            break;

        case 0xd5: // CMP X-indexed zero-page
            mos6502_cmp(
                l_cpu,
                l_cpu->m_regA,
                busRead(l_cpu->m_bus, mos6502_fetch8(l_cpu) + l_cpu->m_regX)
            );
            break;

        case 0xd6: // DEC X-indexed zero-page
            l_tmpAddress = mos6502_fetch8(l_cpu) + l_cpu->m_regX;
            l_tmpData = mos6502_dec(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0xd8: // CLD
            l_cpu->m_flagD = false;
            break;

        case 0xd9: // CMP Y-indexed absolute
            mos6502_cmp(
                l_cpu,
                l_cpu->m_regA,
                busRead(l_cpu->m_bus, mos6502_fetch16(l_cpu) + l_cpu->m_regY)
            );
            break;

        case 0xdd: // CMP X-indexed absolute
            mos6502_cmp(
                l_cpu,
                l_cpu->m_regA,
                busRead(l_cpu->m_bus, mos6502_fetch16(l_cpu) + l_cpu->m_regX)
            );
            break;

        case 0xde: // DEC X-indexed absolute
            l_tmpAddress = mos6502_fetch16(l_cpu) + l_cpu->m_regX;
            l_tmpData = mos6502_dec(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0xe0: // CPX immediate
            mos6502_cmp(l_cpu, l_cpu->m_regX, mos6502_fetch8(l_cpu));
            break;

        case 0xe1: // SBC indexed indirect
            mos6502_sbc(
                l_cpu,
                busRead(l_cpu->m_bus, mos6502_getIndexedIndirect(l_cpu))
            );
            break;

        case 0xe4: // CPX zero-page
            mos6502_cmp(
                l_cpu,
                l_cpu->m_regX,
                busRead(l_cpu->m_bus, mos6502_fetch8(l_cpu))
            );
            break;

        case 0xe5: // SBC zero-page
            mos6502_sbc(l_cpu, busRead(l_cpu->m_bus, mos6502_fetch8(l_cpu)));
            break;

        case 0xe6: // INC zero-page
            l_tmpAddress = mos6502_fetch8(l_cpu);
            l_tmpData = mos6502_inc(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0xe8: // INX
            l_cpu->m_regX = mos6502_inc(l_cpu, l_cpu->m_regX);
            break;

        case 0xe9: // SBC immediate
            mos6502_sbc(l_cpu, mos6502_fetch8(l_cpu));
            break;

        case 0xea: // NOP
            break;

        case 0xec: // CPX absolute
            mos6502_cmp(
                l_cpu,
                l_cpu->m_regX,
                mos6502_read16(l_cpu, mos6502_fetch16(l_cpu))
            );
            break;
        
        case 0xed: // SBC absolute
            mos6502_sbc(l_cpu, mos6502_read16(l_cpu, mos6502_fetch16(l_cpu)));
            break;

        case 0xee: // INC absolute
            l_tmpAddress = mos6502_fetch16(l_cpu);
            l_tmpData = mos6502_inc(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;
        
        case 0xf0: // BEQ relative
            if(l_cpu->m_flagZ) {
                l_cpu->m_regPC += (int8_t)mos6502_fetch8(l_cpu);
            }

            break;

        case 0xf1: // SBC indirect indexed
            mos6502_sbc(
                l_cpu,
                busRead(l_cpu->m_bus, mos6502_getIndirectIndexed(l_cpu))
            );
            break;

        case 0xf5: // SBC X-indexed zero-page
            mos6502_sbc(
                l_cpu,
                busRead(l_cpu->m_bus, mos6502_fetch8(l_cpu) + l_cpu->m_regX)
            );
            break;

        case 0xf6: // INC X-indexed zero-page
            l_tmpAddress = mos6502_fetch8(l_cpu) + l_cpu->m_regX;
            l_tmpData = mos6502_inc(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0xf8: // SED
            l_cpu->m_flagD = true;
            break;

        case 0xf9: // SBC Y-indexed absolute
            mos6502_sbc(
                l_cpu,
                mos6502_read16(l_cpu, mos6502_fetch16(l_cpu) + l_cpu->m_regY)
            );
            break;

        case 0xfd: // SBC X-indexed absolute
            mos6502_sbc(
                l_cpu,
                mos6502_read16(l_cpu, mos6502_fetch16(l_cpu) + l_cpu->m_regX)
            );
            break;

        case 0xfe: // INC X-indexed absolute
            l_tmpAddress = mos6502_fetch16(l_cpu) + l_cpu->m_regX;
            l_tmpData = mos6502_inc(l_cpu, busRead(l_cpu->m_bus, l_tmpAddress));
            busWrite(l_cpu->m_bus, l_tmpAddress, l_tmpData);
            break;

        default: // Invalid opcode
            break;
    }
}

static inline void mos6502_interrupt(
    struct ts_mos6502 *p_cpu,
    uint16_t p_fetchVector,
    bool p_flagB
) {
    mos6502_pushPC(p_cpu);
    
    // Push flags
    busWrite(
        p_cpu->m_bus,
        p_cpu->m_regSP-- | 0x0100,
        mos6502_getP(p_cpu) & (p_flagB ? 0xff : 0xef)
    );

    // Load vector into PC
    p_cpu->m_regPC = mos6502_read16(p_cpu, p_fetchVector);

    // Disable interrupts
    p_cpu->m_flagI = true;
}

static inline void mos6502_checkInterrupts(struct ts_mos6502 *p_cpu) {
    if(p_cpu->m_pendingNMI) {
        mos6502_interrupt(p_cpu, 0xfffa, false);
    } else if(!p_cpu->m_flagI && p_cpu->m_flagIRQ) {
        mos6502_interrupt(p_cpu, 0xfffe, false);
    }
}

static inline uint8_t mos6502_getP(struct ts_mos6502 *p_cpu) {
    uint8_t l_p = 0x30
        | (p_cpu->m_flagN ? 1 << 7 : 0)
        | (p_cpu->m_flagV ? 1 << 6 : 0)
        | (p_cpu->m_flagD ? 1 << 3 : 0)
        | (p_cpu->m_flagI ? 1 << 2 : 0)
        | (p_cpu->m_flagZ ? 1 << 1 : 0)
        | (p_cpu->m_flagC ? 1 << 0 : 0);

    return l_p;
}

static inline void mos6502_setP(struct ts_mos6502 *p_cpu, uint8_t p_value) {
    p_cpu->m_flagN = (p_value & (1 << 7)) != 0;
    p_cpu->m_flagV = (p_value & (1 << 6)) != 0;
    p_cpu->m_flagD = (p_value & (1 << 3)) != 0;
    p_cpu->m_flagI = (p_value & (1 << 2)) != 0;
    p_cpu->m_flagZ = (p_value & (1 << 1)) != 0;
    p_cpu->m_flagC = (p_value & (1 << 0)) != 0;
}

static inline uint8_t mos6502_fetch8(struct ts_mos6502 *p_cpu) {
    return p_cpu->m_bus->m_read(p_cpu->m_bus, p_cpu->m_regPC++);
}

static inline uint16_t mos6502_fetch16(struct ts_mos6502 *p_cpu) {
    uint8_t l_low = mos6502_fetch8(p_cpu);
    uint8_t l_high = mos6502_fetch8(p_cpu);

    return (l_high << 8) | l_low;
}

static inline void mos6502_setFlagsLogical(
    struct ts_mos6502 *p_cpu,
    uint8_t p_value
) {
    p_cpu->m_flagN = (p_value & (1 << 7)) != 0;
    p_cpu->m_flagZ = p_value == 0;
}

static inline uint16_t mos6502_read16(
    struct ts_mos6502 *p_cpu,
    uint16_t p_address
) {
    uint8_t l_low = busRead(p_cpu->m_bus, p_address);
    uint8_t l_high = busRead(p_cpu->m_bus, p_address + 1);

    return (l_high << 8) | l_low;
}

static inline uint16_t mos6502_getIndexedIndirect(struct ts_mos6502 *p_cpu) {
    return mos6502_read16(p_cpu, mos6502_fetch8(p_cpu) + p_cpu->m_regX);
}

static inline uint16_t mos6502_getIndirectIndexed(struct ts_mos6502 *p_cpu) {
    return mos6502_read16(p_cpu, mos6502_fetch8(p_cpu)) + p_cpu->m_regY;
}

static inline void mos6502_pushPC(struct ts_mos6502 *p_cpu) {
    busWrite(p_cpu->m_bus, p_cpu->m_regSP-- | 0x100, p_cpu->m_regPC >> 8);
    busWrite(p_cpu->m_bus, p_cpu->m_regSP-- | 0x100, p_cpu->m_regPC);
}

static inline void mos6502_popPC(struct ts_mos6502 *p_cpu) {
    p_cpu->m_regPC = busRead(p_cpu->m_bus, ++p_cpu->m_regSP);
    p_cpu->m_regPC |= busRead(p_cpu->m_bus, ++p_cpu->m_regSP) << 8;
}


static inline void mos6502_ora(struct ts_mos6502 *p_cpu, uint8_t p_operand) {
    p_cpu->m_regA |= p_operand;
    mos6502_setFlagsLogical(p_cpu, p_cpu->m_regA);
}

static inline uint8_t mos6502_asl(struct ts_mos6502 *p_cpu, uint8_t p_operand) {
    p_cpu->m_flagC = (p_operand & (1 << 7)) != 0;
    p_operand <<= 1;
    mos6502_setFlagsLogical(p_cpu, p_operand);

    return p_operand;
}

static inline void mos6502_and(struct ts_mos6502 *p_cpu, uint8_t p_operand) {
    p_cpu->m_regA &= p_operand;
    mos6502_setFlagsLogical(p_cpu, p_cpu->m_regA);
}

static inline void mos6502_bit(struct ts_mos6502 *p_cpu, uint8_t p_operand) {
    p_cpu->m_flagN = (p_operand & (1 << 7)) != 0;
    p_cpu->m_flagV = (p_operand & (1 << 6)) != 0;
    p_cpu->m_flagZ = (p_cpu->m_regA & p_operand) == 0;
}

static inline uint8_t mos6502_rol(struct ts_mos6502 *p_cpu, uint8_t p_operand) {
    bool l_tmpFlag = p_cpu->m_flagC;
    p_cpu->m_flagC = (p_operand & (1 << 7)) != 0;
    p_operand <<= 1;

    if(l_tmpFlag) {
        p_operand |= 1;
    }

    mos6502_setFlagsLogical(p_cpu, p_operand);

    return p_operand;
}

static inline void mos6502_eor(struct ts_mos6502 *p_cpu, uint8_t p_operand) {
    p_cpu->m_regA ^= p_operand;
    mos6502_setFlagsLogical(p_cpu, p_cpu->m_regA);
}

static inline uint8_t mos6502_lsr(struct ts_mos6502 *p_cpu, uint8_t p_operand) {
    p_cpu->m_flagC = (p_operand & (1 << 0)) != 0;
    p_operand >>= 1;
    mos6502_setFlagsLogical(p_cpu, p_operand);

    return p_operand;
}

static inline void mos6502_adc(struct ts_mos6502 *p_cpu, uint8_t p_operand) {
    uint8_t l_tmp = p_cpu->m_regA;

    p_cpu->m_regA += p_operand;
    
    if(p_cpu->m_flagC) {
        p_cpu->m_regA++;
    }

    mos6502_setFlagsLogical(p_cpu, p_cpu->m_regA);
    p_cpu->m_flagV =
        (((l_tmp ^ p_operand) | (l_tmp ^ p_cpu->m_regA)) & (1 << 7)) != 0;
    p_cpu->m_flagC =
        l_tmp + p_operand + (p_cpu->m_flagC ? 1 : 0) >= 0x100;    
}

static inline uint8_t mos6502_ror(struct ts_mos6502 *p_cpu, uint8_t p_operand) {
    bool l_tmpFlag = p_cpu->m_flagC;
    p_cpu->m_flagC = (p_operand & (1 << 0)) != 0;
    p_operand >>= 1;

    if(l_tmpFlag) {
        p_operand |= 1 << 7;
    }

    mos6502_setFlagsLogical(p_cpu, p_operand);

    return p_operand;
}

static inline void mos6502_cmp(
    struct ts_mos6502 *p_cpu,
    uint8_t p_operand1,
    uint8_t p_operand2
) {
    uint8_t l_result = p_operand1 - p_operand2;

    p_cpu->m_flagC = p_operand1 >= p_operand2;

    mos6502_setFlagsLogical(p_cpu, l_result);
}

static inline uint8_t mos6502_dec(struct ts_mos6502 *p_cpu, uint8_t p_operand) {
    p_operand--;

    mos6502_setFlagsLogical(p_cpu, p_operand);

    return p_operand;
}

static inline uint8_t mos6502_inc(struct ts_mos6502 *p_cpu, uint8_t p_operand) {
    p_operand++;

    mos6502_setFlagsLogical(p_cpu, p_operand);

    return p_operand;
}

static inline void mos6502_sbc(struct ts_mos6502 *p_cpu, uint8_t p_operand) {
    uint8_t l_tmp = p_cpu->m_regA;

    p_cpu->m_regA -= p_operand;
    
    if(!p_cpu->m_flagC) {
        p_cpu->m_regA--;
    }

    mos6502_setFlagsLogical(p_cpu, p_cpu->m_regA);
    p_cpu->m_flagC =
        (l_tmp >= p_operand)&& (l_tmp - p_operand >= (p_cpu->m_flagC ? 0 : 1));
    p_cpu->m_flagV =
        (((l_tmp ^ p_operand) | (l_tmp ^ p_cpu->m_regA)) & (1 << 7)) == 0;
}