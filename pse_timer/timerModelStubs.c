#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "timerModel.h"

static Uns32 ticks = 0;
static Uns32 irq = 0;
static Uns32 shouldTriggerIrq = 0;
static Uns32 isStarted = 0;

//
// View any 32-bit register
//
PPM_VIEW_CB(viewReg32) {
  bhmPrintf("\n$$$ Timer View\n");
}

//
// Read any 32-bit register
//
PPM_READ_CB(regRd32) {
  bhmPrintf("\n$$$ Timer Read from 0x%08x\n", (Uns32)addr - (Uns32)timerWindow);
  if ((Uns32*)user == &regs.INTENSET || (Uns32*)user == &regs.INTENCLR) {
    bhmPrintf("TIMER READ INTENCLR or INTENSET! - 0x%08x\n", irq);
    *(Uns32*)user = irq;
    return irq;
  }
  return *(Uns32*)user;
}

//
// Update any 32-bit register (with no side effects)
//
PPM_WRITE_CB(regWr32) {
  *(Uns32*)user = data;
  bhmPrintf("\n$$$ Timer Write to 0x%08x (user - 0x%08x, data - 0x%08x, window - 0x%08x, pwr - %d) \n", (Uns32)addr - (Uns32)timerWindow, (Uns32)user, data, (Uns32)timerWindow, regs.POWER);

  if ((Uns32*)user == &regs.TASKS_START && data != 0 && isStarted == 0) {
    isStarted = 1;
    regs.TASKS_STOP = 0;
    bhmTriggerEvent(startEventHandle);
    bhmPrintf("TIMER START! (to be done)");
  }
  if ((Uns32*)user == &regs.TASKS_STOP && data != 0 && isStarted != 0) {
    isStarted = 0;
    regs.TASKS_START = 0;
    bhmPrintf("TIMER STOP! (to be done)");
  }
  if ((Uns32*)user == &regs.POWER && data == 0) {
    isStarted = 0;
    regs.TASKS_STOP = 1;
    regs.TASKS_START = 0;
    ticks = 0;
    bhmPrintf("TIMER POWER DOWN!\n");
  }
  if ((Uns32*)user == &regs.MODE && data == 1) {
    bhmPrintf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! COUNTER MODE NOT SUPPORTED YET!");
    exit(1);
  }
  if ((Uns32*)user == &regs.MODE && data == 0) {
    bhmPrintf("TIMER in timing mode!");
  }
  if ((Uns32*)user == &regs.TASKS_COUNT) {
    bhmPrintf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! COUNTER MODE NOT SUPPORTED YET!");
    exit(1);
  }
  if ((Uns32*)user == &regs.TASKS_CLEAR) {
    bhmPrintf("CLEAR TIMER ticks!");
    ticks = 0;
  }
  if ((Uns32*)user == &regs.TASKS_CAPTURE0) {
    regs.CC0 = ticks;
    bhmPrintf("CAPTURE[0] TIMER ticks!");
  }
  if ((Uns32*)user == &regs.TASKS_CAPTURE1) {
    regs.CC1 = ticks;
    bhmPrintf("CAPTURE[1] TIMER ticks!");
  }
  if ((Uns32*)user == &regs.TASKS_CAPTURE2) {
    regs.CC2 = ticks;
    bhmPrintf("CAPTURE[2] TIMER ticks!");
  }
  if ((Uns32*)user == &regs.TASKS_CAPTURE3) {
    regs.CC3 = ticks;
    bhmPrintf("CAPTURE[3] TIMER ticks!");
  }

  if ((Uns32*)user == &regs.EVENTS_COMPARE0) {
    bhmPrintf("COMPARE0 TIMER write!");
    ppmWriteNet(timer0NotificationHandle, 8);
  }
  if ((Uns32*)user == &regs.EVENTS_COMPARE1) {
    bhmPrintf("COMPARE1 TIMER write!");
    ppmWriteNet(timer0NotificationHandle, 8);
  }
  if ((Uns32*)user == &regs.EVENTS_COMPARE2) {
    bhmPrintf("COMPARE2 TIMER write!");
    ppmWriteNet(timer0NotificationHandle, 8);
  }
  if ((Uns32*)user == &regs.EVENTS_COMPARE3) {
    bhmPrintf("COMPARE3 TIMER write!");
    ppmWriteNet(timer0NotificationHandle, 8);
  }

  if ((Uns32*)user == &regs.SHORTS) {
    bhmPrintf("TIMER SHORTS!");
  }
  if ((Uns32*)user == &regs.INTENSET) {
    irq = irq | data;
    bhmPrintf("TIMER INTENSET!");
  }
  if ((Uns32*)user == &regs.INTENCLR) {
    /*
       0 0 -> 0
       0 1 -> 0
       1 1 -> 0
       1 0 -> 1
     */
    irq = irq & (~data);
    bhmPrintf("TIMER INTENCLR!");
  }

  if ((Uns32*)user == &regs.BITMODE) {
    bhmPrintf("TIMER BITMODE!");
  }
  if ((Uns32*)user == &regs.PRESCALER) {
    bhmPrintf("TIMER PRESCALER!");
    if (data < 0 || data > 9) {
      bhmPrintf("unsupported TIMER PRESCALER value = %d!", data);
      exit(1);
    }
  }

  if ((Uns32*)user == &regs.CC0) {
    bhmPrintf("CC0 TIMER write!");
  }
  if ((Uns32*)user == &regs.CC1) {
    bhmPrintf("CC1 TIMER write!");
  }
  if ((Uns32*)user == &regs.CC2) {
    bhmPrintf("CC2 TIMER write!");
  }
  if ((Uns32*)user == &regs.CC3) {
    bhmPrintf("CC3 TIMER write!");
  }

}

PPM_CONSTRUCTOR_CB(init) {

  periphConstructor();

  bhmPrintf("\n\n\n$$$$$ constructor \n\n\n");

  startEventHandle = bhmCreateNamedEvent("start", "start the timer");

  regs.TASKS_STOP = 1;
  regs.TASKS_START = 0;
}

void triggerIrq() {
  shouldTriggerIrq = 1;
}

void updateIrqLines() {
  if (shouldTriggerIrq == 1) {
    bhmPrintf("\n$$$$$ TIMER IRQ ON \n");
    ppmWriteNet(irqHandle, 1);
    shouldTriggerIrq = 0;
    bhmWaitDelay(50.0);
    ppmWriteNet(irqHandle, 0);
    bhmPrintf("\n$$$$$ TIMER IRQ OFF \n");
  }
}

void loop() {

  while (1) {

    while (regs.TASKS_STOP != 0 && regs.TASKS_START == 0) {
      bhmWaitEvent(startEventHandle);
    }

    if (ticks == regs.CC0) {
      bhmPrintf("\n\n$$$$$ Timer CC0 match\n");
      regs.EVENTS_COMPARE0 = 1;
      ppmWriteNet(timer0NotificationHandle, 0x8);
      if ((regs.SHORTS & 0x1) != 0) {
        ticks = 0;
      }
      if ((regs.SHORTS & (1 << 8)) != 0) {
        regs.TASKS_STOP = 1;
        regs.TASKS_START = 0;
        continue;
      }
      if ((irq & (1 << 16)) != 0) {
        // trigger interrupt
        triggerIrq();
      }
    }
    if (ticks == regs.CC1) {
      bhmPrintf("\n\n$$$$$ Timer CC1 match\n");
      regs.EVENTS_COMPARE1 = 1;
      ppmWriteNet(timer0NotificationHandle, 0x8);
      if ((regs.SHORTS & 0x2) != 0) {
        ticks = 0;
      }
      if ((regs.SHORTS & (1 << 9)) != 0) {
        regs.TASKS_STOP = 1;
        regs.TASKS_START = 0;
        continue;
      }
      if ((irq & (1 << 17)) != 0) {
        // trigger interrupt
        triggerIrq();
      }
    }
    if (ticks == regs.CC2) {
      bhmPrintf("\n\n$$$$$ Timer CC2 match\n");
      regs.EVENTS_COMPARE2 = 1;
      ppmWriteNet(timer0NotificationHandle, 0x8);
      if ((regs.SHORTS & 0x4) != 0) {
        ticks = 0;
      }
      if ((regs.SHORTS & (1 << 10)) != 0) {
        regs.TASKS_STOP = 1;
        regs.TASKS_START = 0;
        continue;
      }
      if ((irq & (1 << 18)) != 0) {
        // trigger interrupt
        triggerIrq();
      }
    }
    if (ticks == regs.CC3) {
      bhmPrintf("\n\n$$$$$ Timer CC3 match irq = 0x%08x, shorts = 0x%08x\n", irq, regs.SHORTS);
      regs.EVENTS_COMPARE3 = 1;
      ppmWriteNet(timer0NotificationHandle, 0x8);
      if ((regs.SHORTS & 0x8) != 0) {
        ticks = 0;
      }
      if ((regs.SHORTS & (1 << 11)) != 0) {
        regs.TASKS_STOP = 1;
        regs.TASKS_START = 0;
        continue;
      }
      if ((irq & (1 << 19)) != 0) {
        // trigger interrupt
        triggerIrq();
      }
    }

    //bhmPrintf("\n\n\n$$$$$ loop ticks = %d, cc0 = %d, cc1 = %d \n\n\n", ticks, regs.CC0, regs.CC1);

    updateIrqLines();

    // increment the tick counter and wait for "some time to pass".
    ticks++;

    if (regs.BITMODE == 0) {
      ticks = ticks & 0xFFFF;
    } else if (regs.BITMODE == 1) {
      ticks = ticks & 0xFF;
    } else if (regs.BITMODE == 2) {
      ticks = ticks & 0xFFFFFF;
    }
    
    bhmWaitDelay( 1000000.0 / (double)(16000000 >> regs.PRESCALER)); // in uS
  }

}
