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

// Function prototype and variables used
static void parseArgs(int argc, char ** argv);
static ICM_MEM_READ_FN(extMemReadCB);
static ICM_MEM_WRITE_FN(extMemWriteCB);
static ICM_MEM_READ_FN(extMemReadGPIOCB);
static ICM_MEM_WRITE_FN(extMemWriteGPIOCB);
const char *application;
const char *processorType;
const char *alternateVendor;
const char *model;
const char *semihosting;

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
    icmAddDoubleAttr(icmAttr, "mips", 15.04);

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
    icmMemoryP memory3 = icmNewMemory("mem3", ICM_PRIV_RWX, 0x40001000 - 0x40000000 - 1);
    //icmMemoryP memory4 = icmNewMemory("mem4", ICM_PRIV_RWX, 0xFFFFFFFF - 0x40080000);

    icmMapExternalMemory(
      bus, "external_gpio", ICM_PRIV_RW, 0x50000000UL, 0x50000FFFUL,
      extMemReadGPIOCB, extMemWriteGPIOCB, 0
    );
    icmMapExternalMemory(
      bus, "external", ICM_PRIV_RW, 0x4000D000UL, 0x4000DFFFUL,
      extMemReadCB, extMemWriteCB, 0
    );
    // connect memories to bus
    icmConnectMemoryToBus(bus, "mp1", memory1, 0x20000000); // ram
    icmConnectMemoryToBus(bus, "mp2", memory2, 0x00000000); // code
    icmConnectMemoryToBus(bus, "mp3", memory3, 0x40000000); // AMLI, POWER, CLOCK, MPU, PU
    //icmConnectMemoryToBus(bus, "mp4", memory4, 0x40080000); // AMLI, POWER, CLOCK, MPU, PU
    //icmConnectMemoryToBus(bus, "mp_gpio", memory_gpio, 0x50000000);
    icmConnectMemoryToBus(bus, "mp_secret", memory_secret, 0xf0000fe0); // secret registers - https://devzone.nordicsemi.com/question/17943/secret-registers-at-memory-locations-0xf0000fe0-and-0xf0000fe8-of-the-nrf51822/
    // show the bus connections
    icmPrintBusConnections(bus);
    

    // load the processor object file
    icmLoadProcessorMemory(processor, application, ICM_LOAD_PHYSICAL, True, True);

    // Run the simulation
    icmSimulatePlatform();

    // free simulation data structures
    icmTerminate();

    return 0;
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
