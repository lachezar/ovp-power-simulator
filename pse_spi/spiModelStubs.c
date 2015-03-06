#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "spiModel.h"

// At the moment only SPI Master is supported
// Also no double buffering of the RX/TX registers

static Uns32 irq = 0;
static Uns32 waitingForRXD = 1;

//
// View any 32-bit register
//
PPM_VIEW_CB(viewReg32) {
  bhmPrintf("\n$$$ SPI View\n");
}

//
// Read any 32-bit register
//
PPM_READ_CB(regRd32) {
  if ((Uns32*)user == &regs.RXD) {
    regs.RXD &= 0xff;
    waitingForRXD = 0;
    bhmPrintf("\n!!!READ RXD REGISTER - %d\n", *(Uns32*)user);
  }

  return *(Uns32*)user;
}

//
// Update any 32-bit register (with no side effects)
//
PPM_WRITE_CB(regWr32) {
  *(Uns32*)user = data;

  bhmPrintf("\n!!! SPI WRITE %d to 0x%x\n", data, (Uns32)addr - (Uns32)spiWindow);

  if ((Uns32*)user == &regs.ENABLE && data != 0) {
    bhmTriggerEvent(startEventHandle);
    bhmPrintf("\n!!! SPI IS ENABLED\n");
  } else if ((Uns32*)user == &regs.ENABLE && data == 0) {
    bhmPrintf("\n!!! SPI IS DISABLED\n");
  } else if ((Uns32*)user == &regs.TXD) {
    bhmPrintf("\n!!! SPI TXD %d\n", data);
  } else if ((Uns32*)user == &regs.FREQUENCY) {
    Uns32 x = (data / 0x02000000);
    if (x == 0 || ((x & (x-1)) != 0)) {
      // incorrect value
      bhmPrintf("unsupported SPI FREQUENCY value = %d!\n", data);
      exit(1);
    }
  } else if ((Uns32*)user == &regs.INTENSET) {
    irq = irq | data;
    regs.INTENSET = irq;
    regs.INTENCLR = irq;
    bhmPrintf("SPI INTENSET! irq - %d\n", irq);
  } else if ((Uns32*)user == &regs.INTENCLR) {
    /*
       0 0 -> 0
       0 1 -> 0
       1 1 -> 0
       1 0 -> 1
     */
    irq = irq & (~data);
    regs.INTENSET = irq;
    regs.INTENCLR = irq;
    bhmPrintf("SPI INTENCLR!");
  }
}

PPM_CONSTRUCTOR_CB(init) {

  periphConstructor();

  startEventHandle = bhmCreateNamedEvent("start", "start the SPI");
  rxdEventHandle = bhmCreateNamedEvent("wait_for_rxd", "wait for RXD");

  regs.ENABLE = 0;
  regs.FREQUENCY = 0x02000000;
}

static void updateIrqLines() {
  ppmWriteNet(irqHandle, 1);
  bhmPrintf("\n$$$$$ SPI IRQ ON \n");
  bhmWaitDelay(5.0);
  ppmWriteNet(irqHandle, 0);
  bhmPrintf("\n$$$$$ SPI IRQ OFF \n");
}

void loop() {

  while (1) {

    while (regs.ENABLE == 0) {
      bhmWaitEvent(startEventHandle);
    }

    while (waitingForRXD != 0) {
      bhmWaitEvent(rxdEventHandle);
    }
    waitingForRXD = 1;

    regs.RXD++; // fake data received in RXD

    if (regs.EVENTS_READY == 0) {
      regs.EVENTS_READY = 1;
      ppmWriteNet(spiNotificationHandle, SPI_PERIPHERAL_ID);
      if ((irq & (1 << 2)) != 0) {
        updateIrqLines();
      }
    }

    bhmWaitDelay( 50 ); // in uS
    // 0x80000000 / regs.FREQUENCY
  }
}
