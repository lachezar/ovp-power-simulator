#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "instructions_analyser.h"

void assert_parse_line(const char* line, const int expected_result, const unsigned int expected_address, const char* expected_instruction, const char* expected_args) {
  
  int result;
  unsigned int address;
  char instruction_name[8];
  char instruction_args[64];
  
  result = parse_line(line, &address, instruction_name, instruction_args);
  assert(result == expected_result);
  
  if (expected_result == 0) {
    assert(address == expected_address);
    assert(strcmp(instruction_name, expected_instruction) == 0);
    assert(strcmp(instruction_args, expected_args) == 0);
  }
}

void test_parse_line() {
  assert_parse_line("    3000:	ff10 ffff 	vmaxnm.f<illegal width 64>	<illegal reg q7.5>, q8, <illegal reg q15.5>", -1, 0, "", "");
  assert_parse_line("    3004:	e5db      	b.n	0x2bbe", 0, 0x3004, "b.n", "0x2bbe");
  assert_parse_line("    304e:	a1f3      	add	r1, pc, #972	; (adr r1, 0x341c)", 0, 0x304e, "add", "r1, pc, #972");
  assert_parse_line(" 600:	bf00      	nop", 0, 0x600, "nop", "");
  assert_parse_line(" 250:	4ad7      	ldr	r2, [pc, #860]	; (0x5b0)", 0, 0x250, "ldr", "r2, [pc, #860]");
  assert_parse_line(" 276:	b5f0      	push	{r4, r5, r6, r7, lr}", 0, 0x276, "push", "{r4, r5, r6, r7, lr}");
  assert_parse_line(" 28a:	f7ff ffdd 	bl	0x248", 0, 0x28a, "bl", "0x248");
  assert_parse_line("00001000 <.sec2>:", -1, 0, "", "");
  assert_parse_line("	...", -1, 0, "", "");
  assert_parse_line("", -1, 0, "", "");
}

void test_meta_data() {
  assert(meta_data(ADD, "") == 0xffff);
  assert(meta_data(LDR, "r1, [r0, #0]") == 0);
  assert(meta_data(LDR, "r1, [r2, #128]") == (2 << 11) + 128);
  assert(meta_data(LDR, "r1, [r2, r3]") == 0x8000 + (2 << 11) + 3);
  assert(meta_data(LDR, "r1, [pc, #4]") == (0xf << 11));
  assert(meta_data(STR, "r2, [sp, #4]") == (0xe << 11));
}

void test_file_loading(const char* filename) {
  unsigned int table[0x18000];
  load(filename, table, 0x18000);
}

void test_normalize_assembly_format() {
  char buffer[100];
  normalize_assembly_format("4210     tst     r0,r2", buffer);
  assert(strcmp(buffer, "4210\ttst\tr0, r2") == 0);
  normalize_assembly_format("6808     ldr     r0,[r1]", buffer);
  assert(strcmp(buffer, "6808\tldr\tr0, [r1]") == 0);
  normalize_assembly_format("d0fc     beq     2000000c", buffer);
  assert(strcmp(buffer, "d0fc\tbeq\t2000000c") == 0);
  normalize_assembly_format("d0fc     ldr     r1,[pc,#360]", buffer);
  assert(strcmp(buffer, "d0fc\tldr\tr1, [pc, #360]") == 0);
}

void test_parse_ovp_disassembled_line() {
  char buffer[2][100];
  parse_ovp_disassembled_line("d0fc     ldr     r1,[pc,#360]", buffer[0], buffer[1]);
  assert(strcmp(buffer[0], "ldr") == 0);
  assert(strcmp(buffer[1], "r1, [pc, #360]") == 0);
  parse_ovp_disassembled_line("d0fc     nop", buffer[0], buffer[1]);
  assert(strcmp(buffer[0], "nop") == 0);
  assert(strcmp(buffer[1], "") == 0);
}

int main() {

  test_parse_line();
  test_meta_data();
  test_normalize_assembly_format();
  test_parse_ovp_disassembled_line();

  printf("All tests have passed!");

  return 0;
}
