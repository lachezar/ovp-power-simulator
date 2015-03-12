#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "radioModel.h"
#include "radio_state_machine.h"

#undef info
#define info(format, ...) bhmPrintf("$$$ RADIO(%d): ", RADIO_PERIPHERAL_ID);\
bhmPrintf(format, ##__VA_ARGS__);\
bhmPrintf("\n")

#undef error
#define error(format, ...) bhmPrintf(">>> ERROR IN RADIO(%d): ", RADIO_PERIPHERAL_ID);\
bhmPrintf(format, ##__VA_ARGS__);\
bhmPrintf("\n")

static Uns32 shouldTriggerIrq = 0;

static void triggerIrq() {
  shouldTriggerIrq = 1;
}

inline static Uns32 irqPredicate(radio_token_t token, Uns32 intenset) { // @TODO implemented event based interrupts (0x20, 0x40, 0x80, 0x400)
  return ((token == FULLY_DISABLED || token == DISABLE) && (intenset & 0x10) != 0)
        || (token == READY && (intenset & 1) != 0)
        || (token == ADDRESS && (intenset & 2) != 0)
        || (token == PAYLOAD && (intenset & 4) != 0)
        || (token == END && (intenset & 8) != 0);
}

static void updateIrqLines() {
  if (shouldTriggerIrq == 1) {
    shouldTriggerIrq = 0;
    info("IRQ ON \n");
    ppmWriteNet(irqHandle, 1);
    bhmWaitDelay(10.0);
    ppmWriteNet(irqHandle, 0);

    info("IRQ OFF \n");
  }
}

static void setState(Uns32 state) {
  regs.STATE = state;
  info("NEW STATE = %d\n", state);
}

static void stateTransit(radio_token_t token) {
  setState(transit(regs.STATE, token));

  info("state change with token = %d\n", token);

  if (token == READY && (regs.SHORTS & 1) != 0) {
    info("SHORTS trigger START\n");
    regs.TASKS_START = 1;
    stateTransit(START); // start state
  } else if (token == END && (regs.SHORTS & 2) != 0) {
    info("SHORTS trigger DISABLE\n");
    regs.TASKS_DISABLE = 1;
    regs.TASKS_TXEN = 0;
    stateTransit(DISABLE); // disable state
  } else if (token == FULLY_DISABLED && (regs.SHORTS & 4) != 0) { // not tested yet
    info("SHORTS trigger TXEN\n");
    regs.TASKS_DISABLE = 0;
    regs.TASKS_TXEN = 1;
    stateTransit(TXEN); // txen state
  } else if (token == FULLY_DISABLED && (regs.SHORTS & 8) != 0) { // not tested yet
    info("SHORTS trigger RXEN\n");
    regs.TASKS_DISABLE = 0;
    regs.TASKS_RXEN = 1;
    stateTransit(RXEN); // rxen state
  } else if (token == ADDRESS && (regs.SHORTS & 0x10) != 0) { // not tested yet
    info("SHORTS trigger RSSISTART\n");
    regs.TASKS_RSSISTART = 1;
    regs.TASKS_RSSISTOP = 0;
  } else if (token == END && (regs.SHORTS & 0x20) != 0) { // not tested yet
    info("SHORTS trigger START\n");
    regs.TASKS_START = 1;
    regs.TASKS_TXEN = 0;
    stateTransit(START); // start state
  } else if (token == ADDRESS && (regs.SHORTS & 0x40) != 0) { // not tested yet
    info("SHORTS trigger BCSTART\n");
    regs.TASKS_BCSTART = 1;
    regs.TASKS_BCSTOP = 0;
    //stateTransit(RXEN); // rxen state
  } else if (token == ADDRESS && (regs.SHORTS & 0x100) != 0) { // not tested yet
    info("SHORTS trigger RSSISTOP\n");
    regs.TASKS_RSSISTOP = 1;
    regs.TASKS_RSSISTART = 0;
  }

  if (irqPredicate(token, regs.INTENSET)) {
    triggerIrq();
  }
}

//
// View any 32-bit register
//
PPM_VIEW_CB(viewReg32) {
  info("View");
}

//
// Read any 32-bit register
//
PPM_READ_CB(regRd32) {
  Uns32 offset = (Uns32)addr - (Uns32)radioWindow;
  if (offset != 0x110 && offset != 0x104) {
    info("Read from 0x%08x = 0x%08x\n", offset, *(Uns32*)user);
  }
  return *(Uns32*)user;
}

//
// Update any 32-bit register (with no side effects)
//
PPM_WRITE_CB(regWr32) {
  *(Uns32*)user = data;
  info("Write 0x%08x to address 0x%08x\n", data, (Uns32)addr - (Uns32)radioWindow);

  if ((Uns32*)user == &regs.TASKS_TXEN && regs.STATE == RADIO_STATE_STATE_Disabled) {
    info("Write FOR TXEN!!!\n");
    bhmTriggerEvent(txenEventHandle);
    stateTransit(TXEN);
  } else if ((Uns32*)user == &regs.INTENSET) {
    info("INTENSET = 0x%08x\n", data);
  } else if ((Uns32*)user == &regs.TASKS_START) {
    info("START! (to be done), data = %d\n", data);
  } else if ((Uns32*)user == &regs.TASKS_START && data != 0) {
    info("START! (to be done)");
  } else if ((Uns32*)user == &regs.TASKS_DISABLE) {
    info("DISABLE!");
    stateTransit(DISABLE);
  } else if (((Uns32)addr - (Uns32)radioWindow) <= 0x20) {
    info("task not handled 0x%x state: %d\n", ((Uns32)addr - (Uns32)radioWindow), regs.STATE);
  } else {
    error("write not handled");
  }
}

PPM_CONSTRUCTOR_CB(init) {

  periphConstructor();

  info("\n\n\n$$$$$ constructor \n\n\n");

  txenEventHandle = bhmCreateNamedEvent("txen", "txen");

  regs.TASKS_STOP = 1;
  regs.TASKS_START = 0;
  regs.STATE = RADIO_STATE_STATE_Disabled;
}

void loop() {

  while (1) {

    while (regs.STATE == RADIO_STATE_STATE_Disabled) {
      bhmWaitEvent(txenEventHandle);
    }

    if (regs.STATE == RADIO_STATE_STATE_TxRu) {
      bhmWaitDelay( 140.0 ); // in uS
      info("READY EVENT!!! \n\n\n");
      stateTransit(READY); // idle state
      regs.EVENTS_READY = 1;
      ppmWriteNet(ppiNotificationHandle, RADIO_PERIPHERAL_ID);
    } else if (regs.STATE == RADIO_STATE_STATE_TxIdle) {
      // ?
    } else if (regs.STATE == RADIO_STATE_STATE_Tx) {
      stateTransit(ADDRESS); // address sent state
      regs.EVENTS_ADDRESS = 1;
      ppmWriteNet(ppiNotificationHandle, RADIO_PERIPHERAL_ID);
      bhmWaitDelay( 20.0 ); // in uS
      stateTransit(PAYLOAD); // payload sent state
      regs.EVENTS_PAYLOAD = 1;
      ppmWriteNet(ppiNotificationHandle, RADIO_PERIPHERAL_ID);
      bhmWaitDelay( 20.0 ); // in uS
      stateTransit(END); // end state
      regs.EVENTS_END = 1;
      ppmWriteNet(ppiNotificationHandle, RADIO_PERIPHERAL_ID);
    } else if (regs.STATE == RADIO_STATE_STATE_TxDisable) {

      stateTransit(FULLY_DISABLED); // fully disabled state
      regs.EVENTS_DISABLED = 1;
      ppmWriteNet(ppiNotificationHandle, RADIO_PERIPHERAL_ID);
    }

    updateIrqLines();

    bhmWaitDelay( 1.0 ); // in uS
  }

}
