#ifndef __INCLUDE_COMPONENT_CPU_CPU_H__
#define __INCLUDE_COMPONENT_CPU_CPU_H__

struct ts_cpu {
    void (*m_step)(struct ts_cpu *p_cpu);
    void (*m_reset)(struct ts_cpu *p_cpu);
};

#endif
