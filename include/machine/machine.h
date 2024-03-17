#ifndef __INCLUDE_MACHINE_MACHINE_H__
#define __INCLUDE_MACHINE_MACHINE_H__

struct ts_machine {
    void (*m_step)(struct ts_machine *p_machine);
};

static inline void machineStep(struct ts_machine *p_machine) {
    p_machine->m_step(p_machine);
}

#endif
