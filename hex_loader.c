#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "icm/icmCpuManager.h"

//
// Load a hex file into simulation memory
//
#define MAX_LINE_LENGTH 64

static int switch_endianness(int data);
static int load_data(icmProcessorP processor, int address, int data);

int load_hex_file(icmProcessorP processor, char *fileName) {

    FILE *fp;
    fp = fopen(fileName, "r");

    if (!fp) {
        printf ("Failed to open Memory Initialization File %s\n", fileName);
        return -1;
    }

    icmPrintf("\nLoading Hex file %s\n", fileName);
    
    unsigned int byte_count, address_offset, record_type, base_address;
    while (!feof(fp) && fscanf(fp, ":%02x%04x%02x", &byte_count, &address_offset, &record_type) == 3) {
      
      //icmPrintf("\nbyte count %d address %x record type %d\n", byte_count, address_offset, record_type);
      
      //icmPrintf("\nfp position %d\n", (int)ftell(fp));
      
      if (byte_count == 0 && address_offset == 0 && record_type == 1) {
        icmPrintf("Load Complete\n\n");
        
        if (fclose(fp) != 0) {
          icmPrintf("Failed to close Memory Initialization File\n");
          return -1;
        }
        
        return 0; // eof
      
      } else if (byte_count == 2 && record_type == 4) {
        fscanf(fp, "%04x", &base_address);
        base_address = (base_address << 16);
        
        icmPrintf("\nNew base address 0x%08x\n", base_address);
        
      } else if (record_type == 0) {
        unsigned int data, i;
        for (i = 0; i < byte_count; i += sizeof(int)) {
          fscanf(fp, "%08x", &data);
          
          data = switch_endianness(data);
          
          if (load_data(processor, base_address + address_offset + i, data) != 0) {
            return -1;
          }        
        }
      } else if (record_type == 3) {
        // load CS:IP
        unsigned int registers, cs_reg, ip_reg;
        fscanf(fp, "%08x", &registers);
        registers = switch_endianness(registers);
        
        cs_reg = ((registers & 0xFFFF0000) >> 16);
        ip_reg = (registers & 0x0000FFFF);
        //ip_reg = 0xb102;
        
        // put the values in the registers
        icmPrintf("\nloading CS 0x%08x and IP 0x%08x\n", cs_reg, ip_reg);
        //icmWriteReg(processor, "CS", &cs_reg);
        
        //ip_reg = switch_endianness(ip_reg);
        //icmPrintf("\nIP 0x%08x\n", ip_reg);
        //icmWriteReg(processor, "PC", &ip_reg);
      }
      fscanf(fp, "%*02x\n"); // @TODO check the checksum 
    }
    
    if (fclose(fp) != 0) {
      icmPrintf("Failed to close Memory Initialization File\n");
    }
    
    icmPrintf("No EOF marker was found - the file format is wrong\n");
    
    return -1; // no eof marker was found - the file format is wrong
}

static int switch_endianness(int data) {
  data = (data & 0x000000ff) << 24 |
         (data & 0x0000ff00) <<  8 |
         (data & 0x00ff0000) >>  8 |
         (data & 0xff000000) >> 24 ;
  return data;
}

static int load_data(icmProcessorP processor, int address, int data) {
  
  int data_check;
  
  //
  // Access the memory through the processor memory space
  //
  icmWriteProcessorMemory(processor,      // processor
                          address,        // memory address
                          &data,          // data buffer of data to write
                          4);             // number of bytes to write

  icmReadProcessorMemory(processor, address, &data_check, 4);

  if (data != data_check) {
      icmPrintf("Failed Data Read Back at 0x%08x, 0x%08x 0x%08x\n", (Uns32)address, (Uns32)data, (Uns32)data_check);
      return -1;
  }

  //icmPrintf("  0x%08x <= 0x%08x\n", address, data);
  return 0;
}