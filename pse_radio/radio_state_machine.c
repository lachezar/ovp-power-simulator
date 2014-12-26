#include "radio_state_machine.h"

radio_state_t transit(radio_state_t state, radio_token_t token) {
  
  if (state == RADIO_STATE_STATE_Disabled && token == TXEN) {
    return RADIO_STATE_STATE_TxRu;
  } else if ((state == RADIO_STATE_STATE_TxRu || state == RADIO_STATE_STATE_TxIdle || state == RADIO_STATE_STATE_Tx) && token == DISABLE) {
    return RADIO_STATE_STATE_TxDisable;
  } else if (state == RADIO_STATE_STATE_TxRu && token == READY) {
    return RADIO_STATE_STATE_TxIdle;
  } else if (state == RADIO_STATE_STATE_TxIdle && token == START) {
    return RADIO_STATE_STATE_Tx;
  } else if (state == RADIO_STATE_STATE_Tx && token == ADDRESS) {
    return RADIO_STATE_STATE_Tx;
  } else if (state == RADIO_STATE_STATE_Tx && token == PAYLOAD) {
    return RADIO_STATE_STATE_Tx;
  } else if (state == RADIO_STATE_STATE_Tx && token == STOP) {
    return RADIO_STATE_STATE_TxIdle;
  } else if (state == RADIO_STATE_STATE_Tx && token == END) {
    return RADIO_STATE_STATE_TxIdle;
  } else if (state == RADIO_STATE_STATE_TxDisable && token == DISABLED_EVENT) {
    return RADIO_STATE_STATE_Disabled;

  } else if (state == RADIO_STATE_STATE_Disabled && token == RXEN) {
    return RADIO_STATE_STATE_RxRu;
  } else if ((state == RADIO_STATE_STATE_RxRu || state == RADIO_STATE_STATE_RxIdle || state == RADIO_STATE_STATE_Rx) && token == DISABLE) {
    return RADIO_STATE_STATE_RxDisable;
  } else if (state == RADIO_STATE_STATE_RxRu && token == READY) {
    return RADIO_STATE_STATE_RxIdle;
  } else if (state == RADIO_STATE_STATE_RxIdle && token == START) {
    return RADIO_STATE_STATE_Rx;
  } else if (state == RADIO_STATE_STATE_Rx && token == ADDRESS) {
    return RADIO_STATE_STATE_Rx;
  } else if (state == RADIO_STATE_STATE_Rx && token == PAYLOAD) {
    return RADIO_STATE_STATE_Rx;
  } else if (state == RADIO_STATE_STATE_Rx && token == STOP) {
    return RADIO_STATE_STATE_RxIdle;
  } else if (state == RADIO_STATE_STATE_Rx && token == END) {
    return RADIO_STATE_STATE_RxIdle;
  } else if (state == RADIO_STATE_STATE_RxDisable && token == DISABLED_EVENT) {
    return RADIO_STATE_STATE_Disabled;
  }
  
  return state;
}
