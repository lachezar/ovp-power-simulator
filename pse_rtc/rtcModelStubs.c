#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rtcModel.h"

static Uns32 counter = 0;
static Uns32 irq = 0;
static Uns32 resetCounter = 0;
static Uns32 shouldTriggerIrq = 0;
static Uns32 irqAsserted = 0;

//
// View any 32-bit register
//
PPM_VIEW_CB(viewReg32) {
  bhmPrintf("\n$$$ RTC View\n");
}

//
// Read any 32-bit register
//
PPM_READ_CB(regRd32) {
  if ((Uns32*)user == &regs.COUNTER) {

    *(Uns32*)user = counter;
    bhmPrintf("\n!!!READ COUNTER REGISTER - %d\n", *(Uns32*)user);
    return counter;
  }
  //bhmPrintf("\n$$$ RTC Read from 0x%08x = 0x%08x (counter 0x%08x)\n", (Uns32)addr - (Uns32)rtc_window, *(Uns32*)user, counter);
  return *(Uns32*)user;
}

//
// Update any 32-bit register (with no side effects)
//
PPM_WRITE_CB(regWr32) {
  *(Uns32*)user = data;
  bhmPrintf("\n$$$ RTC Write to 0x%08x (user - 0x%08x, data - 0x%08x, window - 0x%08x, pwr - %d) \n", (Uns32)addr - (Uns32)rtcWindow, (Uns32)user, data, (Uns32)rtcWindow, regs.POWER);

  if ((Uns32*)user == &regs.TASKS_START) {
    bhmPrintf("RTC START! (to be done), data = %d\n", data);
  }

  if ((Uns32*)user == &regs.TASKS_START && data != 0) {
    regs.TASKS_STOP = 0;
    bhmTriggerEvent(startEventHandle);
    bhmPrintf("RTC START! (to be done)");
  }
  if ((Uns32*)user == &regs.TASKS_STOP && data != 0) {
    regs.TASKS_START = 0;
    bhmPrintf("RTC STOP! (to be done)");
  }
  if ((Uns32*)user == &regs.TASKS_CLEAR) {
    bhmPrintf("CLEAR RTC counter!");
    resetCounter = 1;
  }
  if ((Uns32*)user == &regs.TASKS_TRIGOVRFLW) {
    bhmPrintf("RTC COUNTER OVERFLOW!");
    counter = 0xFFFFF0;
  }

  if ((Uns32*)user == &regs.EVENTS_TICK) {
    bhmPrintf("RTC tick event write!");
    ppmWriteNet(rtcNotificationHandle, RTC_PERIPHERAL_ID);
  }
  if ((Uns32*)user == &regs.EVENTS_OVRFLW) {
    bhmPrintf("RTC overflow event write!");
    ppmWriteNet(rtcNotificationHandle, RTC_PERIPHERAL_ID);
  }

  if ((Uns32*)user == &regs.EVENTS_COMPARE0) {
    bhmPrintf("COMPARE0 RTC write!");
    ppmWriteNet(rtcNotificationHandle, RTC_PERIPHERAL_ID);
  }
  if ((Uns32*)user == &regs.EVENTS_COMPARE1) {
    bhmPrintf("COMPARE1 RTC write!");
    ppmWriteNet(rtcNotificationHandle, RTC_PERIPHERAL_ID);
  }
  if ((Uns32*)user == &regs.EVENTS_COMPARE2) {
    bhmPrintf("COMPARE2 RTC write!");
    ppmWriteNet(rtcNotificationHandle, RTC_PERIPHERAL_ID);
  }
  if ((Uns32*)user == &regs.EVENTS_COMPARE3) {
    bhmPrintf("COMPARE3 RTC write!");
    ppmWriteNet(rtcNotificationHandle, RTC_PERIPHERAL_ID);
  }

  if ((Uns32*)user == &regs.EVTEN) {
    bhmPrintf("RTC EVTEN overwrite!");
  }
  if ((Uns32*)user == &regs.EVTENSET) {
    regs.EVTEN = regs.EVTEN | data;
    bhmPrintf("RTC EVTENSET! - 0x%08x\n", regs.EVTEN);
  }
  if ((Uns32*)user == &regs.EVTENCLR) {
    /*
       0 0 -> 0
       0 1 -> 0
       1 1 -> 0
       1 0 -> 1
     */
    regs.EVTEN = regs.EVTEN & (~data);
    bhmPrintf("RTC EVTENCLR!");
  }
  if ((Uns32*)user == &regs.INTENSET) {
    irq = irq | data;
    regs.INTENSET = irq;
    regs.INTENCLR = irq;
    bhmPrintf("RTC INTENSET! irq - %d\n", irq);
  }
  if ((Uns32*)user == &regs.INTENCLR) {
    /*
       0 0 -> 0
       0 1 -> 0
       1 1 -> 0
       1 0 -> 1
     */
    irq = irq & (~data);
    regs.INTENSET = irq;
    regs.INTENCLR = irq;
    bhmPrintf("RTC INTENCLR!");
  }

  if ((Uns32*)user == &regs.PRESCALER) {
    bhmPrintf("RTC PRESCALER!");
    if (data < 0 || data > 4095) {
      bhmPrintf("unsupported RTC PRESCALER value = %d!", data);
      exit(1);
    }
  }

  if ((Uns32*)user == &regs.CC0) {
    bhmPrintf("CC0 RTC write!");
  }
  if ((Uns32*)user == &regs.CC1) {
    bhmPrintf("CC1 RTC write!");
  }
  if ((Uns32*)user == &regs.CC2) {
    bhmPrintf("CC2 RTC write!");
  }
  if ((Uns32*)user == &regs.CC3) {
    bhmPrintf("CC3 RTC write!");
  }
}

PPM_CONSTRUCTOR_CB(init) {

  periphConstructor();

  startEventHandle = bhmCreateNamedEvent("start", "start the RTC");

  regs.TASKS_STOP = 1;
  regs.TASKS_START = 0;
}

void triggerIrq() {
  shouldTriggerIrq = 1;
}

void updateIrqLines() {
  if (shouldTriggerIrq == 1) {
    ppmWriteNet(irqHandle, 1);
    irqAsserted = 1;
    shouldTriggerIrq = 0;
    bhmPrintf("\n$$$$$ RTC IRQ ON \n");
    bhmWaitDelay(50.0);
    ppmWriteNet(irqHandle, 0);
    irqAsserted = 0;
    bhmPrintf("\n$$$$$ RTC IRQ OFF \n");
  } else if (shouldTriggerIrq == 0 && irqAsserted == 1) {
    ppmWriteNet(irqHandle, 0);
    irqAsserted = 0;
    bhmPrintf("\n$$$$$ RTC IRQ OFF \n");
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
      bhmPrintf("RTC OVERFLOW on next cycle!");
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

    if ((regs.EVTEN & (1 << 16)) != 0 && counter == regs.CC0 && skipCCMatch == 0) {
      bhmPrintf("\n\n\n$$ RTC match cc0 \n\n\n");
      regs.EVENTS_COMPARE0 = 1;
      ppmWriteNet(rtcNotificationHandle, RTC_PERIPHERAL_ID);
      if ((irq & (1 << 16)) != 0) {
        triggerIrq();
      }
    }

    if ((regs.EVTEN & (1 << 17)) != 0 && counter == regs.CC1 && skipCCMatch == 0) {
      regs.EVENTS_COMPARE1 = 1;
      ppmWriteNet(rtcNotificationHandle, RTC_PERIPHERAL_ID);
      bhmPrintf("\n\n\n$$ RTC match cc1 \n\n\n");
      if ((irq & (1 << 17)) != 0) {
        bhmPrintf("\n\n\n$$ RTC match cc1 IRQ \n\n\n");
        triggerIrq();
      }
    }

    if ((regs.EVTEN & (1 << 18)) != 0 && counter == regs.CC2 && skipCCMatch == 0) {
      regs.EVENTS_COMPARE2 = 1;
      ppmWriteNet(rtcNotificationHandle, RTC_PERIPHERAL_ID);
      if ((irq & (1 << 18)) != 0) {
        triggerIrq();
      }
    }

    if ((regs.EVTEN & (1 << 19)) != 0 && counter == regs.CC3 && skipCCMatch == 0) {
      regs.EVENTS_COMPARE3 = 1;
      ppmWriteNet(rtcNotificationHandle, RTC_PERIPHERAL_ID);
      if ((irq & (1 << 19)) != 0) {
        triggerIrq();
      }
    }

    updateIrqLines();

    counter = (counter + 1) & 0xFFFFFF;

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
