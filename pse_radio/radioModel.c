#include "radioModel.h"

static void createRegisters(void *w) {

  ppmCreateRegister("TASKS_TXEN", "", w, 0x00, 4, NULL, regWr32, viewReg32, &regs.TASKS_TXEN, True);
  ppmCreateRegister("TASKS_RXEN", "", w, 0x04, 4, NULL, regWr32, viewReg32, &regs.TASKS_RXEN, True);
  ppmCreateRegister("TASKS_START", "", w, 0x08, 4, NULL, regWr32, viewReg32, &regs.TASKS_START, True);
  ppmCreateRegister("TASKS_STOP", "", w, 0x0C, 4, NULL, regWr32, viewReg32, &regs.TASKS_STOP, True);
  ppmCreateRegister("TASKS_DISABLE", "", w, 0x10, 4, NULL, regWr32, viewReg32, &regs.TASKS_DISABLE, True);
  ppmCreateRegister("TASKS_RSSISTART", "", w, 0x14, 4, NULL, regWr32, viewReg32, &regs.TASKS_RSSISTART, True);
  ppmCreateRegister("TASKS_RSSISTOP", "", w, 0x18, 4, NULL, regWr32, viewReg32, &regs.TASKS_RSSISTOP, True);
  ppmCreateRegister("TASKS_BCSTART", "", w, 0x1C, 4, NULL, regWr32, viewReg32, &regs.TASKS_BCSTART, True);
  ppmCreateRegister("TASKS_BCSTOP", "", w, 0x20, 4, NULL, regWr32, viewReg32, &regs.TASKS_BCSTOP, True);

  ppmCreateRegister("EVENTS_READY", "", w, 0x100, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_READY, True);
  ppmCreateRegister("EVENTS_ADDRESS", "", w, 0x104, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_ADDRESS, True);
  ppmCreateRegister("EVENTS_PAYLOAD", "", w, 0x108, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_PAYLOAD, True);
  ppmCreateRegister("EVENTS_END", "", w, 0x10C, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_END, True);
  ppmCreateRegister("EVENTS_DISABLED", "", w, 0x110, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_DISABLED, True);
  ppmCreateRegister("EVENTS_DEVMATCH", "", w, 0x114, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_DEVMATCH, True);
  ppmCreateRegister("EVENTS_DEVMISS", "", w, 0x118, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_DEVMISS, True);
  ppmCreateRegister("EVENTS_RSSIEND", "", w, 0x11C, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_RSSIEND, True);
  ppmCreateRegister("EVENTS_BCMATCH", "", w, 0x128, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_BCMATCH, True);

  ppmCreateRegister("SHORTS", "", w, 0x200, 4, regRd32, regWr32, viewReg32, &regs.SHORTS, True);
  ppmCreateRegister("INTENSET", "", w, 0x304, 4, regRd32, regWr32, viewReg32, &regs.INTENSET, True);
  ppmCreateRegister("INTENCLR", "", w, 0x308, 4, regRd32, regWr32, viewReg32, &regs.INTENCLR, True);
  ppmCreateRegister("CRCSTATUS", "", w, 0x400, 4, regRd32, NULL, viewReg32, &regs.CRCSTATUS, True);
  ppmCreateRegister("CD", "", w, 0x404, 4, regRd32, NULL, viewReg32, &regs.CD, True);
  ppmCreateRegister("RXMATCH", "", w, 0x408, 4, regRd32, NULL, viewReg32, &regs.RXMATCH, True);
  ppmCreateRegister("RXCRC", "", w, 0x40C, 4, regRd32, NULL, viewReg32, &regs.RXCRC, True);
  ppmCreateRegister("DAI", "", w, 0x410, 4, regRd32, NULL, viewReg32, &regs.DAI, True);

  ppmCreateRegister("PACKETPTR", "", w, 0x504, 4, regRd32, regWr32, viewReg32, &regs.PACKETPTR, True);
  ppmCreateRegister("FREQUENCY", "", w, 0x508, 4, regRd32, regWr32, viewReg32, &regs.FREQUENCY, True);
  ppmCreateRegister("TXPOWER", "", w, 0x50C, 4, regRd32, regWr32, viewReg32, &regs.TXPOWER, True);
  ppmCreateRegister("MODE", "", w, 0x510, 4, regRd32, regWr32, viewReg32, &regs.MODE, True);
  ppmCreateRegister("PCNF0", "", w, 0x514, 4, regRd32, regWr32, viewReg32, &regs.PCNF0, True);
  ppmCreateRegister("PCNF1", "", w, 0x518, 4, regRd32, regWr32, viewReg32, &regs.PCNF1, True);
  ppmCreateRegister("BASE0", "", w, 0x51C, 4, regRd32, regWr32, viewReg32, &regs.BASE0, True);
  ppmCreateRegister("BASE1", "", w, 0x520, 4, regRd32, regWr32, viewReg32, &regs.BASE1, True);
  ppmCreateRegister("PREFIX0", "", w, 0x524, 4, regRd32, regWr32, viewReg32, &regs.PREFIX0, True);
  ppmCreateRegister("PREFIX1", "", w, 0x528, 4, regRd32, regWr32, viewReg32, &regs.PREFIX1, True);
  ppmCreateRegister("TXADDRESS", "", w, 0x52C, 4, regRd32, regWr32, viewReg32, &regs.TXADDRESS, True);
  ppmCreateRegister("RXADDRESSES", "", w, 0x530, 4, regRd32, regWr32, viewReg32, &regs.RXADDRESSES, True);
  ppmCreateRegister("CRCCNF", "", w, 0x534, 4, regRd32, regWr32, viewReg32, &regs.CRCCNF, True);
  ppmCreateRegister("CRCPOLY", "", w, 0x538, 4, regRd32, regWr32, viewReg32, &regs.CRCPOLY, True);
  ppmCreateRegister("CRCINIT", "", w, 0x53C, 4, regRd32, regWr32, viewReg32, &regs.CRCINIT, True);
  ppmCreateRegister("TEST", "", w, 0x540, 4, regRd32, regWr32, viewReg32, &regs.TEST, True);
  ppmCreateRegister("TIFS", "", w, 0x544, 4, regRd32, regWr32, viewReg32, &regs.TIFS, True);
  ppmCreateRegister("RSSISAMPLE", "", w, 0x548, 4, regRd32, NULL, viewReg32, &regs.RSSISAMPLE, True);
  ppmCreateRegister("STATE", "", w, 0x550, 4, regRd32, NULL, viewReg32, &regs.STATE, True);
  ppmCreateRegister("DATAWHITEIV", "", w, 0x554, 4, regRd32, regWr32, viewReg32, &regs.DATAWHITEIV, True);
  ppmCreateRegister("BCC", "", w, 0x560, 4, regRd32, regWr32, viewReg32, &regs.BCC, True);

  ppmCreateRegister("DAB[0]", "", w, 0x600, 4, regRd32, regWr32, viewReg32, &regs.DAB[0], True);
  ppmCreateRegister("DAB[1]", "", w, 0x604, 4, regRd32, regWr32, viewReg32, &regs.DAB[1], True);
  ppmCreateRegister("DAB[2]", "", w, 0x608, 4, regRd32, regWr32, viewReg32, &regs.DAB[2], True);
  ppmCreateRegister("DAB[3]", "", w, 0x60C, 4, regRd32, regWr32, viewReg32, &regs.DAB[3], True);
  ppmCreateRegister("DAB[4]", "", w, 0x610, 4, regRd32, regWr32, viewReg32, &regs.DAB[4], True);
  ppmCreateRegister("DAB[5]", "", w, 0x614, 4, regRd32, regWr32, viewReg32, &regs.DAB[5], True);
  ppmCreateRegister("DAB[6]", "", w, 0x618, 4, regRd32, regWr32, viewReg32, &regs.DAB[6], True);
  ppmCreateRegister("DAB[7]", "", w, 0x61C, 4, regRd32, regWr32, viewReg32, &regs.DAB[7], True);

  ppmCreateRegister("DAP[0]", "", w, 0x620, 4, regRd32, regWr32, viewReg32, &regs.DAP[0], True);
  ppmCreateRegister("DAP[1]", "", w, 0x624, 4, regRd32, regWr32, viewReg32, &regs.DAP[1], True);
  ppmCreateRegister("DAP[2]", "", w, 0x628, 4, regRd32, regWr32, viewReg32, &regs.DAP[2], True);
  ppmCreateRegister("DAP[3]", "", w, 0x62C, 4, regRd32, regWr32, viewReg32, &regs.DAP[3], True);
  ppmCreateRegister("DAP[4]", "", w, 0x630, 4, regRd32, regWr32, viewReg32, &regs.DAP[4], True);
  ppmCreateRegister("DAP[5]", "", w, 0x634, 4, regRd32, regWr32, viewReg32, &regs.DAP[5], True);
  ppmCreateRegister("DAP[6]", "", w, 0x638, 4, regRd32, regWr32, viewReg32, &regs.DAP[6], True);
  ppmCreateRegister("DAP[7]", "", w, 0x63C, 4, regRd32, regWr32, viewReg32, &regs.DAP[7], True);

  ppmCreateRegister("DACNF", "", w, 0x640, 4, regRd32, regWr32, viewReg32, &regs.DACNF, True);

  ppmCreateRegister("OVERRIDE0", "", w, 0x724, 4, regRd32, regWr32, viewReg32, &regs.OVERRIDE0, True);
  ppmCreateRegister("OVERRIDE1", "", w, 0x728, 4, regRd32, regWr32, viewReg32, &regs.OVERRIDE1, True);
  ppmCreateRegister("OVERRIDE2", "", w, 0x72C, 4, regRd32, regWr32, viewReg32, &regs.OVERRIDE2, True);
  ppmCreateRegister("OVERRIDE3", "", w, 0x730, 4, regRd32, regWr32, viewReg32, &regs.OVERRIDE3, True);
  ppmCreateRegister("OVERRIDE4", "", w, 0x734, 4, regRd32, regWr32, viewReg32, &regs.OVERRIDE4, True);

  ppmCreateRegister("POWER", "", w, 0xffc, 4, regRd32, regWr32, viewReg32, &regs.POWER, True);
}

//
// Connect bus ports
//
static void busPortConnections(void) {

  radioWindow = ppmCreateSlaveBusPort("RADIO", 0x1000);

  if (!radioWindow) {
    bhmMessage("E", "PPM_SPNC", "Could not connect port 'radio_window'");
  }

  createRegisters(radioWindow);
}

//
// Connect net ports
//
static void netPortConnections(void) {
  irqHandle = ppmOpenNetPort("radio_irq");
  ppiNotificationHandle = ppmOpenNetPort("radio_ppi");
}

PPM_CONSTRUCTOR_CB(periphConstructor) {
  busPortConnections();
  netPortConnections();
  bhmPrintf("\n\n\nHELLO WORLD! From the Radio Constructor!\n\n\n");
}

int main(int argc, char **argv) {
  init();
  loop();
  return 0;
}

