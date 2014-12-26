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

//  Peripheral RTC

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "peripheral/impTypes.h"
#include "peripheral/bhm.h"
#include "peripheral/ppm.h"
#include "rtcModel.h"

static Uns32 counter = 0;
static Uns32 irq = 0;
static int reset_counter = 0;
static int should_trigger_irq = 0;
static int irq_asserted = 0;

//
// View any 32-bit register
//
PPM_VIEW_CB(viewReg32) {
    //*(Uns32*)data = byteSwap(*(Uns32*)user);
    bhmPrintf("\n$$$ RTC View\n");
}

//
// Read any 32-bit register
//
PPM_READ_CB(regRd32) {
    //return byteSwap(*(Uns32*)user);
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
    bhmPrintf("\n$$$ RTC Write to 0x%08x (user - 0x%08x, data - 0x%08x, window - 0x%08x, pwr - %d) \n", (Uns32)addr - (Uns32)rtc_window, (Uns32)user, data, (Uns32)rtc_window, regs.POWER);
    
    if ((Uns32*)user == &regs.TASKS_START) {
      bhmPrintf("RTC START! (to be done), data = %d\n", data);
    }
    
    if ((Uns32*)user == &regs.TASKS_START && data != 0) {
      regs.TASKS_STOP = 0;
      bhmTriggerEvent(start_eh);
      bhmPrintf("RTC START! (to be done)");
    }
    if ((Uns32*)user == &regs.TASKS_STOP && data != 0) {
      regs.TASKS_START = 0;
      bhmPrintf("RTC STOP! (to be done)");
    }
    if ((Uns32*)user == &regs.TASKS_CLEAR) {
      bhmPrintf("CLEAR RTC counter!");
      reset_counter = 1;
    }
    if ((Uns32*)user == &regs.TASKS_TRIGOVRFLW) {
      bhmPrintf("RTC COUNTER OVERFLOW!");
      counter = 0xFFFFF0;
    }
    
    if ((Uns32*)user == &regs.EVENTS_TICK) {
      bhmPrintf("RTC tick event write!");
      ppmWriteNet(rtc_notification_handle, 0xB);
    }
    if ((Uns32*)user == &regs.EVENTS_OVRFLW) {
      bhmPrintf("RTC overflow event write!");
      ppmWriteNet(rtc_notification_handle, 0xB);
    }
    
    if ((Uns32*)user == &regs.EVENTS_COMPARE0) {
      bhmPrintf("COMPARE0 RTC write!");
      ppmWriteNet(rtc_notification_handle, 0xB);
    }
    if ((Uns32*)user == &regs.EVENTS_COMPARE1) {
      bhmPrintf("COMPARE1 RTC write!");
      ppmWriteNet(rtc_notification_handle, 0xB);
    }
    if ((Uns32*)user == &regs.EVENTS_COMPARE2) {
      bhmPrintf("COMPARE2 RTC write!");
      ppmWriteNet(rtc_notification_handle, 0xB);
    }
    if ((Uns32*)user == &regs.EVENTS_COMPARE3) {
      bhmPrintf("COMPARE3 RTC write!");
      ppmWriteNet(rtc_notification_handle, 0xB);
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
    
    bhmPrintf("\n\n\n$$$$$ constructor \n\n\n");
    
    start_eh = bhmCreateNamedEvent("start", "start the RTC");
    
    regs.TASKS_STOP = 1;
    regs.TASKS_START = 0;
}

void trigger_irq() {
  should_trigger_irq = 1;
}

void update_irq_lines() {
  if (should_trigger_irq == 1) {
    ppmWriteNet(irq_handle, 1);
    irq_asserted = 1;
    should_trigger_irq = 0;
    bhmPrintf("\n$$$$$ RTC IRQ ON \n");
    bhmWaitDelay(1.0);
    ppmWriteNet(irq_handle, 0);
    irq_asserted = 0;
    bhmPrintf("\n$$$$$ RTC IRQ OFF \n");
  } else if (should_trigger_irq == 0 && irq_asserted == 1) {
    ppmWriteNet(irq_handle, 0);
    irq_asserted = 0;
    bhmPrintf("\n$$$$$ RTC IRQ OFF \n");
  }
}

void loop() {
  
  int is_overflow = 0;
  int skip_cc_match = 0;
  
  while (1) {
    
    //bhmPrintf("RTC new cycle!");
    
    while (regs.TASKS_STOP != 0 && regs.TASKS_START == 0) {
      bhmWaitEvent(start_eh);
    }
    
    if ((regs.EVTEN & 2) != 0 && counter == 0xFFFFFF) {
      // overflow happened!!!
      bhmPrintf("RTC OVERFLOW on next cycle!");
      is_overflow = 1;
    }
    
    if ((regs.EVTEN & 1) != 0) {
      regs.EVENTS_TICK = 1;
      if ((irq & 1) != 0) {
        trigger_irq();
      }
    }
    
    if (is_overflow == 1 && counter == 0 && skip_cc_match == 0) {
      regs.EVENTS_OVRFLW = 1;
      ppmWriteNet(rtc_notification_handle, 0xB);
      if ((irq & 2) != 0) {
        trigger_irq();
      }
      is_overflow = 0;
    }
    
    if ((regs.EVTEN & (1 << 16)) != 0 && counter == regs.CC0 && skip_cc_match == 0) {
      bhmPrintf("\n\n\n$$ RTC match cc0 \n\n\n");
      regs.EVENTS_COMPARE0 = 1;
      ppmWriteNet(rtc_notification_handle, 0xB);
      if ((irq & (1 << 16)) != 0) {
        trigger_irq();
      }
    }
    
    if ((regs.EVTEN & (1 << 17)) != 0 && counter == regs.CC1 && skip_cc_match == 0) {
      regs.EVENTS_COMPARE1 = 1;
      ppmWriteNet(rtc_notification_handle, 0xB);
      bhmPrintf("\n\n\n$$ RTC match cc1 \n\n\n");
      if ((irq & (1 << 17)) != 0) {
        bhmPrintf("\n\n\n$$ RTC match cc1 IRQ \n\n\n");
        trigger_irq();
      }
    }

    if ((regs.EVTEN & (1 << 18)) != 0 && counter == regs.CC2 && skip_cc_match == 0) {
      regs.EVENTS_COMPARE2 = 1;
      ppmWriteNet(rtc_notification_handle, 0xB);
      if ((irq & (1 << 18)) != 0) {
        trigger_irq();
      }
    }
    
    if ((regs.EVTEN & (1 << 19)) != 0 && counter == regs.CC3 && skip_cc_match == 0) {
      regs.EVENTS_COMPARE3 = 1;
      ppmWriteNet(rtc_notification_handle, 0xB);
      if ((irq & (1 << 19)) != 0) {
        trigger_irq();
      }
    }
    
    update_irq_lines();

    //bhmPrintf("\n\n\n$$$$$ RTC loop counter = %d, cc1 = %d, evten = 0x%08x, %d, irq = 0x%08x, cc0 = %d, compare0 = %d, compare1 = %d \n\n\n", counter, regs.CC1, regs.EVTEN, skip_cc_match, irq, regs.CC0, regs.EVENTS_COMPARE0, regs.EVENTS_COMPARE1);
    
    counter = (counter + 1) & 0xFFFFFF;
    
    if (reset_counter != 0) { 
      reset_counter = 0;
      counter = 0;
      skip_cc_match = 1;
    } else {
      skip_cc_match = 0;
    }
    regs.TASKS_CLEAR = 0;
    
    bhmWaitDelay( 1000000.0 / (double)(32768UL / (regs.PRESCALER + 1))); // in uS
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


