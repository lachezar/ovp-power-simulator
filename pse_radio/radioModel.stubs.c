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

//  Peripheral Radio

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "peripheral/impTypes.h"
#include "peripheral/bhm.h"
#include "peripheral/ppm.h"
#include "radioModel.h"
#include "radio_state_machine.h"

static int should_trigger_irq = 0;
void trigger_irq();
void update_irq_lines();

int trigger_flow = 0;

void flow() {
  trigger_flow = 0;
  radio_state_t new_state = transit(regs.STATE, TXEN);
      bhmPrintf("\n$$$ Radio NEW STATE = %d\n", new_state);
      regs.STATE = new_state;
      new_state = transit(regs.STATE, READY);
      bhmPrintf("\n$$$ Radio NEW STATE = %d\n", new_state);
      regs.STATE = new_state;
      regs.EVENTS_READY = 1;
      ppmWriteNet(ppi_notification_handle, 1);
      bhmWaitDelay( 0.2 ); // in uS
      /*new_state = transit(regs.STATE, START);
      bhmPrintf("\n$$$ Radio NEW STATE = %d\n", new_state);
      regs.STATE = new_state;
      regs.TASKS_START = 1;*/
      if ((regs.SHORTS & 1) != 0) {
        bhmPrintf("\n$$$ Radio SHORTS trigger START\n");
        regs.TASKS_START = 1;
        
        new_state = transit(regs.STATE, START);
        bhmPrintf("\n$$$ Radio NEW STATE = %d\n", new_state);
        regs.STATE = new_state;
        //bhmWaitDelay( 1.0 ); // in uS
        
        new_state = transit(regs.STATE, ADDRESS);
        bhmPrintf("\n$$$ Radio NEW STATE = %d\n", new_state);
        regs.STATE = new_state;
        regs.EVENTS_ADDRESS = 1;
        ppmWriteNet(ppi_notification_handle, 1);
        bhmWaitDelay( 0.2 ); // in uS
        
        new_state = transit(regs.STATE, PAYLOAD);
        bhmPrintf("\n$$$ Radio NEW STATE = %d\n", new_state);
        regs.STATE = new_state;
        regs.EVENTS_PAYLOAD = 1;
        //bhmWaitDelay( 1.0 ); // in uS
        
        new_state = transit(regs.STATE, END);
        bhmPrintf("\n$$$ Radio NEW STATE = %d\n", new_state);
        regs.STATE = new_state;
        regs.EVENTS_END = 1;
        ppmWriteNet(ppi_notification_handle, 1);
        //bhmWaitDelay( 1.0 ); // in uS
        
        if ((regs.SHORTS & 2) != 0) {
          bhmPrintf("\n$$$ Radio SHORTS trigger DISABLE\n");
          regs.TASKS_DISABLE = 1;
          new_state = transit(regs.STATE, DISABLE);
          bhmPrintf("\n$$$ Radio NEW STATE = %d\n", new_state);
          regs.STATE = new_state;
          bhmWaitDelay( 0.2 ); // in uS
          
          ppmWriteNet(ppi_notification_handle, 1);
          regs.EVENTS_DISABLED = 1;
          new_state = transit(regs.STATE, DISABLED_EVENT);
          bhmPrintf("\n$$$ Radio NEW STATE = %d\n", new_state);
          regs.STATE = new_state;
          //bhmWaitDelay( 1.0 ); // in uS
          
          if ((regs.INTENSET & 0x10) != 0) {
            // trigger irq
            trigger_irq();
            update_irq_lines();
            bhmWaitDelay( 50.0 ); // in uS
            bhmPrintf("\n WTF???? \n");
            trigger_irq();
            update_irq_lines();
            //bhmWaitDelay( 0.02 );
          }
        }
      }
}

//
// View any 32-bit register
//
PPM_VIEW_CB(viewReg32) {
    //*(Uns32*)data = byteSwap(*(Uns32*)user);
    bhmPrintf("\n$$$ Radio View\n");
}

//
// Read any 32-bit register
//
PPM_READ_CB(regRd32) {
    //return byteSwap(*(Uns32*)user);
    /*if ((Uns32*)user == &regs.COUNTER) {
      
    }*/
    Uns32 offset = (Uns32)addr - (Uns32)radio_window;
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
    bhmPrintf("\n$$$ Radio Write 0x%08x to address 0x%08x\n", data, (Uns32)addr - (Uns32)radio_window);
    
    if ((Uns32*)user == &regs.TASKS_TXEN) {
      trigger_flow = 1;
      bhmPrintf("\n WTF!!!! \n");
    } else if ((Uns32*)user == &regs.INTENSET) {
      bhmPrintf("Radio INTENSET = 0x%08x\n", data);
    } else if ((Uns32*)user == &regs.TASKS_START) {
      bhmPrintf("Radio START! (to be done), data = %d\n", data);
    } else if ((Uns32*)user == &regs.TASKS_START && data != 0) {
      //regs.TASKS_STOP = 0;
      //bhmTriggerEvent(start_eh);
      bhmPrintf("Radio START! (to be done)");
    } else if (((Uns32)addr - (Uns32)radio_window) <= 0x20) {
      bhmPrintf("Radio task not handled");
    } else {
      //bhmPrintf("Radio not handled");
    }
}

PPM_CONSTRUCTOR_CB(init) {

    periphConstructor();
    
    bhmPrintf("\n\n\n$$$$$ constructor \n\n\n");
    
    start_eh = bhmCreateNamedEvent("start", "start the Radio");
    
    regs.TASKS_STOP = 1;
    regs.TASKS_START = 0;
    regs.STATE = RADIO_STATE_STATE_Disabled;
}

void trigger_irq() {
  should_trigger_irq = 1;
}

void update_irq_lines() {
  if (should_trigger_irq == 1) {
    ppmWriteNet(irq_handle, 1);
    should_trigger_irq = 0;
    bhmPrintf("\n$$$$$ Radio IRQ ON \n");
    bhmWaitDelay(0.2);
    ppmWriteNet(irq_handle, 0);
    bhmPrintf("\n$$$$$ Radio IRQ OFF \n");
  }
}

void loop() {
  
  while (1) {
    
    //bhmPrintf("RTC new cycle!");
    
    /*while (regs.TASKS_STOP != 0 && regs.TASKS_START == 0) {
      bhmWaitEvent(start_eh);
    }*/
    
    
    
  /*  if ((regs.EVTEN & (1 << 19)) != 0 && counter == regs.CC3 && skip_cc_match == 0) {
      regs.EVENTS_COMPARE3 = 1;
      if ((irq & (1 << 19)) != 0) {
        trigger_irq();
      }
    }
    */
  
    if (trigger_flow != 0) {
      flow();
    }
  
    //update_irq_lines();

    //bhmPrintf("\n\n\n$$$$$ RTC loop counter = %d, cc1 = %d, evten = 0x%08x, %d, irq = 0x%08x, cc0 = %d, compare0 = %d, compare1 = %d \n\n\n", counter, regs.CC1, regs.EVTEN, skip_cc_match, irq, regs.CC0, regs.EVENTS_COMPARE0, regs.EVENTS_COMPARE1);
    
    //counter = (counter + 1) & 0xFFFFFF;

    
    bhmWaitDelay( 1.0 ); // in uS
  }
  
}

void userReset(Uns32 v) {

}




