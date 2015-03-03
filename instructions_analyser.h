#ifndef _INSTRUCTIONS_ANALYSER_H
#define _INSTRUCTIONS_ANALYSER_H

typedef enum {BNE, BEQ, B, BL32, BRANCH, LDR, STR, STMIA, LDMIA, POP, PUSH, MOV, MVN, NEG, CMP, TST, ADD, SUB, MUL, EOR, AND, ROR, BIC, ASR, LSL, LSR, _32BIT, WF_, SEV, NOP, EXTENSION, UNKNOWN} instruction_type_t; // 30 different types

int load(const char* filename, unsigned int* table, int size);

int parse_line(const char* line, unsigned int* address, char* instruction_name, char* instruction_args);

unsigned int calculate_cycles(instruction_type_t instruction_type, const char* args);

instruction_type_t enumerate_instruction(const char* instruction_name);

unsigned short meta_data(instruction_type_t instruction_type, const char* args);

unsigned int get_instruction(unsigned int address, unsigned int* table);

unsigned int is_conditional_branch(instruction_type_t instruction_type);

unsigned int encode_instruction_data(char* instruction_name, char* instruction_args);

int normalize_assembly_format(const char* original_line, char* normalized_line);

int parse_ovp_disassembled_line(const char* line, char* instruction_name, char* instruction_args);

#endif //_INSTRUCTIONS_ANALYSER_H
