#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "timerModel.h"

static Uns32 ticks = 0;
static Uns32 irq = 0;
static Uns32 shouldTriggerIrq = 0;
static Uns32 isStarted = 0;
static Uns32 skipCCMatch[4];

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
  } else if ((Uns32*)user == &regs.TASKS_STOP && data != 0 && isStarted != 0) {
    isStarted = 0;
    regs.TASKS_START = 0;
    bhmPrintf("TIMER STOP! (to be done)");
  } else if ((Uns32*)user == &regs.POWER && data != 0) {
    // power on
  } else if ((Uns32*)user == &regs.POWER && data == 0) {
    isStarted = 0;
    regs.TASKS_STOP = 1;
    regs.TASKS_START = 0;
    ticks = 0;
    bhmPrintf("TIMER POWER DOWN!\n");
  } else if ((Uns32*)user == &regs.MODE && data == 1) {
    bhmPrintf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! COUNTER MODE NOT SUPPORTED YET!");
    exit(1);
  } else if ((Uns32*)user == &regs.MODE && data == 0) {
    bhmPrintf("TIMER in timing mode!");
  } else if ((Uns32*)user == &regs.TASKS_COUNT) {
    bhmPrintf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! COUNTER MODE NOT SUPPORTED YET!");
    exit(1);
  } else if ((Uns32*)user == &regs.TASKS_CLEAR) {
    bhmPrintf("CLEAR TIMER ticks!");
    ticks = 0;
  } else if ((Uns32*)user >= &regs.TASKS_CAPTURE[0] && (Uns32*)user <= &regs.TASKS_CAPTURE[3]) {
    Uns32 id = (Uns32*)user - &regs.TASKS_CAPTURE[0];
    skipCCMatch[id] = 1;
    regs.CC[id] = ticks;
    bhmPrintf("CAPTURE[%d] TIMER ticks!", id);
  } else if ((Uns32*)user >= &regs.EVENTS_COMPARE[0] && (Uns32*)user <= &regs.EVENTS_COMPARE[3]) {
    Uns32 id = (Uns32*)user - &regs.EVENTS_COMPARE[0];
    bhmPrintf("COMPARE[%d] TIMER write!", id);
    ppmWriteNet(timer0NotificationHandle, TIMER0_PERIPHERAL_ID);
  } else if ((Uns32*)user == &regs.SHORTS) {
    bhmPrintf("TIMER SHORTS!");
  } else if ((Uns32*)user == &regs.INTENSET) {
    irq = irq | data;
    bhmPrintf("TIMER INTENSET!");
  } else if ((Uns32*)user == &regs.INTENCLR) {
    /*
       0 0 -> 0
       0 1 -> 0
       1 1 -> 0
       1 0 -> 1
     */
    irq = irq & (~data);
    bhmPrintf("TIMER INTENCLR!");
  } else if ((Uns32*)user == &regs.BITMODE) {
    bhmPrintf("TIMER BITMODE!");
  } else if ((Uns32*)user == &regs.PRESCALER) {
    bhmPrintf("TIMER PRESCALER!");
    if (data < 0 || data > 9) {
      bhmPrintf("unsupported TIMER PRESCALER value = %d!", data);
      exit(1);
    }
  } else if ((Uns32*)user >= &regs.CC[0] && (Uns32*)user <= &regs.CC[3]) {
    Uns32 id = (Uns32*)user - &regs.CC[0];
    bhmPrintf("CC[%d] TIMER write!", id);
  } else {
    bhmPrintf("\n\n\nTIMER0 unsupported memory write! 0x%08x \n\n\n", (Uns32)addr - (Uns32)timerWindow);
    exit(1);
  }
}

PPM_CONSTRUCTOR_CB(init) {

  periphConstructor();

  bhmPrintf("\n\n\n$$$$$ constructor \n\n\n");

  startEventHandle = bhmCreateNamedEvent("start", "start the timer");

  regs.TASKS_STOP = 1;
  regs.TASKS_START = 0;
}

static void triggerIrq() {
  shouldTriggerIrq = 1;
}

static void updateIrqLines() {
  if (shouldTriggerIrq == 1) {
    bhmPrintf("\n$$$$$ TIMER IRQ ON \n");
    ppmWriteNet(irqHandle, 1);
    shouldTriggerIrq = 0;
    bhmWaitDelay(5.0);
    ppmWriteNet(irqHandle, 0);
    bhmPrintf("\n$$$$$ TIMER IRQ OFF \n");
  }
}

void loop() {

  while (1) {

    while (regs.TASKS_STOP != 0 && regs.TASKS_START == 0) {
      bhmWaitEvent(startEventHandle);
    }

    const Uns32 signalBitOffset = 8;
    const Uns32 irqBitOffset = 16;
    Uns32 resetTicks = 0;
    Uns32 i;
    for (i = 0; i < 4; i++) {
      if (ticks == regs.CC[i] && skipCCMatch[i] == 0) {
        bhmPrintf("\n\n$$$$$ Timer CC[%d] match\n", i);
        regs.EVENTS_COMPARE[i] = 1;
        ppmWriteNet(timer0NotificationHandle, TIMER0_PERIPHERAL_ID);
        if ((regs.SHORTS & (1 << i)) != 0) {
          resetTicks = 1;
        }
        if ((regs.SHORTS & (1 << (signalBitOffset + i))) != 0) {
          regs.TASKS_STOP = 1;
          regs.TASKS_START = 0;
          break;
        }
        if ((irq & (1 << (irqBitOffset + i))) != 0) {
          // trigger interrupt
          triggerIrq();
        }
      }
      
      skipCCMatch[i] = 0;
    }

    if (regs.TASKS_STOP == 1 && regs.TASKS_START == 0) {
      // this will trigger wait on "start event"
      continue;
    }

    bhmPrintf("\n\n\n$$$$$ loop ticks = %d, cc[0] = %d, cc[1] = %d, cc[2] = %d, cc[3] = %d \n\n\n", ticks, regs.CC[0], regs.CC[1], regs.CC[2], regs.CC[3]);

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

    if (resetTicks == 1) {
      bhmPrintf("\n\n\n$$$$$ Timer reset ticks!!!\n\n\n");
      ticks = 0;
    }

    bhmWaitDelay( 1000000.0 / (double)(16000000 >> regs.PRESCALER)); // in uS
  }

}
