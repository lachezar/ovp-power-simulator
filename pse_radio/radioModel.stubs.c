#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "radioModel.h"
#include "radio_state_machine.h"

static Uns32 shouldTriggerIrq = 0;

static void triggerIrq() {
  shouldTriggerIrq = 1;
}

static void updateIrqLines() {
  if (shouldTriggerIrq == 1) {
    shouldTriggerIrq = 0;
    bhmPrintf("\n$$$$$ Radio IRQ ON \n");
    //bhmWaitDelay(22.0);
    ppmWriteNet(irqHandle, 1);
    bhmWaitDelay(10.0);
    ppmWriteNet(irqHandle, 0);

    bhmPrintf("\n$$$$$ Radio IRQ OFF \n");
  }
}

static void setState(Uns32 state) {
  regs.STATE = state;
  bhmPrintf("\n$$$ Radio NEW STATE = %d\n", state);
}

static void stateTransit(radio_token_t token) {
  setState(transit(regs.STATE, token));

  bhmPrintf("\n$$$ Radio state change with token = %d\n", token);

  if (token == READY && (regs.SHORTS & 1) != 0) {
    bhmPrintf("\n$$$ Radio SHORTS trigger START\n");
    regs.TASKS_START = 1;
    stateTransit(START); // start state
  } else if (token == END && (regs.SHORTS & 2) != 0) {
    bhmPrintf("\n$$$ Radio SHORTS trigger DISABLE\n");
    regs.TASKS_DISABLE = 1;
    regs.TASKS_TXEN = 0;
    stateTransit(DISABLE); // disable state
  }

  if ((token == FULLY_DISABLED || token == DISABLE) && (regs.INTENSET & 0x10) != 0) {
    triggerIrq();
  }
}


/*
   void flow() {
   triggerFlow = 0;
   radio_state_t newState = transit(regs.STATE, TXEN);
   bhmPrintf("\n$$$ Radio NEW STATE = %d\n", newState);
   regs.STATE = newState;
   newState = transit(regs.STATE, READY);
   bhmPrintf("\n$$$ Radio NEW STATE = %d\n", newState);
   regs.STATE = newState;
   regs.EVENTS_READY = 1;
   ppmWriteNet(ppiNotificationHandle, 1);
   bhmWaitDelay( 0.2 ); // in uS
   / / *newState = transit(regs.STATE, START);
     bhmPrintf("\n$$$ Radio NEW STATE = %d\n", newState);
     regs.STATE = newState;
     regs.TASKS_START = 1;* /
   if ((regs.SHORTS & 1) != 0) {
    bhmPrintf("\n$$$ Radio SHORTS trigger START\n");
    regs.TASKS_START = 1;

    newState = transit(regs.STATE, START);
    bhmPrintf("\n$$$ Radio NEW STATE = %d\n", newState);
    regs.STATE = newState;
    //bhmWaitDelay( 1.0 ); // in uS

    newState = transit(regs.STATE, ADDRESS);
    bhmPrintf("\n$$$ Radio NEW STATE = %d\n", newState);
    regs.STATE = newState;
    regs.EVENTS_ADDRESS = 1;
    ppmWriteNet(ppiNotificationHandle, 1);
    bhmWaitDelay( 0.2 ); // in uS

    newState = transit(regs.STATE, PAYLOAD);
    bhmPrintf("\n$$$ Radio NEW STATE = %d\n", newState);
    regs.STATE = newState;
    regs.EVENTS_PAYLOAD = 1;
    //bhmWaitDelay( 1.0 ); // in uS

    newState = transit(regs.STATE, END);
    bhmPrintf("\n$$$ Radio NEW STATE = %d\n", newState);
    regs.STATE = newState;
    regs.EVENTS_END = 1;
    ppmWriteNet(ppiNotificationHandle, 1);
    //bhmWaitDelay( 1.0 ); // in uS

    if ((regs.SHORTS & 2) != 0) {
      bhmPrintf("\n$$$ Radio SHORTS trigger DISABLE\n");
      regs.TASKS_DISABLE = 1;
      regs.TASKS_TXEN = 0;
      newState = transit(regs.STATE, DISABLE);
      bhmPrintf("\n$$$ Radio NEW STATE = %d\n", newState);
      regs.STATE = newState;
      bhmWaitDelay( 0.2 ); // in uS

      ppmWriteNet(ppiNotificationHandle, 1);
      regs.EVENTS_DISABLED = 1;
      newState = transit(regs.STATE, DISABLED_EVENT);
      bhmPrintf("\n$$$ Radio NEW STATE = %d\n", newState);
      regs.STATE = newState;
      //bhmWaitDelay( 1.0 ); // in uS

      if ((regs.INTENSET & 0x10) != 0) {
        // trigger irq
        triggerIrq();
        updateIrqLines();
        bhmWaitDelay( 50.0 ); // in uS
        bhmPrintf("\n WTF???? \n");
        triggerIrq();
        updateIrqLines();
        //bhmWaitDelay( 0.02 );
      }
    }
   }
   }*/

//
// View any 32-bit register
//
PPM_VIEW_CB(viewReg32) {
  bhmPrintf("\n$$$ Radio View\n");
}

//
// Read any 32-bit register
//
PPM_READ_CB(regRd32) {
  Uns32 offset = (Uns32)addr - (Uns32)radioWindow;
  if (offset != 0x110 && offset != 0x104) {
    bhmPrintf("\n$$$ Radio Read from 0x%08x = 0x%08x\n", offset, *(Uns32*)user);
  }
  return *(Uns32*)user;
}

//
// Update any 32-bit register (with no side effects)
//
PPM_WRITE_CB(regWr32) {
  *(Uns32*)user = data;
  bhmPrintf("\n$$$ Radio Write 0x%08x to address 0x%08x\n", data, (Uns32)addr - (Uns32)radioWindow);

  if ((Uns32*)user == &regs.TASKS_TXEN && regs.STATE == RADIO_STATE_STATE_Disabled) {
    bhmPrintf("\n$$$ Radio Write FOR TXEN!!!\n");
    bhmTriggerEvent(txenEventHandle);
    stateTransit(TXEN);
  } else if ((Uns32*)user == &regs.INTENSET) {
    bhmPrintf("Radio INTENSET = 0x%08x\n", data);
  } else if ((Uns32*)user == &regs.TASKS_START) {
    bhmPrintf("Radio START! (to be done), data = %d\n", data);
  } else if ((Uns32*)user == &regs.TASKS_START && data != 0) {
    bhmPrintf("Radio START! (to be done)");
  } else if (((Uns32)addr - (Uns32)radioWindow) <= 0x20) {
    bhmPrintf("Radio task not handled");
  } else {
    //bhmPrintf("Radio not handled");
  }
}

PPM_CONSTRUCTOR_CB(init) {

  periphConstructor();

  bhmPrintf("\n\n\n$$$$$ constructor \n\n\n");

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
