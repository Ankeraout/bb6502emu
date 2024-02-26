#include "machine/bb6502.h"

static void bb6502_step(struct ts_machine *p_machine);

void bb6502_init(
    struct ts_bb6502 *p_machine,
    const void *p_rom,
    size_t p_size
) {
    p_machine->m_machine.m_step = bb6502_step;

    ram32k_init(&p_machine->m_ram);
    rom32k_init(&p_machine->m_rom, p_rom, p_size);
    busBB6502_init(
        &p_machine->m_bus,
        &p_machine->m_rom,
        &p_machine->m_ram
    );
    nmos6502_init(&p_machine->m_cpu, &p_machine->m_bus.m_bus);
}

static void bb6502_step(struct ts_machine *p_machine) {
    struct ts_bb6502 *l_machine = (struct ts_bb6502 *)p_machine;
    l_machine->m_cpu.m_cpu.m_step(&l_machine->m_cpu.m_cpu);
}