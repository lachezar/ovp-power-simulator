#include "rtcModel.h"

static void createRegisters(void *w) {

  ppmCreateRegister("TASKS_START", "", w, 0x00, 4, NULL, regWr32, viewReg32, &regs.TASKS_START, True);
  ppmCreateRegister("TASKS_STOP", "", w, 0x04, 4, NULL, regWr32, viewReg32, &regs.TASKS_STOP, True);
  ppmCreateRegister("TASKS_CLEAR", "", w, 0x08, 4, NULL, regWr32, viewReg32, &regs.TASKS_CLEAR, True);
  ppmCreateRegister("TASKS_TRIGOVRFLW", "", w, 0x0c, 4, NULL, regWr32, viewReg32, &regs.TASKS_TRIGOVRFLW, True);

  ppmCreateRegister("EVENTS_TICK", "", w, 0x100, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_TICK, True);
  ppmCreateRegister("EVENTS_OVRFLW", "", w, 0x104, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_OVRFLW, True);

  ppmCreateRegister("EVENTS_COMPARE[0]", "", w, 0x140, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_COMPARE[0], True);
  ppmCreateRegister("EVENTS_COMPARE[1]", "", w, 0x144, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_COMPARE[1], True);
  ppmCreateRegister("EVENTS_COMPARE[2]", "", w, 0x148, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_COMPARE[2], True);
  ppmCreateRegister("EVENTS_COMPARE[3]", "", w, 0x14c, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_COMPARE[3], True);

  ppmCreateRegister("INTENSET", "", w, 0x304, 4, regRd32, regWr32, viewReg32, &regs.INTENSET, True);
  ppmCreateRegister("INTENCLR", "", w, 0x308, 4, regRd32, regWr32, viewReg32, &regs.INTENCLR, True);
  ppmCreateRegister("EVTEN", "", w, 0x340, 4, regRd32, regWr32, viewReg32, &regs.EVTEN, True);
  ppmCreateRegister("EVTENSET", "", w, 0x344, 4, regRd32, regWr32, viewReg32, &regs.EVTENSET, True);
  ppmCreateRegister("EVTENCLR", "", w, 0x348, 4, regRd32, regWr32, viewReg32, &regs.EVTENCLR, True);

  ppmCreateRegister("COUNTER", "", w, 0x504, 4, regRd32, NULL, viewReg32, &regs.COUNTER, True);
  ppmCreateRegister("PRESCALER", "", w, 0x508, 4, regRd32, regWr32, viewReg32, &regs.PRESCALER, True);

  ppmCreateRegister("CC[0]", "", w, 0x540, 4, regRd32, regWr32, viewReg32, &regs.CC[0], True);
  ppmCreateRegister("CC[1]", "", w, 0x544, 4, regRd32, regWr32, viewReg32, &regs.CC[1], True);
  ppmCreateRegister("CC[2]", "", w, 0x548, 4, regRd32, regWr32, viewReg32, &regs.CC[2], True);
  ppmCreateRegister("CC[3]", "", w, 0x54c, 4, regRd32, regWr32, viewReg32, &regs.CC[3], True);

  ppmCreateRegister("POWER", "", w, 0xffc, 4, regRd32, regWr32, viewReg32, &regs.POWER, True);
}

static void busPortConnections(void) {
  rtcWindow = ppmCreateSlaveBusPort("RTC", 0x1000);

  if (!rtcWindow) {
    bhmMessage("E", "PPM_SPNC", "Could not connect port 'rtc_window'");
  }

  createRegisters(rtcWindow);
}

static void netPortConnections(void) {
  irqHandle = ppmOpenNetPort("rtc_irq");
  rtcNotificationHandle = ppmOpenNetPort("rtc_ppi");
}

PPM_CONSTRUCTOR_CB(periphConstructor) {
  busPortConnections();
  netPortConnections();
}

int main(int argc, char **argv) {
  init();
  loop();
  return 0;
}

