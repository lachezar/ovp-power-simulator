#include "spiModel.h"

// At the moment only SPI Master is supported

static void createRegisters(void *w) {

  ppmCreateRegister("EVENTS_READY", "", w, 0x108, 4, regRd32, regWr32, viewReg32, &regs.EVENTS_READY, True);
  ppmCreateRegister("INTENSET", "", w, 0x304, 4, regRd32, regWr32, viewReg32, &regs.INTENSET, True);
  ppmCreateRegister("INTENCLR", "", w, 0x308, 4, regRd32, regWr32, viewReg32, &regs.INTENCLR, True);
  ppmCreateRegister("ENABLE", "", w, 0x500, 4, regRd32, regWr32, viewReg32, &regs.ENABLE, True);
  ppmCreateRegister("PSELSCK", "", w, 0x508, 4, regRd32, regWr32, viewReg32, &regs.PSELSCK, True);
  ppmCreateRegister("PSELMOSI", "", w, 0x50c, 4, regRd32, regWr32, viewReg32, &regs.PSELMOSI, True);
  ppmCreateRegister("PSELMISO", "", w, 0x510, 4, regRd32, regWr32, viewReg32, &regs.PSELMISO, True);
  ppmCreateRegister("RXD", "", w, 0x518, 4, regRd32, NULL, viewReg32, &regs.RXD, True);
  ppmCreateRegister("TXD", "", w, 0x51c, 4, regRd32, regWr32, viewReg32, &regs.TXD, True);
  ppmCreateRegister("FREQUENCY", "", w, 0x524, 4, regRd32, regWr32, viewReg32, &regs.FREQUENCY, True);
  ppmCreateRegister("CONFIG", "", w, 0x554, 4, regRd32, regWr32, viewReg32, &regs.CONFIG, True);
  ppmCreateRegister("POWER", "", w, 0xffc, 4, regRd32, regWr32, viewReg32, &regs.POWER, True);
}

static void busPortConnections(void) {
  spiWindow = ppmCreateSlaveBusPort("SPI", 0x1000);

  if (!spiWindow) {
    bhmMessage("E", "PPM_SPNC", "Could not connect port 'spi_window'");
  }

  createRegisters(spiWindow);
}

static void netPortConnections(void) {
  irqHandle = ppmOpenNetPort("spi_irq");
  spiNotificationHandle = ppmOpenNetPort("spi_ppi");
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

