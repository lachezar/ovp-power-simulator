#include "ppi.h"

#undef info
#define info(format, ...) icmPrintf("       +++ PPI: ");\
icmPrintf(format, ##__VA_ARGS__);\
icmPrintf("\n")

#undef error
#define error(format, ...) icmPrintf(">>> ERROR IN PPI: ");\
icmPrintf(format, ##__VA_ARGS__);\
icmPrintf("\n")

static Uns32 ppiQueue[PPI_CHANNELS_COUNT];
static Uns32 ppiQueueProcessed[PPI_CHANNELS_COUNT];
static Uns32 ppiQueueSize = 0;
static NRF_PPI_Type ppi;
static Uns32 shouldRunPPI = 0;

static void syncPPIQueue(Uns32 bitmask);

void runPPI(icmProcessorP processor) {
  
  if (shouldRunPPI == 0) {
    return;
  }
  info("running ppi");
  shouldRunPPI = 0;
  
  Uns32 i, j;
  Uns32 data;
  const Uns32 signal = 1;
  
  for (i = 0; i < ppiQueueSize; i++) {
    j = ppiQueue[i];
    PPI_CH_Type ppich = ppi.CH[j];

    if (ppich.EEP != 0 && ppich.TEP != 0 && (ppi.CHEN & (1 << j)) != 0) {
      // get data from address ppich.EEP    
      icmReadProcessorMemory(processor, ppich.EEP, &data, 4);

      if (data != 0) {
        
        if (ppiQueueProcessed[j] == 0) {
          // write 1 to ppich.TEP address
          icmWriteProcessorMemory(processor, ppich.TEP, &signal, 4);
          info("CONNECTION DONE! EEP 0x%08x -> TEP 0x%08x", ppich.EEP, ppich.TEP);
        } else {
          info("ALREADY TRIGGERED! EEP 0x%08x -> TEP 0x%08x", ppich.EEP, ppich.TEP);
        }
        ppiQueueProcessed[j] = 1;

      } else {
        ppiQueueProcessed[j] = 0;
      }
    }
  }
}

void syncPPIQueue(Uns32 bitmask) {
  ppiQueueSize = 0;
  Uns32 i;
  for (i = 0; i < PPI_CHANNELS_COUNT; i++) {
    if ((bitmask & (1 << i)) != 0) {
      ppiQueue[ppiQueueSize++] = i;
    }
  } 
}

NET_WRITE_FN(intPPINetWritten) {
  info("Trigger PPI Nets with value = %d", value);
  shouldRunPPI = value;
}

ICM_MEM_READ_FN(extMemReadPPICB) {
  if ((Uns32)address == 0x4001f800) { // PPI Channel group config CHG[0]
    *(Uns32 *)value = ppi.CHG[0];
  } else if ((Uns32)address == 0x4001f804) { // PPI Channel group config CHG[1]
    *(Uns32 *)value = ppi.CHG[1];
  } else {
    error("********** Not handled mem location - PPI - 0x%08x", (Uns32)address);
    icmTerminate();
  }
  info(
    "Reading 0x%08x from 0x%08x",
    *(Uns32 *)value, (Uns32)address
  );
}

ICM_MEM_WRITE_FN(extMemWritePPICB) {
  if ((Uns32)address == 0x4001f504) { // PPI Channel enable set
    ppi.CHENSET = *(Uns32*)value;
    ppi.CHEN |= *(Uns32*)value;
    syncPPIQueue(ppi.CHEN);
    info("Write to PPI CHENSET");
  } else if ((Uns32)address >= 0x4001f510 && (Uns32)address <= 0x4001f58c) { // PPI CHANNEL
    Uns32 id = ((Uns32)address - 0x4001f510) / (2 * sizeof(Uns32));

    if ((Uns32)address % (2 * sizeof(Uns32)) == 0) {  
      ppi.CH[id].EEP = (*(Uns32*)value);
      info("Write to PPI CH[%d] EEP", id);
    } else {
      ppi.CH[id].TEP = (*(Uns32*)value);
      info("Write to PPI CH[%d] TEP", id);
    }

    ppiQueueProcessed[id] = 0;

  } else if ((Uns32)address >= 0x4001f800 && (Uns32)address <= 0x4001f80c) { // PPI Channel group configuration 
    Uns32 id = ((Uns32)address - 0x4001f800) / sizeof(Uns32);
    ppi.CHG[id] = (*(Uns32*)value);
    info("Write to PPI CHG[%d] (group!!!)", id);
  } else if ((Uns32)address == 0x4001f508) { // PPI Channel clear
    ppi.CHEN &= ~(*(Uns32*)value);
    syncPPIQueue(ppi.CHEN);
    info("Write to PPI CHENCLR ");
  } else {
    error("********** Not handled mem location for writing - PPI - 0x%08x", (Uns32)address);
    icmTerminate();
  }

  info(
    "Writing 0x%08x to 0x%08x (%d, %d, %d)",
    (*(Uns32*)value), (Uns32)address, (Uns32)bytes, (Uns32)value, (Uns32)userData
  );
}
