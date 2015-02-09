#ifndef _INSTRUCTIONS_ANALYSER_H
#define _INSTRUCTIONS_ANALYSER_H

typedef enum {BNE, BEQ, B, BL32, BRANCH, LDR, STR, STMIA, LDMIA, POP, PUSH, MOV, MVN, NEG, CMP, TST, ADD, SUB, MUL, EOR, AND, ROR, BIC, ASR, LSL, LSR, _32BIT, WF_, SEV, NOP, EXTENSION, UNKNOWN} instruction_type_t; // 30 different types

int load(const char* filename, unsigned int* table, int size);

int parse_line(const char* line, unsigned int* address, char* instruction_name, char* instruction_args);

/*int encode_instruction(unsigned int address, instruction_type_t instruction_type, char* args, unsigned int* table, int size);*/

unsigned int calculate_cycles(instruction_type_t instruction_type, const char* args);

instruction_type_t enumerate_instruction(const char* instruction_name);

unsigned short meta_data(instruction_type_t instruction_type, const char* args);

unsigned int get_instruction(unsigned int address, unsigned int* table);

unsigned int is_branch(unsigned int address, unsigned int* table);
/*
unsigned int is_ldr(unsigned int address);

unsigned int is_str(unsigned int address);*/

#endif //_INSTRUCTIONS_ANALYSER_H
