#include "component/serial/mos6551.h"

#define C_MOS6551_FIFO_SIZE 1

enum te_mos6551_register {
    E_MOS6551_REGISTER_DATA,
    E_MOS6551_REGISTER_STATUS,
    E_MOS6551_REGISTER_COMMAND,
    E_MOS6551_REGISTER_CONTROL
};

static int mos6551_fifoRead(
    struct ts_serial *p_serial,
    uint8_t *p_buffer,
    size_t p_size
);
static int mos6551_fifoWrite(
    struct ts_serial *p_serial,
    const uint8_t *p_buffer,
    size_t p_size
);

int mos6551_init(struct ts_mos6551 *p_serial) {
    p_serial->m_serial.m_read = mos6551_fifoRead;
    p_serial->m_serial.m_write = mos6551_fifoWrite;
    
    int l_returnValue = fifo_init(&p_serial->m_fifoRead, C_MOS6551_FIFO_SIZE);

    if(l_returnValue != 0) {
        return l_returnValue;
    }

    return fifo_init(&p_serial->m_fifoWrite, C_MOS6551_FIFO_SIZE);
}

t_busData mos6551_read(struct ts_mos6551 *p_serial, t_busAddress p_address) {
    switch(p_address & 0x0003) {
        case E_MOS6551_REGISTER_DATA:
            p_serial->m_regStatus &= ~(1 << 3);
            return p_serial->m_regRxData;

        case E_MOS6551_REGISTER_STATUS:
            return p_serial->m_regStatus;

        case E_MOS6551_REGISTER_COMMAND:
            return p_serial->m_regCommand;

        case E_MOS6551_REGISTER_CONTROL:
            return p_serial->m_regControl;

        default:
            return 0xff;
    }
}

void mos6551_write(
    struct ts_mos6551 *p_serial,
    t_busAddress p_address,
    t_busData p_data
) {
    switch(p_address & 0x0003) {
        case E_MOS6551_REGISTER_DATA:
            fifo_tryWrite(&p_serial->m_fifoRead, (void *)(size_t)p_data);
            p_serial->m_regStatus &= ~(1 << 4);
            break;

        case E_MOS6551_REGISTER_STATUS:
            // TODO: reset
            break;

        case E_MOS6551_REGISTER_COMMAND:
            p_serial->m_regCommand = p_data;
            break;

        case E_MOS6551_REGISTER_CONTROL:
            p_serial->m_regControl = p_data;
            break;
    }
}

void mos6551_cycle(struct ts_mos6551 *p_serial) {
    if((p_serial->m_regStatus & (1 << 3))) {
        return;
    }

    if(fifo_getCount(&p_serial->m_fifoRead) != C_MOS6551_FIFO_SIZE) {
        p_serial->m_regStatus |= 1 << 4;
    }

    void *l_buffer;
    int l_returnValue = fifo_tryRead(&p_serial->m_fifoWrite, &l_buffer);

    if(l_returnValue != 1) {
        return;
    }

    p_serial->m_regRxData = (size_t)l_buffer;
    p_serial->m_regStatus |= 1 << 3;
}

static int mos6551_fifoRead(
    struct ts_serial *p_serial,
    uint8_t *p_buffer,
    size_t p_size
) {
    struct ts_mos6551 *l_serial = (struct ts_mos6551 *)p_serial;
    void *l_buffer;

    size_t l_index = 0;

    while(l_index < p_size) {
        int l_returnValue = fifo_tryRead(&l_serial->m_fifoRead, &l_buffer);

        if(l_returnValue <= 0) {
            break;
        }

        p_buffer[l_index++] = (uint8_t)(size_t)l_buffer;
    }

    return l_index;
}

static int mos6551_fifoWrite(
    struct ts_serial *p_serial,
    const uint8_t *p_buffer,
    size_t p_size
) {
    struct ts_mos6551 *l_serial = (struct ts_mos6551 *)p_serial;
    void *l_buffer;

    size_t l_index = 0;

    while(l_index < p_size) {
        l_buffer = (void *)(size_t)p_buffer[l_index];

        int l_returnValue = fifo_tryWrite(
            &l_serial->m_fifoRead,
            (void *)l_buffer
        );

        if(l_returnValue <= 0) {
            break;
        }

        l_index++;
    }

    return l_index;
}
