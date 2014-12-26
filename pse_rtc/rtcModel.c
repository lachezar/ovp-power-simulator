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
#include "rtcModel.h"

rtcRegs regs;

Uns32 diag = 0;

//
// BusPort handles
//


//
// netPort handles
//
ppmNetHandle irq_handle;
ppmNetHandle rtc_notification_handle;

static void createRegisters(void *w) { // w = window?

    //                 name              description           base offset size read          write           view      user-data

    // common registers
    ppmCreateRegister("TASKS_START", "", w, 0x00, 4, NULL, regWr32, viewReg32, &(regs.TASKS_START), True);
    ppmCreateRegister("TASKS_STOP", "", w, 0x04, 4, NULL, regWr32, viewReg32, &regs.TASKS_STOP, True);
    ppmCreateRegister("TASKS_CLEAR", "", w, 0x08, 4, NULL, regWr32, viewReg32, &regs.TASKS_CLEAR, True);
    ppmCreateRegister("TASKS_TRIGOVRFLW", "", w, 0x0c, 4, NULL, regWr32, viewReg32, &regs.TASKS_TRIGOVRFLW, True);
    
    ppmCreateRegister("EVENTS_TICK", "", w, 0x100, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_TICK, True);
    ppmCreateRegister("EVENTS_OVRFLW", "", w, 0x104, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_OVRFLW, True);
    
    ppmCreateRegister("EVENTS_COMPARE0", "", w, 0x140, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_COMPARE0, True);
    ppmCreateRegister("EVENTS_COMPARE1", "", w, 0x144, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_COMPARE1, True);
    ppmCreateRegister("EVENTS_COMPARE2", "", w, 0x148, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_COMPARE2, True);
    ppmCreateRegister("EVENTS_COMPARE3", "", w, 0x14c, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_COMPARE3, True);
    
    ppmCreateRegister("INTENSET", "", w, 0x304, 4, regRd32, regWr32, viewReg32, &regs.INTENSET, True);
    ppmCreateRegister("INTENCLR", "", w, 0x308, 4, regRd32, regWr32, viewReg32, &regs.INTENCLR, True);
    ppmCreateRegister("EVTEN", "", w, 0x340, 4, regRd32, regWr32, viewReg32, &regs.EVTEN, True);
    ppmCreateRegister("EVTENSET", "", w, 0x344, 4, regRd32, regWr32, viewReg32, &regs.EVTENSET, True);
    ppmCreateRegister("EVTENCLR", "", w, 0x348, 4, regRd32, regWr32, viewReg32, &regs.EVTENCLR, True);
    
    ppmCreateRegister("COUNTER", "", w, 0x504, 4, regRd32, NULL, viewReg32, &regs.COUNTER, True);
    ppmCreateRegister("PRESCALER", "", w, 0x508, 4, regRd32, regWr32, viewReg32, &regs.PRESCALER, True);
    
    ppmCreateRegister("CC0", "", w, 0x540, 4, regRd32, regWr32, viewReg32, &regs.CC0, True);
    ppmCreateRegister("CC1", "", w, 0x544, 4, regRd32, regWr32, viewReg32, &regs.CC1, True);
    ppmCreateRegister("CC2", "", w, 0x548, 4, regRd32, regWr32, viewReg32, &regs.CC2, True);
    ppmCreateRegister("CC3", "", w, 0x54c, 4, regRd32, regWr32, viewReg32, &regs.CC3, True);
    
    ppmCreateRegister("POWER", "", w, 0xffc, 4, regRd32, regWr32, viewReg32, &regs.POWER, True);
}

//
// Connect bus ports
//
static void busPortConnections(void) {
    //static unsigned char rtc_Window[0x1000];

    /*rtc_handle = ppmOpenSlaveBusPort(
        "RTC",
        rtc_Window,
        sizeof(rtc_Window)
    );*/
    
    rtc_window = ppmCreateSlaveBusPort("RTC", 0x1000); 
    
    if (!rtc_window) {
        bhmMessage("E", "PPM_SPNC", "Could not connect port 'rtc_window'");
    }

    createRegisters(rtc_window);
}

//
// Connect net ports
//
static void netPortConnections(void) {
    irq_handle  = ppmOpenNetPort("rtc_irq");
    rtc_notification_handle  = ppmOpenNetPort("rtc_ppi");
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
    bhmPrintf("\n\n\nHELLO WORLD! From the RTC Constructor!\n\n\n");
}

//
// Main for  RTC
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
    
    loop();
    //bhmPrintf("\n$$$4");
    return 0;
}

