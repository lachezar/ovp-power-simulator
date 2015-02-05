#include <stdio.h>
#include <string.h>

int load_table(const char* filename, unsigned short* table, int size) {
  
  FILE *fp;
  fp = fopen(filename, "r");

  if (!fp) {
    return -1;
  }
  
  unsigned int address, cycles, branch, memory_utilization;
  char instruction_name[10];
  
  while (!feof(fp) && fscanf(fp, "%x:%d:%d:%[^:]:%d\n", &address, &cycles, &branch, instruction_name, &memory_utilization) == 5) {
    
    if (strstr(instruction_name, "ldr") != NULL) {
      cycles = cycles | 0x40;
      cycles = cycles | (memory_utilization << 8);
    }
    if (branch == 1) {
      cycles = cycles | 0x80;
    }
    if (address >> 1 >= size) {
      return -1;
    }
    table[address >> 1] = cycles;
  }
  
  if (fclose(fp) != 0) {
    return -1;
  }
  
  return 0;
}