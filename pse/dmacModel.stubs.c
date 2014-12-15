/*
 * Copyright (c) 2005-2014 Imperas Software Ltd., www.imperas.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied.
 *
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

//  Peripheral DMAC

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "peripheral/impTypes.h"
#include "peripheral/bhm.h"
#include "peripheral/ppm.h"
#include "dmacModel.h"

static Uns32 ticks = 0;
static Uns32 irq = 0;
static Uns32 should_trigger_irq = 0;

//
// View any 32-bit register
//
PPM_VIEW_CB(viewReg32) {
    //*(Uns32*)data = byteSwap(*(Uns32*)user);
    bhmPrintf("\n$$$ Timer View\n");
}

//
// Read any 32-bit register
//
PPM_READ_CB(regRd32) {
    //return byteSwap(*(Uns32*)user);
    bhmPrintf("\n$$$ Timer Read from 0x%08x\n", *(Uns32*)addr);
    return *(Uns32*)user;
}

//
// Update any 32-bit register (with no side effects)
//
PPM_WRITE_CB(regWr32) {
    *(Uns32*)user = data;
    bhmPrintf("\n$$$ Timer Write to 0x%08x (user - 0x%08x, data - 0x%08x, window - 0x%08x, pwr - %d) \n", (Uns32)addr - (Uns32)timer_window, (Uns32)user, data, (Uns32)timer_window, regs.POWER);
    
    if ((Uns32*)user == &regs.TASKS_START) {
      bhmPrintf("TIMER START! (to be done), data = %d\n", data);
    }
    
    if ((Uns32*)user == &regs.TASKS_START && data != 0) {
      regs.TASKS_STOP = 0;
      bhmTriggerEvent(start_eh);
      bhmPrintf("TIMER START! (to be done)");
    }
    if ((Uns32*)user == &regs.TASKS_STOP && data != 0) {
      regs.TASKS_START = 0;
      bhmPrintf("TIMER STOP! (to be done)");
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
    }
    if ((Uns32*)user == &regs.EVENTS_COMPARE1) {
      bhmPrintf("COMPARE1 TIMER write!");
    }
    if ((Uns32*)user == &regs.EVENTS_COMPARE2) {
      bhmPrintf("COMPARE2 TIMER write!");
    }
    if ((Uns32*)user == &regs.EVENTS_COMPARE3) {
      bhmPrintf("COMPARE3 TIMER write!");
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
    
    start_eh = bhmCreateNamedEvent("start", "start the timer");
    
    regs.TASKS_STOP = 1;
    regs.TASKS_START = 0;
}

void trigger_irq() {
  should_trigger_irq = 1;
}

void update_irq_lines() {
  if (should_trigger_irq == 1) {
    bhmPrintf("\n$$$$$ RTC IRQ ON \n");
    ppmWriteNet(irq_handle, 1);
    should_trigger_irq = 0;
    bhmWaitDelay(1.0);
    ppmWriteNet(irq_handle, 0);
    bhmPrintf("\n$$$$$ RTC IRQ OFF \n");
  }
}

void loop() {
  
  while (1) {
    
    while (regs.TASKS_STOP != 0 && regs.TASKS_START == 0) {
      bhmWaitEvent(start_eh);
    }
    
    ticks++;
    
    if (regs.BITMODE == 0) {
      ticks = ticks & 0xFFFF;
    } else if (regs.BITMODE == 1) {
      ticks = ticks & 0xFF;
    } else if (regs.BITMODE == 2) {
      ticks = ticks & 0xFFFFFF;
    }
    
    if (ticks == regs.CC0) {
      regs.EVENTS_COMPARE0 = 1;
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
        trigger_irq();
      }
    }
    if (ticks == regs.CC1) {
      regs.EVENTS_COMPARE1 = 1;
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
        trigger_irq();
      }
    }
    if (ticks == regs.CC2) {
      regs.EVENTS_COMPARE2 = 1;
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
        trigger_irq();
      }
    }
    if (ticks == regs.CC3) {
      regs.EVENTS_COMPARE3 = 1;
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
        trigger_irq();
      }
    }
    
    bhmPrintf("\n\n\n$$$$$ loop ticks = %d \n\n\n", ticks);
    
    update_irq_lines();
    
    bhmWaitDelay( 1000000.0 / (double)(16000000 >> regs.PRESCALER)); // in uS
  }
  
}


/*static void updateInterrupts(void)
{

}*/

void userReset(Uns32 v) {

}

/*void userInit(void) {
  bhmPrintf("\n\n\nHELLO WORLD!\n\n\n");
  int i = 10;
  while (i-- > 0) {
    bhmWaitDelay(10);
    bhmPrintf("tick\n");
    bhmWaitDelay(10);
    bhmPrintf("tock\n");
  }
  bhmPrintf("\n\n\nHELLO WORLD!\n\n\n");
}*/


