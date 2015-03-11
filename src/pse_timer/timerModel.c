#include "timerModel.h"

static void createRegisters(void *w) {

  ppmCreateRegister("TASKS_START", "", w, 0x00, 4, NULL, regWr32, viewReg32, &(regs.TASKS_START), True);
  ppmCreateRegister("TASKS_STOP", "", w, 0x04, 4, NULL, regWr32, viewReg32, &regs.TASKS_STOP, True);
  ppmCreateRegister("TASKS_COUNT", "", w, 0x08, 4, NULL, regWr32, viewReg32, &regs.TASKS_COUNT, True);
  ppmCreateRegister("TASKS_CLEAR", "", w, 0x0c, 4, NULL, regWr32, viewReg32, &regs.TASKS_CLEAR, True);
  ppmCreateRegister("TASKS_CAPTURE[0]", "", w, 0x40, 4, NULL, regWr32, viewReg32, &regs.TASKS_CAPTURE[0], True);
  ppmCreateRegister("TASKS_CAPTURE[1]", "", w, 0x44, 4, NULL, regWr32, viewReg32, &regs.TASKS_CAPTURE[1], True);
  ppmCreateRegister("TASKS_CAPTURE[2]", "", w, 0x48, 4, NULL, regWr32, viewReg32, &regs.TASKS_CAPTURE[2], True);
  ppmCreateRegister("TASKS_CAPTURE[3]", "", w, 0x4c, 4, NULL, regWr32, viewReg32, &regs.TASKS_CAPTURE[3], True);

  ppmCreateRegister("EVENTS_COMPARE[0]", "", w, 0x140, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_COMPARE[0], True);
  ppmCreateRegister("EVENTS_COMPARE[1]", "", w, 0x144, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_COMPARE[1], True);
  ppmCreateRegister("EVENTS_COMPARE[2]", "", w, 0x148, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_COMPARE[2], True);
  ppmCreateRegister("EVENTS_COMPARE[3]", "", w, 0x14c, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_COMPARE[3], True);

  ppmCreateRegister("SHORTS", "", w, 0x200, 4, regRd32, regWr32, viewReg32, &regs.SHORTS, True);
  ppmCreateRegister("INTENSET", "", w, 0x304, 4, regRd32, regWr32, viewReg32, &regs.INTENSET, True);
  ppmCreateRegister("INTENCLR", "", w, 0x308, 4, regRd32, regWr32, viewReg32, &regs.INTENCLR, True);
  ppmCreateRegister("MODE", "", w, 0x504, 4, regRd32, regWr32, viewReg32, &regs.MODE, True);
  ppmCreateRegister("BITMODE", "", w, 0x508, 4, regRd32, regWr32, viewReg32, &regs.BITMODE, True);
  ppmCreateRegister("PRESCALER", "", w, 0x510, 4, regRd32, regWr32, viewReg32, &regs.PRESCALER, True);

  ppmCreateRegister("CC[0]", "", w, 0x540, 4, regRd32, regWr32, viewReg32, &regs.CC[0], True);
  ppmCreateRegister("CC[1]", "", w, 0x544, 4, regRd32, regWr32, viewReg32, &regs.CC[1], True);
  ppmCreateRegister("CC[2]", "", w, 0x548, 4, regRd32, regWr32, viewReg32, &regs.CC[2], True);
  ppmCreateRegister("CC[3]", "", w, 0x54c, 4, regRd32, regWr32, viewReg32, &regs.CC[3], True);

  ppmCreateRegister("POWER", "", w, 0xffc, 4, regRd32, regWr32, viewReg32, &(regs.POWER), True);
}

//
// Connect bus ports
//
static void busPortConnections(void) {

  timerWindow = ppmCreateSlaveBusPort("TIMER", 0x1000);

  if (!timerWindow) {
    bhmMessage("E", "PPM_SPNC", "Could not connect port 'timerWindow'");
  }

  createRegisters(timerWindow);
}

//
// Connect net ports
//
static void netPortConnections(void) {
  irqHandle = ppmOpenNetPort("timer_irq");
  timer0NotificationHandle = ppmOpenNetPort("timer0_ppi");
}

PPM_CONSTRUCTOR_CB(periphConstructor) {
  busPortConnections();
  netPortConnections();
  bhmPrintf("\n\n\nHELLO WORLD! From the Timer Constructor!\n\n\n");
}

int main(int argc, char **argv) {
  init();
  loop();
  return 0;
}

