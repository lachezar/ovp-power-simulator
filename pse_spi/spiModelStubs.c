#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "spiModel.h"
#include "spiDeviceImplementation.h"

#define DEV_IMPL_LEN 20

// At the moment only SPI Master is supported
// Also no double buffering of the RX/TX registers

static Uns32 irq = 0;
static Uns32 peripheralId;
static char deviceImplementation[DEV_IMPL_LEN];
static Uns8 txd[2];
static Uns8 txdSize = 0;

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
    bhmPrintf("\n!!!READ RXD REGISTER - %d (%d)\n", *(Uns32*)user, peripheralId);
  }

  return *(Uns32*)user;
}

//
// Update any 32-bit register (with no side effects)
//
PPM_WRITE_CB(regWr32) {
  *(Uns32*)user = data;

  bhmPrintf("\n!!! SPI WRITE %d to 0x%x (%d)\n", data, (Uns32)addr - (Uns32)spiWindow, peripheralId);

  if ((Uns32*)user == &regs.ENABLE && data != 0) {
    bhmTriggerEvent(startEventHandle);
    bhmPrintf("\n!!! SPI IS ENABLED (%d)\n", peripheralId);
  } else if ((Uns32*)user == &regs.ENABLE && data == 0) {
    bhmPrintf("\n!!! SPI IS DISABLED (%d)\n", peripheralId);
  } else if ((Uns32*)user == &regs.TXD) {

    if (txdSize > 1) {
      bhmPrintf("!!!ERROR invalid TXD SIZE %d!\n", txdSize);
      exit(1);
    }

    txd[txdSize] = data;
    txdSize++;

    bhmTriggerEvent(txdEventHandle);
    bhmPrintf("\n!!! SPI TXD %d (%d)\n", data, peripheralId);
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
  txdEventHandle = bhmCreateNamedEvent("wait_for_txd", "wait for TXD");

  regs.ENABLE = 0;
  regs.FREQUENCY = 0x02000000;

  bhmIntegerAttribute("peripheral_id", &peripheralId);
  bhmPrintf("\n$$$$$ SPI PERIPHERAL ID: %d \n", peripheralId);
  bhmStringAttribute("device_implementation", deviceImplementation, DEV_IMPL_LEN);
  bhmPrintf("\n$$$$$ SPI DEVICE IMPLEMENTATION: %s \n", deviceImplementation);
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

    while (txdSize == 0) {
      bhmWaitEvent(txdEventHandle);
    }

    if (strcmp(deviceImplementation, DEVICE_IMPLEMENTATION_REPEAT) == 0) {
      if (txdSize == 0) {
        bhmPrintf("!!!ERROR TXD SIZE IS 0!\n");
        exit(1);
      } else {
        regs.RXD = repeat(txd[0]);
        txd[0] = txd[1];
        txdSize--;
      }
    } else {
      bhmPrintf("!!!NO DEVICE IMPLEMENTATION SELECTED!\n");
      exit(1);
    }

    if (regs.EVENTS_READY == 0) {
      regs.EVENTS_READY = 1;
      ppmWriteNet(spiNotificationHandle, peripheralId);
      if ((irq & (1 << 2)) != 0) {
        updateIrqLines();
      }
    }

    bhmWaitDelay( 0x80000000 / regs.FREQUENCY ); // in uS
  }
}
