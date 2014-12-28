#include <stdio.h>

int load_table(const char* filename, unsigned char* table, int size) {
  
  FILE *fp;
  fp = fopen(filename, "r");

  if (!fp) {
    return -1;
  }
  
  unsigned int address, cycles, branch;
  
  while (!feof(fp) && fscanf(fp, "%x:%d:%d\n", &address, &cycles, &branch) == 3) {
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