#include "component/serial/mos6551.h"

#define C_FLAG_STATUS_PARITY_ERROR (1 << 0)
#define C_FLAG_STATUS_FRAMING_ERROR (1 << 1)
#define C_FLAG_STATUS_OVERRUN (1 << 2)
#define C_FLAG_STATUS_RX_FULL (1 << 3)
#define C_FLAG_STATUS_TX_EMPTY (1 << 4)
#define C_FLAG_STATUS_CARRIER_DETECT (1 << 5)
#define C_FLAG_STATUS_DATA_SET_READY (1 << 6)
#define C_FLAG_STATUS_INTERRUPT (1 << 7)

#define C_FLAG_COMMAND_ENABLE (1 << 0)
#define C_FLAG_COMMAND_DISABLE_IRQ (1 << 1)
#define C_FLAG_COMMAND_ECHO (1 << 4)

#define C_FLAG_CONTROL_CLOCK_GENERATOR (1 << 4)

enum te_mos6551_register {
    E_MOS6551_REGISTER_DATA,
    E_MOS6551_REGISTER_STATUS,
    E_MOS6551_REGISTER_COMMAND,
    E_MOS6551_REGISTER_CONTROL
};

static int mos6551_send(struct ts_serial *p_serial, uint8_t p_data);
static int mos6551_receive(struct ts_serial *p_serial);

int mos6551_init(struct ts_mos6551 *p_mos6551) {
    p_mos6551->m_serial.m_send = mos6551_send;
    p_mos6551->m_serial.m_receive = mos6551_receive;

    p_mos6551->m_regStatus = C_FLAG_STATUS_TX_EMPTY;
    p_mos6551->m_regCommand = C_FLAG_COMMAND_DISABLE_IRQ;
    p_mos6551->m_regControl = 0x00;

    return 0;
}

static int mos6551_send(struct ts_serial *p_serial, uint8_t p_data) {
    struct ts_mos6551 *l_mos6551 = (struct ts_mos6551 *)p_serial;
    int l_returnValue;

    if((l_mos6551->m_regStatus & C_FLAG_STATUS_RX_FULL) != 0) {
        l_mos6551->m_regStatus |= C_FLAG_STATUS_OVERRUN;
        l_returnValue = 1;
    } else {
        l_mos6551->m_regStatus |= C_FLAG_STATUS_RX_FULL;
        l_mos6551->m_regRxBuffer = p_data;
        l_returnValue = 0;
    }

    return l_returnValue;
}

static int mos6551_receive(struct ts_serial *p_serial) {
    struct ts_mos6551 *l_mos6551 = (struct ts_mos6551 *)p_serial;
    int l_returnValue = -1;

    if((l_mos6551->m_regStatus & C_FLAG_STATUS_TX_EMPTY) == 0) {
        l_mos6551->m_regStatus |= C_FLAG_STATUS_TX_EMPTY;
        l_returnValue = l_mos6551->m_regTxBuffer;
    }

    return l_returnValue;
}

uint8_t mos6551_read8(struct ts_mos6551 *p_mos6551, t_busAddress p_address) {
    uint8_t l_returnValue = 0xff;
    
    switch(p_address & 0x3) {
        case E_MOS6551_REGISTER_DATA:
            p_mos6551->m_regStatus &=
                ~(C_FLAG_STATUS_RX_FULL | C_FLAG_STATUS_OVERRUN);
            l_returnValue = p_mos6551->m_regRxBuffer;
            break;

        case E_MOS6551_REGISTER_STATUS:
            l_returnValue = p_mos6551->m_regStatus;
            break;

        case E_MOS6551_REGISTER_COMMAND:
            l_returnValue = p_mos6551->m_regCommand;
            break;

        case E_MOS6551_REGISTER_CONTROL:
            l_returnValue = p_mos6551->m_regControl;
            break;

        default:
            break;
    }

    return l_returnValue;
}

void mos6551_write8(
    struct ts_mos6551 *p_mos6551,
    t_busAddress p_address,
    uint8_t p_data
) {
    switch(p_address & 0x3) {
        case E_MOS6551_REGISTER_DATA:
            if((p_mos6551->m_regStatus & C_FLAG_STATUS_TX_EMPTY) != 0) {
                p_mos6551->m_regStatus &= ~C_FLAG_STATUS_TX_EMPTY;
                p_mos6551->m_regTxBuffer = p_data;
            }

            break;

        case E_MOS6551_REGISTER_STATUS:
            // TODO: warm reset
            break;

        case E_MOS6551_REGISTER_COMMAND:
            p_mos6551->m_regCommand = p_data;
            break;

        case E_MOS6551_REGISTER_CONTROL:
            p_mos6551->m_regControl = p_data;
            break;
    }
}
