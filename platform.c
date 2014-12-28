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
#include "cycles_table.h"
#include "platform.h"

// Function prototype and variables used
static void simulate_custom_platform(icmProcessorP processor);
static void parseArgs(int argc, char ** argv);
static NET_WRITE_FN(intPPINetWritten);
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
icmNetP timer0Net, rtcNet, rngNet, radioNet, radioPPINet, rtcPPINet, timer0PPINet;

int should_start_rng_int = 0;
int should_stop_rng_int = 0;
int rng_started = 0;
int rng_irq_enabled = 0;

static NRF_PPI_Type ppi;
static int ppi_queue[16];
static int ppi_queue_size = 0;
static Uns32 oldPPIValue;
static int should_run_ppi = 0;

#define CYCLES_TABLE_SIZE 0xC000
static unsigned char cycles_table[CYCLES_TABLE_SIZE];

//#define TRACE 1

#ifdef TRACE
#define SIM_FLAGS (ICM_ATTR_TRACE | ICM_ATTR_TRACE_ICOUNT)
#else 
#define SIM_FLAGS 0
#endif

//
// Main simulation routine
//
int main(int argc, char ** argv) {

    // read the arguments and set application, processorType and alternateVendor
    parseArgs(argc, argv);
    
    if (load_table("cycles_map.txt", cycles_table, CYCLES_TABLE_SIZE) != 0) {
      icmPrintf("Could not load the instruction cycles table!\n");
      return -1;
    }
    
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
        SIM_FLAGS,          // enable tracing or register values
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
    icmMemoryP memory_temp = icmNewMemory("mem_temp", ICM_PRIV_RWX, 3);
    //icmMemoryP memory_timer0 = icmNewMemory("mem_timer0", ICM_PRIV_RWX, 4);
    icmMemoryP memory_crypto = icmNewMemory("mem_crypto", ICM_PRIV_RWX, 3);
    //icmMemoryP memory_aar = icmNewMemory("mem_aar", ICM_PRIV_RWX, 3);
    //icmMemoryP memory_rtc = icmNewMemory("mem_rtc", ICM_PRIV_RWX, 4);
    //icmMemoryP memory_wdt = icmNewMemory("mem_wdt", ICM_PRIV_RWX, 4);
    //icmMemoryP memory_ppi = icmNewMemory("mem_ppi", ICM_PRIV_RWX, 1024);
    icmMemoryP memory_ccm = icmNewMemory("mem_ccm", ICM_PRIV_RWX, 3);
    icmMemoryP memory_ccm2 = icmNewMemory("mem_ccm2", ICM_PRIV_RWX, 3);
    
    icmMapExternalMemory(
      bus, "external_gpio", ICM_PRIV_RW, 0x50000000UL, 0x50000FFFUL,
      extMemReadGPIOCB, extMemWriteGPIOCB, 0
    );
    icmMapExternalMemory( // rng
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
      bus, "external_clock", ICM_PRIV_RW, 0x40000000UL, 0x4000061fUL,
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
    icmConnectMemoryToBus(bus, "mptemp", memory_temp, 0x4000cffc);
    //icmConnectMemoryToBus(bus, "mptimer0", memory_timer0, 0x40008ffc);
    icmConnectMemoryToBus(bus, "mpcrypto", memory_crypto, 0x4000effc);
    //icmConnectMemoryToBus(bus, "mpaar", memory_aar, 0x4000fffc);
    icmConnectMemoryToBus(bus, "mpccm", memory_ccm, 0x4000fffc);
    icmConnectMemoryToBus(bus, "mpccm2", memory_ccm2, 0x4000f500);
    //icmConnectMemoryToBus(bus, "mprtc", memory_rtc, 0x4000bffc);
    //icmConnectMemoryToBus(bus, "mpwdt", memory_wdt, 0x4001f508); // this is PPI actually
    //icmConnectMemoryToBus(bus, "mpppi", memory_ppi, 0x4001f550);
    
    //const char *pse = icmGetVlnvString(vlnvRoot, "ovpworld.org", "peripheral", "Timer", "1.0", "pse");
    
    // instantiate the peripheral
    icmAttrListP icmAttrTimer = icmNewAttrList();
    icmPseP timer0 = icmNewPSE("timer", "pse/pse.pse", icmAttrTimer, NULL, NULL);
    icmAttrListP icmAttrRTC = icmNewAttrList();
    icmPseP rtc0 = icmNewPSE("rtc", "pse_rtc/pse.pse", icmAttrRTC, NULL, NULL);
    icmAttrListP icmAttrRadio = icmNewAttrList();
    icmPseP radio = icmNewPSE("radio", "pse_radio/pse.pse", icmAttrRadio, NULL, NULL);
    // connect the Timer slave port on the bus
    //define the address range it occupies
    icmConnectPSEBus(timer0, bus, "TIMER", False, 0x40008000, 0x40008FFF);
    icmConnectPSEBus(rtc0, bus, "RTC", False, 0x4000B000, 0x4000BFFF);
    icmConnectPSEBus(radio, bus, "RADIO", False, 0x40001000, 0x40001FFF);
       
    // show the bus connections
    icmPrintBusConnections(bus);
    
    rtcNet = icmNewNet("rtc_irq");
    timer0Net = icmNewNet("timer_irq");
    radioNet = icmNewNet("radio_irq");
    rtcPPINet = icmNewNet("rtc_ppi");
    timer0PPINet = icmNewNet("timer0_ppi");
    radioPPINet = icmNewNet("radio_ppi");

    // connect the processor interrupt port to the net
    icmConnectProcessorNet(processor, rtcNet, "int11", ICM_INPUT);
    icmConnectProcessorNet(processor, timer0Net, "int8", ICM_INPUT);
    icmConnectProcessorNet(processor, radioNet, "int1", ICM_INPUT);

    // connect the RTC0 interrupt port to the net
    icmConnectPSENet(rtc0, rtcNet, "rtc_irq", ICM_OUTPUT);
    icmConnectPSENet(timer0, timer0Net, "timer_irq", ICM_OUTPUT);
    icmConnectPSENet(radio, radioNet, "radio_irq", ICM_OUTPUT);
    icmConnectPSENet(rtc0, rtcPPINet, "rtc_ppi", ICM_OUTPUT);
    icmConnectPSENet(timer0, timer0PPINet, "timer0_ppi", ICM_OUTPUT);
    icmConnectPSENet(radio, radioPPINet, "radio_ppi", ICM_OUTPUT);
    
    icmAddNetCallback(rtcPPINet, intPPINetWritten, &oldPPIValue);
    icmAddNetCallback(timer0PPINet, intPPINetWritten, &oldPPIValue);
    icmAddNetCallback(radioPPINet, intPPINetWritten, &oldPPIValue);
    
    rngNet = icmNewNet("rng_irq");
    icmConnectProcessorNet(processor, rngNet, "int13", ICM_INPUT);
    
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

inline void start_pending_irqs() {
  if (should_start_rng_int != 0) {
    icmPrintf("****RNG IRQ SIGNAL STARTED\n");
    icmWriteNet(rngNet, 1);
    should_start_rng_int = 0;
    should_stop_rng_int = 5; // no idea why 5!
  }
}

inline void stop_pending_irqs() {
  if (should_stop_rng_int > 1) {
    should_stop_rng_int--;
  } else if (should_stop_rng_int != 0) {
    icmWriteNet(rngNet, 0);
    should_stop_rng_int = 0;
    icmPrintf("****RNG IRQ SIGNAL STOPPED\n");
  }
}

inline void run_ppi(icmProcessorP processor, NRF_PPI_Type* ppi) {
  int i, j;
  Uns32 data;
  const Uns32 signal = 1;
  //const Uns32 stop_signal = 0;
  
  //icmPrintf("PPI->CHEN 0x%08x\n", ppi->CHEN);
  
  for (i = 0; i < ppi_queue_size; i++) {
    //icmPrintf("ppi_queue[i] 0x%08x\n", ppi_queue[i]);
    j = ppi_queue[i];
    PPI_CH_Type ppich = ppi->CH[j];

    if (ppich.EEP != 0 && ppich.TEP != 0 && (ppi->CHEN & (1 << j)) != 0) {
      // get data from address ppich.EEP    
      icmReadProcessorMemory(processor, ppich.EEP, &data, 4);

      if (data != 0) {
        // write 1 to ppich.TEP address
        icmWriteProcessorMemory(processor, ppich.TEP, &signal, 4);
        icmPrintf("****PPI CONNECTION DONE! EEP 0x%08x -> TEP 0x%08x\n", ppich.EEP, ppich.TEP);
        //icmWriteProcessorMemory(processor, ppich.EEP, &stop_signal, 4);
      }
    }
  }
}

static void simulate_custom_platform(icmProcessorP processor) {
  //Bool done = False;
  //unsigned long long cnt = 0;
  //unsigned int print = 0;
  //int result = 0;
  //int prev_result = 0;
  
  //int mem = 5;
  //icmWriteProcessorMemory(processor, 0x40008000, &mem, sizeof(mem));
  //icmReadProcessorMemory(processor, 0x40008140, &mem, sizeof(mem));
  //icmReadProcessorMemory(processor, 0x40008ffc, &mem, sizeof(mem));
  
  Uns32 currentPC = 0, prevPC = 0;
  unsigned char is_branch = 0;
  double TIME_SLICE = 1.0 / 16000000.0;
  //icmTime myTime;
  icmStopReason rtnVal = ICM_SR_SCHED;
  Uns32 tick = 0;
  for (tick = 0; rtnVal == ICM_SR_SCHED || rtnVal == ICM_SR_HALT; tick++) {
    
      start_pending_irqs();

      if (rtnVal == ICM_SR_SCHED) {
        icmPrintf("%f -> 0x%08x\n", (double)tick * TIME_SLICE, currentPC);
      }
      
      #ifdef TRACE
      Uns32 currentPC = (Uns32)icmGetPC(processor);
      const char* disassemble = icmDisassemble(processor, currentPC);
      #endif
      
      prevPC = currentPC;
      currentPC = (Uns32)icmGetPC(processor);
      if (rtnVal == ICM_SR_SCHED && is_branch != 0 && (prevPC + sizeof(int) != currentPC || prevPC + sizeof(short) != currentPC)) {
        //myTime += 2 * TIME_SLICE;
        tick += 2;
      }

      unsigned char cycles = 0;
      is_branch = 0;
      const Uns32 address = currentPC >> 1;
      if (address < CYCLES_TABLE_SIZE) {
        cycles = cycles_table[address] & 0x7F;
        is_branch = ((cycles_table[address] & 0x80) != 0);
      }
      
      //icmPrintf("??? 0x%08x\n", cycles_table[currentPC]);
      if (rtnVal == ICM_SR_SCHED && cycles == 0) {
        const char* disassemble = icmDisassemble(processor, currentPC);
        icmPrintf(" (unknown instruction in the cycles table) : %s\n", disassemble);
        //icmPrintf(" (unknown instruction in the cycles table) :\n");
        //icmPrintf(disassemble);
        cycles = 1;
      }
      
      /*if (currentPC == 0x13020) {
        print = 1;
        icmPrintf(">>>>>>>>>>>>>>>>>>>>\n");
        Uns32 q;
        Uns32 i;
        for (i = 0; i < 0x8000; i++) {
          icmReadProcessorMemory(processor, 0x20000000 + i*sizeof(Uns32), &q, sizeof(Uns32));
          icmPrintf("0x%08x\n", q);
        }
        icmPrintf(">>>>>>>>>>>>>>>>>>>>\n");
      }*/
      /*if (rtnVal == ICM_SR_SCHED && (cnt % 10 == 0 || print == 1)) {
        const char* disassemble = icmDisassemble(processor, currentPC);
        icmPrintf(" : %s\n", disassemble);
        icmDumpRegisters(processor);
      }*/
      // execute one instruction
      //done = (icmSimulate(processor, 1) != ICM_SR_SCHED);
      rtnVal = icmSimulate(processor, 1); // it could return "halt" on wfe?
      if (rtnVal != ICM_SR_SCHED) {
        //myTime += 15.0 * TIME_SLICE;
        tick += 31; // to 127?
      } else if (cycles > 1) {
        //icmPrintf("cycles: %f\n", ((double)(cycles-1)));
        //myTime += (((double)(cycles-1)) * TIME_SLICE);
        tick += cycles - 1;
      }
      /*if (result != prev_result) icmPrintf("************* new result %d", result);
      prev_result = result;
      rtnVal = result;*/
      
      /*if (cnt % 10 == 0 || print == 1) {
        icmPrintf("cpu status %d\n", rtnVal);
      }*/
      
      //icmAdvanceTime(myTime);
      icmAdvanceTime(((double)tick) * TIME_SLICE);
      
      #ifdef TRACE
      int dbg_status = (strstr(disassemble, "e7fe") == NULL && rtnVal == ICM_SR_SCHED);
      if (dbg_status) {
        // disassemble instruction at current PC
        icmPrintf("0x%08x : %s\n", currentPC, disassemble);
      }
      #endif
            
      // exit if pc is specific value?
      //if (!done && strstr(disassemble, "e7fe") != NULL) {
        //done = True;
      //}
      
      /*unsigned int d;
      icmReadProcessorMemory(processor, 0xe000e414, &d, 4);
      icmPrintf("0xe000e41x is %x", d);*/

      #ifdef TRACE
      // dump registers
      if (dbg_status) icmDumpRegisters(processor);
      #endif
      
      /*if (cnt == 24200) {
        icmWriteNet(timer0Net, 1); // interruption on Timer0
        icmPrintf("******Timer0 interruption");
      }
      
      if (cnt == 24600) {
        icmWriteNet(rtcNet, 1); // interruption on Timer0
        icmPrintf("******RTC interruption");
      }*/
      
      stop_pending_irqs();
     
      if (/*cnt % 10 == 0 ||*/ should_run_ppi != 0) {
        icmPrintf("**** should run ppi\n");
        should_run_ppi = 0;
        run_ppi(processor, &ppi);
      }
      
      //if (cnt++ > 16000000 / 2) break;
      //cnt++;
      if (tick > 16000000 / 2) break;
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

static NET_WRITE_FN(intPPINetWritten) {
  icmPrintf("@@@ Trigger PPI Nets with value = %d\n", value);
  should_run_ppi = value;
}

// @TODO move RNG to external file

static ICM_MEM_READ_FN(extMemReadCB) {
  Int32 data = 0;
  if ((Int32)address == 0x4000d100) {
    data = 1;
  } else if ((Int32)address == 0x4000d508) {
    data = rand();
    icmPrintf("\nRNG RAND GENERATED and READ - %d\n", data);
  } else {
    icmPrintf("********** Not handled mem location - RNG - 0x%08x\n", (Int32)address);
    icmTerminate();
  }
  
  *(Int32 *)value = data;
  
  icmPrintf(
    "RNG MEMORY: Reading 0x%08x from 0x%08x\n",
    data, (Int32)address
  );
}

static ICM_MEM_WRITE_FN(extMemWriteCB) {
  if ((Int32)address == 0x4000d000) {
    rng_started = *(Int32*)value;
    icmPrintf("\nRNG START - %d\n", *(Int32*)value);
    if (rng_irq_enabled != 0 && rng_started != 0) {
      should_start_rng_int = 1;
      icmPrintf("\nRNG SHOULD START IRQ\n");
    }
  } else if ((Int32)address == 0x4000d304) {
    rng_irq_enabled = *(Int32*)value;
    icmPrintf("\nRNG IRQ SET - %d\n", *(Int32*)value);
  } else if ((Int32)address == 0x4000d100) {
    if (*(Int32*)value == 0 && rng_irq_enabled != 0 && rng_started != 0) {
      should_start_rng_int = 1;
      icmPrintf("\nRNG SHOULD START IRQ\n");
    }
    icmPrintf("\nRNG VALUE READY - %d\n", *(Int32*)value);
  } else if ((Int32)address == 0x4000dffc) {
    icmPrintf("\nRNG POWER - %d\n", *(Int32*)value);
  } else if ((Int32)address == 0x4000d004) {
    rng_started = 0;
    icmPrintf("\nRNG STOP - %d\n", *(Int32*)value);
  } else {
    icmPrintf("********** Not handled mem location writing - RNG - 0x%08x\n", (Int32)address);
    icmTerminate();
  }
  
  icmPrintf(
    "RNG MEMORY: Writing 0x%08x to 0x%08x (%d, %d, %d)\n",
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
  } else if ((Int32)address == 0x10001000) {
    *(Int32 *)value = 0xFFFFFFFF;
    icmPrintf("FFS!!!");
  } else {
    icmPrintf("********** Not handled mem location - UICR");
    icmTerminate();
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
  
  const Int32 segment1[] = { // starts in 0x10000080
    0x39938601, 0xFC6B97AE, 0xE7108AE1, 0x21E9F86C, // encryption root?
    0x6EA6F8C0, 0x7C356886, 0x93DDF20D, 0xBCBB9428, // identity root?
    0xFFFFFFFF, 0x1963137B, 0x472B354C, 0xFFFFFFF6  // DEVICEADDRTYPE, DEVICEADDR{2}, UNDOCUMENTED
  };
  
  const Int32 segment2[] = { // starts in 0x100000EC
    0x7D005600, 0x5C000050, 0x680E8804, 0x00726424, 0x824B423E // UNDOCUMENTED
  };
  
  if ((Int32)address == 0x1000002C) { // Pre-programmed factory code present
    *(Int32 *)value = 0xFFFFFFFF;
  } else if ((Int32)address >= 0x10000080 && (Int32)address <= 0x100000AC) {
    Int32 id = ((Int32)address - 0x10000080) >> 2;
    *(Int32 *)value = segment1[id];
  } else if ((Int32)address == 0x10000014) { // CODESIZE
    *(Int32 *)value = 0x00000100;
  } else if ((Int32)address == 0x10000010) { // CODEPAGESIZE
    *(Int32 *)value = 0x00000400;
  } else if ((Int32)address == 0x10000028) { // CLENR0
    *(Int32 *)value = 0xFFFFFFFF;
  } else if ((Int32)address >= 0x100000EC && (Int32)address <= 0x100000FC) { // UNDOCUMENTED
    Int32 id = ((Int32)address - 0x100000EC) >> 2;
    *(Int32 *)value = segment2[id];
  } else {
    icmPrintf("********** Not handled mem location - FICR");
    icmTerminate();
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

// @TODO move the clock/power handler to a separate file

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
  } else if ((Int32)address == 0x40000100) { // HFCLKSTARTED  
    *(Int32 *)value = 1;
  } else if ((Int32)address == 0x40000604) { // WDT Reload request 1  
    *(Int32 *)value = 0;
  } else {
    icmPrintf("********** Not handled mem location - power/clock/mpu - 0x%08x\n", (Int32)address);
    icmTerminate();
  }
  icmPrintf(
    "Clock MEMORY: Reading 0x%08x from 0x%08x\n",
    *(Int32 *)value, (Int32)address
  );
}

static ICM_MEM_WRITE_FN(extMemWriteClockCB) {
  if ((Int32)address == 0x40000524) { // Power - RAM on/off
    icmPrintf("Write to Power - RAM on/off\n");
  } else if ((Int32)address == 0x40000514) { // Power - POFCON - power failure config
    icmPrintf("Write to Power - POFCON - power failure config\n");
  } else if ((Int32) address == 0x40000554) { // XTALFREQ
    icmPrintf("Write to Clock - XTALFREQ\n");
  } else if ((Int32)address == 0x4000000c) { // TASKS_LFCLKSTOP
    icmPrintf("Write to Clock TASKS_LFCLKSTOP\n");
  } else if ((Int32)address == 0x40000104) { // EVENTS_HFCLKSTARTED
    icmPrintf("Write to Clock EVENTS_HFCLKSTARTED\n");
  } else if ((Int32)address == 0x40000518) { // LFCLKSRC
    icmPrintf("Write to Clock LFCLKSRC\n");
  } else if ((Int32)address == 0x40000304) { // INTENSET
    icmPrintf("Write to Clock INTENSET\n");
  } else if ((Int32)address == 0x40000008) { // TASKS_LFCLKSTART 
    icmPrintf("Write to Clock TASKS_LFCLKSTART\n");   
  } else if ((Int32)address == 0x40000308) { // INTENCLR
    icmPrintf("Write to Clock INTENCLR\n");
  } else if ((Int32)address == 0x40000108) { // LFCLKSRCCOPY  
    icmPrintf("Write to Clock LFCLKSRCCOPY\n");
  } else if ((Int32)address == 0x40000100) { // HFCLKSTARTED  
    icmPrintf("Write to Clock HFCLKSTARTED\n");
  } else if ((Int32)address == 0x40000000) { // HFCLKSTART
    icmPrintf("Write to Clock HFCLKSTART\n");
  } else if ((Int32)address == 0x40000004) { // HFCLKSTOP
    icmPrintf("Write to Clock HFCLKSTOP\n");
  } else {
    icmPrintf("********** Not handled mem location for writing - power/clock/mpu - 0x%08x\n", (Int32)address);
    icmTerminate();
  }
  
  icmPrintf(
    "Clock MEMORY: Writing 0x%08x to 0x%08x (%d, %d, %d)\n",
    *(Int32*)value, (Int32)address, (int)bytes, (int)value, (int)userData
  );
}

// @TODO move PPI implementation in separate file

void sync_ppi_queue(unsigned int bitmask) {
  ppi_queue_size = 0;
  int i;
  for (i = 0; i < 16; i++) {
    if ((bitmask & (1 << i)) != 0) {
      ppi_queue[ppi_queue_size++] = i;
    }
  } 
}

static ICM_MEM_READ_FN(extMemReadPPICB) {
  if ((Int32)address == 0x4001f800) { // PPI Channel group config CHG[0]
    *(Int32 *)value = ppi.CHG[0];
  } else if ((Int32)address == 0x4001f804) { // PPI Channel group config CHG[1]
    *(Int32 *)value = ppi.CHG[1];
  } else {
    icmPrintf("********** Not handled mem location - PPI - 0x%08x\n", (Int32)address);
    icmTerminate();
  }
  icmPrintf(
    "PPI MEMORY: Reading 0x%08x from 0x%08x\n",
    *(Int32 *)value, (Int32)address
  );
}

static ICM_MEM_WRITE_FN(extMemWritePPICB) {
  if ((Int32)address == 0x4001f504) { // PPI Channel enable set
    ppi.CHENSET = *(Int32*)value;
    ppi.CHEN |= *(Int32*)value;
    sync_ppi_queue(ppi.CHEN);
    icmPrintf("Write to PPI CHENSET\n");
  } else if ((Int32)address >= 0x4001f510 && (Int32)address <= 0x4001f58c) { // PPI CHANNEL
    Int32 id = ((Int32)address - 0x4001f510) / 8;
    
    if ((Int32)address % 8 == 0) {  
      ppi.CH[id].EEP = (*(Int32*)value);
      icmPrintf("Write to PPI CH[%d] EEP\n", id);
    } else {
      ppi.CH[id].TEP = (*(Int32*)value);
      icmPrintf("Write to PPI CH[%d] TEP\n", id);
    }
  } else if ((Int32)address >= 0x4001f800 && (Int32)address <= 0x4001f80c) { // PPI Channel group configuration 
    Int32 id = ((Int32)address - 0x4001f800) / 4;
    ppi.CHG[id] = (*(Int32*)value);
    icmPrintf("Write to PPI CHG[%d] (group!!!)\n", id);
  } else if ((Int32)address == 0x4001f508) { // PPI Channel clear
    ppi.CHEN &= ~(*(Int32*)value);
    sync_ppi_queue(ppi.CHEN);
    icmPrintf("Write to PPI CHENCLR ");    
  } else {
    icmPrintf("********** Not handled mem location for writing - PPI - 0x%08x\n", (Int32)address);
    icmTerminate();
  }
  
  icmPrintf(
    "PPI MEMORY: Writing 0x%08x to 0x%08x (%d, %d, %d)\n",
    (*(Int32*)value), (Int32)address, (int)bytes, (int)value, (int)userData
  );
}
