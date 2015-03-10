#include <stdio.h>
#include <string.h>

#include "icm/icmCpuManager.h"
#include "platform.h"
#include "hexLoader.h"
#include "ppi.h"
#include "commonPeripherals.h"
#include "instructions_analyser.h"
#include "currentUsage.h"
#include "cycles_table.h"

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

  parseArgs(argc, argv);
 
  if (load(assemblerFile, table, TABLE_SIZE) != 0) {
    icmPrintf("Error loading the assembler file - %s\n", assemblerFile);
    return -1;
  }
  
  // initialize CpuManager
  icmInitPlatform(ICM_VERSION, ICM_VERBOSE|ICM_ENABLE_IMPERAS_INTERCEPTS|ICM_STOP_ON_CTRLC, 0, 0, "platform");

  //icmSetWallClockFactor(1.0);

  // select library components
  const char *vlnvRoot = NULL;   //When NULL use default library

  model = icmGetVlnvString(vlnvRoot, "arm.ovpworld.org", "processor", "armm", "1.0", "model");
  semihosting = icmGetVlnvString(vlnvRoot, "arm.ovpworld.org", "semihosting", "armNewlib", "1.0", "model");

  icmAttrListP icmAttr = icmNewAttrList();
  icmAddStringAttr(icmAttr, "endian",        "little");
  icmAddStringAttr(icmAttr, "compatibility", "nopBKPT");
  icmAddStringAttr(icmAttr, "variant",       "Cortex-M3");
  icmAddStringAttr(icmAttr, "UAL",           "1");
  icmAddDoubleAttr(icmAttr, "mips", 100.0);
  icmAddUns32Attr(icmAttr, "override_numInterrupts", 32);   // we use more interrupts than the default value of the CPU model
  icmAddStringAttr(icmAttr, "showHiddenRegs", "1");


  // create a processor
  icmProcessorP processor = icmNewProcessor(
    "cpu1",                 // CPU name
    "arm",                  // CPU type
    0,                      // CPU cpuId
    0,                      // CPU model flags
    32,                     // address bits
    model,                  // model file
    "modelAttrs",           // morpher attributes
    SIM_FLAGS,              // enable tracing or register values
    icmAttr,                // user-defined attributes
    semihosting,            // semi-hosting file
    "modelAttrs"            // semi-hosting attributes
    );

  // create the processor bus
  icmBusP bus = icmNewBus("bus", 32);
  // connect the processor busses
  icmConnectProcessorBusses(processor, bus, bus);
  icmMemoryP memory_ram = icmNewMemory("memory_ram", ICM_PRIV_RWX, 0x20004000 - 0x20000000 - 1);
  icmMemoryP memory_code = icmNewMemory("memory_code", ICM_PRIV_RWX, 0x00040000 - 0x00000000 - 1);
  icmMemoryP memory_secret = icmNewMemory("mem_secret", ICM_PRIV_R, 0xf);
  icmMemoryP memory_temp = icmNewMemory("mem_temp", ICM_PRIV_RWX, 3);
  icmMemoryP memory_crypto = icmNewMemory("mem_crypto", ICM_PRIV_RWX, 3);
  icmMemoryP memory_ccm = icmNewMemory("mem_ccm", ICM_PRIV_RWX, 3);
  icmMemoryP memory_ccm2 = icmNewMemory("mem_ccm2", ICM_PRIV_RWX, 3);

  icmMapExternalMemory(
    bus, "external_gpio", ICM_PRIV_RW, 0x50000000UL, 0x50000FFFUL,
    extMemReadGPIOCB, extMemWriteGPIOCB, 0
    );
  icmMapExternalMemory(   // rng
    bus, "external_rng", ICM_PRIV_RW, 0x4000D000UL, 0x4000DFFFUL,
    extMemReadRNGCB, extMemWriteRNGCB, 0
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
  icmMapExternalMemory(
    bus, "external_ppi", ICM_PRIV_RW, 0x4001F000UL, 0x4001FFFFUL,
    extMemReadPPICB, extMemWritePPICB, 0
    );

  // connect memories to bus
  icmConnectMemoryToBus(bus, "mp_ram", memory_ram, 0x20000000);   // ram
  icmConnectMemoryToBus(bus, "mp_code", memory_code, 0x00000000);   // code
  icmConnectMemoryToBus(bus, "mp_secret", memory_secret, 0xf0000fe0);   // secret registers - https://devzone.nordicsemi.com/question/17943/secret-registers-at-memory-locations-0xf0000fe0-and-0xf0000fe8-of-the-nrf51822/
  icmConnectMemoryToBus(bus, "mptemp", memory_temp, 0x4000cffc);
  icmConnectMemoryToBus(bus, "mpcrypto", memory_crypto, 0x4000effc);
  icmConnectMemoryToBus(bus, "mpccm", memory_ccm, 0x4000fffc);
  icmConnectMemoryToBus(bus, "mpccm2", memory_ccm2, 0x4000f500);

  // instantiate the peripheral
  icmAttrListP icmAttrTimer = icmNewAttrList();
  icmPseP timer0 = icmNewPSE("timer", "pse_timer/pse.pse", icmAttrTimer, NULL, NULL);
  icmAttrListP icmAttrRTC = icmNewAttrList();
  icmPseP rtc0 = icmNewPSE("rtc", "pse_rtc/pse.pse", icmAttrRTC, NULL, NULL);
  icmAttrListP icmAttrRadio = icmNewAttrList();
  icmPseP radio = icmNewPSE("radio", "pse_radio/pse.pse", icmAttrRadio, NULL, NULL);
  // Note: following the SPI peripherals definition here you can easily duplicate the RTC/Timer in few instances mapped on different memory areas if you need.
  icmAttrListP icmAttrSpi0 = icmNewAttrList();
  icmAddUns32Attr(icmAttrSpi0, "peripheral_id", 3);
  icmAddStringAttr(icmAttrSpi0, "device_implementation", "repeat");
  icmPseP spi0 = icmNewPSE("spi0", "pse_spi/pse.pse", icmAttrSpi0, NULL, NULL);
  icmAttrListP icmAttrSpi1 = icmNewAttrList();
  icmAddUns32Attr(icmAttrSpi1, "peripheral_id", 4);
  icmAddStringAttr(icmAttrSpi1, "device_implementation", "repeat");
  icmPseP spi1 = icmNewPSE("spi1", "pse_spi/pse.pse", icmAttrSpi1, NULL, NULL);

  icmConnectPSEBus(timer0, bus, "TIMER", False, 0x40008000, 0x40008FFF);
  icmConnectPSEBus(rtc0, bus, "RTC", False, 0x4000B000, 0x4000BFFF);
  icmConnectPSEBus(radio, bus, "RADIO", False, 0x40001000, 0x40001FFF);
  icmConnectPSEBus(spi0, bus, "SPI", False, 0x40003000, 0x40003FFF);
  icmConnectPSEBus(spi1, bus, "SPI", False, 0x40004000, 0x40004FFF);

  // show the bus connections
  icmPrintBusConnections(bus);

  rtcNet = icmNewNet("rtc_irq");
  timer0Net = icmNewNet("timer_irq");
  radioNet = icmNewNet("radio_irq");
  spi0Net = icmNewNet("spi0_irq");
  spi1Net = icmNewNet("spi1_irq");
  rtcPPINet = icmNewNet("rtc_ppi");
  timer0PPINet = icmNewNet("timer0_ppi");
  radioPPINet = icmNewNet("radio_ppi");
  spi0PPINet = icmNewNet("spi0_ppi");
  spi1PPINet = icmNewNet("spi1_ppi");

  // connect the processor interrupt port to the net
  icmConnectProcessorNet(processor, rtcNet, "int11", ICM_INPUT);
  icmConnectProcessorNet(processor, timer0Net, "int8", ICM_INPUT);
  icmConnectProcessorNet(processor, radioNet, "int1", ICM_INPUT);
  icmConnectProcessorNet(processor, spi0Net, "int3", ICM_INPUT);
  icmConnectProcessorNet(processor, spi1Net, "int4", ICM_INPUT);

  // connect the RTC0 interrupt port to the net
  icmConnectPSENet(rtc0, rtcNet, "rtc_irq", ICM_OUTPUT);
  icmConnectPSENet(timer0, timer0Net, "timer_irq", ICM_OUTPUT);
  icmConnectPSENet(radio, radioNet, "radio_irq", ICM_OUTPUT);
  icmConnectPSENet(spi0, spi0Net, "spi_irq", ICM_OUTPUT);
  icmConnectPSENet(spi1, spi1Net, "spi_irq", ICM_OUTPUT);
  icmConnectPSENet(rtc0, rtcPPINet, "rtc_ppi", ICM_OUTPUT);
  icmConnectPSENet(timer0, timer0PPINet, "timer0_ppi", ICM_OUTPUT);
  icmConnectPSENet(radio, radioPPINet, "radio_ppi", ICM_OUTPUT);
  icmConnectPSENet(spi0, spi0PPINet, "spi_ppi", ICM_OUTPUT);
  icmConnectPSENet(spi1, spi1PPINet, "spi_ppi", ICM_OUTPUT);

  icmAddNetCallback(rtcPPINet, intPPINetWritten, NULL);
  icmAddNetCallback(timer0PPINet, intPPINetWritten, NULL);
  icmAddNetCallback(radioPPINet, intPPINetWritten, NULL);
  icmAddNetCallback(spi0PPINet, intPPINetWritten, NULL);
  icmAddNetCallback(spi1PPINet, intPPINetWritten, NULL);

  rngNet = icmNewNet("rng_irq");
  icmConnectProcessorNet(processor, rngNet, "int13", ICM_INPUT);

  icmPrintNetConnections();

  // init memory with FFs
  Uns32 i, initmemory = 0xFFFFFFFF;
  for (i = 0; i < 0x00040000 - 0x1000; i+=sizeof(int)) {
    icmWriteProcessorMemory(processor, i, &initmemory, sizeof(int));
  }

  // Load Hex file into Simulator Memory
  char *dot = strrchr(application, '.');
  if (dot && !strcmp(dot, ".hex")) {
    if (loadHexFile(processor, (char*)application) != 0) {
      icmPrintf("Hex File Load of %s Failed\n", application);
      icmTerminate();
      return -1;
    }
  } else {
    icmLoadProcessorMemory(processor, application, ICM_LOAD_PHYSICAL, True, True);
  }

  const Uns32 reg = 0xFFFFFFFF;
  char register_name_buffer[4];
  for (i = 0; i < 12; i++) {
    sprintf(register_name_buffer, "r%d", i);
    icmWriteReg(processor, register_name_buffer, &reg);
  }

  unsigned int cafebabe = 0xcafebabe;
  icmWriteProcessorMemory(processor, 0x2000011c, &cafebabe, 4);

  // Run the simulation
  simulate_custom_platform(processor);

  // free simulation data structures
  icmTerminate();

  return 0;
}

static void simulate_custom_platform(icmProcessorP processor) {

  initRng();

  Uns32 currentPC = 0, prevPC = 0;
  unsigned char isbranch = 0;
  double TIME_SLICE = 1.0 / 16000000.0;
  icmStopReason rtnVal = ICM_SR_SCHED;
  Uns32 tick = 0;
  Uns32 dbgcnt = 0;
  for (tick = 0; rtnVal == ICM_SR_SCHED || rtnVal == ICM_SR_HALT; tick++) {

    startPendingRngIrq(rngNet);

    prevPC = currentPC;
    currentPC = (Uns32)icmGetPC(processor);
    if (rtnVal == ICM_SR_SCHED && isbranch != 0 && (prevPC + sizeof(short) != currentPC)) {
      tick += 2;
    }

    //icmPrintf("%f -> 0x%08x\n", (double)tick * TIME_SLICE, currentPC);

    isbranch = 0;
    instruction_type_t instruction_type = 0;
    unsigned char cycles = 1;
    Uns16 data = 0xffff;

    Uns32 instruction;
    if ((currentPC >> 1) < TABLE_SIZE) {
      instruction = get_instruction(currentPC, table);
    } else {
      // put some default values in the case when code is executed from RAM... for now
      const char* disassemble = icmDisassemble(processor, currentPC);
      char instructionName[8];
      char instructionArgs[20];
      parse_ovp_disassembled_line(disassemble, instructionName, instructionArgs);
      instruction = encode_instruction_data(instructionName, instructionArgs);
      icmPrintf("In-RAM instruction: %s -> %s %s\n", disassemble, instructionName, instructionArgs);
    }

    instruction_type = ((instruction >> 24) & 0xff);
    isbranch = is_conditional_branch(instruction_type);
    cycles = ((instruction >> 16) & 0xff);
    data = (instruction & 0xffff);
    calculateAverageCurrent(processor, tick, currentPC, instruction_type, cycles, data);

    rtnVal = icmSimulate(processor, 1); // it could return "halt" on wfe?
    if (rtnVal != ICM_SR_SCHED) {
      tick += 31; // to 127?
    } else if (cycles > 1) {
      tick += cycles - 1;
    }

    icmAdvanceTime(((double)tick) * TIME_SLICE);

    stopPendingRngIrq(rngNet);

    runPPI(processor);

    if (++dbgcnt % 100 == 0) {
      icmPrintf("CURRENT TIME IS: %f\n", ((double)tick) * TIME_SLICE);
    }

    if (tick > 1.2*16000000) {
      icmPrintf("****end of execution\n");
      printAverageCurrentPerTimeSlot();
      icmPrintf("ticks - %d\n", tick);
      break;
    }
  }
}

static void parseArgs(int argc, char ** argv) {

  // check for the application program name argument
  if (argc != 3) {
    icmMessage("F", "ARGS", "Usage: %s <application elf/hex file> <elf/hex disassembled file>", argv[0] );
  }

  application = argv[1];
  assemblerFile = argv[2];
}
