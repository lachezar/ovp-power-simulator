#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rtcModel.h"

#undef info
#define info(format, ...) bhmPrintf("@@@ RTC(%d): ", RTC_PERIPHERAL_ID);\
bhmPrintf(format, ##__VA_ARGS__);\
bhmPrintf("\n")

#undef error
#define error(format, ...) bhmPrintf(">>> ERROR IN RTC(%d): ", RTC_PERIPHERAL_ID);\
bhmPrintf(format, ##__VA_ARGS__);\
bhmPrintf("\n")

static Uns32 counter = 0;
static Uns32 irq = 0;
static Uns32 resetCounter = 0;
static Uns32 shouldTriggerIrq = 0;
static Uns32 irqAsserted = 0;

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
  if ((Uns32*)user == &regs.COUNTER) {

    *(Uns32*)user = counter;
    info("READ COUNTER REGISTER - %d\n", *(Uns32*)user);
    return counter;
  }
  //info("Read from 0x%08x = 0x%08x (counter 0x%08x)\n", (Uns32)addr - (Uns32)rtc_window, *(Uns32*)user, counter);
  return *(Uns32*)user;
}

//
// Update any 32-bit register (with no side effects)
//
PPM_WRITE_CB(regWr32) {
  *(Uns32*)user = data;
  info("Write to 0x%08x (user - 0x%08x, data - 0x%08x, window - 0x%08x, pwr - %d) \n", (Uns32)addr - (Uns32)rtcWindow, (Uns32)user, data, (Uns32)rtcWindow, regs.POWER);

  if ((Uns32*)user == &regs.TASKS_START && data != 0) {
    regs.TASKS_STOP = 0;
    bhmTriggerEvent(startEventHandle);
    info("START! (to be done)");
  } else if ((Uns32*)user == &regs.TASKS_STOP && data != 0) {
    regs.TASKS_START = 0;
    info("STOP! (to be done)");
  } else if ((Uns32*)user == &regs.TASKS_CLEAR) {
    info("CLEAR RTC counter!");
    resetCounter = 1;
  } else if ((Uns32*)user == &regs.TASKS_TRIGOVRFLW) {
    info("COUNTER OVERFLOW!");
    counter = 0xFFFFF0;
  } else if ((Uns32*)user == &regs.EVENTS_TICK) {
    info("tick event write!");
    ppmWriteNet(rtcNotificationHandle, RTC_PERIPHERAL_ID);
  } else if ((Uns32*)user == &regs.EVENTS_OVRFLW) {
    info("overflow event write!");
    ppmWriteNet(rtcNotificationHandle, RTC_PERIPHERAL_ID);
  } else if ((Uns32*)user >= &regs.EVENTS_COMPARE[0] && (Uns32*)user <= &regs.EVENTS_COMPARE[3]) {
    Uns32 id = (Uns32*)user - &regs.EVENTS_COMPARE[0];
    info("COMPARE[%d] RTC write!", id);
    ppmWriteNet(rtcNotificationHandle, RTC_PERIPHERAL_ID);
  } else if ((Uns32*)user == &regs.EVTEN) {
    info("EVTEN overwrite!");
  } else if ((Uns32*)user == &regs.EVTENSET) {
    regs.EVTEN = regs.EVTEN | data;
    info("EVTENSET! - 0x%08x\n", regs.EVTEN);
  } else if ((Uns32*)user == &regs.EVTENCLR) {
    /*
       0 0 -> 0
       0 1 -> 0
       1 1 -> 0
       1 0 -> 1
     */
    regs.EVTEN = regs.EVTEN & (~data);
    info("EVTENCLR!");
  } else if ((Uns32*)user == &regs.INTENSET) {
    irq = irq | data;
    regs.INTENSET = irq;
    regs.INTENCLR = irq;
    info("INTENSET! irq - %d\n", irq);
  } else if ((Uns32*)user == &regs.INTENCLR) {
    /*
       0 0 -> 0
       0 1 -> 0
       1 1 -> 0
       1 0 -> 1
     */
    irq = irq & (~data);
    regs.INTENSET = irq;
    regs.INTENCLR = irq;
    info("INTENCLR!");
  } else if ((Uns32*)user == &regs.PRESCALER) {
    info("PRESCALER!");
    if (data < 0 || data > 4095) {
      error("unsupported RTC PRESCALER value = %d!", data);
      exit(1);
    }
  } else if ((Uns32*)user >= &regs.CC[0] && (Uns32*)user <= &regs.CC[3]) {
    Uns32 id = (Uns32*)user - &regs.CC[0];
    info("CC[%d] RTC write!", id);
  }
}

PPM_CONSTRUCTOR_CB(init) {

  periphConstructor();

  startEventHandle = bhmCreateNamedEvent("start", "start the RTC");

  regs.TASKS_STOP = 1;
  regs.TASKS_START = 0;
}

static void triggerIrq() {
  shouldTriggerIrq = 1;
}

static void updateIrqLines() {
  if (shouldTriggerIrq == 1) {
    ppmWriteNet(irqHandle, 1);
    irqAsserted = 1;
    shouldTriggerIrq = 0;
    info("IRQ ON \n");
    bhmWaitDelay(5.0);
    ppmWriteNet(irqHandle, 0);
    irqAsserted = 0;
    info("IRQ OFF \n");
  } else if (shouldTriggerIrq == 0 && irqAsserted == 1) {
    ppmWriteNet(irqHandle, 0);
    irqAsserted = 0;
    info("IRQ OFF \n");
  }
}

void loop() {

  Uns32 isOverflow = 0;
  Uns32 skipCCMatch = 0;

  while (1) {

    while (regs.TASKS_STOP != 0 && regs.TASKS_START == 0) {
      bhmWaitEvent(startEventHandle);
    }

    if ((regs.EVTEN & 2) != 0 && counter == 0xFFFFFF) {
      // overflow happened!!!
      info("OVERFLOW on next cycle!");
      isOverflow = 1;
    }

    if ((regs.EVTEN & 1) != 0) {
      regs.EVENTS_TICK = 1;
      if ((irq & 1) != 0) {
        triggerIrq();
      }
    }

    if (isOverflow == 1 && counter == 0 && skipCCMatch == 0) {
      regs.EVENTS_OVRFLW = 1;
      ppmWriteNet(rtcNotificationHandle, RTC_PERIPHERAL_ID);
      if ((irq & 2) != 0) {
        triggerIrq();
      }
      isOverflow = 0;
    }
    
    Uns32 i;
    const Uns32 signalBitOffset = 16;
    for (i = 0; i < 4; i++) {
      if ((regs.EVTEN & (1 << (signalBitOffset + i))) != 0 && counter == regs.CC[i] && skipCCMatch == 0) {
        info("match CC[%d] \n\n\n", i);
        regs.EVENTS_COMPARE[i] = 1;
        ppmWriteNet(rtcNotificationHandle, RTC_PERIPHERAL_ID);
        if ((irq & (1 << (signalBitOffset + i))) != 0) {
          triggerIrq();
        }
      }
    }

    updateIrqLines();

    counter = (counter + 1) & 0xFFFFFF;
    info("tick: %d\n", counter);

    if (resetCounter != 0) {
      resetCounter = 0;
      counter = 0;
      skipCCMatch = 1;
    } else {
      skipCCMatch = 0;
    }
    regs.TASKS_CLEAR = 0;

    bhmWaitDelay( 1000000.0 / (double)(32768UL / (regs.PRESCALER + 1))); // in uS
  }
}
