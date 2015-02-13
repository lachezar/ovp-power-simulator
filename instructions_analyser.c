#include <stdio.h>
#include <string.h>

#include "instructions_analyser.h"

int load(const char* filename, unsigned int* table, int size) {

  FILE *fp;
  fp = fopen(filename, "r");

  if (!fp) {
    fprintf(stderr, "Can not open the file\n");
    return -1;
  }

  char line[200];
  unsigned int address;
  char instruction_name[8], instruction_args[64];
  int result;

#ifdef TESTING
  FILE *fp2;
  fp2 = fopen("test_results.txt", "w");
#endif

  while (!feof(fp)) {

    fgets(line, 200, fp);

    result = parse_line((const char*)line, &address, instruction_name, instruction_args);

    if (result == 0) {
      // enumerate the instruction, parse args and use them as meta data, calculate cycles, put the result in a reference table
      instruction_type_t instruction_type = enumerate_instruction((const char*)instruction_name);
      unsigned char cycles = calculate_cycles(instruction_type, instruction_args);
      unsigned short data = meta_data(instruction_type, instruction_args);

      table[address >> 1] = (((unsigned char) instruction_type) << 24) + (cycles << 16) + data;

#ifdef TESTING
      if (data == 0xffff) data = 0;
      fprintf(fp2, "%x:%d:%d:%s:%d\n", address, cycles, is_branch(address, table), instruction_name, (data & 0x7fff) >> 11);
#endif

    } else {
      // error parsing line
    }
  }

#ifdef TESTING
  fclose(fp2);
#endif

  if (fclose(fp) != 0) {
    fprintf(stderr, "Can not close the file\n");
    return -1;
  }

  return 0;
}

int parse_line(const char* line, unsigned int* address, char* instruction_name, char* instruction_args) {

  if (strstr(line, "illegal") != NULL || strstr(line, "<UNDEFINED>") != NULL) {
    fprintf(stderr, "PARSE ERROR: %s\n", line);
    return -1;
  }

  int parts = sscanf(line, "%*[ ]%x:\t%*[^\t]\t%[^\t]\t%[^\t\n;]", address, instruction_name, instruction_args);
  if (parts != 3) {

    parts = sscanf(line, "%*[ ]%x:\t%*[^\t]\t%[^\t\n]", address, instruction_name);
    if (parts == 2 && (strcmp(instruction_name, "nop") == 0 || strcmp(instruction_name, "wfe") == 0 || strcmp(instruction_name, "wfi") == 0 || strcmp(instruction_name, "sev") == 0)) {
      strcpy(instruction_args, "");
      return 0;
    } else {
      fprintf(stderr, "PARSE ERROR: %s\n", line);
      return -1;
    }
  } else {
    //fprintf(stderr, "PARSED: %x, %s - %s\n", *address, instruction_name, instruction_args);
  }

  return 0;
}

static int is_32bit_instruction(const char* instruction_name) {
  char list[5][5] = {"dmb", "dsb", "isb", "msr", "mrs"};
  int i;
  for (i = 0; i < 5; i++) {
    if (strstr(instruction_name, list[i]) != NULL) {
      return 1;
    }
  }
  return 0;
}

static int is_extension_instruction(const char* instruction_name) {
  char list[7][5] = {"uxth", "adcs", "sxth", "uxtb", "sxtb", "tst", "sbcs"};
  int i;
  for (i = 0; i < 7; i++) {
    if (strstr(instruction_name, list[i]) != NULL) {
      return 1;
    }
  }
  return 0;
}

instruction_type_t enumerate_instruction(const char* instruction_name) {
  // BNE, BEQ, B, BRANCH, BL32, LDR, STR, STMIA, LDMIA, POP, PUSH, MOV, MVN, NEG, CMP, TST, ADD, SUB, MUL, EOR, AND, ROR, BIC, ASR, LSL, LSR, _32BIT, WF_, SEV, NOP, EXTENSION, UNKNOWN

  instruction_type_t instruction = UNKNOWN;
  if (strstr(instruction_name, "mul") != NULL) instruction = MUL;
  else if (strstr(instruction_name, "bne") != NULL) instruction = BNE;
  else if (strstr(instruction_name, "beq") != NULL) instruction = BEQ;
  else if (strstr(instruction_name, "nop") != NULL) instruction = NOP;
  else if (strstr(instruction_name, "add") != NULL) instruction = ADD;
  else if (strstr(instruction_name, "sub") != NULL) instruction = SUB;
  else if (strstr(instruction_name, "mov") != NULL) instruction = MOV;
  else if (strstr(instruction_name, "mvn") != NULL) instruction = MVN;
  else if (strstr(instruction_name, "neg") != NULL) instruction = NEG;
  else if (strstr(instruction_name, "eor") != NULL || strstr(instruction_name, "orr") != NULL) instruction = EOR;
  else if (strstr(instruction_name, "ror") != NULL) instruction = ROR;
  else if (strstr(instruction_name, "and") != NULL) instruction = AND;
  else if (strstr(instruction_name, "bic") != NULL) instruction = BIC;
  else if (strstr(instruction_name, "asr") != NULL) instruction = ASR;
  else if (strstr(instruction_name, "lsl") != NULL) instruction = LSL;
  else if (strstr(instruction_name, "lsr") != NULL) instruction = LSR;
  else if (strstr(instruction_name, "cmp") != NULL) instruction = CMP;
  else if (strstr(instruction_name, "tst") != NULL) instruction = TST;
  else if (strstr(instruction_name, "stmia") != NULL) instruction = STMIA;
  else if (strstr(instruction_name, "ldmia") != NULL) instruction = LDMIA;
  else if (strstr(instruction_name, "pop") != NULL) instruction = POP;
  else if (strstr(instruction_name, "push") != NULL) instruction = PUSH;
  else if (strstr(instruction_name, "sev") != NULL) instruction = SEV;
  else if (strstr(instruction_name, "wf") != NULL) instruction = WF_;
  else if (strstr(instruction_name, "b.n") != NULL || (strstr(instruction_name, "b") != NULL && strlen(instruction_name) == 1)) instruction = B;
  else if (strstr(instruction_name, "bl") != NULL && strlen(instruction_name) == 2) instruction = BL32;
  else if (strstr(instruction_name, "b") == instruction_name) instruction = BRANCH;
  else if (is_32bit_instruction(instruction_name)) instruction = _32BIT;
  else if (is_extension_instruction(instruction_name)) instruction = EXTENSION;
  else if (strstr(instruction_name, "ldr") != NULL) instruction = LDR;
  else if (strstr(instruction_name, "str") != NULL) instruction = STR;

  return instruction;
}

unsigned int get_instruction(unsigned int address, unsigned int* table) {
  return table[address >> 1];
}

unsigned short meta_data(instruction_type_t instruction_type, const char* args) {
  unsigned short result = 0;
  if (instruction_type == STR || instruction_type == LDR) {
    int reg1_id, reg2_id, immediate, parts;

    parts = sscanf(args, "%*s [r%d, #%d]", &reg1_id, &immediate);
    if (parts == 2) {
      result = (reg1_id << 11) + immediate;
      return result;
    }

    parts = sscanf(args, "%*s [r%d, r%d]", &reg1_id, &reg2_id);
    if (parts == 2) {
      result = 0x8000;
      result |= (reg1_id << 11) + reg2_id;
      return result;
    }

    char register_name[8];
    parts = sscanf(args, "%*s [%s", register_name);
    if (parts == 1) {
      if (instruction_type == LDR && (strstr(register_name, "pc") != NULL || strstr(register_name, "ip") != NULL)) {
        // check for pc, ip - flash
        result = (0xF << 11);
        return result;
      } else if (strstr(register_name, "sp") != NULL || strstr(register_name, "lr") != NULL) {
        // check for sp, lr - RAM
        result = (0xE << 11);
        return result;
      }
    }
  }
  return -1;
}

static int count_tokens(const char* text, const char* tokens) {
  // strtok is fucked up function!!!
  char _text[64];
  strcpy(_text, text);
  int count = 0;
  char* pch = strtok(_text, tokens);
  while (pch != NULL) {
    pch = strtok(NULL, tokens);
    count++;
  }
  return count;
}

unsigned int calculate_cycles(instruction_type_t instruction_type, const char* args) {

  unsigned int cycles = 0;
  if (instruction_type == MUL) {
    cycles = 1;
  } else if (instruction_type == POP) {
    cycles = 1 + count_tokens(args, ",");
    if (strstr(args, "pc") != NULL) {
      cycles += 3;
    }
  } else if (instruction_type == PUSH) {
    cycles = 1 + count_tokens(args, ",");
  } else if (instruction_type == STMIA || instruction_type == LDMIA) {
    cycles = count_tokens(args, ",");
  } else if (instruction_type == _32BIT || instruction_type == BL32) {
    cycles = 4;
  } else if (strstr(args, "pc") == args) {
    fprintf(stderr, "Register pc in the destination for %d %s\n", instruction_type, args);
    cycles = 3;
  } else if (instruction_type == B) {
    cycles = 3;
  } else if (instruction_type == BNE || instruction_type == BEQ || instruction_type == BRANCH) {
    cycles = 1; // 3 cycles - if branch is taken
  } else if (instruction_type == LDR || instruction_type == STR) {
    cycles = 2;
  } else if (instruction_type == WF_ || instruction_type == SEV) {
    cycles = 2;
  } else if (instruction_type == UNKNOWN) {
    fprintf(stderr, "Unknown instruction - 1 cycle by default %s\n", args);
    cycles = 1;
  } else {
    cycles = 1;
  }

  return cycles;
}

/*unsigned int is_branch(unsigned int address, unsigned int* table) {
  unsigned int instruction = get_instruction(address, table);
  instruction_type_t instruction_type = (instruction_type_t)(instruction >> 24);
  return instruction_type == B || instruction_type == BNE || instruction_type == BEQ || instruction_type == BRANCH || instruction_type == BL32;
}*/

unsigned int is_conditional_branch(unsigned int address, unsigned int* table) {
  unsigned int instruction = get_instruction(address, table);
  instruction_type_t instruction_type = (instruction_type_t)(instruction >> 24);
  return instruction_type == BNE || instruction_type == BEQ || instruction_type == BRANCH;
}