#include "commonPeripherals.h"

static Uns32 shouldStartRngInt = 0;
static Uns32 shouldStopRngInt = 0;
static Uns32 rngStarted = 0;
static Uns32 rngIrqEnabled = 0;

void initRng() {
  srand(time(NULL));
}

void startPendingRngIrq(icmNetP rngNet) {
  if (shouldStartRngInt != 0) {
    icmPrintf("****RNG IRQ SIGNAL STARTED\n");
    icmWriteNet(rngNet, 1);
    shouldStartRngInt = 0;
    shouldStopRngInt = 5; // no idea why 5!
  }
}

void stopPendingRngIrq(icmNetP rngNet) {
  if (shouldStopRngInt > 1) {
    shouldStopRngInt--;
  } else if (shouldStopRngInt != 0) {
    icmWriteNet(rngNet, 0);
    shouldStopRngInt = 0;
    icmPrintf("****RNG IRQ SIGNAL STOPPED\n");
  }
}

ICM_MEM_READ_FN(extMemReadRNGCB) {
  static Uns32 vlrdy_counter = 0;
  Uns32 data = 0;
  if ((Uns32)address == 0x4000d100) {
    if (vlrdy_counter > 0x180) { // it takes roughly 380 cycles (24us) until the random value is generated 
      data = 1;
      vlrdy_counter = 0;
    } else {
      data = 0;
      vlrdy_counter++;
    }
    icmPrintf("0.0 -> 0x%08x r\n", (Uns32)address);
  } else if ((Uns32)address == 0x4000d508) {
    data = rand();
    icmPrintf("\nRNG RAND GENERATED and READ - %d\n", data);
    icmPrintf("0.0 -> 0x%08x r\n", (Uns32)address);
  } else {
    icmPrintf("********** Not handled mem location - RNG - 0x%08x\n", (Uns32)address);
    icmTerminate();
  }
  
  *(Uns32 *)value = data;
  
  icmPrintf(
    "RNG MEMORY: Reading 0x%08x from 0x%08x\n",
    data, (Uns32)address
  );
}

ICM_MEM_WRITE_FN(extMemWriteRNGCB) {
  if ((Uns32)address == 0x4000d000) {
    rngStarted = *(Uns32*)value;
    icmPrintf("\nRNG START - %d\n", *(Uns32*)value);
    if (rngIrqEnabled != 0 && rngStarted != 0) {
      shouldStartRngInt = 1;
      icmPrintf("\nRNG SHOULD START IRQ\n");
    }
  } else if ((Uns32)address == 0x4000d304) {
    rngIrqEnabled = *(Uns32*)value;
    icmPrintf("\nRNG IRQ SET - %d\n", *(Uns32*)value);
  } else if ((Uns32)address == 0x4000d100) {
    if (*(Uns32*)value == 0 && rngIrqEnabled != 0 && rngStarted != 0) {
      shouldStartRngInt = 1;
      icmPrintf("\nRNG SHOULD START IRQ\n");
    }
    icmPrintf("\nRNG VALUE READY - %d\n", *(Uns32*)value);
    icmPrintf("0.0 -> 0x%08x w\n", (Uns32)address);
  } else if ((Uns32)address == 0x4000dffc) {
    icmPrintf("\nRNG POWER - %d\n", *(Uns32*)value);
  } else if ((Uns32)address == 0x4000d004) {
    rngStarted = 0;
    icmPrintf("\nRNG STOP - %d\n", *(Uns32*)value);
  } else {
    icmPrintf("********** Not handled mem location writing - RNG - 0x%08x\n", (Uns32)address);
    icmTerminate();
  }
  
  icmPrintf(
    "RNG MEMORY: Writing 0x%08x to 0x%08x (%d, %d, %d)\n",
    *(Uns32*)value, (Uns32)address, (Uns32)bytes, (Uns32)value, (Uns32)userData
  );
}

ICM_MEM_READ_FN(extMemReadGPIOCB) {
  icmPrintf(
    "GPIO MEMORY: Reading 0x%08x from 0x%08x\n",
    (Uns32)value, (Uns32)address
  );
}

ICM_MEM_WRITE_FN(extMemWriteGPIOCB) {
  icmPrintf(
    "GPIO MEMORY: Writing 0x%08x to 0x%08x (%d, %d, %d)\n",
    *(Uns32*)value, (Uns32)address, (Uns32)bytes, (Uns32)value, (Uns32)userData
  );
}

ICM_MEM_READ_FN(extMemReadUICRCB) {
  if ((Uns32)address == 0x10001014) {
    *(Uns32 *)value = 0xFFFFFFFF;
    icmPrintf("WTF!!!");
  } else if ((Uns32)address == 0x10001000) {
    *(Uns32 *)value = 0xFFFFFFFF;
    icmPrintf("FFS!!!");
  } else {
    icmPrintf("********** Not handled mem location - UICR");
    icmTerminate();
  }
  icmPrintf(
    "UICR MEMORY: Reading 0x%08x from 0x%08x\n",
    *(Uns32 *)value, (Uns32)address
  );
}

ICM_MEM_WRITE_FN(extMemWriteUICRCB) {
  icmPrintf(
    "UICR MEMORY: Writing 0x%08x to 0x%08x (%d, %d, %d)\n",
    *(Uns32*)value, (Uns32)address, (Uns32)bytes, (Uns32)value, (Uns32)userData
  );
}

ICM_MEM_READ_FN(extMemReadFICRCB) {
  
  const Uns32 segment1[] = { // starts in 0x10000080
    0x39938601, 0xFC6B97AE, 0xE7108AE1, 0x21E9F86C, // encryption root?
    0x6EA6F8C0, 0x7C356886, 0x93DDF20D, 0xBCBB9428, // identity root?
    0xFFFFFFFF, 0x1963137B, 0x472B354C, 0xFFFFFFF6  // DEVICEADDRTYPE, DEVICEADDR{2}, UNDOCUMENTED
  };
  
  const Uns32 segment2[] = { // starts in 0x100000EC
    0x7D005600, 0x5C000050, 0x680E8804, 0x00726424, 0x824B423E // UNDOCUMENTED
  };
  
  if ((Uns32)address == 0x1000002C) { // Pre-programmed factory code present
    *(Uns32 *)value = 0xFFFFFFFF;
  } else if ((Uns32)address >= 0x10000080 && (Uns32)address <= 0x100000AC) {
    Uns32 id = ((Uns32)address - 0x10000080) >> 2;
    *(Uns32 *)value = segment1[id];
  } else if ((Uns32)address == 0x10000014) { // CODESIZE
    *(Uns32 *)value = 0x00000100;
  } else if ((Uns32)address == 0x10000010) { // CODEPAGESIZE
    *(Uns32 *)value = 0x00000400;
  } else if ((Uns32)address == 0x10000028) { // CLENR0
    *(Uns32 *)value = 0xFFFFFFFF;
  } else if ((Uns32)address >= 0x100000EC && (Uns32)address <= 0x100000FC) { // UNDOCUMENTED
    Uns32 id = ((Uns32)address - 0x100000EC) >> 2;
    *(Uns32 *)value = segment2[id];
  } else {
    icmPrintf("********** Not handled mem location - FICR");
    icmTerminate();
  }
  icmPrintf(
    "FICR MEMORY: Reading 0x%08x from 0x%08x\n",
    *(Uns32 *)value, (Uns32)address
  );
}

ICM_MEM_WRITE_FN(extMemWriteFICRCB) {
  icmPrintf(
    "FICR MEMORY: Writing 0x%08x to 0x%08x (%d, %d, %d)\n",
    *(Uns32*)value, (Uns32)address, (Uns32)bytes, (Uns32)value, (Uns32)userData
  );
}

ICM_MEM_READ_FN(extMemReadNVMCCB) {
  if ((Uns32)address == 0x4001e400) {
    *(Uns32 *)value = 1;
  }
  icmPrintf(
    "NVMC MEMORY: Reading 0x%08x from 0x%08x\n",
    *(Uns32 *)value, (Uns32)address
  );
}

ICM_MEM_WRITE_FN(extMemWriteNVMCCB) {
  icmPrintf(
    "NVMC MEMORY: Writing 0x%08x to 0x%08x (%d, %d, %d)\n",
    *(Uns32*)value, (Uns32)address, (Uns32)bytes, (Uns32)value, (Uns32)userData
  );
}

ICM_MEM_READ_FN(extMemReadClockCB) {
  if ((Uns32)address == 0x40000104) { // EVENTS_HFCLKSTARTED
    *(Uns32 *)value = 1;
  } else if ((Uns32) address == 0x40000524) { // Power - RAM on/off
    *(Uns32 *)value = 3;
  } else if ((Uns32) address == 0x40000554) { // XTALFREQ
    *(Uns32 *)value = 0;
  } else if ((Uns32) address == 0x40000408) { // HFCLKRUN
    *(Uns32 *)value = 0;
  } else if ((Uns32) address == 0x40000414) { // LFCLKRUN
    *(Uns32 *)value = 0;
  } else if ((Uns32)address == 0x40000100) { // HFCLKSTARTED  
    *(Uns32 *)value = 1;
  } else if ((Uns32)address == 0x40000604) { // WDT Reload request 1  
    *(Uns32 *)value = 0;
  } else {
    icmPrintf("********** Not handled mem location - power/clock/mpu - 0x%08x\n", (Uns32)address);
    icmTerminate();
  }
  icmPrintf(
    "Clock MEMORY: Reading 0x%08x from 0x%08x\n",
    *(Uns32 *)value, (Uns32)address
  );
}

ICM_MEM_WRITE_FN(extMemWriteClockCB) {
  if ((Uns32)address == 0x40000524) { // Power - RAM on/off
    icmPrintf("Write to Power - RAM on/off\n");
  } else if ((Uns32)address == 0x40000514) { // Power - POFCON - power failure config
    icmPrintf("Write to Power - POFCON - power failure config\n");
  } else if ((Uns32) address == 0x40000550) { // change XTALFREQ
    icmPrintf("Write to Power - 0x40000550\n");
  } else if ((Uns32) address == 0x40000554) { // XTALFREQ
    icmPrintf("Write to Clock - XTALFREQ\n");
  } else if ((Uns32)address == 0x4000000c) { // TASKS_LFCLKSTOP
    icmPrintf("Write to Clock TASKS_LFCLKSTOP\n");
  } else if ((Uns32)address == 0x40000104) { // EVENTS_HFCLKSTARTED
    icmPrintf("Write to Clock EVENTS_HFCLKSTARTED\n");
  } else if ((Uns32)address == 0x40000518) { // LFCLKSRC
    icmPrintf("Write to Clock LFCLKSRC\n");
  } else if ((Uns32)address == 0x40000304) { // INTENSET
    icmPrintf("Write to Clock INTENSET\n");
  } else if ((Uns32)address == 0x40000008) { // TASKS_LFCLKSTART 
    icmPrintf("Write to Clock TASKS_LFCLKSTART\n");   
  } else if ((Uns32)address == 0x40000308) { // INTENCLR
    icmPrintf("Write to Clock INTENCLR\n");
  } else if ((Uns32)address == 0x40000108) { // LFCLKSRCCOPY  
    icmPrintf("Write to Clock LFCLKSRCCOPY\n");
  } else if ((Uns32)address == 0x40000100) { // HFCLKSTARTED  
    icmPrintf("Write to Clock HFCLKSTARTED\n");
  } else if ((Uns32)address == 0x40000000) { // HFCLKSTART
    icmPrintf("Write to Clock HFCLKSTART\n");
  } else if ((Uns32)address == 0x40000004) { // HFCLKSTOP
    icmPrintf("Write to Clock HFCLKSTOP\n");
  } else {
    icmPrintf("********** Not handled mem location for writing - power/clock/mpu - 0x%08x\n", (Uns32)address);
    icmTerminate();
  }
  
  icmPrintf(
    "Clock MEMORY: Writing 0x%08x to 0x%08x (%d, %d, %d)\n",
    *(Uns32*)value, (Uns32)address, (Uns32)bytes, (Uns32)value, (Uns32)userData
  );
}
