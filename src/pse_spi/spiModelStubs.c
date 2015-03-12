#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "spiModel.h"
#include "spiDeviceImplementation.h"

#define DEV_IMPL_LEN 20

#undef info
#define info(format, ...) bhmPrintf("*** SPI(%d): ", peripheralId);\
bhmPrintf(format, ##__VA_ARGS__);\
bhmPrintf("\n")

#undef error
#define error(format, ...) bhmPrintf(">>> ERROR IN SPI(%d): ", peripheralId);\
bhmPrintf(format, ##__VA_ARGS__);\
bhmPrintf("\n")


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
  info("View");
}

//
// Read any 32-bit register
//
PPM_READ_CB(regRd32) {
  if ((Uns32*)user == &regs.RXD) {
    regs.RXD &= 0xff;
    info("READ RXD REGISTER - %d (%d)\n", *(Uns32*)user, peripheralId);
  }

  return *(Uns32*)user;
}

//
// Update any 32-bit register (with no side effects)
//
PPM_WRITE_CB(regWr32) {
  *(Uns32*)user = data;

  info("WRITE %d to 0x%x (%d)\n", data, (Uns32)addr - (Uns32)spiWindow, peripheralId);

  if ((Uns32*)user == &regs.ENABLE && data != 0) {
    bhmTriggerEvent(startEventHandle);
    info("ENABLED (%d)\n", peripheralId);
  } else if ((Uns32*)user == &regs.ENABLE && data == 0) {
    info("DISABLED (%d)\n", peripheralId);
  } else if ((Uns32*)user == &regs.TXD) {

    if (txdSize > 1) {
      error("invalid TXD SIZE %d!\n", txdSize);
      exit(1);
    }

    txd[txdSize] = data;
    txdSize++;

    bhmTriggerEvent(txdEventHandle);
    info("TXD %d (%d)\n", data, peripheralId);
  } else if ((Uns32*)user == &regs.FREQUENCY) {
    Uns32 x = (data / 0x02000000);
    if (x == 0 || ((x & (x-1)) != 0)) {
      // incorrect value
      error("unsupported SPI FREQUENCY value = %d!\n", data);
      exit(1);
    }
  } else if ((Uns32*)user == &regs.INTENSET) {
    irq = irq | data;
    regs.INTENSET = irq;
    regs.INTENCLR = irq;
    info("INTENSET! irq - %d\n", irq);
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
    info("INTENCLR!");
  }
}

PPM_CONSTRUCTOR_CB(init) {

  periphConstructor();

  startEventHandle = bhmCreateNamedEvent("start", "start the SPI");
  txdEventHandle = bhmCreateNamedEvent("wait_for_txd", "wait for TXD");

  regs.ENABLE = 0;
  regs.FREQUENCY = 0x02000000;

  bhmIntegerAttribute("peripheral_id", &peripheralId);
  info("PERIPHERAL ID: %d \n", peripheralId);
  bhmStringAttribute("device_implementation", deviceImplementation, DEV_IMPL_LEN);
  info("DEVICE IMPLEMENTATION: %s \n", deviceImplementation);
}

static void updateIrqLines() {
  ppmWriteNet(irqHandle, 1);
  info("IRQ ON \n");
  bhmWaitDelay(5.0);
  ppmWriteNet(irqHandle, 0);
  info("IRQ OFF \n");
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
        error("TXD SIZE IS 0!\n");
        exit(1);
      } else {
        regs.RXD = repeat(txd[0]);
        txd[0] = txd[1];
        txdSize--;
      }
    } else {
      error("!!!NO DEVICE IMPLEMENTATION SELECTED!\n");
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
