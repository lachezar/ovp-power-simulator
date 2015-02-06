import re
import sys
import math
import operator

from generate_cycle_map import count_instruction_cycles

TIME_SLOT = 0.04 # in s
MULS_CYCLES = 1
RESISTOR_VALUE = 15.0
BRANCH_NOT_TAKEN_CURRENT = 58.8 / RESISTOR_VALUE

def read_trace_log(filename):
  pattern = re.compile(r'^(\d+.\d+) -> (0x[0-9a-f]+)\s*(ldr_flash|[rw])?$')

  with open(filename) as f:
    for line in f:
      instruction = re.match(pattern, line)
      if instruction is not None:
        yield (float(instruction.group(1)), int(instruction.group(2), 16), instruction.group(3))
  
def read_instructions_map(filename):
  pattern = re.compile(r'^([0-9a-f]+):(\d+):(\d+):(.+):(\d+)$')
  instructions_map = dict()

  with open(filename) as f:
    for line in f:
      instruction = re.match(pattern, line)
      address = int(instruction.group(1), 16)
      cycles = int(instruction.group(2))
      is_branch = bool(int(instruction.group(3)))
      instruction_type = instruction.group(4)
      instructions_map[address] = (instruction_type, cycles, is_branch)

  return instructions_map
  
def map_trace_log_to_instructions(trace_log, instructions_map):
  stats = dict()
  current_per_time_slot = dict()
  cycles_per_time_slot = dict()
  post_process_branch = False
  prev_address = 0
  slot_id = 0
  for t, x, descriptor in trace_log:
    
    flash_current_value = flash_current(prev_address, x)
    prev_address = x
    instruction, cycles, is_branch = instructions_map.get(x, (None, None, None))
    if instruction is None:
      if x > 0x40000000 and descriptor in ('w', 'r'):
        peripheral_operation = "%x+%s" % (x, descriptor)
        stats[peripheral_operation] = stats.get(peripheral_operation, 0) + 1
        current_per_time_slot[slot_id] = current_per_time_slot.get(slot_id, 0.0) + peripheral_current(x, descriptor)
        cycles_per_time_slot[slot_id] = cycles_per_time_slot.get(slot_id, 0) + 2
      continue

    stats[instruction] = stats.get(instruction, 0) + 1
    
    slot_id = int(float(t) / TIME_SLOT)
    
    if post_process_branch:
      post_process_branch = False
      if x - prev_address != 2:
        current_per_time_slot[slot_id] = current_per_time_slot.get(slot_id, 0.0) + branch_current
        cycles_per_time_slot[slot_id] = cycles_per_time_slot.get(slot_id, 0) + branch_cycles
      else:
        current_per_time_slot[slot_id] = current_per_time_slot.get(slot_id, 0.0) + BRANCH_NOT_TAKEN_CURRENT
        cycles_per_time_slot[slot_id] = cycles_per_time_slot.get(slot_id, 0) + 1
    
    if is_branch:
      post_process_branch = True
      branch_current = current(instruction, cycles + 2, is_branch, descriptor) + flash_current_value
      branch_cycles = cycles + 2 # we keep the branch cycles as 1, but taken branches are 3 cycles
      continue # check branch state on the next instruction execution

    current_per_time_slot[slot_id] = current_per_time_slot.get(slot_id, 0.0) + current(instruction, cycles, is_branch, descriptor) + flash_current_value
    cycles_per_time_slot[slot_id] = cycles_per_time_slot.get(slot_id, 0) + cycles

  return (stats, current_per_time_slot, cycles_per_time_slot)
  
def current(instruction, cycles, is_branch, descriptor):
  resistor = RESISTOR_VALUE
  # I = U/R
  avg_current = float(cycles) * instruction_avg_voltage(instruction, cycles, is_branch, descriptor) / resistor
  return avg_current
  
def instruction_avg_voltage(instruction, cycles, is_branch, descriptor):
  if instruction.startswith('mul'):
    voltage = 57.0
  elif instruction.startswith('nop'):
    voltage = 57.4
  elif instruction.startswith('add'):
    voltage = 57.6
  elif instruction.startswith('sub'):
    voltage = 59.0
  elif instruction.startswith('cmp') or instruction.startswith('cmn'):
    voltage = 56.4
  elif instruction.startswith('mov') or instruction.startswith('mvn') or instruction.startswith('neg'): # MOV, MVN, and NEG
    voltage = 56.4
  elif instruction.startswith('and') or instruction.startswith('orr') or instruction.startswith('eor') or instruction.startswith('bic'): #AND, ORR, EOR,or BIC
    voltage = 55.6
  elif instruction.startswith('asr') or instruction.startswith('lsl') or instruction.startswith('lsr') or instruction.startswith('ror'): # ASR, LSL, LSR, and ROR
    voltage = 55.4
  elif instruction.startswith('push') or instruction.startswith('str') or instruction.startswith('stmia'):
    voltage = 62.0 - ((cycles - 2) * 3.6)
  elif (instruction.startswith('ldr') or instruction.startswith('ldmia')) and descriptor == 'ldr_flash':
    voltage = 86.4 - ((cycles - 2) * 3.6)
  elif instruction.startswith('pop') or instruction.startswith('ldr') or instruction.startswith('ldmia'):
    voltage = 52.8 - ((cycles - 2) * 3.6)
  elif instruction in ('bl', 'dmb', 'dsb', 'isb', 'msr', 'mrs'): # no clue
    voltage = 60.0
  elif instruction.startswith('bne'):
    voltage = 73.0
  elif instruction.startswith('beq'):
    voltage = 74.2
  elif instruction in ('b', 'b.n'):
    voltage = 71.6
  elif instruction.startswith('b') or is_branch:
    voltage = 74.0
  elif instruction in ('wfe', 'wfi'): # no clue
    voltage = 60.0
  elif instruction == '' or instruction is None: # no clue
    voltage = 60.0
  elif instruction in ('uxth', 'adcs', 'sxth', 'uxtb', 'sxtb', 'tst', 'sbcs'): # extension of half-words to words etc.
    voltage = 58.0
  else:
    print "BAD!!!! ", instruction
    voltage = 60.0
  
  return voltage
  
def flash_current(prev_address, current_address):
  resistor = RESISTOR_VALUE

  if prev_address ^ current_address == 0:
    return 0.0
    
  e = int(math.log(prev_address ^ current_address, 2))
  voltage = 0.0
  if e in (4, 5):
    voltage = 1.0
  elif e in (6, 7):
    voltage = 3.5
  elif e in (8, 9):
    voltage = 13.0
  elif e in (10, 11):
    voltage = 14.5
  elif e >= 12:
    voltage = 16.0

  return voltage / resistor
  
def peripheral_current(address, access_type):
  resistor = RESISTOR_VALUE
  voltage = 0.0
  if address == 0x4000d100 and access_type == 'r':
    voltage = 35.0
  elif address == 0x4000d508 and access_type == 'r':
    voltage = 35.0
  elif address == 0x4000d100 and access_type == 'w':
    voltage = 52.0 # it is around 52 in fact, but the numbers don't match may be because we loop on EVT_VALRDY and this lowers the average current
  return voltage / resistor

def print_execution_stats(stats):
  executed_instructions = sum(stats.values())
  print 'Executed instructions stats:'
  for x in sorted(stats.iteritems(), key=operator.itemgetter(1), reverse=True):
    print x[0], x[1], "{0:.3f}%".format(100 * (x[1] / float(executed_instructions)))
    
def print_current_log(current_per_time_slot, cycles_per_time_slot):
  print 'Current (mA) per time slot:'
  for i, x in current_per_time_slot.items():
    print "[{0:.3f}, {1:.3f}) -> {2:.5f}".format(i * TIME_SLOT, (i + 1) * TIME_SLOT, x / float(cycles_per_time_slot[i]))
  
if __name__ == '__main__':
  if len(sys.argv) >= 2:
    trace_log_filename = sys.argv[1]
  trace_log = read_trace_log(trace_log_filename)
  instructions_map = read_instructions_map('cycles_map.txt')
  stats, current_per_time_slot, cycles_per_time_slot = map_trace_log_to_instructions(trace_log, instructions_map)
  print_execution_stats(stats)
  print
  print "----------------"
  print
  print_current_log(current_per_time_slot, cycles_per_time_slot)

  