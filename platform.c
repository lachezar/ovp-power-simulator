/*
 *
 * Copyright (c) 2005-2014 Imperas Software Ltd., www.imperas.com
 *
 * The contents of this file are provided under the Software License
 * Agreement that you accepted before downloading this file.
 *
 * This source forms part of the Software and can be used for educational,
 * training, and demonstration purposes but cannot be used for derivative
 * works except in cases where the derivative works require OVP technology
 * to run.
 *
 * For open source models released under licenses that you can use for
 * derivative works, please visit www.OVPworld.org or www.imperas.com
 * for the location of the open source models.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "icm/icmCpuManager.h"
#include "hex_loader.h"

// Function prototype and variables used
static void simulate_custom_platform(icmProcessorP processor);
static void parseArgs(int argc, char ** argv);
static ICM_MEM_READ_FN(extMemReadCB);
static ICM_MEM_WRITE_FN(extMemWriteCB);
static ICM_MEM_READ_FN(extMemReadGPIOCB);
static ICM_MEM_WRITE_FN(extMemWriteGPIOCB);
static ICM_MEM_READ_FN(extMemReadUICRCB);
static ICM_MEM_WRITE_FN(extMemWriteUICRCB);
static ICM_MEM_READ_FN(extMemReadFICRCB);
static ICM_MEM_WRITE_FN(extMemWriteFICRCB);
static ICM_MEM_READ_FN(extMemReadNVMCCB);
static ICM_MEM_WRITE_FN(extMemWriteNVMCCB);
static ICM_MEM_READ_FN(extMemReadClockCB);
static ICM_MEM_WRITE_FN(extMemWriteClockCB);
/*static ICM_MEM_READ_FN(extMemReadRTCCB);
static ICM_MEM_WRITE_FN(extMemWriteRTCCB);*/
/*static ICM_MEM_READ_FN(extMemReadTimer0CB);
static ICM_MEM_WRITE_FN(extMemWriteTimer0CB);*/
static ICM_MEM_READ_FN(extMemReadPPICB);
static ICM_MEM_WRITE_FN(extMemWritePPICB);
const char *application;
const char *processorType;
const char *alternateVendor;
const char *model;
const char *semihosting;
icmNetP timer0Net, rtcNet;

//#define SIM_FLAGS (ICM_ATTR_TRACE | ICM_ATTR_TRACE_REGS_AFTER)
#define SIM_FLAGS (ICM_ATTR_TRACE | ICM_ATTR_TRACE_ICOUNT)

//
// Main simulation routine
//
int main(int argc, char ** argv) {
  
    srand(time(NULL));

    // read the arguments and set application, processorType and alternateVendor
    parseArgs(argc, argv);
    
    // initialize CpuManager
    icmInitPlatform(ICM_VERSION, ICM_VERBOSE|ICM_ENABLE_IMPERAS_INTERCEPTS|ICM_STOP_ON_CTRLC, 0, 0, "platform");
    
    icmSetWallClockFactor(1.0);

    // select library components
    const char *vlnvRoot = NULL; //When NULL use default library

    model = icmGetVlnvString(vlnvRoot, "arm.ovpworld.org", "processor", "armm", "1.0", "model");
    semihosting = icmGetVlnvString(vlnvRoot, "arm.ovpworld.org", "semihosting", "armNewlib", "1.0", "model");

    icmAttrListP icmAttr = icmNewAttrList();
    icmAddStringAttr(icmAttr, "endian",        "little");
    icmAddStringAttr(icmAttr, "compatibility", "nopBKPT");
    icmAddStringAttr(icmAttr, "variant",       "Cortex-M3");
    icmAddStringAttr(icmAttr, "UAL",           "1"); 
    //icmAddDoubleAttr(icmAttr, "mips", 15.04);
    icmAddUns32Attr(icmAttr, "override_numInterrupts", 32); // we use more interrupts than the default value of the CPU model
    icmAddStringAttr(icmAttr, "showHiddenRegs", "1");
    

    // create a processor
    icmProcessorP processor = icmNewProcessor(
        "cpu1",             // CPU name
        "arm",              // CPU type
        0,                  // CPU cpuId
        0,                  // CPU model flags
        32,                 // address bits
        model,              // model file
        "modelAttrs",       // morpher attributes
        0, //SIM_FLAGS,          // enable tracing or register values
        icmAttr,            // user-defined attributes
        semihosting,        // semi-hosting file
        "modelAttrs"        // semi-hosting attributes
    );
    
    // create the processor bus
    icmBusP bus = icmNewBus("bus", 32);
    // connect the processor busses
    icmConnectProcessorBusses(processor, bus, bus);
    // create two simulated memories for low and high regions
    icmMemoryP memory1 = icmNewMemory("mem1", ICM_PRIV_RWX, 0x20004000 - 0x20000000 - 1);
    icmMemoryP memory2 = icmNewMemory("mem2", ICM_PRIV_RWX, 0x00040000 - 0x00000000 - 1);
    icmMemoryP memory_secret = icmNewMemory("mem_secret", ICM_PRIV_R, 0xf);
    //icmMemoryP memory_gpio = icmNewMemory("mem_gpio", ICM_PRIV_RW, 0x50001000 - 0x50000000 - 1);
    //icmMemoryP memory3 = icmNewMemory("mem3", ICM_PRIV_RWX, 0x40001000 - 0x40000000 - 1);
    //icmMemoryP memory4 = icmNewMemory("mem4", ICM_PRIV_RWX, 0xFFFFFFFF - 0x40080000);
    //icmMemoryP memory_nvmc = icmNewMemory("mem_nvmc", ICM_PRIV_RWX, 0x4001F000 - 0x4001E000 - 1);
    icmMemoryP memory_radio = icmNewMemory("mem_radio", ICM_PRIV_RWX, 4);
    icmMemoryP memory_temp = icmNewMemory("mem_temp", ICM_PRIV_RWX, 4);
    //icmMemoryP memory_timer0 = icmNewMemory("mem_timer0", ICM_PRIV_RWX, 4);
    icmMemoryP memory_crypto = icmNewMemory("mem_crypto", ICM_PRIV_RWX, 4);
    icmMemoryP memory_aar = icmNewMemory("mem_aar", ICM_PRIV_RWX, 4);
    //icmMemoryP memory_rtc = icmNewMemory("mem_rtc", ICM_PRIV_RWX, 4);
    //icmMemoryP memory_wdt = icmNewMemory("mem_wdt", ICM_PRIV_RWX, 4);
    //icmMemoryP memory_ppi = icmNewMemory("mem_ppi", ICM_PRIV_RWX, 1024);

    icmMapExternalMemory(
      bus, "external_gpio", ICM_PRIV_RW, 0x50000000UL, 0x50000FFFUL,
      extMemReadGPIOCB, extMemWriteGPIOCB, 0
    );
    icmMapExternalMemory(
      bus, "external", ICM_PRIV_RW, 0x4000D000UL, 0x4000DFFFUL,
      extMemReadCB, extMemWriteCB, 0
    );
    icmMapExternalMemory(
      bus, "external_uicr", ICM_PRIV_R, 0x10001000UL, 0x100013FFUL,
      extMemReadUICRCB, extMemWriteUICRCB, 0
    );
    icmMapExternalMemory(
      bus, "external_nvmc", ICM_PRIV_RW, 0x4001E000UL, 0x4001EFFFUL,
      extMemReadNVMCCB, extMemWriteNVMCCB, 0
    );
    icmMapExternalMemory(
      bus, "external_ficr", ICM_PRIV_R, 0x10000000UL, 0x100003FFUL,
      extMemReadFICRCB, extMemWriteFICRCB, 0
    );
    icmMapExternalMemory(
      bus, "external_clock", ICM_PRIV_RW, 0x40000000UL, 0x40000558UL,
      extMemReadClockCB, extMemWriteClockCB, 0
    );
    /*icmMapExternalMemory(
      bus, "external_rtc", ICM_PRIV_RW, 0x4000B000UL, 0x4000BFFFUL,
      extMemReadRTCCB, extMemWriteRTCCB, 0
    );*/
    /*icmMapExternalMemory(
      bus, "external_timer0", ICM_PRIV_RW, 0x40008000UL, 0x40008FFFUL,
      extMemReadTimer0CB, extMemWriteTimer0CB, 0
    );*/
    icmMapExternalMemory(
      bus, "external_ppi", ICM_PRIV_RW, 0x4001F000UL, 0x4001FFFFUL,
      extMemReadPPICB, extMemWritePPICB, 0
    );
    // connect memories to bus
    icmConnectMemoryToBus(bus, "mp1", memory1, 0x20000000); // ram
    icmConnectMemoryToBus(bus, "mp2", memory2, 0x00000000); // code
    //icmConnectMemoryToBus(bus, "mp3", memory3, 0x40000000); // AMLI, POWER, CLOCK, MPU, PU
    //icmConnectMemoryToBus(bus, "mp4", memory4, 0x40080000); // AMLI, POWER, CLOCK, MPU, PU
    //icmConnectMemoryToBus(bus, "mp_gpio", memory_gpio, 0x50000000);
    icmConnectMemoryToBus(bus, "mp_secret", memory_secret, 0xf0000fe0); // secret registers - https://devzone.nordicsemi.com/question/17943/secret-registers-at-memory-locations-0xf0000fe0-and-0xf0000fe8-of-the-nrf51822/
    //icmConnectMemoryToBus(bus, "mp_nvmc", memory_nvmc, 0x4001E000);    
    icmConnectMemoryToBus(bus, "mpradio", memory_radio, 0x40001ffc);
    icmConnectMemoryToBus(bus, "mptemp", memory_temp, 0x4000cffc);
    //icmConnectMemoryToBus(bus, "mptimer0", memory_timer0, 0x40008ffc);
    icmConnectMemoryToBus(bus, "mpcrypto", memory_crypto, 0x4000effc);
    icmConnectMemoryToBus(bus, "mpaar", memory_aar, 0x4000fffc);
    //icmConnectMemoryToBus(bus, "mprtc", memory_rtc, 0x4000bffc);
    //icmConnectMemoryToBus(bus, "mpwdt", memory_wdt, 0x4001f508); // this is PPI actually
    //icmConnectMemoryToBus(bus, "mpppi", memory_ppi, 0x4001f550);
    
    //const char *pse = icmGetVlnvString(vlnvRoot, "ovpworld.org", "peripheral", "Timer", "1.0", "pse");
    
    // instantiate the peripheral
    icmAttrListP icmAttrTimer = icmNewAttrList();
    icmPseP timer0 = icmNewPSE("timer", "pse/pse.pse", icmAttrTimer, NULL, NULL);
    icmAttrListP icmAttrRTC = icmNewAttrList();
    icmPseP rtc0 = icmNewPSE("rtc", "pse_rtc/pse.pse", icmAttrRTC, NULL, NULL);
    // connect the Timer slave port on the bus
    //define the address range it occupies
    icmConnectPSEBus(timer0, bus, "TIMER", False, 0x40008000, 0x40008FFF);
    icmConnectPSEBus(rtc0, bus, "RTC", False, 0x4000B000, 0x4000BFFF);
    
    // show the bus connections
    icmPrintBusConnections(bus);
    
    rtcNet = icmNewNet("rtc_irq");
    timer0Net = icmNewNet("timer0_irq");

    // connect the processor interrupt port to the net
    icmConnectProcessorNet(processor, rtcNet, "int11", ICM_INPUT);
    icmConnectProcessorNet(processor, timer0Net, "int8", ICM_INPUT);

    // connect the RTC0 interrupt port to the net
    icmConnectPSENet(rtc0, rtcNet, "rtc_irq", ICM_OUTPUT);
    icmConnectPSENet(timer0, timer0Net, "timer_irq", ICM_OUTPUT);
    
    icmPrintNetConnections();

    // load the processor object file
    //icmLoadProcessorMemory(processor, application, ICM_LOAD_PHYSICAL, True, True);

    // init memory with FFs
    int i, initmemory = 0xFFFFFFFF;
    for (i = 0; i < 0x00040000 - 0x1000; i+=sizeof(int)) {
      icmWriteProcessorMemory(processor, i, &initmemory, sizeof(int));
    }
    
    // Load Hex file into Simulator Memory
    char *dot = strrchr(application, '.');
    if (dot && !strcmp(dot, ".hex")) {      
      if (load_hex_file(processor, (char*)application) != 0) {
          icmPrintf("Hex File Load of %s Failed\n", application);
          icmTerminate();
          return -1;
      }
    } else {
      icmLoadProcessorMemory(processor, application, ICM_LOAD_PHYSICAL, True, True);
    }
    
    unsigned int reg = 0xFFFFFFFF;
    icmWriteReg(processor, "r4", &reg);
    
    unsigned int cafebabe = 0xcafebabe;
    icmWriteProcessorMemory(processor, 0x2000011c, &cafebabe, 4);
        
    // Run the simulation
    //icmSimulatePlatform();
    simulate_custom_platform(processor);

    // free simulation data structures
    icmTerminate();

    return 0;
}

static void simulate_custom_platform(icmProcessorP processor) {
  //Bool done = False;
  unsigned long long cnt = 0;
  //int result = 0;
  //int prev_result = 0;
  
  //int mem = 5;
  //icmWriteProcessorMemory(processor, 0x40008000, &mem, sizeof(mem));
  //icmReadProcessorMemory(processor, 0x40008140, &mem, sizeof(mem));
  //icmReadProcessorMemory(processor, 0x40008ffc, &mem, sizeof(mem));
  
  double TIME_SLICE = 1.0 / 16000000.0;
  icmTime myTime;
  icmStopReason rtnVal = ICM_SR_SCHED;
  for (myTime = TIME_SLICE; rtnVal == ICM_SR_SCHED || rtnVal == ICM_SR_HALT; myTime += TIME_SLICE) {

      //Uns32 currentPC = (Uns32)icmGetPC(processor);
      //const char* disassemble = icmDisassemble(processor, currentPC);

      // execute one instruction
      //done = (icmSimulate(processor, 1) != ICM_SR_SCHED);
      rtnVal = icmSimulate(processor, 1); // it could return "halt" on wfe?
      /*if (result != prev_result) icmPrintf("************* new result %d", result);
      prev_result = result;
      rtnVal = result;*/
      
      icmAdvanceTime(myTime);
      
      //if (strstr(disassemble, "e7fe") == NULL && result == ICM_SR_SCHED) {
        // disassemble instruction at current PC
        //icmPrintf("0x%08x : %s\n", currentPC, disassemble);
      //}
            
      // exit if pc is specific value?
      //if (!done && strstr(disassemble, "e7fe") != NULL) {
        //done = True;
      //}
      
      /*unsigned int d;
      icmReadProcessorMemory(processor, 0xe000e414, &d, 4);
      icmPrintf("0xe000e41x is %x", d);*/

      // dump registers
      //icmDumpRegisters(processor);
      
      /*if (cnt == 24200) {
        icmWriteNet(timer0Net, 1); // interruption on Timer0
        icmPrintf("******Timer0 interruption");
      }
      
      if (cnt == 24600) {
        icmWriteNet(rtcNet, 1); // interruption on Timer0
        icmPrintf("******RTC interruption");
      }*/
      
      if (cnt++ > 7*16000000) break;
  } 
}

static void parseArgs(int argc, char ** argv) {

    // check for the application program name argument
    if(argc!=2) {
        icmMessage("F", "ARGS", "Usage: %s <application elf file> <processor type or1k|mips32|arm7> [alternative vendor]", argv[0] );
    }

    application = argv[1];
    //processorType = argv[2];
    //alternateVendor = NULL;

    /*if(argc==4) {
        alternateVendor = argv[3];
    }*/
}

int mdata = 0;

static ICM_MEM_READ_FN(extMemReadCB) {
  Int32 data;
  if ((Int32)address == 0x4000d100) {
    data = 1;
  } else {
    data = rand();
  }
  
  *(Int32 *)value = data;
  
  icmPrintf(
    "EXTERNAL MEMORY: Reading 0x%08x from 0x%08x\n",
    data, (Int32)address
  );
}

static ICM_MEM_WRITE_FN(extMemWriteCB) {
  icmPrintf(
    "EXTERNAL MEMORY: Writing 0x%08x to 0x%08x (%d, %d, %d)\n",
    *(Int32*)value, (Int32)address, (int)bytes, (int)value, (int)userData
  );
}

static ICM_MEM_READ_FN(extMemReadGPIOCB) {
  icmPrintf(
    "GPIO MEMORY: Reading 0x%08x from 0x%08x\n",
    (Int32)value, (Int32)address
  );
}

static ICM_MEM_WRITE_FN(extMemWriteGPIOCB) {
  icmPrintf(
    "GPIO MEMORY: Writing 0x%08x to 0x%08x (%d, %d, %d)\n",
    *(Int32*)value, (Int32)address, (int)bytes, (int)value, (int)userData
  );
}

static ICM_MEM_READ_FN(extMemReadUICRCB) {
  if ((Int32)address == 0x10001014) {
    *(Int32 *)value = 0xFFFFFFFF;
    icmPrintf("WTF!!!");
  }
  if ((Int32)address == 0x10001000) {
    *(Int32 *)value = 0xFFFFFFFF;
    icmPrintf("FFS!!!");
  }
  icmPrintf(
    "UICR MEMORY: Reading 0x%08x from 0x%08x\n",
    *(Int32 *)value, (Int32)address
  );
}

static ICM_MEM_WRITE_FN(extMemWriteUICRCB) {
  icmPrintf(
    "UICR MEMORY: Writing 0x%08x to 0x%08x (%d, %d, %d)\n",
    *(Int32*)value, (Int32)address, (int)bytes, (int)value, (int)userData
  );
}

static ICM_MEM_READ_FN(extMemReadFICRCB) {
  if ((Int32)address == 0x1000002C) {
    *(Int32 *)value = 0xFFFFFFFF;
  }
  if ((Int32)address == 0x100000a4) {
    *(Int32 *)value = 0x1963137B;
  }
  if ((Int32)address == 0x100000a0) {
    *(Int32 *)value = 0xFFFFFFFF;
  }
  if ((Int32)address == 0x100000a8) {
    *(Int32 *)value = 0x472B354C;
  }
  if ((Int32)address == 0x10000080) {
    *(Int32 *)value = 0x39938601;
  }
  if ((Int32)address == 0x10000084) {
    *(Int32 *)value = 0xFC6B97AE;
  }
  if ((Int32)address == 0x10000088) {
    *(Int32 *)value = 0xE7108AE1;
  }
  if ((Int32)address == 0x1000008C) {
    *(Int32 *)value = 0x21E9F86C;
  }
  if ((Int32)address == 0x10000090) {
    *(Int32 *)value = 0x6EA6F8C0;
  }
  if ((Int32)address == 0x10000094) {
    *(Int32 *)value = 0x7C356886;
  }
  if ((Int32)address == 0x10000098) {
    *(Int32 *)value = 0x93DDF20D;
  }
  if ((Int32)address == 0x1000009C) {
    *(Int32 *)value = 0xBCBB9428;
  }
  icmPrintf(
    "FICR MEMORY: Reading 0x%08x from 0x%08x\n",
    *(Int32 *)value, (Int32)address
  );
}

static ICM_MEM_WRITE_FN(extMemWriteFICRCB) {
  icmPrintf(
    "FICR MEMORY: Writing 0x%08x to 0x%08x (%d, %d, %d)\n",
    *(Int32*)value, (Int32)address, (int)bytes, (int)value, (int)userData
  );
}

static ICM_MEM_READ_FN(extMemReadNVMCCB) {
  if ((Int32)address == 0x4001e400) {
    *(Int32 *)value = 1;
  }
  icmPrintf(
    "NVMC MEMORY: Reading 0x%08x from 0x%08x\n",
    *(Int32 *)value, (Int32)address
  );
}

static ICM_MEM_WRITE_FN(extMemWriteNVMCCB) {
  icmPrintf(
    "NVMC MEMORY: Writing 0x%08x to 0x%08x (%d, %d, %d)\n",
    *(Int32*)value, (Int32)address, (int)bytes, (int)value, (int)userData
  );
}

static ICM_MEM_READ_FN(extMemReadClockCB) {
  if ((Int32)address == 0x40000104) { // EVENTS_HFCLKSTARTED
    *(Int32 *)value = 1;
  } else if ((Int32) address == 0x40000524) { // Power - RAM on/off
    *(Int32 *)value = 3;
  } else if ((Int32) address == 0x40000554) { // XTALFREQ
    *(Int32 *)value = 0;
  } else if ((Int32) address == 0x40000408) { // HFCLKRUN
    *(Int32 *)value = 0;
  } else if ((Int32) address == 0x40000414) { // LFCLKRUN
    *(Int32 *)value = 0;
  } else {
    icmPrintf("********** Not handled mem location - power/clock/mpu");
    icmTerminate();
  }
  icmPrintf(
    "Clock MEMORY: Reading 0x%08x from 0x%08x\n",
    *(Int32 *)value, (Int32)address
  );
}

static ICM_MEM_WRITE_FN(extMemWriteClockCB) {
  if ((Int32)address == 0x40000524) { // Power - RAM on/off
    icmPrintf("Write to Power - RAM on/off");
  } else if ((Int32)address == 0x40000514) { // Power - POFCON - power failure config
    icmPrintf("Write to Power - POFCON - power failure config");
  } else if ((Int32) address == 0x40000554) { // XTALFREQ
    icmPrintf("Write to Clock - XTALFREQ");
  } else if ((Int32)address == 0x4000000c) { // TASKS_LFCLKSTOP
    icmPrintf("Write to Clock TASKS_LFCLKSTOP");
  } else if ((Int32)address == 0x40000104) { // EVENTS_HFCLKSTARTED
    icmPrintf("Write to Clock EVENTS_HFCLKSTARTED");
  } else if ((Int32)address == 0x40000518) { // LFCLKSRC
    icmPrintf("Write to Clock LFCLKSRC");
  } else if ((Int32)address == 0x40000304) { // INTENSET
    icmPrintf("Write to Clock INTENSET");
  } else if ((Int32)address == 0x40000008) { // TASKS_LFCLKSTART 
    icmPrintf("Write to Clock TASKS_LFCLKSTART ");   
  } else if ((Int32)address == 0x40000308) { // INTENCLR
    icmPrintf("Write to Clock INTENCLR");
  } else if ((Int32)address == 0x40000108) { // LFCLKSRCCOPY  
    icmPrintf("Write to Clock LFCLKSRCCOPY");   
  } else {
    icmPrintf("********** Not handled mem location for writing - power/clock/mpu");
    icmTerminate();
  }
  
  icmPrintf(
    "Clock MEMORY: Writing 0x%08x to 0x%08x (%d, %d, %d)\n",
    *(Int32*)value, (Int32)address, (int)bytes, (int)value, (int)userData
  );
}

/*static ICM_MEM_READ_FN(extMemReadRTCCB) {
  
  if ((Int32) address == 0x4000b504) { // COUNTER
    icmPrintf("Read from RTC COUNTER");
    *(Int32 *)value = *(Int32 *)value + 1;
  } else {
    icmPrintf("********** Not handled mem location - RTC");
    icmTerminate();
  }
  
  icmPrintf(
    "RTC MEMORY: Reading 0x%08x from 0x%08x\n",
    *(Int32 *)value, (Int32)address
  );
}

static ICM_MEM_WRITE_FN(extMemWriteRTCCB) {
  if ((Int32) address == 0x4000b308) { // INTENSET
    icmPrintf("Write to RTC INTENSET");
  } else if ((Int32) address == 0x4000bffc) { // POWER
    icmPrintf("Write to RTC POWER");
  } else if ((Int32) address == 0x4000b348) { // EVTENSET
    icmPrintf("Write to RTC EVTENSET");
  } else if ((Int32) address == 0x4000b540) { // CC[1]
    icmPrintf("Write to RTC CC[0]");
  } else if ((Int32) address == 0x4000b140) { // COMPARE[1]
    icmPrintf("Write to RTC COMPARE[0]");
  } else if ((Int32) address == 0x4000b544) { // CC[1]
    icmPrintf("Write to RTC CC[1]");
  } else if ((Int32) address == 0x4000b144) { // COMPARE[1]
    icmPrintf("Write to RTC COMPARE[1]");
  } else if ((Int32) address == 0x4000b004) { // STOP
    icmPrintf("Write to RTC STOP");  
  } else if ((Int32) address == 0x4000b008) { // CLEAR
    icmPrintf("Write to RTC CLEAR");
  } else if ((Int32) address == 0x4000b548) { // CC[2]
    icmPrintf("Write to RTC CC[2]");
  } else if ((Int32) address == 0x4000b148) { // COMPARE[2]
    icmPrintf("Write to RTC COMPARE[2]");
  } else if ((Int32) address == 0x4000b000) { // START
    icmPrintf("Write to RTC START");
  } else if ((Int32) address == 0x4000b504) { // COUNTER
    icmPrintf("Write to RTC COUNTER");
  } else if ((Int32) address == 0x4000b304) { // INTENSET
    icmPrintf("Write to RTC INTENSET");
  } else if ((Int32) address == 0x4000b344) { // EVENTSET
    icmPrintf("Write to RTC EVENTSET");  
  } else {
    icmPrintf("********** Not handled mem location for writing - RTC");
    icmTerminate();
  }
  
  icmPrintf(
    "RTC MEMORY: Writing 0x%08x to 0x%08x (%d, %d, %d)\n",
    *(Int32*)value, (Int32)address, (int)bytes, (int)value, (int)userData
  );
}*/

/*static ICM_MEM_READ_FN(extMemReadTimer0CB) {
  if ((Int32)address == 0) { // hm?
    // *(Int32 *)value = 1;
  } else {
    icmPrintf("********** Not handled mem location - timer0");
    icmTerminate();
  }
  icmPrintf(
    "Timer0 MEMORY: Reading 0x%08x from 0x%08x\n",
    *(Int32 *)value, (Int32)address
  );
}

static ICM_MEM_WRITE_FN(extMemWriteTimer0CB) {
  if ((Int32)address == 0x40008FFC) { // Timer0 power
    icmPrintf("Write to Timer0 power");
  } else if ((Int32)address == 0x40008308) { // Timer0 INTENCLEAR
    icmPrintf("Write to Timer0 INTENCLEAR");
  } else if ((Int32)address == 0x40008540) { // Timer0 Capture/compare registers [0]
    icmPrintf("Write to Timer0 CC[0]");
  } else if ((Int32)address == 0x40008140) { // Timer0 EVENTS_COMPARE[0]
    icmPrintf("Write to Timer0 EVENTS_COMPARE[0]");
  } else if ((Int32)address == 0x40008200) { // Timer0 Shortcuts for timer
    icmPrintf("Write to Timer0 - Shortcuts for timer SHORTS");
  } else if ((Int32)address == 0x40008504) { // Timer0 MODE
    icmPrintf("Write to Timer0 - MODE");
  } else if ((Int32)address == 0x40008508) { // Timer0 BITMODE - sets timer behavior
    icmPrintf("Write to Timer0 - BITMODE");
  } else if ((Int32)address == 0x40008510) { // Timer0 PRESCALER
    icmPrintf("Write to Timer0 - PRESCALER - %d", *(Int32*)value);
  } else if ((Int32)address == 0x40008304) { // Timer0 INTENSET
    icmPrintf("Write to Timer0 - INTENSET");
    
  } else {
    icmPrintf("********** Not handled mem location for writing - timer0");
    icmTerminate();
  }
  
  icmPrintf(
    "Timer0 MEMORY: Writing 0x%08x to 0x%08x (%d, %d, %d)\n",
    *(Int32*)value, (Int32)address, (int)bytes, (int)value, (int)userData
  );
}*/

static ICM_MEM_READ_FN(extMemReadPPICB) {
  if ((Int32)address == 0x4001f800) { // PPI Channel group config CHG[0]
    *(Int32 *)value = 0;
  } else if ((Int32)address == 0x4001f804) { // PPI Channel group config CHG[1]
    *(Int32 *)value = 0;
  } else {
    icmPrintf("********** Not handled mem location - PPI");
    icmTerminate();
  }
  icmPrintf(
    "PPI MEMORY: Reading 0x%08x from 0x%08x\n",
    *(Int32 *)value, (Int32)address
  );
}

static ICM_MEM_WRITE_FN(extMemWritePPICB) {
  if ((Int32)address == 0x4001f508) { // PPI Channel enable set
    icmPrintf("Write to PPI CHENSET");
  } else if ((Int32)address == 0x4001f550) { // PPI CH[2] Channel event end-point
    icmPrintf("Write to PPI CH[2] EEP");
  } else if ((Int32)address == 0x4001f554) { // PPI CH[2] Channel task end-point
    icmPrintf("Write to PPI CH[2] TEP");
  } else if ((Int32)address == 0x4001f558) { // PPI CH[3] Channel event end-point
    icmPrintf("Write to PPI CH[3] EEP");
  } else if ((Int32)address == 0x4001f55c) { // PPI CH[3] Channel task end-point
    icmPrintf("Write to PPI CH[3] TEP");
  } else if ((Int32)address == 0x4001f560) { // PPI CH[4] Channel event end-point
    icmPrintf("Write to PPI CH[4] EEP");
  } else if ((Int32)address == 0x4001f564) { // PPI CH[4] Channel task end-point
    icmPrintf("Write to PPI CH[4] TEP");
  } else if ((Int32)address == 0x4001f568) { // PPI CH[5] Channel event end-point
    icmPrintf("Write to PPI CH[5] EEP");
  } else if ((Int32)address == 0x4001f56c) { // PPI CH[5] Channel task end-point
    icmPrintf("Write to PPI CH[5] TEP");
  } else if ((Int32)address == 0x4001f570) { // PPI CH[6] Channel event end-point
    icmPrintf("Write to PPI CH[6] EEP");
  } else if ((Int32)address == 0x4001f574) { // PPI CH[6] Channel task end-point
    icmPrintf("Write to PPI CH[6] TEP");
  } else if ((Int32)address == 0x4001f578) { // PPI CH[7] Channel event end-point
    icmPrintf("Write to PPI CH[7] EEP");
  } else if ((Int32)address == 0x4001f57c) { // PPI CH[7] Channel task end-point
    icmPrintf("Write to PPI CH[7] TEP");
  } else if ((Int32)address == 0x4001f580) { // PPI CH[8] Channel event end-point
    icmPrintf("Write to PPI CH[8] EEP");
  } else if ((Int32)address == 0x4001f584) { // PPI CH[8] Channel task end-point
    icmPrintf("Write to PPI CH[8] TEP");
  } else if ((Int32)address == 0x4001f588) { // PPI CH[9] Channel event end-point
    icmPrintf("Write to PPI CH[9] EEP");
  } else if ((Int32)address == 0x4001f58c) { // PPI CH[9] Channel task end-point
    icmPrintf("Write to PPI CH[9] TEP");
  } else if ((Int32)address == 0x4001f800) { // PPI Channel group configuration CHG[0]
    icmPrintf("Write to PPI CHG[0]");
  } else if ((Int32)address == 0x4001f804) { // PPI Channel group configuration CHG[1]
    icmPrintf("Write to PPI CHG[1]");
  } else if ((Int32)address == 0x4001f808) { // PPI Channel group configuration CHG[2]
    icmPrintf("Write to PPI CHG[2]");
  } else if ((Int32)address == 0x4001f80c) { // PPI Channel group configuration CHG[3]
    icmPrintf("Write to PPI CHG[3]");
  } else if ((Int32)address == 0x4001f504) { // PPI Channel enable
    icmPrintf("Write to PPI CHEN");    
  } else {
    icmPrintf("********** Not handled mem location for writing - PPI");
    icmTerminate();
  }
  
  icmPrintf(
    "PPI MEMORY: Writing 0x%08x to 0x%08x (%d, %d, %d)\n",
    *(Int32*)value, (Int32)address, (int)bytes, (int)value, (int)userData
  );
}

