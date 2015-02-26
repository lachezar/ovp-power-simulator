#include "hexLoader.h"

static Int32 switchEndianness(Uns32 data);
static Int32 loadData(icmProcessorP processor, Uns32 address, Uns32 data);

Int32 loadHexFile(icmProcessorP processor, char *fileName) {

  FILE *fp;
  fp = fopen(fileName, "r");

  if (!fp) {
    icmPrintf ("Failed to open Memory Initialization File %s\n", fileName);
    return -1;
  }
  
  icmPrintf("\nLoading Hex file %s\n", fileName);

  Uns32 byteCount, addressOffset, recordType, baseAddress;
  while (!feof(fp) && fscanf(fp, ":%02x%04x%02x", &byteCount, &addressOffset, &recordType) == 3) {

    if (byteCount == 0 && addressOffset == 0 && recordType == 1) {
      
      icmPrintf("Load Complete\n\n");

      if (fclose(fp) != 0) {
        icmPrintf("Failed to close Memory Initialization File\n");
        return -1;
      }

      return 0; // eof
      
    } else if (byteCount == 2 && recordType == 4) {
      fscanf(fp, "%04x", &baseAddress);
      baseAddress = (baseAddress << 16);
      
      icmPrintf("\nNew base address 0x%08x\n", baseAddress);
    } else if (recordType == 0) {
      Uns32 data, i;
      for (i = 0; i < byteCount; i += sizeof(Uns32)) {
        fscanf(fp, "%08x", &data);
        data = switchEndianness(data);
        if (loadData(processor, baseAddress + addressOffset + i, data) != 0) {
          return -1;
        }
      }
    } else if (recordType == 3) {
      // load CS:IP
      Uns32 registers;
      fscanf(fp, "%08x", &registers);
      registers = switchEndianness(registers);

      //Uns32 cs = ((registers & 0xFFFF0000) >> 16);
      //Uns32 ipReg = (registers & 0x0000FFFF);
    }
    fscanf(fp, "%*02x\n");
  }

  if (fclose(fp) != 0) {
    icmPrintf("Failed to close Memory Initialization File\n");
    return -1;
  }
  
  icmPrintf("No EOF marker was found - the file format is wrong\n");

  return -1;   // no eof marker was found - the file format is wrong
}

Int32 switchEndianness(Uns32 data) {
  data = (data & 0x000000ff) << 24 |
         (data & 0x0000ff00) <<  8 |
         (data & 0x00ff0000) >>  8 |
         (data & 0xff000000) >> 24 ;
  return data;
}

static Int32 loadData(icmProcessorP processor, Uns32 address, Uns32 data) {

  Uns32 dataCheck;

  //
  // Access the memory through the processor memory space
  //
  icmWriteProcessorMemory(processor, address, &data, 4);
  icmReadProcessorMemory(processor, address, &dataCheck, 4);

  if (data != dataCheck) {
    return -1;
  }

  return 0;
}
