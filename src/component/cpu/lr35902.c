#include <string.h>

#include "component/cpu/lr35902.h"

static void lr35902_reset(struct ts_cpu *p_cpu);
static void lr35902_step(struct ts_cpu *p_cpu);
static inline uint8_t lr35902_fetch8(struct ts_lr35902 *p_lr35902);
static inline uint16_t lr35902_fetch16(struct ts_lr35902 *p_lr35902);
static inline void lr35902_write16(
    struct ts_lr35902 *p_lr35902,
    uint16_t p_address,
    uint16_t p_data
);
static inline uint16_t lr35902_getAF(struct ts_lr35902 *p_lr35902);
static inline uint16_t lr35902_getBC(struct ts_lr35902 *p_lr35902);
static inline uint16_t lr35902_getDE(struct ts_lr35902 *p_lr35902);
static inline uint16_t lr35902_getHL(struct ts_lr35902 *p_lr35902);
static inline void lr35902_setAF(
    struct ts_lr35902 *p_lr35902,
    uint16_t p_value
);
static inline void lr35902_setBC(
    struct ts_lr35902 *p_lr35902,
    uint16_t p_value
);
static inline void lr35902_setDE(
    struct ts_lr35902 *p_lr35902,
    uint16_t p_value
);
static inline void lr35902_setHL(
    struct ts_lr35902 *p_lr35902,
    uint16_t p_value
);
static inline uint8_t lr35902_getF(struct ts_lr35902 *p_lr35902);
static inline void lr35902_setF(struct ts_lr35902 *p_lr35902, uint8_t p_value);
static inline void lr35902_opcodeAdc8(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
);
static inline void lr35902_opcodeAdd8(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
);
static inline void lr35902_opcodeAddHL(
    struct ts_lr35902 *p_lr35902,
    uint16_t p_value
);
static inline void lr35902_opcodeAnd8(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
);
static inline void lr35902_opcodeCallCondition(
    struct ts_lr35902 *p_lr35902,
    bool p_condition
);
static inline void lr35902_opcodeCp(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
);
static inline uint8_t lr35902_opcodeDec8(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
);
static inline uint8_t lr35902_opcodeInc8(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
);
static inline void lr35902_opcodeJpCondition(
    struct ts_lr35902 *p_lr35902,
    bool p_condition
);
static inline void lr35902_opcodeJr(
    struct ts_lr35902 *p_lr35902,
    bool p_condition
);
static inline void lr35902_opcodeJpCondition(
    struct ts_lr35902 *p_lr35902,
    bool p_condition
);
static inline void lr35902_opcodeOr8(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
);
static inline uint16_t lr35902_opcodePop(struct ts_lr35902 *p_lr35902);
static inline void lr35902_opcodePush(
    struct ts_lr35902 *p_lr35902,
    uint16_t p_value
);
static inline void lr35902_opcodeRetCondition(
    struct ts_lr35902 *p_lr35902,
    bool p_condition
);
static inline uint8_t lr35902_opcodeRlBase(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
);
static inline uint8_t lr35902_opcodeRlcBase(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
);
static inline uint8_t lr35902_opcodeRrBase(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
);
static inline uint8_t lr35902_opcodeRrcBase(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
);
static inline void lr35902_opcodeRst(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
);
static inline void lr35902_opcodeSbc8(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
);
static inline uint8_t lr35902_opcodeSla(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
);
static inline uint8_t lr35902_opcodeSra(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
);
static inline uint8_t lr35902_opcodeSrl(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
);
static inline void lr35902_opcodeSub8(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
);
static inline uint8_t lr35902_opcodeSwap(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
);
static inline void lr35902_opcodeXor8(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
);

void lr35902_init(struct ts_lr35902 *p_lr35902, struct ts_bus *p_bus) {
    memset(p_lr35902, 0, sizeof(struct ts_lr35902));

    p_lr35902->m_bus = p_bus;
    p_lr35902->m_cpu.m_step = lr35902_step;
    p_lr35902->m_cpu.m_reset = lr35902_reset;

    lr35902_reset(&p_lr35902->m_cpu);
}

static void lr35902_reset(struct ts_cpu *p_cpu) {
    struct ts_lr35902 *l_lr35902 = (struct ts_lr35902 *)p_cpu;

    l_lr35902->m_regA = 0x00;
    l_lr35902->m_regB = 0x00;
    l_lr35902->m_regC = 0x00;
    l_lr35902->m_regD = 0x00;
    l_lr35902->m_regE = 0x00;
    l_lr35902->m_regH = 0x00;
    l_lr35902->m_regL = 0x00;
    l_lr35902->m_regSP = 0x0000;
    l_lr35902->m_regPC = 0x0000;
    l_lr35902->m_flagZ = false;
    l_lr35902->m_flagN = false;
    l_lr35902->m_flagH = false;
    l_lr35902->m_flagC = false;
}

static void lr35902_step(struct ts_cpu *p_cpu) {
    struct ts_lr35902 *l_lr35902 = (struct ts_lr35902 *)p_cpu;

    uint8_t l_opcode = lr35902_fetch8(l_lr35902);
    uint16_t l_tmpAddress;
    uint8_t l_tmpData;

    switch(l_opcode) {
        case 0x00: // NOP
            break;

        case 0x01: // LD BC, u16
            lr35902_setBC(l_lr35902, lr35902_fetch16(l_lr35902));
            break;

        case 0x02: // LD (BC), A
            l_lr35902->m_bus->m_write(
                l_lr35902->m_bus,
                lr35902_getBC(l_lr35902),
                l_lr35902->m_regA
            );
            break;

        case 0x03: // INC BC
            lr35902_setBC(l_lr35902, lr35902_getBC(l_lr35902) + 1);
            break;

        case 0x04: // INC B
            l_lr35902->m_regB =
                lr35902_opcodeInc8(l_lr35902, l_lr35902->m_regB);
            break;

        case 0x05: // DEC B
            l_lr35902->m_regB =
                lr35902_opcodeDec8(l_lr35902, l_lr35902->m_regB);
            break;

        case 0x06: // LD B, d8
            l_lr35902->m_regB = lr35902_fetch8(l_lr35902);
            break;

        case 0x07: // RLCA
            l_lr35902->m_regA =
                lr35902_opcodeRlcBase(l_lr35902, l_lr35902->m_regA);
            l_lr35902->m_flagZ = false;
            break;

        case 0x08: // LD (a16), SP
            lr35902_write16(
                l_lr35902,
                lr35902_fetch16(l_lr35902),
                l_lr35902->m_regSP
            );
            break;
        
        case 0x09: // ADD HL, BC
            lr35902_opcodeAddHL(l_lr35902, lr35902_getBC(l_lr35902));
            break;

        case 0x0a: // LD A, (BC)
            l_lr35902->m_regA =
                busRead(l_lr35902->m_bus, lr35902_getBC(l_lr35902));
            break;

        case 0x0b: // DEC BC
            lr35902_setBC(l_lr35902, lr35902_getBC(l_lr35902) - 1);
            break;

        case 0x0c: // INC C
            l_lr35902->m_regC =
                lr35902_opcodeInc8(l_lr35902, l_lr35902->m_regC);
            break;

        case 0x0d: // DEC C
            l_lr35902->m_regC =
                lr35902_opcodeDec8(l_lr35902, l_lr35902->m_regC);
            break;

        case 0x0e: // LD C, d8
            l_lr35902->m_regC = lr35902_fetch8(l_lr35902);
            break;

        case 0x0f: // RRCA
            l_lr35902->m_regA =
                lr35902_opcodeRrcBase(l_lr35902, l_lr35902->m_regA);
            l_lr35902->m_flagZ = false;
            break;

        case 0x10: // STOP 0
            lr35902_fetch8(l_lr35902);
            // TODO
            break;

        case 0x11: // LD DE, d16
            lr35902_setDE(l_lr35902, lr35902_fetch16(l_lr35902));
            break;

        case 0x12: // LD (DE), A
            busWrite(
                l_lr35902->m_bus,
                lr35902_getDE(l_lr35902),
                l_lr35902->m_regA
            );
            break;

        case 0x13: // INC DE
            lr35902_setDE(l_lr35902, lr35902_getDE(l_lr35902) + 1);
            break;

        case 0x14: // INC D
            l_lr35902->m_regD =
                lr35902_opcodeInc8(l_lr35902, l_lr35902->m_regD);
            break;

        case 0x15: // DEC D
            l_lr35902->m_regD =
                lr35902_opcodeDec8(l_lr35902, l_lr35902->m_regD);
            break;

        case 0x16: // LD D, d8
            l_lr35902->m_regD = lr35902_fetch8(l_lr35902);
            break;

        case 0x17: // RLA
            l_lr35902->m_regA =
                lr35902_opcodeRlBase(l_lr35902, l_lr35902->m_regA);
            l_lr35902->m_flagZ = false;
            break;

        case 0x18: // JR r8
            lr35902_opcodeJr(l_lr35902, true);
            break;

        case 0x19: // ADD HL, DE
            lr35902_opcodeAddHL(l_lr35902, lr35902_getDE(l_lr35902));
            break;

        case 0x1a: // LD A, (DE)
            l_lr35902->m_regA =
                busRead(l_lr35902->m_bus, lr35902_getDE(l_lr35902));
            break;

        case 0x1b: // DEC DE
            lr35902_setDE(l_lr35902, lr35902_getDE(l_lr35902) - 1);
            break;

        case 0x1c: // INC E
            l_lr35902->m_regE =
                lr35902_opcodeInc8(l_lr35902, l_lr35902->m_regE);
            break;

        case 0x1d: // DEC E
            l_lr35902->m_regE =
                lr35902_opcodeDec8(l_lr35902, l_lr35902->m_regE);
            break;

        case 0x1e: // LD E, d8
            l_lr35902->m_regE = lr35902_fetch8(l_lr35902);
            break;

        case 0x1f: // RRA
            l_lr35902->m_regA =
                lr35902_opcodeRrBase(l_lr35902, l_lr35902->m_regA);
            break;

        case 0x20: // JR NZ, r8
            lr35902_opcodeJr(l_lr35902, !l_lr35902->m_flagZ);
            break;

        case 0x21: // LD HL, d16
            lr35902_setHL(l_lr35902, lr35902_fetch16(l_lr35902));
            break;

        case 0x22: // LD (HL+), A
            l_tmpAddress = lr35902_getHL(l_lr35902);
            busWrite(l_lr35902->m_bus, l_tmpAddress, l_lr35902->m_regA);
            lr35902_setHL(l_lr35902, l_tmpAddress + 1);
            break;

        case 0x23: // INC HL
            lr35902_setHL(l_lr35902, lr35902_getHL(l_lr35902) + 1);
            break;

        case 0x24: // INC H
            l_lr35902->m_regH =
                lr35902_opcodeInc8(l_lr35902, l_lr35902->m_regH);
            break;

        case 0x25: // DEC H
            l_lr35902->m_regH =
                lr35902_opcodeDec8(l_lr35902, l_lr35902->m_regH);
            break;

        case 0x26: // LD H, d8
            l_lr35902->m_regH = lr35902_fetch8(l_lr35902);
            break;

        case 0x27: // DAA
            if(l_lr35902->m_flagN) {
                if(l_lr35902->m_flagC) {
                    l_lr35902->m_regA -= 0x60;
                }

                if(l_lr35902->m_flagH) {
                    l_lr35902->m_regA -= 0x06;
                }
            } else {
                if(l_lr35902->m_flagC || (l_lr35902->m_regA > 0x99)) {
                    l_lr35902->m_regA += 0x60;
                    l_lr35902->m_flagC = true;
                }

                if(l_lr35902->m_flagH || ((l_lr35902->m_regA & 0x0f) > 0x09)) {
                    l_lr35902->m_regA += 0x06;
                }
            }

            l_lr35902->m_flagZ = l_lr35902->m_regA == 0;
            l_lr35902->m_flagH = false;

            break;

        case 0x28: // JR Z, r8
            lr35902_opcodeJr(l_lr35902, l_lr35902->m_flagZ);
            break;
        
        case 0x29: // ADD HL, HL
            lr35902_opcodeAddHL(l_lr35902, lr35902_getHL(l_lr35902));
            break;

        case 0x2a: // LD A, (HL+)
            l_tmpAddress = lr35902_getHL(l_lr35902);
            l_lr35902->m_regA = busRead(l_lr35902->m_bus, l_tmpAddress);
            lr35902_setHL(l_lr35902, l_tmpAddress + 1);
            break;

        case 0x2b: // DEC HL
            lr35902_setHL(l_lr35902, lr35902_getHL(l_lr35902) - 1);
            break;

        case 0x2c: // INC L
            l_lr35902->m_regL =
                lr35902_opcodeInc8(l_lr35902, l_lr35902->m_regL);
            break;

        case 0x2d: // DEC L
            l_lr35902->m_regL =
                lr35902_opcodeDec8(l_lr35902, l_lr35902->m_regL);
            break;

        case 0x2e: // LD L, d8
            l_lr35902->m_regL = lr35902_fetch8(l_lr35902);
            break;
        
        case 0x2f: // CPL
            l_lr35902->m_regA = ~l_lr35902->m_regA;
            l_lr35902->m_flagN = true;
            l_lr35902->m_flagH = true;
            break;

        case 0x30: // JR NC, r8
            lr35902_opcodeJr(l_lr35902, !l_lr35902->m_flagC);
            break;

        case 0x31: // LD SP, d16
            l_lr35902->m_regSP = lr35902_fetch16(l_lr35902);
            break;

        case 0x32: // LD (HL-), A
            l_tmpAddress = lr35902_getHL(l_lr35902);
            busWrite(l_lr35902->m_bus, l_tmpAddress, l_lr35902->m_regA);
            lr35902_setHL(l_lr35902, l_tmpAddress - 1);
            break;

        case 0x33: // INC SP
            l_lr35902->m_regSP++;
            break;

        case 0x34: // INC (HL)
            l_tmpAddress = lr35902_getHL(l_lr35902);
            l_tmpData = busRead(l_lr35902->m_bus, l_tmpAddress);
            l_tmpData = lr35902_opcodeInc8(l_lr35902, l_tmpData);
            busWrite(l_lr35902->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x35: // DEC (HL)
            l_tmpAddress = lr35902_getHL(l_lr35902);
            l_tmpData = busRead(l_lr35902->m_bus, l_tmpAddress);
            l_tmpData = lr35902_opcodeDec8(l_lr35902, l_tmpData);
            busWrite(l_lr35902->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x36: // LD (HL), d8
            l_tmpAddress = lr35902_getHL(l_lr35902);
            l_tmpData = lr35902_fetch8(l_lr35902);
            busWrite(l_lr35902->m_bus, l_tmpAddress, l_tmpData);
            break;

        case 0x37: // SCF
            l_lr35902->m_flagN = false;
            l_lr35902->m_flagH = false;
            l_lr35902->m_flagC = true;
            break;

        case 0x38: // JR C, r8
            lr35902_opcodeJr(l_lr35902, l_lr35902->m_flagC);
            break;

        case 0x39: // ADD HL, SP
            lr35902_opcodeAddHL(l_lr35902, l_lr35902->m_regSP);
            break;

        case 0x3a: // LD A, (HL-)
            l_tmpAddress = lr35902_getHL(l_lr35902);
            l_lr35902->m_regA = busRead(l_lr35902->m_bus, l_tmpAddress);
            lr35902_setHL(l_lr35902, l_tmpAddress - 1);
            break;

        case 0x3b: // DEC SP
            l_lr35902->m_regSP--;
            break;

        case 0x3c: // INC A
            l_lr35902->m_regA =
                lr35902_opcodeInc8(l_lr35902, l_lr35902->m_regA);
            break;

        case 0x3d: // DEC A
            l_lr35902->m_regA =
                lr35902_opcodeDec8(l_lr35902, l_lr35902->m_regA);
            break;

        case 0x3e: // LD A, d8
            l_lr35902->m_regA = lr35902_fetch8(l_lr35902);
            break;

        case 0x3f: // CCF
            l_lr35902->m_flagN = false;
            l_lr35902->m_flagH = false;
            l_lr35902->m_flagC = !l_lr35902->m_flagC;
            break;

        case 0x40: // LD B, B
            break;

        case 0x41: // LD B, C
            l_lr35902->m_regB = l_lr35902->m_regC;
            break;

        case 0x42: // LD B, D
            l_lr35902->m_regB = l_lr35902->m_regD;
            break;

        case 0x43: // LD B, E
            l_lr35902->m_regB = l_lr35902->m_regE;
            break;

        case 0x44: // LD B, H
            l_lr35902->m_regB = l_lr35902->m_regH;
            break;

        case 0x45: // LD B, L
            l_lr35902->m_regB = l_lr35902->m_regL;
            break;

        case 0x46: // LD B, (HL)
            l_lr35902->m_regB =
                busRead(l_lr35902->m_bus, lr35902_getHL(l_lr35902));
            break;

        case 0x47: // LD B, A
            l_lr35902->m_regB = l_lr35902->m_regA;
            break;

        case 0x48: // LD C, B
            l_lr35902->m_regC = l_lr35902->m_regB;
            break;

        case 0x49: // LD C, C
            break;

        case 0x4a: // LD C, D
            l_lr35902->m_regC = l_lr35902->m_regD;
            break;

        case 0x4b: // LD C, E
            l_lr35902->m_regC = l_lr35902->m_regE;
            break;

        case 0x4c: // LD C, H
            l_lr35902->m_regC = l_lr35902->m_regH;
            break;

        case 0x4d: // LD C, L
            l_lr35902->m_regC = l_lr35902->m_regL;
            break;

        case 0x4e: // LD C, (HL)
            l_lr35902->m_regC =
                busRead(l_lr35902->m_bus, lr35902_getHL(l_lr35902));
            break;

        case 0x4f: // LD C, A
            l_lr35902->m_regC = l_lr35902->m_regA;
            break;

        case 0x50: // LD D, B
            l_lr35902->m_regD = l_lr35902->m_regB;
            break;

        case 0x51: // LD D, C
            l_lr35902->m_regD = l_lr35902->m_regC;
            break;

        case 0x52: // LD D, D
            break;

        case 0x53: // LD D, E
            l_lr35902->m_regD = l_lr35902->m_regE;
            break;

        case 0x54: // LD D, H
            l_lr35902->m_regD = l_lr35902->m_regH;
            break;

        case 0x55: // LD D, L
            l_lr35902->m_regD = l_lr35902->m_regL;
            break;

        case 0x56: // LD D, (HL)
            l_lr35902->m_regD =
                busRead(l_lr35902->m_bus, lr35902_getHL(l_lr35902));
            break;

        case 0x57: // LD D, A
            l_lr35902->m_regD = l_lr35902->m_regA;
            break;

        case 0x58: // LD E, B
            l_lr35902->m_regE = l_lr35902->m_regB;
            break;

        case 0x59: // LD E, C
            l_lr35902->m_regE = l_lr35902->m_regC;
            break;

        case 0x5a: // LD E, D
            l_lr35902->m_regE = l_lr35902->m_regD;
            break;

        case 0x5b: // LD E, E
            break;

        case 0x5c: // LD E, H
            l_lr35902->m_regE = l_lr35902->m_regH;
            break;

        case 0x5d: // LD E, L
            l_lr35902->m_regE = l_lr35902->m_regL;
            break;

        case 0x5e: // LD E, (HL)
            l_lr35902->m_regE =
                busRead(l_lr35902->m_bus, lr35902_getHL(l_lr35902));
            break;

        case 0x5f: // LD E, A
            l_lr35902->m_regE = l_lr35902->m_regA;
            break;

        case 0x60: // LD H, B
            l_lr35902->m_regH = l_lr35902->m_regB;
            break;

        case 0x61: // LD H, C
            l_lr35902->m_regH = l_lr35902->m_regC;
            break;

        case 0x62: // LD H, D
            l_lr35902->m_regH = l_lr35902->m_regD;
            break;

        case 0x63: // LD H, E
            l_lr35902->m_regH = l_lr35902->m_regE;
            break;

        case 0x64: // LD H, H
            break;

        case 0x65: // LD H, L
            l_lr35902->m_regH = l_lr35902->m_regL;
            break;

        case 0x66: // LD H, (HL)
            l_lr35902->m_regH =
                busRead(l_lr35902->m_bus, lr35902_getHL(l_lr35902));
            break;

        case 0x67: // LD H, A
            l_lr35902->m_regH = l_lr35902->m_regA;
            break;

        case 0x68: // LD L, B
            l_lr35902->m_regL = l_lr35902->m_regB;
            break;

        case 0x69: // LD L, C
            l_lr35902->m_regL = l_lr35902->m_regC;
            break;

        case 0x6a: // LD L, D
            l_lr35902->m_regL = l_lr35902->m_regD;
            break;

        case 0x6b: // LD L, E
            l_lr35902->m_regL = l_lr35902->m_regE;
            break;

        case 0x6c: // LD L, H
            l_lr35902->m_regL = l_lr35902->m_regH;
            break;

        case 0x6d: // LD L, L
            break;

        case 0x6e: // LD L, (HL)
            l_lr35902->m_regL =
                busRead(l_lr35902->m_bus, lr35902_getHL(l_lr35902));
            break;

        case 0x6f: // LD L, A
            l_lr35902->m_regL = l_lr35902->m_regA;
            break;

        case 0x70: // LD (HL), B
            busWrite(
                l_lr35902->m_bus,
                lr35902_getHL(l_lr35902),
                l_lr35902->m_regB
            );
            break;

        case 0x71: // LD (HL), C
            busWrite(
                l_lr35902->m_bus,
                lr35902_getHL(l_lr35902),
                l_lr35902->m_regC
            );
            break;

        case 0x72: // LD (HL), D
            busWrite(
                l_lr35902->m_bus,
                lr35902_getHL(l_lr35902),
                l_lr35902->m_regD
            );
            break;

        case 0x73: // LD (HL), E
            busWrite(
                l_lr35902->m_bus,
                lr35902_getHL(l_lr35902),
                l_lr35902->m_regE
            );
            break;

        case 0x74: // LD (HL), H
            busWrite(
                l_lr35902->m_bus,
                lr35902_getHL(l_lr35902),
                l_lr35902->m_regH
            );
            break;

        case 0x75: // LD (HL), L
            busWrite(
                l_lr35902->m_bus,
                lr35902_getHL(l_lr35902),
                l_lr35902->m_regL
            );
            break;

        case 0x76: // HALT
            // TODO
            break;

        case 0x77: // LD (HL), A
            busWrite(
                l_lr35902->m_bus,
                lr35902_getHL(l_lr35902),
                l_lr35902->m_regA
            );
            break;

        case 0x78: // LD A, B
            l_lr35902->m_regA = l_lr35902->m_regB;
            break;

        case 0x79: // LD A, C
            l_lr35902->m_regA = l_lr35902->m_regC;
            break;

        case 0x7a: // LD A, D
            l_lr35902->m_regA = l_lr35902->m_regD;
            break;

        case 0x7b: // LD A, E
            l_lr35902->m_regA = l_lr35902->m_regE;
            break;

        case 0x7c: // LD A, H
            l_lr35902->m_regA = l_lr35902->m_regH;
            break;

        case 0x7d: // LD A, L
            l_lr35902->m_regA = l_lr35902->m_regL;
            break;

        case 0x7e: // LD A, (HL)
            l_lr35902->m_regA =
                busRead(l_lr35902->m_bus, lr35902_getHL(l_lr35902));
            break;

        case 0x7f: // LD A, A
            break;

        case 0x80: // ADD A, B
            lr35902_opcodeAdd8(l_lr35902, l_lr35902->m_regB);
            break;

        case 0x81: // ADD A, C
            lr35902_opcodeAdd8(l_lr35902, l_lr35902->m_regC);
            break;

        case 0x82: // ADD A, D
            lr35902_opcodeAdd8(l_lr35902, l_lr35902->m_regD);
            break;

        case 0x83: // ADD A, E
            lr35902_opcodeAdd8(l_lr35902, l_lr35902->m_regE);
            break;

        case 0x84: // ADD A, H
            lr35902_opcodeAdd8(l_lr35902, l_lr35902->m_regH);
            break;

        case 0x85: // ADD A, L
            lr35902_opcodeAdd8(l_lr35902, l_lr35902->m_regL);
            break;

        case 0x86: // ADD A, (HL)
            lr35902_opcodeAdd8(
                l_lr35902,
                busRead(l_lr35902->m_bus, lr35902_getHL(l_lr35902))
            );
            break;

        case 0x87: // ADD A, A
            lr35902_opcodeAdd8(l_lr35902, l_lr35902->m_regA);
            break;

        case 0x88: // ADC A, B
            lr35902_opcodeAdc8(l_lr35902, l_lr35902->m_regB);
            break;

        case 0x89: // ADC A, C
            lr35902_opcodeAdc8(l_lr35902, l_lr35902->m_regC);
            break;

        case 0x8a: // ADC A, D
            lr35902_opcodeAdc8(l_lr35902, l_lr35902->m_regD);
            break;

        case 0x8b: // ADC A, E
            lr35902_opcodeAdc8(l_lr35902, l_lr35902->m_regE);
            break;

        case 0x8c: // ADC A, H
            lr35902_opcodeAdc8(l_lr35902, l_lr35902->m_regH);
            break;

        case 0x8d: // ADC A, L
            lr35902_opcodeAdc8(l_lr35902, l_lr35902->m_regL);
            break;

        case 0x8e: // ADC A, (HL)
            lr35902_opcodeAdc8(
                l_lr35902,
                busRead(l_lr35902->m_bus, lr35902_getHL(l_lr35902))
            );
            break;

        case 0x8f: // ADC A, A
            lr35902_opcodeAdc8(l_lr35902, l_lr35902->m_regA);
            break;

        case 0x90: // SUB A, B
            lr35902_opcodeSub8(l_lr35902, l_lr35902->m_regB);
            break;

        case 0x91: // SUB A, C
            lr35902_opcodeSub8(l_lr35902, l_lr35902->m_regC);
            break;

        case 0x92: // SUB A, D
            lr35902_opcodeSub8(l_lr35902, l_lr35902->m_regD);
            break;

        case 0x93: // SUB A, E
            lr35902_opcodeSub8(l_lr35902, l_lr35902->m_regE);
            break;

        case 0x94: // SUB A, H
            lr35902_opcodeSub8(l_lr35902, l_lr35902->m_regH);
            break;

        case 0x95: // SUB A, L
            lr35902_opcodeSub8(l_lr35902, l_lr35902->m_regL);
            break;

        case 0x96: // SUB A, (HL)
            lr35902_opcodeSub8(
                l_lr35902,
                busRead(l_lr35902->m_bus, lr35902_getHL(l_lr35902))
            );
            break;

        case 0x97: // SUB A, A
            lr35902_opcodeSub8(l_lr35902, l_lr35902->m_regA);
            break;

        case 0x98: // SBC A, B
            lr35902_opcodeSbc8(l_lr35902, l_lr35902->m_regB);
            break;

        case 0x99: // SBC A, C
            lr35902_opcodeSbc8(l_lr35902, l_lr35902->m_regC);
            break;

        case 0x9a: // SBC A, D
            lr35902_opcodeSbc8(l_lr35902, l_lr35902->m_regD);
            break;

        case 0x9b: // SBC A, E
            lr35902_opcodeSbc8(l_lr35902, l_lr35902->m_regE);
            break;

        case 0x9c: // SBC A, H
            lr35902_opcodeSbc8(l_lr35902, l_lr35902->m_regH);
            break;

        case 0x9d: // SBC A, L
            lr35902_opcodeSbc8(l_lr35902, l_lr35902->m_regL);
            break;

        case 0x9e: // SBC A, (HL)
            lr35902_opcodeSbc8(
                l_lr35902,
                busRead(l_lr35902->m_bus, lr35902_getHL(l_lr35902))
            );
            break;

        case 0x9f: // SBC A, A
            lr35902_opcodeSbc8(l_lr35902, l_lr35902->m_regA);
            break;

        case 0xa0: // AND A, B
            lr35902_opcodeAnd8(l_lr35902, l_lr35902->m_regB);
            break;

        case 0xa1: // AND A, C
            lr35902_opcodeAnd8(l_lr35902, l_lr35902->m_regC);
            break;

        case 0xa2: // AND A, D
            lr35902_opcodeAnd8(l_lr35902, l_lr35902->m_regD);
            break;

        case 0xa3: // AND A, E
            lr35902_opcodeAnd8(l_lr35902, l_lr35902->m_regE);
            break;

        case 0xa4: // AND A, H
            lr35902_opcodeAnd8(l_lr35902, l_lr35902->m_regH);
            break;

        case 0xa5: // AND A, L
            lr35902_opcodeAnd8(l_lr35902, l_lr35902->m_regL);
            break;

        case 0xa6: // AND A, (HL)
            lr35902_opcodeAnd8(
                l_lr35902,
                busRead(l_lr35902->m_bus, lr35902_getHL(l_lr35902))
            );
            break;

        case 0xa7: // AND A, A
            lr35902_opcodeAnd8(l_lr35902, l_lr35902->m_regA);
            break;

        case 0xa8: // XOR A, B
            lr35902_opcodeXor8(l_lr35902, l_lr35902->m_regB);
            break;

        case 0xa9: // XOR A, C
            lr35902_opcodeXor8(l_lr35902, l_lr35902->m_regC);
            break;

        case 0xaa: // XOR A, D
            lr35902_opcodeXor8(l_lr35902, l_lr35902->m_regD);
            break;

        case 0xab: // XOR A, E
            lr35902_opcodeXor8(l_lr35902, l_lr35902->m_regE);
            break;

        case 0xac: // XOR A, H
            lr35902_opcodeXor8(l_lr35902, l_lr35902->m_regH);
            break;

        case 0xad: // XOR A, L
            lr35902_opcodeXor8(l_lr35902, l_lr35902->m_regL);
            break;

        case 0xae: // XOR A, (HL)
            lr35902_opcodeXor8(
                l_lr35902,
                busRead(l_lr35902->m_bus, lr35902_getHL(l_lr35902))
            );
            break;

        case 0xaf: // XOR A, A
            lr35902_opcodeXor8(l_lr35902, l_lr35902->m_regA);
            break;

        case 0xb0: // OR A, B
            lr35902_opcodeOr8(l_lr35902, l_lr35902->m_regB);
            break;

        case 0xb1: // OR A, C
            lr35902_opcodeOr8(l_lr35902, l_lr35902->m_regC);
            break;

        case 0xb2: // OR A, D
            lr35902_opcodeOr8(l_lr35902, l_lr35902->m_regD);
            break;

        case 0xb3: // OR A, E
            lr35902_opcodeOr8(l_lr35902, l_lr35902->m_regE);
            break;

        case 0xb4: // OR A, H
            lr35902_opcodeOr8(l_lr35902, l_lr35902->m_regH);
            break;

        case 0xb5: // OR A, L
            lr35902_opcodeOr8(l_lr35902, l_lr35902->m_regL);
            break;

        case 0xb6: // OR A, (HL)
            lr35902_opcodeOr8(
                l_lr35902,
                busRead(l_lr35902->m_bus, lr35902_getHL(l_lr35902))
            );
            break;

        case 0xb7: // OR A, A
            lr35902_opcodeOr8(l_lr35902, l_lr35902->m_regA);
            break;

        case 0xb8: // CP A, B
            lr35902_opcodeCp(l_lr35902, l_lr35902->m_regB);
            break;

        case 0xb9: // CP A, C
            lr35902_opcodeCp(l_lr35902, l_lr35902->m_regC);
            break;

        case 0xba: // CP A, D
            lr35902_opcodeCp(l_lr35902, l_lr35902->m_regD);
            break;

        case 0xbb: // CP A, E
            lr35902_opcodeCp(l_lr35902, l_lr35902->m_regE);
            break;

        case 0xbc: // CP A, H
            lr35902_opcodeCp(l_lr35902, l_lr35902->m_regH);
            break;

        case 0xbd: // CP A, L
            lr35902_opcodeCp(l_lr35902, l_lr35902->m_regL);
            break;

        case 0xbe: // CP A, (HL)
            lr35902_opcodeCp(
                l_lr35902,
                busRead(l_lr35902->m_bus, lr35902_getHL(l_lr35902))
            );
            break;

        case 0xbf: // CP A, A
            lr35902_opcodeCp(l_lr35902, l_lr35902->m_regA);
            break;

        case 0xc0: // RET NZ
            lr35902_opcodeRetCondition(l_lr35902, !l_lr35902->m_flagZ);
            break;

        case 0xc1: // POP BC
            lr35902_setBC(l_lr35902, lr35902_opcodePop(l_lr35902));
            break;

        case 0xc2: // JP NZ, a16
            lr35902_opcodeJpCondition(l_lr35902, !l_lr35902->m_flagZ);
            break;

        case 0xc3: // JP a16
            lr35902_opcodeJpCondition(l_lr35902, true);
            break;

        case 0xc4: // CALL NZ, a16
            lr35902_opcodeCallCondition(l_lr35902, !l_lr35902->m_flagZ);
            break;

        case 0xc5: // PUSH BC
            lr35902_opcodePush(l_lr35902, lr35902_getBC(l_lr35902));
            break;

        case 0xc6: // ADD A, d8
            lr35902_opcodeAdd8(l_lr35902, lr35902_fetch8(l_lr35902));
            break;

        case 0xc7: // RST 0x00
            lr35902_opcodeRst(l_lr35902, 0x00);
            break;

        case 0xc8: // RET Z
            lr35902_opcodeRetCondition(l_lr35902, l_lr35902->m_flagZ);
            break;

        case 0xc9: // RET
            l_lr35902->m_regPC = lr35902_opcodePop(l_lr35902);
            busCycle(l_lr35902->m_bus);
            break;

        case 0xca: // JP Z, a16
            lr35902_opcodeJpCondition(l_lr35902, l_lr35902->m_flagZ);
            break;

        case 0xcb: // CB prefix
            switch(lr35902_fetch8(l_lr35902)) {
                case 0x00: // RLC B
                    l_lr35902->m_regB = lr35902_opcodeRlcBase(
                        l_lr35902,
                        l_lr35902->m_regB
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regB == 0;
                    break;

                case 0x01: // RLC C
                    l_lr35902->m_regC = lr35902_opcodeRlcBase(
                        l_lr35902,
                        l_lr35902->m_regC
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regC == 0;
                    break;

                case 0x02: // RLC D
                    l_lr35902->m_regD = lr35902_opcodeRlcBase(
                        l_lr35902,
                        l_lr35902->m_regD
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regD == 0;
                    break;

                case 0x03: // RLC E
                    l_lr35902->m_regE = lr35902_opcodeRlcBase(
                        l_lr35902,
                        l_lr35902->m_regE
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regE == 0;
                    break;

                case 0x04: // RLC H
                    l_lr35902->m_regH = lr35902_opcodeRlcBase(
                        l_lr35902,
                        l_lr35902->m_regH
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regH == 0;
                    break;

                case 0x05: // RLC L
                    l_lr35902->m_regL = lr35902_opcodeRlcBase(
                        l_lr35902,
                        l_lr35902->m_regL
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regL == 0;
                    break;

                case 0x06: // RLC (HL)
                    l_tmpData = lr35902_opcodeRlcBase(
                        l_lr35902,
                        busRead(l_lr35902->m_bus, lr35902_getHL(l_lr35902))
                    );
                    busWrite(
                        l_lr35902->m_bus,
                        lr35902_getHL(l_lr35902),
                        l_tmpData
                    );
                    l_lr35902->m_flagZ = l_tmpData == 0;
                    break;

                case 0x07: // RLC A
                    l_lr35902->m_regA = lr35902_opcodeRlcBase(
                        l_lr35902,
                        l_lr35902->m_regA
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regA == 0;
                    break;

                case 0x08: // RRC B
                    l_lr35902->m_regB = lr35902_opcodeRrcBase(
                        l_lr35902,
                        l_lr35902->m_regB
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regB == 0;
                    break;

                case 0x09: // RRC C
                    l_lr35902->m_regC = lr35902_opcodeRrcBase(
                        l_lr35902,
                        l_lr35902->m_regC
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regC == 0;
                    break;

                case 0x0a: // RRC D
                    l_lr35902->m_regD = lr35902_opcodeRrcBase(
                        l_lr35902,
                        l_lr35902->m_regD
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regD == 0;
                    break;

                case 0x0b: // RRC E
                    l_lr35902->m_regE = lr35902_opcodeRrcBase(
                        l_lr35902,
                        l_lr35902->m_regE
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regE == 0;
                    break;

                case 0x0c: // RRC H
                    l_lr35902->m_regH = lr35902_opcodeRrcBase(
                        l_lr35902,
                        l_lr35902->m_regH
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regH == 0;
                    break;

                case 0x0d: // RRC L
                    l_lr35902->m_regL = lr35902_opcodeRrcBase(
                        l_lr35902,
                        l_lr35902->m_regL
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regL == 0;
                    break;

                case 0x0e: // RRC (HL)
                    l_tmpData = lr35902_opcodeRrcBase(
                        l_lr35902,
                        busRead(l_lr35902->m_bus, lr35902_getHL(l_lr35902))
                    );
                    busWrite(
                        l_lr35902->m_bus,
                        lr35902_getHL(l_lr35902),
                        l_tmpData
                    );
                    l_lr35902->m_flagZ = l_tmpData == 0;
                    break;

                case 0x0f: // RRC A
                    l_lr35902->m_regA = lr35902_opcodeRrcBase(
                        l_lr35902,
                        l_lr35902->m_regA
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regA == 0;
                    break;

                case 0x10: // RL B
                    l_lr35902->m_regB = lr35902_opcodeRlBase(
                        l_lr35902,
                        l_lr35902->m_regB
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regB == 0;
                    break;

                case 0x11: // RL C
                    l_lr35902->m_regC = lr35902_opcodeRlBase(
                        l_lr35902,
                        l_lr35902->m_regC
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regC == 0;
                    break;

                case 0x12: // RL D
                    l_lr35902->m_regD = lr35902_opcodeRlBase(
                        l_lr35902,
                        l_lr35902->m_regD
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regD == 0;
                    break;

                case 0x13: // RL E
                    l_lr35902->m_regE = lr35902_opcodeRlBase(
                        l_lr35902,
                        l_lr35902->m_regE
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regE == 0;
                    break;

                case 0x14: // RL H
                    l_lr35902->m_regH = lr35902_opcodeRlBase(
                        l_lr35902,
                        l_lr35902->m_regH
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regH == 0;
                    break;

                case 0x15: // RL L
                    l_lr35902->m_regL = lr35902_opcodeRlBase(
                        l_lr35902,
                        l_lr35902->m_regL
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regL == 0;
                    break;

                case 0x16: // RL (HL)
                    l_tmpData = lr35902_opcodeRlBase(
                        l_lr35902,
                        busRead(l_lr35902->m_bus, lr35902_getHL(l_lr35902))
                    );
                    busWrite(
                        l_lr35902->m_bus,
                        lr35902_getHL(l_lr35902),
                        l_tmpData
                    );
                    l_lr35902->m_flagZ = l_tmpData == 0;
                    break;

                case 0x17: // RL A
                    l_lr35902->m_regA = lr35902_opcodeRlBase(
                        l_lr35902,
                        l_lr35902->m_regA
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regA == 0;
                    break;

                case 0x18: // RR B
                    l_lr35902->m_regB = lr35902_opcodeRrBase(
                        l_lr35902,
                        l_lr35902->m_regB
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regB == 0;
                    break;

                case 0x19: // RR C
                    l_lr35902->m_regC = lr35902_opcodeRrBase(
                        l_lr35902,
                        l_lr35902->m_regC
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regC == 0;
                    break;

                case 0x1a: // RR D
                    l_lr35902->m_regD = lr35902_opcodeRrBase(
                        l_lr35902,
                        l_lr35902->m_regD
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regD == 0;
                    break;

                case 0x1b: // RR E
                    l_lr35902->m_regE = lr35902_opcodeRrBase(
                        l_lr35902,
                        l_lr35902->m_regE
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regE == 0;
                    break;

                case 0x1c: // RR H
                    l_lr35902->m_regH = lr35902_opcodeRrBase(
                        l_lr35902,
                        l_lr35902->m_regH
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regH == 0;
                    break;

                case 0x1d: // RR L
                    l_lr35902->m_regL = lr35902_opcodeRrBase(
                        l_lr35902,
                        l_lr35902->m_regL
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regL == 0;
                    break;

                case 0x1e: // RR (HL)
                    l_tmpData = lr35902_opcodeRrBase(
                        l_lr35902,
                        busRead(l_lr35902->m_bus, lr35902_getHL(l_lr35902))
                    );
                    busWrite(
                        l_lr35902->m_bus,
                        lr35902_getHL(l_lr35902),
                        l_tmpData
                    );
                    l_lr35902->m_flagZ = l_tmpData == 0;
                    break;

                case 0x1f: // RR A
                    l_lr35902->m_regA = lr35902_opcodeRrBase(
                        l_lr35902,
                        l_lr35902->m_regA
                    );
                    l_lr35902->m_flagZ = l_lr35902->m_regA == 0;
                    break;

                case 0x20: // SLA B
                    l_lr35902->m_regB =
                        lr35902_opcodeSla(l_lr35902, l_lr35902->m_regB);
                    break;

                case 0x21: // SLA C
                    l_lr35902->m_regC =
                        lr35902_opcodeSla(l_lr35902, l_lr35902->m_regC);
                    break;

                case 0x22: // SLA D
                    l_lr35902->m_regD =
                        lr35902_opcodeSla(l_lr35902, l_lr35902->m_regD);
                    break;

                case 0x23: // SLA E
                    l_lr35902->m_regE =
                        lr35902_opcodeSla(l_lr35902, l_lr35902->m_regE);
                    break;

                case 0x24: // SLA H
                    l_lr35902->m_regH =
                        lr35902_opcodeSla(l_lr35902, l_lr35902->m_regH);
                    break;

                case 0x25: // SLA L
                    l_lr35902->m_regL =
                        lr35902_opcodeSla(l_lr35902, l_lr35902->m_regL);
                    break;

                case 0x26: // SLA (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);

                    busWrite(
                        l_lr35902->m_bus,
                        l_tmpAddress,
                        lr35902_opcodeSla(
                            l_lr35902,
                            busRead(l_lr35902->m_bus, l_tmpAddress)
                        )
                    );
                    break;

                case 0x27: // SLA A
                    l_lr35902->m_regA =
                        lr35902_opcodeSla(l_lr35902, l_lr35902->m_regA);
                    break;

                case 0x28: // SRA B
                    l_lr35902->m_regB =
                        lr35902_opcodeSra(l_lr35902, l_lr35902->m_regB);
                    break;

                case 0x29: // SRA C
                    l_lr35902->m_regC =
                        lr35902_opcodeSra(l_lr35902, l_lr35902->m_regC);
                    break;

                case 0x2a: // SRA D
                    l_lr35902->m_regD =
                        lr35902_opcodeSra(l_lr35902, l_lr35902->m_regD);
                    break;

                case 0x2b: // SRA E
                    l_lr35902->m_regE =
                        lr35902_opcodeSra(l_lr35902, l_lr35902->m_regE);
                    break;

                case 0x2c: // SRA H
                    l_lr35902->m_regH =
                        lr35902_opcodeSra(l_lr35902, l_lr35902->m_regH);
                    break;

                case 0x2d: // SRA L
                    l_lr35902->m_regL =
                        lr35902_opcodeSra(l_lr35902, l_lr35902->m_regL);
                    break;

                case 0x2e: // SRA (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);

                    busWrite(
                        l_lr35902->m_bus,
                        l_tmpAddress,
                        lr35902_opcodeSra(
                            l_lr35902,
                            busRead(l_lr35902->m_bus, l_tmpAddress)
                        )
                    );
                    break;

                case 0x2f: // SRA A
                    l_lr35902->m_regA =
                        lr35902_opcodeSra(l_lr35902, l_lr35902->m_regA);
                    break;

                case 0x30: // SWAP B
                    l_lr35902->m_regB =
                        lr35902_opcodeSwap(l_lr35902, l_lr35902->m_regB);
                    break;

                case 0x31: // SWAP C
                    l_lr35902->m_regC =
                        lr35902_opcodeSwap(l_lr35902, l_lr35902->m_regC);
                    break;

                case 0x32: // SWAP D
                    l_lr35902->m_regD =
                        lr35902_opcodeSwap(l_lr35902, l_lr35902->m_regD);
                    break;

                case 0x33: // SWAP E
                    l_lr35902->m_regE =
                        lr35902_opcodeSwap(l_lr35902, l_lr35902->m_regE);
                    break;

                case 0x34: // SWAP H
                    l_lr35902->m_regH =
                        lr35902_opcodeSwap(l_lr35902, l_lr35902->m_regH);
                    break;

                case 0x35: // SWAP L
                    l_lr35902->m_regL =
                        lr35902_opcodeSwap(l_lr35902, l_lr35902->m_regL);
                    break;

                case 0x36: // SWAP (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    busWrite(
                        l_lr35902->m_bus,
                        l_tmpAddress,
                        lr35902_opcodeSwap(
                            l_lr35902,
                            busRead(l_lr35902->m_bus, l_tmpAddress)
                        )
                    );
                    break;

                case 0x37: // SWAP A
                    l_lr35902->m_regA =
                        lr35902_opcodeSwap(l_lr35902, l_lr35902->m_regA);
                    break;

                case 0x38: // SRL B
                    l_lr35902->m_regB =
                        lr35902_opcodeSrl(l_lr35902, l_lr35902->m_regB);
                    break;

                case 0x39: // SRL C
                    l_lr35902->m_regC =
                        lr35902_opcodeSrl(l_lr35902, l_lr35902->m_regC);
                    break;

                case 0x3a: // SRL D
                    l_lr35902->m_regD =
                        lr35902_opcodeSrl(l_lr35902, l_lr35902->m_regD);
                    break;

                case 0x3b: // SRL E
                    l_lr35902->m_regE =
                        lr35902_opcodeSrl(l_lr35902, l_lr35902->m_regE);
                    break;

                case 0x3c: // SRL H
                    l_lr35902->m_regH =
                        lr35902_opcodeSrl(l_lr35902, l_lr35902->m_regH);
                    break;

                case 0x3d: // SRL L
                    l_lr35902->m_regL =
                        lr35902_opcodeSrl(l_lr35902, l_lr35902->m_regL);
                    break;

                case 0x3e: // SRL (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);

                    busWrite(
                        l_lr35902->m_bus,
                        l_tmpAddress,
                        lr35902_opcodeSrl(
                            l_lr35902,
                            busRead(l_lr35902->m_bus, l_tmpAddress)
                        )
                    );
                    break;

                case 0x3f: // SRL A
                    l_lr35902->m_regA =
                        lr35902_opcodeSrl(l_lr35902, l_lr35902->m_regA);
                    break;

                case 0x40: // BIT 0, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regB & (1 << 0)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x41: // BIT 0, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regC & (1 << 0)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x42: // BIT 0, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regD & (1 << 0)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x43: // BIT 0, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regE & (1 << 0)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x44: // BIT 0, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regH & (1 << 0)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x45: // BIT 0, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regL & (1 << 0)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x46: // BIT 0, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (busRead(l_lr35902->m_bus, l_tmpAddress) & (1 << 0)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x47: // BIT 0, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regA & (1 << 0)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x48: // BIT 1, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regB & (1 << 1)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x49: // BIT 1, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regC & (1 << 1)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x4a: // BIT 1, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regD & (1 << 1)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x4b: // BIT 1, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regE & (1 << 1)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x4c: // BIT 1, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regH & (1 << 1)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x4d: // BIT 1, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regL & (1 << 1)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x4e: // BIT 1, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (busRead(l_lr35902->m_bus, l_tmpAddress) & (1 << 1)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x4f: // BIT 1, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regA & (1 << 1)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x50: // BIT 2, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regB & (1 << 2)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x51: // BIT 2, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regC & (1 << 2)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x52: // BIT 2, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regD & (1 << 2)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x53: // BIT 2, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regE & (1 << 2)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x54: // BIT 2, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regH & (1 << 2)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x55: // BIT 2, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regL & (1 << 2)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x56: // BIT 2, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (busRead(l_lr35902->m_bus, l_tmpAddress) & (1 << 2)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x57: // BIT 2, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regA & (1 << 2)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x58: // BIT 3, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regB & (1 << 3)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x59: // BIT 3, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regC & (1 << 3)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x5a: // BIT 3, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regD & (1 << 3)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x5b: // BIT 3, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regE & (1 << 3)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x5c: // BIT 3, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regH & (1 << 3)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x5d: // BIT 3, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regL & (1 << 3)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x5e: // BIT 3, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (busRead(l_lr35902->m_bus, l_tmpAddress) & (1 << 3)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x5f: // BIT 3, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regA & (1 << 3)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x60: // BIT 4, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regB & (1 << 4)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x61: // BIT 4, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regC & (1 << 4)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x62: // BIT 4, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regD & (1 << 4)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x63: // BIT 4, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regE & (1 << 4)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x64: // BIT 4, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regH & (1 << 4)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x65: // BIT 4, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regL & (1 << 4)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x66: // BIT 4, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (busRead(l_lr35902->m_bus, l_tmpAddress) & (1 << 4)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x67: // BIT 4, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regA & (1 << 4)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x68: // BIT 5, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regB & (1 << 5)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x69: // BIT 5, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regC & (1 << 5)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x6a: // BIT 5, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regD & (1 << 5)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x6b: // BIT 5, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regE & (1 << 5)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x6c: // BIT 5, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regH & (1 << 5)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x6d: // BIT 5, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regL & (1 << 5)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x6e: // BIT 5, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (busRead(l_lr35902->m_bus, l_tmpAddress) & (1 << 5)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x6f: // BIT 5, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regA & (1 << 5)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x70: // BIT 6, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regB & (1 << 6)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x71: // BIT 6, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regC & (1 << 6)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x72: // BIT 6, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regD & (1 << 6)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x73: // BIT 6, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regE & (1 << 6)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x74: // BIT 6, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regH & (1 << 6)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x75: // BIT 6, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regL & (1 << 6)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x76: // BIT 6, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (busRead(l_lr35902->m_bus, l_tmpAddress) & (1 << 6)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x77: // BIT 6, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regA & (1 << 6)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x78: // BIT 7, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regB & (1 << 7)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x79: // BIT 7, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regC & (1 << 7)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x7a: // BIT 7, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regD & (1 << 7)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x7b: // BIT 7, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regE & (1 << 7)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x7c: // BIT 7, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regH & (1 << 7)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x7d: // BIT 7, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regL & (1 << 7)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x7e: // BIT 7, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (busRead(l_lr35902->m_bus, l_tmpAddress) & (1 << 7)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x7f: // BIT 7, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_flagZ = (l_lr35902->m_regA & (1 << 7)) == 0;
                    l_lr35902->m_flagN = false;
                    l_lr35902->m_flagH = true;
                    break;

                case 0x80: // RES 0, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regB = l_lr35902->m_regB & ~(1 << 0);
                    break;

                case 0x81: // RES 0, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regC = l_lr35902->m_regC & ~(1 << 0);
                    break;

                case 0x82: // RES 0, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regD = l_lr35902->m_regD & ~(1 << 0);
                    break;

                case 0x83: // RES 0, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regE = l_lr35902->m_regE & ~(1 << 0);
                    break;

                case 0x84: // RES 0, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regH = l_lr35902->m_regH & ~(1 << 0);
                    break;

                case 0x85: // RES 0, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regL = l_lr35902->m_regL & ~(1 << 0);
                    break;

                case 0x86: // RES 0, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    busWrite(l_lr35902->m_bus, l_tmpAddress, busRead(l_lr35902->m_bus, l_tmpAddress) & ~(1 << 0));
                    break;

                case 0x87: // RES 0, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regA = l_lr35902->m_regA & ~(1 << 0);
                    break;

                case 0x88: // RES 1, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regB = l_lr35902->m_regB & ~(1 << 1);
                    break;

                case 0x89: // RES 1, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regC = l_lr35902->m_regC & ~(1 << 1);
                    break;

                case 0x8a: // RES 1, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regD = l_lr35902->m_regD & ~(1 << 1);
                    break;

                case 0x8b: // RES 1, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regE = l_lr35902->m_regE & ~(1 << 1);
                    break;

                case 0x8c: // RES 1, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regH = l_lr35902->m_regH & ~(1 << 1);
                    break;

                case 0x8d: // RES 1, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regL = l_lr35902->m_regL & ~(1 << 1);
                    break;

                case 0x8e: // RES 1, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    busWrite(l_lr35902->m_bus, l_tmpAddress, busRead(l_lr35902->m_bus, l_tmpAddress) & ~(1 << 1));
                    break;

                case 0x8f: // RES 1, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regA = l_lr35902->m_regA & ~(1 << 1);
                    break;

                case 0x90: // RES 2, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regB = l_lr35902->m_regB & ~(1 << 2);
                    break;

                case 0x91: // RES 2, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regC = l_lr35902->m_regC & ~(1 << 2);
                    break;

                case 0x92: // RES 2, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regD = l_lr35902->m_regD & ~(1 << 2);
                    break;

                case 0x93: // RES 2, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regE = l_lr35902->m_regE & ~(1 << 2);
                    break;

                case 0x94: // RES 2, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regH = l_lr35902->m_regH & ~(1 << 2);
                    break;

                case 0x95: // RES 2, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regL = l_lr35902->m_regL & ~(1 << 2);
                    break;

                case 0x96: // RES 2, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    busWrite(l_lr35902->m_bus, l_tmpAddress, busRead(l_lr35902->m_bus, l_tmpAddress) & ~(1 << 2));
                    break;

                case 0x97: // RES 2, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regA = l_lr35902->m_regA & ~(1 << 2);
                    break;

                case 0x98: // RES 3, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regB = l_lr35902->m_regB & ~(1 << 3);
                    break;

                case 0x99: // RES 3, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regC = l_lr35902->m_regC & ~(1 << 3);
                    break;

                case 0x9a: // RES 3, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regD = l_lr35902->m_regD & ~(1 << 3);
                    break;

                case 0x9b: // RES 3, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regE = l_lr35902->m_regE & ~(1 << 3);
                    break;

                case 0x9c: // RES 3, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regH = l_lr35902->m_regH & ~(1 << 3);
                    break;

                case 0x9d: // RES 3, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regL = l_lr35902->m_regL & ~(1 << 3);
                    break;

                case 0x9e: // RES 3, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    busWrite(l_lr35902->m_bus, l_tmpAddress, busRead(l_lr35902->m_bus, l_tmpAddress) & ~(1 << 3));
                    break;

                case 0x9f: // RES 3, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regA = l_lr35902->m_regA & ~(1 << 3);
                    break;

                case 0xa0: // RES 4, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regB = l_lr35902->m_regB & ~(1 << 4);
                    break;

                case 0xa1: // RES 4, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regC = l_lr35902->m_regC & ~(1 << 4);
                    break;

                case 0xa2: // RES 4, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regD = l_lr35902->m_regD & ~(1 << 4);
                    break;

                case 0xa3: // RES 4, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regE = l_lr35902->m_regE & ~(1 << 4);
                    break;

                case 0xa4: // RES 4, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regH = l_lr35902->m_regH & ~(1 << 4);
                    break;

                case 0xa5: // RES 4, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regL = l_lr35902->m_regL & ~(1 << 4);
                    break;

                case 0xa6: // RES 4, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    busWrite(l_lr35902->m_bus, l_tmpAddress, busRead(l_lr35902->m_bus, l_tmpAddress) & ~(1 << 4));
                    break;

                case 0xa7: // RES 4, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regA = l_lr35902->m_regA & ~(1 << 4);
                    break;

                case 0xa8: // RES 5, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regB = l_lr35902->m_regB & ~(1 << 5);
                    break;

                case 0xa9: // RES 5, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regC = l_lr35902->m_regC & ~(1 << 5);
                    break;

                case 0xaa: // RES 5, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regD = l_lr35902->m_regD & ~(1 << 5);
                    break;

                case 0xab: // RES 5, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regE = l_lr35902->m_regE & ~(1 << 5);
                    break;

                case 0xac: // RES 5, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regH = l_lr35902->m_regH & ~(1 << 5);
                    break;

                case 0xad: // RES 5, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regL = l_lr35902->m_regL & ~(1 << 5);
                    break;

                case 0xae: // RES 5, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    busWrite(l_lr35902->m_bus, l_tmpAddress, busRead(l_lr35902->m_bus, l_tmpAddress) & ~(1 << 5));
                    break;

                case 0xaf: // RES 5, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regA = l_lr35902->m_regA & ~(1 << 5);
                    break;

                case 0xb0: // RES 6, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regB = l_lr35902->m_regB & ~(1 << 6);
                    break;

                case 0xb1: // RES 6, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regC = l_lr35902->m_regC & ~(1 << 6);
                    break;

                case 0xb2: // RES 6, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regD = l_lr35902->m_regD & ~(1 << 6);
                    break;

                case 0xb3: // RES 6, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regE = l_lr35902->m_regE & ~(1 << 6);
                    break;

                case 0xb4: // RES 6, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regH = l_lr35902->m_regH & ~(1 << 6);
                    break;

                case 0xb5: // RES 6, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regL = l_lr35902->m_regL & ~(1 << 6);
                    break;

                case 0xb6: // RES 6, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    busWrite(l_lr35902->m_bus, l_tmpAddress, busRead(l_lr35902->m_bus, l_tmpAddress) & ~(1 << 6));
                    break;

                case 0xb7: // RES 6, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regA = l_lr35902->m_regA & ~(1 << 6);
                    break;

                case 0xb8: // RES 7, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regB = l_lr35902->m_regB & ~(1 << 7);
                    break;

                case 0xb9: // RES 7, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regC = l_lr35902->m_regC & ~(1 << 7);
                    break;

                case 0xba: // RES 7, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regD = l_lr35902->m_regD & ~(1 << 7);
                    break;

                case 0xbb: // RES 7, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regE = l_lr35902->m_regE & ~(1 << 7);
                    break;

                case 0xbc: // RES 7, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regH = l_lr35902->m_regH & ~(1 << 7);
                    break;

                case 0xbd: // RES 7, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regL = l_lr35902->m_regL & ~(1 << 7);
                    break;

                case 0xbe: // RES 7, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    busWrite(l_lr35902->m_bus, l_tmpAddress, busRead(l_lr35902->m_bus, l_tmpAddress) & ~(1 << 7));
                    break;

                case 0xbf: // RES 7, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regA = l_lr35902->m_regA & ~(1 << 7);
                    break;

                case 0xc0: // SET 0, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regB = l_lr35902->m_regB | (1 << 0);
                    break;

                case 0xc1: // SET 0, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regC = l_lr35902->m_regC | (1 << 0);
                    break;

                case 0xc2: // SET 0, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regD = l_lr35902->m_regD | (1 << 0);
                    break;

                case 0xc3: // SET 0, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regE = l_lr35902->m_regE | (1 << 0);
                    break;

                case 0xc4: // SET 0, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regH = l_lr35902->m_regH | (1 << 0);
                    break;

                case 0xc5: // SET 0, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regL = l_lr35902->m_regL | (1 << 0);
                    break;

                case 0xc6: // SET 0, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    busWrite(l_lr35902->m_bus, l_tmpAddress, busRead(l_lr35902->m_bus, l_tmpAddress) | (1 << 0));
                    break;

                case 0xc7: // SET 0, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regA = l_lr35902->m_regA | (1 << 0);
                    break;

                case 0xc8: // SET 1, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regB = l_lr35902->m_regB | (1 << 1);
                    break;

                case 0xc9: // SET 1, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regC = l_lr35902->m_regC | (1 << 1);
                    break;

                case 0xca: // SET 1, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regD = l_lr35902->m_regD | (1 << 1);
                    break;

                case 0xcb: // SET 1, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regE = l_lr35902->m_regE | (1 << 1);
                    break;

                case 0xcc: // SET 1, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regH = l_lr35902->m_regH | (1 << 1);
                    break;

                case 0xcd: // SET 1, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regL = l_lr35902->m_regL | (1 << 1);
                    break;

                case 0xce: // SET 1, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    busWrite(l_lr35902->m_bus, l_tmpAddress, busRead(l_lr35902->m_bus, l_tmpAddress) | (1 << 1));
                    break;

                case 0xcf: // SET 1, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regA = l_lr35902->m_regA | (1 << 1);
                    break;

                case 0xd0: // SET 2, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regB = l_lr35902->m_regB | (1 << 2);
                    break;

                case 0xd1: // SET 2, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regC = l_lr35902->m_regC | (1 << 2);
                    break;

                case 0xd2: // SET 2, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regD = l_lr35902->m_regD | (1 << 2);
                    break;

                case 0xd3: // SET 2, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regE = l_lr35902->m_regE | (1 << 2);
                    break;

                case 0xd4: // SET 2, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regH = l_lr35902->m_regH | (1 << 2);
                    break;

                case 0xd5: // SET 2, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regL = l_lr35902->m_regL | (1 << 2);
                    break;

                case 0xd6: // SET 2, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    busWrite(l_lr35902->m_bus, l_tmpAddress, busRead(l_lr35902->m_bus, l_tmpAddress) | (1 << 2));
                    break;

                case 0xd7: // SET 2, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regA = l_lr35902->m_regA | (1 << 2);
                    break;

                case 0xd8: // SET 3, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regB = l_lr35902->m_regB | (1 << 3);
                    break;

                case 0xd9: // SET 3, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regC = l_lr35902->m_regC | (1 << 3);
                    break;

                case 0xda: // SET 3, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regD = l_lr35902->m_regD | (1 << 3);
                    break;

                case 0xdb: // SET 3, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regE = l_lr35902->m_regE | (1 << 3);
                    break;

                case 0xdc: // SET 3, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regH = l_lr35902->m_regH | (1 << 3);
                    break;

                case 0xdd: // SET 3, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regL = l_lr35902->m_regL | (1 << 3);
                    break;

                case 0xde: // SET 3, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    busWrite(l_lr35902->m_bus, l_tmpAddress, busRead(l_lr35902->m_bus, l_tmpAddress) | (1 << 3));
                    break;

                case 0xdf: // SET 3, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regA = l_lr35902->m_regA | (1 << 3);
                    break;

                case 0xe0: // SET 4, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regB = l_lr35902->m_regB | (1 << 4);
                    break;

                case 0xe1: // SET 4, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regC = l_lr35902->m_regC | (1 << 4);
                    break;

                case 0xe2: // SET 4, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regD = l_lr35902->m_regD | (1 << 4);
                    break;

                case 0xe3: // SET 4, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regE = l_lr35902->m_regE | (1 << 4);
                    break;

                case 0xe4: // SET 4, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regH = l_lr35902->m_regH | (1 << 4);
                    break;

                case 0xe5: // SET 4, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regL = l_lr35902->m_regL | (1 << 4);
                    break;

                case 0xe6: // SET 4, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    busWrite(l_lr35902->m_bus, l_tmpAddress, busRead(l_lr35902->m_bus, l_tmpAddress) | (1 << 4));
                    break;

                case 0xe7: // SET 4, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regA = l_lr35902->m_regA | (1 << 4);
                    break;

                case 0xe8: // SET 5, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regB = l_lr35902->m_regB | (1 << 5);
                    break;

                case 0xe9: // SET 5, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regC = l_lr35902->m_regC | (1 << 5);
                    break;

                case 0xea: // SET 5, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regD = l_lr35902->m_regD | (1 << 5);
                    break;

                case 0xeb: // SET 5, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regE = l_lr35902->m_regE | (1 << 5);
                    break;

                case 0xec: // SET 5, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regH = l_lr35902->m_regH | (1 << 5);
                    break;

                case 0xed: // SET 5, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regL = l_lr35902->m_regL | (1 << 5);
                    break;

                case 0xee: // SET 5, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    busWrite(l_lr35902->m_bus, l_tmpAddress, busRead(l_lr35902->m_bus, l_tmpAddress) | (1 << 5));
                    break;

                case 0xef: // SET 5, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regA = l_lr35902->m_regA | (1 << 5);
                    break;

                case 0xf0: // SET 6, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regB = l_lr35902->m_regB | (1 << 6);
                    break;

                case 0xf1: // SET 6, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regC = l_lr35902->m_regC | (1 << 6);
                    break;

                case 0xf2: // SET 6, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regD = l_lr35902->m_regD | (1 << 6);
                    break;

                case 0xf3: // SET 6, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regE = l_lr35902->m_regE | (1 << 6);
                    break;

                case 0xf4: // SET 6, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regH = l_lr35902->m_regH | (1 << 6);
                    break;

                case 0xf5: // SET 6, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regL = l_lr35902->m_regL | (1 << 6);
                    break;

                case 0xf6: // SET 6, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    busWrite(l_lr35902->m_bus, l_tmpAddress, busRead(l_lr35902->m_bus, l_tmpAddress) | (1 << 6));
                    break;

                case 0xf7: // SET 6, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regA = l_lr35902->m_regA | (1 << 6);
                    break;

                case 0xf8: // SET 7, B
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regB = l_lr35902->m_regB | (1 << 7);
                    break;

                case 0xf9: // SET 7, C
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regC = l_lr35902->m_regC | (1 << 7);
                    break;

                case 0xfa: // SET 7, D
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regD = l_lr35902->m_regD | (1 << 7);
                    break;

                case 0xfb: // SET 7, E
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regE = l_lr35902->m_regE | (1 << 7);
                    break;

                case 0xfc: // SET 7, H
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regH = l_lr35902->m_regH | (1 << 7);
                    break;

                case 0xfd: // SET 7, L
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regL = l_lr35902->m_regL | (1 << 7);
                    break;

                case 0xfe: // SET 7, (HL)
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    busWrite(l_lr35902->m_bus, l_tmpAddress, busRead(l_lr35902->m_bus, l_tmpAddress) | (1 << 7));
                    break;

                case 0xff: // SET 7, A
                    l_tmpAddress = lr35902_getHL(l_lr35902);
                    l_lr35902->m_regA = l_lr35902->m_regA | (1 << 7);
                    break;
            }

            break;

        case 0xcc: // CALL Z, a16
            lr35902_opcodeCallCondition(l_lr35902, l_lr35902->m_flagZ);
            break;
        
        case 0xcd: // CALL a16
            lr35902_opcodeCallCondition(l_lr35902, true);
            break;

        case 0xce: // ADC A, d8
            lr35902_opcodeAdc8(l_lr35902, lr35902_fetch8(l_lr35902));
            break;

        case 0xcf: // RST 0x08
            lr35902_opcodeRst(l_lr35902, 0x08);
            break;
        
        case 0xd0: // RET NC
            lr35902_opcodeRetCondition(l_lr35902, !l_lr35902->m_flagC);
            break;

        case 0xd1: // POP DE
            lr35902_setDE(l_lr35902, lr35902_opcodePop(l_lr35902));
            break;

        case 0xd2: // JP NC, a16
            lr35902_opcodeJpCondition(l_lr35902, !l_lr35902->m_flagC);
            break;

        case 0xd4: // CALL NC, a16
            lr35902_opcodeCallCondition(l_lr35902, !l_lr35902->m_flagC);
            break;

        case 0xd5: // PUSH DE
            lr35902_opcodePush(l_lr35902, lr35902_getDE(l_lr35902));
            break;

        case 0xd6: // SUB A, d8
            lr35902_opcodeSub8(l_lr35902, lr35902_fetch8(l_lr35902));
            break;
        
        case 0xd7: // RST 0x10
            lr35902_opcodeRst(l_lr35902, 0x10);
            break;

        case 0xd8: // RET C
            lr35902_opcodeRetCondition(l_lr35902, l_lr35902->m_flagC);
            break;

        case 0xd9: // RETI
            l_lr35902->m_regPC = lr35902_opcodePop(l_lr35902);
            busCycle(l_lr35902->m_bus);
            // TODO: Enable interrupts
            break;

        case 0xda: // JP C, a16
            lr35902_opcodeJpCondition(l_lr35902, l_lr35902->m_flagC);
            break;

        case 0xdc: // CALL C, a16
            lr35902_opcodeCallCondition(l_lr35902, l_lr35902->m_flagC);
            break;

        case 0xde: // SBC A, d8
            lr35902_opcodeSbc8(l_lr35902, lr35902_fetch8(l_lr35902));
            break;

        case 0xdf: // RST 0x18
            lr35902_opcodeRst(l_lr35902, 0x18);
            break;

        case 0xe0: // LDH (a8), A
            busWrite(
                l_lr35902->m_bus,
                0xff00 | lr35902_fetch8(l_lr35902),
                l_lr35902->m_regA
            );
            break;

        case 0xe1: // POP HL
            lr35902_setHL(l_lr35902, lr35902_opcodePop(l_lr35902));
            break;

        case 0xe2: // LDH (C), A
            busWrite(
                l_lr35902->m_bus,
                0xff00 | l_lr35902->m_regC,
                l_lr35902->m_regA
            );
            break;

        case 0xe6: // AND d8
            lr35902_opcodeAnd8(l_lr35902, lr35902_fetch8(l_lr35902));
            break;

        case 0xe7: // RST 0x20
            lr35902_opcodeRst(l_lr35902, 0x20);
            break;

        case 0xe8: // ADD SP, r8
            l_lr35902->m_flagZ = false;
            l_lr35902->m_flagN = false;

            l_tmpData = lr35902_fetch8(l_lr35902);

            l_lr35902->m_flagH = (
                (l_lr35902->m_regSP & 0x0f) + (l_tmpData & 0x0f)
            ) >= 0x10;
            l_lr35902->m_flagC = (
                (l_lr35902->m_regSP & 0xff) + l_tmpData
            ) >= 0x100;

            l_lr35902->m_regSP += (int8_t)l_tmpData;

            busCycle(l_lr35902->m_bus);
            busCycle(l_lr35902->m_bus);

            break;
        
        case 0xe9: // JP HL
            l_lr35902->m_regPC = lr35902_getHL(l_lr35902);
            break;

        case 0xea: // LD (a16), A
            busWrite(
                l_lr35902->m_bus,
                lr35902_fetch16(l_lr35902),
                l_lr35902->m_regA
            );
            break;

        case 0xee: // XOR d8
            lr35902_opcodeXor8(l_lr35902, lr35902_fetch8(l_lr35902));
            break;

        case 0xef: // RST 0x28
            lr35902_opcodeRst(l_lr35902, 0x28);
            break;

        case 0xf0: // LDH A, (a8)
            l_lr35902->m_regA = busRead(
                l_lr35902->m_bus,
                0xff00 | lr35902_fetch8(l_lr35902)
            );
            break;

        case 0xf1: // POP AF
            lr35902_setAF(l_lr35902, lr35902_opcodePop(l_lr35902));
            break;

        case 0xf2: // LDH A, (C)
            l_lr35902->m_regA = busRead(
                l_lr35902->m_bus,
                0xff00 | lr35902_fetch8(l_lr35902)
            );
            break;

        case 0xf3: // DI
            // TODO: disable interrupts
            break;

        case 0xf5: // PUSH AF
            lr35902_opcodePush(l_lr35902, lr35902_getAF(l_lr35902));
            break;

        case 0xf6: // OR d8
            lr35902_opcodeOr8(l_lr35902, lr35902_fetch8(l_lr35902));
            break;
        
        case 0xf7: // RST 0x30
            lr35902_opcodeRst(l_lr35902, 0x30);
            break;

        case 0xf8: // LD HL, SP + r8
            l_lr35902->m_flagZ = false;
            l_lr35902->m_flagN = false;

            busCycle(l_lr35902->m_bus);

            l_tmpData = lr35902_fetch8(l_lr35902);

            l_lr35902->m_flagH = (
                (l_lr35902->m_regSP & 0x0f) + (l_tmpData & 0x0f)
            ) >= 0x10;
            l_lr35902->m_flagC = (
                (l_lr35902->m_regSP & 0xff) + l_tmpData
            ) >= 0x100;

            lr35902_setHL(l_lr35902, l_lr35902->m_regSP + (int8_t)l_tmpData);

            break;

        case 0xf9: // LD SP, HL
            busCycle(l_lr35902->m_bus);
            break;

        case 0xfa: // LD A, (a16)
            l_lr35902->m_regA = busRead(
                l_lr35902->m_bus,
                lr35902_fetch16(l_lr35902)
            );
            break;

        case 0xfb: // EI
            // TODO: enable interrupts
            break;

        case 0xfe: // CP d8
            lr35902_opcodeCp(l_lr35902, lr35902_fetch8(l_lr35902));
            break;

        case 0xff: // RST 0x38
            lr35902_opcodeRst(l_lr35902, 0x38);
            break;
    }
}

static uint8_t lr35902_fetch8(struct ts_lr35902 *p_lr35902) {
    return p_lr35902->m_bus->m_read(p_lr35902->m_bus, p_lr35902->m_regPC++);
}

static uint16_t lr35902_fetch16(struct ts_lr35902 *p_lr35902) {
    uint16_t l_returnValue = lr35902_fetch8(p_lr35902);
    l_returnValue |= lr35902_fetch8(p_lr35902) << 8;

    return l_returnValue;
}

static inline void lr35902_write16(
    struct ts_lr35902 *p_lr35902,
    uint16_t p_address,
    uint16_t p_data
) {
    p_lr35902->m_bus->m_write(p_lr35902->m_bus, p_address, p_data);
    p_lr35902->m_bus->m_write(p_lr35902->m_bus, p_address + 1, p_data >> 8);
}

static inline uint16_t lr35902_getAF(struct ts_lr35902 *p_lr35902) {
    return (p_lr35902->m_regA << 8) | lr35902_getF(p_lr35902);
}

static inline uint16_t lr35902_getBC(struct ts_lr35902 *p_lr35902) {
    return (p_lr35902->m_regB << 8) | p_lr35902->m_regC;
}

static inline uint16_t lr35902_getDE(struct ts_lr35902 *p_lr35902) {
    return (p_lr35902->m_regD << 8) | p_lr35902->m_regE;
}

static inline uint16_t lr35902_getHL(struct ts_lr35902 *p_lr35902) {
    return (p_lr35902->m_regH << 8) | p_lr35902->m_regL;
}

static inline void lr35902_setAF(
    struct ts_lr35902 *p_lr35902,
    uint16_t p_value
) {
    lr35902_setF(p_lr35902, p_value);
    p_lr35902->m_regA = p_value >> 8;
}

static inline void lr35902_setBC(
    struct ts_lr35902 *p_lr35902,
    uint16_t p_value
) {
    p_lr35902->m_regC = p_value;
    p_lr35902->m_regB = p_value >> 8;
}

static inline void lr35902_setDE(
    struct ts_lr35902 *p_lr35902,
    uint16_t p_value
) {
    p_lr35902->m_regE = p_value;
    p_lr35902->m_regD = p_value >> 8;
}

static inline void lr35902_setHL(
    struct ts_lr35902 *p_lr35902,
    uint16_t p_value
) {
    p_lr35902->m_regL = p_value;
    p_lr35902->m_regH = p_value >> 8;
}

static inline uint8_t lr35902_getF(struct ts_lr35902 *p_lr35902) {
    uint8_t l_returnValue = 0x00;

    if(p_lr35902->m_flagZ) {
        l_returnValue |= 0x80;
    }

    if(p_lr35902->m_flagN) {
        l_returnValue |= 0x40;
    }

    if(p_lr35902->m_flagH) {
        l_returnValue |= 0x20;
    }

    if(p_lr35902->m_flagC) {
        l_returnValue |= 0x10;
    }

    return l_returnValue;
}

static inline void lr35902_setF(struct ts_lr35902 *p_lr35902, uint8_t p_value) {
    p_lr35902->m_flagZ = (p_value & 0x80) != 0;
    p_lr35902->m_flagN = (p_value & 0x40) != 0;
    p_lr35902->m_flagH = (p_value & 0x20) != 0;
    p_lr35902->m_flagC = (p_value & 0x10) != 0;
}

static inline void lr35902_opcodeAdc8(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
) {
    uint8_t l_carry = p_lr35902->m_flagC ? 1 : 0;

    p_lr35902->m_flagN = false;
    p_lr35902->m_flagH = 
        ((p_lr35902->m_regA & 0x0f) + (p_value & 0x0f) + l_carry) >= 0x10;
    p_lr35902->m_flagC = (p_lr35902->m_regA + p_value + l_carry) >= 0x100;

    p_lr35902->m_regA += p_value + l_carry;

    p_lr35902->m_flagZ = p_lr35902->m_regA == 0;
}

static inline void lr35902_opcodeAdd8(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
) {
    p_lr35902->m_flagN = false;
    p_lr35902->m_flagH = 
        ((p_lr35902->m_regA & 0x0f) + (p_value & 0x0f)) >= 0x10;
    p_lr35902->m_flagC = (p_lr35902->m_regA + p_value) >= 0x100;

    p_lr35902->m_regA += p_value;

    p_lr35902->m_flagZ = p_lr35902->m_regA == 0;
}

static inline void lr35902_opcodeAddHL(
    struct ts_lr35902 *p_lr35902,
    uint16_t p_value
) {
    uint16_t l_value = lr35902_getHL(p_lr35902);

    p_lr35902->m_flagN = false;
    p_lr35902->m_flagH = ((l_value & 0x0fff) + (p_value & 0x0fff)) >= 0x1000;
    p_lr35902->m_flagC = (l_value + p_value) >= 0x10000;

    lr35902_setHL(p_lr35902, l_value + p_value);
}

static inline void lr35902_opcodeAnd8(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
) {
    p_lr35902->m_regA &= p_value;

    p_lr35902->m_flagZ = p_lr35902->m_regA == 0;
    p_lr35902->m_flagN = false;
    p_lr35902->m_flagH = true;
    p_lr35902->m_flagC = false;
}

static inline void lr35902_opcodeCallCondition(
    struct ts_lr35902 *p_lr35902,
    bool p_condition
) {
    uint16_t l_address = lr35902_fetch16(p_lr35902);
    
    if(p_condition) {
        busCycle(p_lr35902->m_bus);
        lr35902_opcodePush(p_lr35902, p_lr35902->m_regPC);
        p_lr35902->m_regPC = l_address;
    }
}

static inline void lr35902_callCondition(
    struct ts_lr35902 *p_lr35902,
    bool p_condition
) {
    uint16_t l_address = lr35902_fetch16(p_lr35902);

    if(p_condition) {
        lr35902_opcodePush(p_lr35902, p_lr35902->m_regPC);
        p_lr35902->m_regPC = l_address;
    }
}

static inline void lr35902_opcodeCp(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
) {
    p_lr35902->m_flagN = true;
    p_lr35902->m_flagH = (p_lr35902->m_regA & 0x0f) < (p_value & 0x0f);
    p_lr35902->m_flagC = p_lr35902->m_regA < p_value;
    p_lr35902->m_flagZ = p_lr35902->m_regA == p_value;
}

static inline uint8_t lr35902_opcodeDec8(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
) {
    p_value--;

    p_lr35902->m_flagZ = p_value == 0;
    p_lr35902->m_flagN = true;
    p_lr35902->m_flagH = (p_value & 0x0f) == 0;

    return p_value;
}

static inline uint8_t lr35902_opcodeInc8(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
) {
    p_value++;

    p_lr35902->m_flagZ = p_value == 0;
    p_lr35902->m_flagN = false;
    p_lr35902->m_flagH = (p_value & 0x0f) == 0;

    return p_value;
}

static inline void lr35902_opcodeJr(
    struct ts_lr35902 *p_lr35902,
    bool p_condition
) {
    int8_t l_offset = lr35902_fetch8(p_lr35902);

    if(p_condition) {
        p_lr35902->m_regPC += l_offset;
        busCycle(p_lr35902->m_bus);
    }
}

static inline void lr35902_opcodeJpCondition(
    struct ts_lr35902 *p_lr35902,
    bool p_condition
) {
    uint16_t l_destination = lr35902_fetch16(p_lr35902);

    if(p_condition) {
        busCycle(p_lr35902->m_bus);
        p_lr35902->m_regPC = l_destination;
    }
}

static inline void lr35902_opcodeOr8(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
) {
    p_lr35902->m_regA |= p_value;

    p_lr35902->m_flagZ = p_lr35902->m_regA == 0;
    p_lr35902->m_flagN = false;
    p_lr35902->m_flagH = false;
    p_lr35902->m_flagC = false;
}

static inline uint16_t lr35902_opcodePop(struct ts_lr35902 *p_lr35902) {
    uint16_t l_result = busRead(p_lr35902->m_bus, p_lr35902->m_regSP++);
    l_result |= busRead(p_lr35902->m_bus, p_lr35902->m_regSP++) << 8;

    return l_result;
}

static inline void lr35902_opcodePush(
    struct ts_lr35902 *p_lr35902,
    uint16_t p_value
) {
    busCycle(p_lr35902->m_bus);
    busWrite(p_lr35902->m_bus, --p_lr35902->m_regSP, p_value >> 8);
    busWrite(p_lr35902->m_bus, p_lr35902->m_regSP, p_value);
}

static inline void lr35902_opcodeRetCondition(
    struct ts_lr35902 *p_lr35902,
    bool p_condition
) {
    busCycle(p_lr35902->m_bus);

    if(p_condition) {
        p_lr35902->m_regPC = lr35902_opcodePop(p_lr35902);
        busCycle(p_lr35902->m_bus);
    }
}

static inline uint8_t lr35902_opcodeRlBase(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
) {
    uint8_t l_carry = p_lr35902->m_flagC ? 1 : 0;

    p_lr35902->m_flagN = false;
    p_lr35902->m_flagH = false;
    p_lr35902->m_flagC = (p_value & 0x80) != 0;

    return (p_value << 1) | l_carry;
}

static inline uint8_t lr35902_opcodeRlcBase(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
) {
    p_lr35902->m_flagN = false;
    p_lr35902->m_flagH = false;
    p_lr35902->m_flagC = (p_value & 0x80) != 0;

    return (p_value << 1) | (p_lr35902->m_flagC ? 1 : 0);
}

static inline uint8_t lr35902_opcodeRrBase(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
) {
    uint8_t l_carry = p_lr35902->m_flagC ? 0x80 : 0;

    p_lr35902->m_flagN = false;
    p_lr35902->m_flagH = false;
    p_lr35902->m_flagC = (p_value & 0x01) != 0;

    return (p_value >> 1) | l_carry;
}

static inline uint8_t lr35902_opcodeRrcBase(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
) {
    p_lr35902->m_flagN = false;
    p_lr35902->m_flagH = false;
    p_lr35902->m_flagC = (p_value & 0x01) != 0;

    return (p_value >> 1) | (p_lr35902->m_flagC ? 0x80 : 0);
}

static inline void lr35902_opcodeRst(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
) {
    lr35902_opcodePush(p_lr35902, p_lr35902->m_regSP);
    p_lr35902->m_regPC = p_value;
}

static inline void lr35902_opcodeSbc8(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
) {
    uint8_t l_carry = p_lr35902->m_flagC ? 1 : 0;

    p_lr35902->m_flagN = true;
    p_lr35902->m_flagH =
        ((p_lr35902->m_regA & 0x0f) - (p_value & 0x0f) - l_carry) < 0;
    p_lr35902->m_flagC =
        (p_lr35902->m_regA - p_value - l_carry) < 0;

    p_lr35902->m_regA -= p_value + l_carry;

    p_lr35902->m_flagZ = p_lr35902->m_regA == 0;
}

static inline uint8_t lr35902_opcodeSla(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
) {
    p_lr35902->m_flagH = false;
    p_lr35902->m_flagN = false;
    p_lr35902->m_flagC = (p_value & 0x80) != 0;

    p_value <<= 1;

    p_lr35902->m_flagZ = p_value == 0;

    return p_value;
}

static inline uint8_t lr35902_opcodeSra(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
) {
    p_lr35902->m_flagH = false;
    p_lr35902->m_flagN = false;
    p_lr35902->m_flagC = (p_value & 0x80) != 0;

    p_value = ((int8_t)p_value) >> 1;

    p_lr35902->m_flagZ = p_value == 0;

    return p_value;
}

static inline uint8_t lr35902_opcodeSrl(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
) {
    p_lr35902->m_flagH = false;
    p_lr35902->m_flagN = false;
    p_lr35902->m_flagC = (p_value & 0x80) != 0;

    p_value = p_value >> 1;

    p_lr35902->m_flagZ = p_value == 0;

    return p_value;
}

static inline void lr35902_opcodeSub8(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
) {
    p_lr35902->m_flagN = true;
    p_lr35902->m_flagH = (p_lr35902->m_regA & 0x0f) < (p_value & 0x0f);
    p_lr35902->m_flagC = p_lr35902->m_regA < p_value;
    
    p_lr35902->m_regA -= p_value;

    p_lr35902->m_flagZ = p_lr35902->m_regA == 0;
}

static inline uint8_t lr35902_opcodeSwap(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
) {
    p_lr35902->m_flagZ = p_value == 0;
    p_lr35902->m_flagN = false;
    p_lr35902->m_flagH = false;
    p_lr35902->m_flagC = false;

    return ((p_value & 0x0f) << 4) | (p_value >> 4);
}

static inline void lr35902_opcodeXor8(
    struct ts_lr35902 *p_lr35902,
    uint8_t p_value
) {
    p_lr35902->m_regA ^= p_value;

    p_lr35902->m_flagZ = p_lr35902->m_regA == 0;
    p_lr35902->m_flagN = false;
    p_lr35902->m_flagH = false;
    p_lr35902->m_flagC = false;
}
