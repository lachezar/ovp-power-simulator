import re
import sys
import operator

MULS_CYCLES = 1 # could be 32 if no hardware support

def parse_dis_line(line):
  stripped_comment_line = re.sub(r';.*$', '', line)
  parsed_line = re.match(r'^\s*([0-9a-f]+):\s*[0-9a-f]{4}(?:\s[0-9a-f]{4})?\s*\t([\w.]+)\s*(.*)$', stripped_comment_line)
  return parsed_line

def parse_dis_file(file_name):
  instructions = list()
  with open(file_name) as f:
    for line in f:
      parsed_line = parse_dis_line(line)
      if parsed_line is None or len(parsed_line.groups()) != 3:
        print "Error parsing line: ", line
      else:
        instructions.append(parsed_line.groups())
  return instructions
  
def store_table(table, file_name):
  with open(file_name, 'w') as f:
    for entry in table:
      f.write("%s:%d:%d:%s:%d\n" % entry)
      
def instructions_stats(instructions):
  stats = dict()
  for i in instructions:
    stats[i] = stats.get(i, 0) + 1
  
  print 'Instructions stats:'
  for x in sorted(stats.iteritems(), key=operator.itemgetter(1), reverse=True):
    print x[0], x[1], "{0:.3f}%".format(100 * (x[1] / float(len(instructions))))
  return stats
  
def count_instruction_cycles(i):
  # based on data from: http://web.mit.edu/clarkds/www/Files/slides1.pdf
  branch = False
  ldr_location = 0

  if i[1].startswith('mul'):
    cycles = MULS_CYCLES
  elif i[1] == 'pop':
    cycles = 1 + len(i[2].split(','))
    if 'pc' in i[2]:
      cycles += 3
  elif i[1] == 'push':
    cycles = 1 + len(i[2].split(','))
  elif i[1] in ('ldmia', 'stmia'):
    registers = re.search(r'({.*})', i[2]).group(1)
    cycles = 1 + len(registers.split(','))
  elif i[1] in ('bl', 'dmb', 'dsb', 'isb', 'msr', 'mrs'):
    cycles = 4
  elif i[2].startswith('pc'):
    cycles = 3
  elif i[1].startswith('b'):
    cycles = 1
    branch = True
  elif i[1].startswith('ldr'):
    register_location = re.search(r'\[(\w+)(,|\])', i[2]).group(1)
    if register_location in ('pc', 'sp'):
      ldr_location = 0xF
    else:
      ldr_location = int(register_location.strip('r'))
    cycles = 2
  elif i[1].startswith('str') or i[1] in ('wfe', 'wfi'):
    cycles = 2
  elif i[1] == '' or i[1] is None or 'illegal' in i[2]:
    cycles = 0
  else:
    cycles = 1
   
  return (i[0], cycles, branch, i[1], ldr_location)
  
def map_cycles(instructions):
  table = list()
  for i in instructions:
    branch = False
    
    instruction_cycles = count_instruction_cycles(i)
    
    if instruction_cycles[1] == 0:
      print "Unsupported instruction: ", i
      
    table.append(instruction_cycles)
    
  return table
      
if __name__ == '__main__':
  if len(sys.argv) >= 2:
    filename = sys.argv[1]
  else:
    filename = 'dis.b.asm'
  instructions = parse_dis_file(filename)
  table = map_cycles(instructions)
  store_table(table, 'cycles_map.txt')
  instructions_stats(map(operator.itemgetter(1), instructions))