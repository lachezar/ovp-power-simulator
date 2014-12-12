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

#include "peripheral/impTypes.h"
#include "peripheral/bhm.h"
#include "peripheral/ppm.h"
#include "dmacModel.h"


static inline Uns32 byteSwap(Uns32 data){
#ifdef ENDIANBIG
    return
        ((data & 0xff000000) >> 24) |
        ((data & 0x00ff0000) >>  8) |
        ((data & 0x0000ff00) <<  8) |
        ((data & 0x000000ff) << 24);
#else
    return data;
#endif
}


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
    //*(Uns32*)user =  byteSwap(data);
    bhmPrintf("\n$$$ Timer Write to 0x%08x (user - 0x%08x, data - 0x%08x, window - 0x%08x) \n", (Uns32)addr - (Uns32)timer_window, (Uns32)user, data, (Uns32)timer_window);
}

PPM_CONSTRUCTOR_CB(init) {

    periphConstructor();

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


