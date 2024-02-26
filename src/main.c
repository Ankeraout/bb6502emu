#include <stdio.h>

#include "machine/bb6502.h"

static int init(struct ts_bb6502 *p_machine);

int main(void) {
    struct ts_bb6502 l_machine;

    if(init(&l_machine) != 0) {
        printf("Failed to initialize machine.\n");
        return 1;
    }

    printf("Machine initialized successfully.\n");

    for(int l_i = 0; l_i < 150; l_i++) {
        printf(
            "%03d: PC=%04x SP=01%02x A=%02x X=%02x Y=%02x\n",
            l_i,
            l_machine.m_cpu.m_regPC,
            l_machine.m_cpu.m_regSP,
            l_machine.m_cpu.m_regA,
            l_machine.m_cpu.m_regX,
            l_machine.m_cpu.m_regY
        );

        l_machine.m_machine.m_step(&l_machine);
    }

    return 0;
}

static int init(struct ts_bb6502 *p_machine) {
    uint8_t l_romData[32768];

    // Load ROM file
    FILE *l_file = fopen("samples/fibonacci/bin/fibonacci.bin", "rb");

    if(l_file == NULL) {
        printf("Failed to open file.\n");
        return 1;
    }

    if(fread(l_romData, 1, 32768, l_file) != 32768) {
        printf("Failed to read file.\n");
        return 1;
    }

    fclose(l_file);

    bb6502_init(p_machine, l_romData, 32768);

    return 0;
}
