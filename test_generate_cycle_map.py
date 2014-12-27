from generate_cycle_map import *

# some tests

if __name__ == '__main__':
  assert parse_dis_line("   16878:	f37d 0000 			; <UNDEFINED> instruction: 0xf37d0000") == None
  assert parse_dis_line("") == None
  assert parse_dis_line("   16872:	46c0      	nop			; (mov r8, r8)").groups() == ('16872', 'nop', '')
  assert parse_dis_line("   1689c:	bd30      	pop	{r4, r5, pc}").groups() == ('1689c', 'pop', '{r4, r5, pc}')
  assert parse_dis_line("  3000:	ff10 ffff 	vmaxnm.f<illegal width 64>	<illegal reg q7.5>, q8, <illegal reg q15.5>").groups() == ('3000', 'vmaxnm.f', '<illegal width 64>	<illegal reg q7.5>, q8, <illegal reg q15.5>')
  
  assert count_instruction_cycles(('1', 'pop', '{r0, r1, pc}')) == ('1', 7, 0)
  assert count_instruction_cycles(('1', 'pop', '{r0, r1}')) == ('1', 3, 0)
  assert count_instruction_cycles(('1', 'add', 'pc, r0')) == ('1', 3, 0)
  assert count_instruction_cycles(('1', 'add', '  illegal ')) == ('1', 0, 0)
  assert count_instruction_cycles(('1', 'push', '{r0, r1, r2, r3,r4,pc } ')) == ('1', 7, 0)
  assert count_instruction_cycles(('1', 'ldmia', 'r4!, {r0, r1, r2} ')) == ('1', 4, 0)
  assert count_instruction_cycles(('1', 'bne.n', 'r4')) == ('1', 1, 1)
  assert count_instruction_cycles(('1', 'bl', '')) == ('1', 4, 0)
  
  