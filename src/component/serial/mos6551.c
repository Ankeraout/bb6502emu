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

int mos6551_init(struct ts_mos6551 *p_mos6551, struct ts_serial *p_serial) {
    p_mos6551->m_serial = p_serial;

    p_mos6551->m_regStatus = 0;
    p_mos6551->m_regCommand = C_FLAG_COMMAND_DISABLE_IRQ;
    p_mos6551->m_regControl = 0;

    return 0;
}

uint8_t mos6551_read8(struct ts_mos6551 *p_mos6551, t_busAddress p_address) {
    uint8_t l_returnValue = 0xff;
    int l_tmp;
    
    switch(p_address & 0x3) {
        case E_MOS6551_REGISTER_DATA:
            l_returnValue = serialReadRx(p_mos6551->m_serial);
            break;

        case E_MOS6551_REGISTER_STATUS:
            l_returnValue = p_mos6551->m_regStatus;

            l_tmp = p_mos6551->m_serial->m_rxBuffer;

            if((l_tmp & C_SERIAL_FLAG_OVERRUN) != 0) {
                l_returnValue |= C_FLAG_STATUS_OVERRUN;
            }

            if((l_tmp & C_SERIAL_FLAG_NODATA) == 0) {
                l_returnValue |= C_FLAG_STATUS_RX_FULL;
            }

            if((p_mos6551->m_serial->m_txBuffer & C_SERIAL_FLAG_NODATA) != 0) {
                l_returnValue |= C_FLAG_STATUS_TX_EMPTY;
            }

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
            serialWriteTx(p_mos6551->m_serial, p_data);
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
