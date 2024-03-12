#include <stdio.h>
#include <stdlib.h>

#include "machine/bb6502.h"

static void bb6502_step(struct ts_machine *p_machine);

static const char *s_data = "FFF0.FFFF\r";
static int s_index = 0;

struct ts_bb6502 *bb6502_init(
    struct ts_commandLineOptions *p_commandLineOptions
) {
    if(p_commandLineOptions->m_romFileName == NULL) {
        fprintf(stderr, "Error: bb6502 machine requires a ROM file.\n");
        return NULL;
    }

    uint8_t l_romData[32768];

    // Load ROM file
    FILE *l_file = fopen(p_commandLineOptions->m_romFileName, "rb");

    if(l_file == NULL) {
        fprintf(stderr, "Error: Failed to open file.\n");
        return NULL;
    }

    if(fread(l_romData, 1, 32768, l_file) != 32768) {
        fprintf(stderr, "Error: failed to read file.\n");
        return NULL;
    }

    fclose(l_file);

    struct ts_bb6502 *l_machine = malloc(sizeof(struct ts_bb6502));

    if(l_machine == NULL) {
        fprintf(stderr, "Failed to allocate memory for the machine.");
        return NULL;
    }

    l_machine->m_machine.m_step = bb6502_step;

    ram32k_init(&l_machine->m_ram);
    rom32k_init(&l_machine->m_rom, l_romData, 32768);
    mos6551_init(&l_machine->m_serial);
    busBB6502_init(
        &l_machine->m_bus,
        &l_machine->m_rom,
        &l_machine->m_ram,
        &l_machine->m_serial
    );
    nmos6502_init(&l_machine->m_cpu, &l_machine->m_bus.m_bus);

    return l_machine;
}

static void bb6502_step(struct ts_machine *p_machine) {
    struct ts_bb6502 *l_machine = (struct ts_bb6502 *)p_machine;
    l_machine->m_cpu.m_cpu.m_step(&l_machine->m_cpu.m_cpu);

    int l_serialData = l_machine->m_serial.m_serial.m_receive(
        &l_machine->m_serial.m_serial
    );

    if(l_serialData >= 0) {
        putchar(l_serialData);
    }

    if(s_data[s_index] != 0) {
        int l_returnValue = l_machine->m_serial.m_serial.m_send(
            &l_machine->m_serial.m_serial,
            s_data[s_index]
        );

        if(l_returnValue == 0) {
            s_index++;
        }
    }
}
