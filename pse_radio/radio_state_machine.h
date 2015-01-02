#ifndef _RADIO_STATE_MACHINE_H
#define _RADIO_STATE_MACHINE_H

#define RADIO_STATE_STATE_Disabled (0x00UL) /*!< Radio is in the Disabled state. */
#define RADIO_STATE_STATE_RxRu (0x01UL) /*!< Radio is in the Rx Ramp Up state. */
#define RADIO_STATE_STATE_RxIdle (0x02UL) /*!< Radio is in the Rx Idle state. */
#define RADIO_STATE_STATE_Rx (0x03UL) /*!< Radio is in the Rx state. */
#define RADIO_STATE_STATE_RxDisable (0x04UL) /*!< Radio is in the Rx Disable state. */
#define RADIO_STATE_STATE_TxRu (0x09UL) /*!< Radio is in the Tx Ramp Up state. */
#define RADIO_STATE_STATE_TxIdle (0x0AUL) /*!< Radio is in the Tx Idle state. */
#define RADIO_STATE_STATE_Tx (0x0BUL) /*!< Radio is in the Tx state. */
#define RADIO_STATE_STATE_TxDisable (0x0CUL) /*!< Radio is in the Tx Disable state. */

typedef unsigned int radio_state_t;
typedef enum {TXEN, RXEN, READY, START, STOP, ADDRESS, PAYLOAD, END, DISABLE, FULLY_DISABLED} radio_token_t;

radio_state_t transit(radio_state_t state, radio_token_t token);

#endif // _RADIO_STATE_MACHINE_H
