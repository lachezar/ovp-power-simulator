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

#include "peripheral/impTypes.h"
#include "peripheral/bhm.h"
#include "peripheral/ppm.h"
#include "dmacModel.h"

timerRegs regs;

Uns32 diag = 0;

//
// BusPort handles
//


//
// netPort handles
//
ppmNetHandle irq_handle;

static void createRegisters(void *w) { // w = window?

    //                 name              description           base offset size read          write           view      user-data

    // common registers
    ppmCreateRegister("TASKS_START", "", w, 0x00, 4, NULL, regWr32, viewReg32, &(regs.TASKS_START), True);
    ppmCreateRegister("TASKS_STOP", "", w, 0x04, 4, NULL, regWr32, viewReg32, &regs.TASKS_STOP, True);
    ppmCreateRegister("TASKS_COUNT", "", w, 0x08, 4, NULL, regWr32, viewReg32, &regs.TASKS_COUNT, True);
    ppmCreateRegister("TASKS_CLEAR", "", w, 0x0c, 4, NULL, regWr32, viewReg32, &regs.TASKS_CLEAR, True);
    ppmCreateRegister("TASKS_CAPTURE0", "", w, 0x40, 4, NULL, regWr32, viewReg32, &regs.TASKS_CAPTURE0, True);
    ppmCreateRegister("TASKS_CAPTURE1", "", w, 0x44, 4, NULL, regWr32, viewReg32, &regs.TASKS_CAPTURE1, True);
    ppmCreateRegister("TASKS_CAPTURE2", "", w, 0x48, 4, NULL, regWr32, viewReg32, &regs.TASKS_CAPTURE2, True);
    ppmCreateRegister("TASKS_CAPTURE3", "", w, 0x4c, 4, NULL, regWr32, viewReg32, &regs.TASKS_CAPTURE3, True);
    
    ppmCreateRegister("EVENTS_COMPARE0", "", w, 0x140, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_COMPARE0, True);
    ppmCreateRegister("EVENTS_COMPARE1", "", w, 0x144, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_COMPARE1, True);
    ppmCreateRegister("EVENTS_COMPARE2", "", w, 0x148, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_COMPARE2, True);
    ppmCreateRegister("EVENTS_COMPARE3", "", w, 0x14c, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_COMPARE3, True);
    
    ppmCreateRegister("SHORTS", "", w, 0x200, 4, regRd32, regWr32, viewReg32, &regs.SHORTS, True);
    ppmCreateRegister("INTENSET", "", w, 0x304, 4, regRd32, regWr32, viewReg32, &regs.INTENSET, True);
    ppmCreateRegister("INTENCLR", "", w, 0x308, 4, regRd32, regWr32, viewReg32, &regs.INTENCLR, True);
    ppmCreateRegister("MODE", "", w, 0x504, 4, regRd32, regWr32, viewReg32, &regs.MODE, True);
    ppmCreateRegister("BITMODE", "", w, 0x508, 4, regRd32, regWr32, viewReg32, &regs.BITMODE, True);
    ppmCreateRegister("PRESCALER", "", w, 0x510, 4, regRd32, regWr32, viewReg32, &regs.PRESCALER, True);
    
    ppmCreateRegister("CC0", "", w, 0x540, 4, regRd32, regWr32, viewReg32, &regs.CC0, True);
    ppmCreateRegister("CC1", "", w, 0x544, 4, regRd32, regWr32, viewReg32, &regs.CC1, True);
    ppmCreateRegister("CC2", "", w, 0x548, 4, regRd32, regWr32, viewReg32, &regs.CC2, True);
    ppmCreateRegister("CC3", "", w, 0x54c, 4, regRd32, regWr32, viewReg32, &regs.CC3, True);
    
    ppmCreateRegister("POWER", "", w, 0xffc, 4, regRd32, regWr32, viewReg32, &(regs.POWER), True);
}

//
// Connect bus ports
//
static void busPortConnections(void) {
    //static unsigned char timer_Window[0x1000];

    /*timer_handle = ppmOpenSlaveBusPort(
        "TIMER",
        timer_Window,
        sizeof(timer_Window)
    );*/
    
    timer_window = ppmCreateSlaveBusPort("TIMER", 0x1000); 
    
    if (!timer_window) {
        bhmMessage("E", "PPM_SPNC", "Could not connect port 'timer_window'");
    }

    createRegisters(timer_window);
}

//
// Connect net ports
//
static void netPortConnections(void) {
    irq_handle  = ppmOpenNetPort("timer_irq");
}

//
// Called when the diagnostic level is changed by the simulator
//
static void setDiagLevel(Uns32 new) {
    diag = new;
}

PPM_CONSTRUCTOR_CB(periphConstructor) {
    busPortConnections();
    netPortConnections();
    bhmPrintf("\n\n\nHELLO WORLD! From the Timer Constructor!\n\n\n");
}

//
// Main for  DMAC
//
int main(int argc, char **argv) {
    //bhmPrintf("\n$$$1");
    bhmInstallDiagCB(setDiagLevel);
    //busPortConnections();
    //bhmPrintf("\n$$$2");
    //netPortConnections();
    //bhmPrintf("\n$$$3");
    //userInit();
    init();
    //bhmPrintf("\n$$$4");
    return 0;
}

