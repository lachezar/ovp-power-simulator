#include <math.h>
#include <stdio.h>

#include "currentUsage.h"

const Uns32 NOT_TAKEN_BRANCH_CURRENT = 3920;
const Uns32 NOT_TAKEN_BNE_CURRENT = 5200;

static Uns64 currentPerTimeSlot[1000];
static Uns32 cyclesPerTimeSlot[1000];
static Uns32 instructionsUsageStats[36];

Uns32 calculateAverageCurrent(icmProcessorP processor, Uns32 tick, Uns32 address, instruction_type_t instruction_type, Uns32 cycles, Uns16 data) {

  // was prev instruction branch - check address delta and apply proper calculations
  
  // calculate current instruction if it is not branch
  // if it is branch set flag that we need to handle branch current next time
  
  // add flashcurrent
  // add persistantPeripheralCurrent based on peripheral state
  
  static Uns32 postProcessBranch = 0;
  static Uns32 prevAddress = 0;
  static Uns32 slotId = 0;
  static Uns32 branchTakenCurrent;
  static Uns32 branchTakenCycles;
  static Uns32 branchNotTakenCurrent;
  static Uns32 branchNotTakenCycles;
  
  Uns32 flashCurrentValue = flashCurrent(prevAddress, address);
  
  if (postProcessBranch != 0) {
    postProcessBranch = 0;
    //icmPrintf("delta %x %x %d\n", address, prevAddress, address - prevAddress);
    if (address - prevAddress == 2) { // branch not taken
      currentPerTimeSlot[slotId] += branchNotTakenCurrent;
      cyclesPerTimeSlot[slotId] += branchNotTakenCycles;
    } else { // branch taken
      currentPerTimeSlot[slotId] += branchTakenCurrent;
      cyclesPerTimeSlot[slotId] += branchTakenCycles;
    }
  }
  
  slotId = tick / CURRENT_CYCLES_SLICE;
  
  if (instruction_type == BNE || instruction_type == BEQ || instruction_type == BRANCH) {
    postProcessBranch = 1;
    
    branchTakenCycles = 3;
    branchTakenCurrent = branchTakenCycles * instructionCurrent(processor, instruction_type, cycles, data) + flashCurrentValue;
    
    branchNotTakenCycles = cycles;
    if (instruction_type == BNE) {
      branchNotTakenCurrent = cycles * NOT_TAKEN_BNE_CURRENT + flashCurrentValue;
    } else {
      branchNotTakenCurrent = cycles * NOT_TAKEN_BRANCH_CURRENT + flashCurrentValue;
    }
    
  } else {
    currentPerTimeSlot[slotId] += cycles * instructionCurrent(processor, instruction_type, cycles, data) + flashCurrentValue;
    cyclesPerTimeSlot[slotId] += cycles;
  }
  
  currentPerTimeSlot[slotId] += persistantPeripheralCurrent(processor);
  
  instructionsUsageStats[instruction_type]++;
  prevAddress = address;
  
  return 0;
}


Uns32 instructionCurrent(icmProcessorP processor, instruction_type_t instruction_type, Uns32 cycles, Uns16 data) {

  Uns32 current;

  if (instruction_type == STR) {
    // run the function to calculate STR
    current = strInstructionCurrent(processor, data);
  } else if (instruction_type == LDR) {
    // run the function to calculate LDR
    current = ldrInstructionCurrent(processor, data);
  } else if (instruction_type == MUL) {
    //current = 3800; // uA
    current = 4520;
  } else if (instruction_type == NOP) {
    current = 3827; // uA
  } else if (instruction_type == ADD) {
    current = 3840; // uA
  } else if (instruction_type == SUB) {
    current = 3933; // uA
  } else if (instruction_type == CMP) {
    current = 3760; // uA
  } else if (instruction_type == TST) {
    current = 3867; // uA
  } else if (instruction_type == MOV || instruction_type == MVN || instruction_type == NEG) {
    current = 3760; // uA
  } else if (instruction_type == AND || instruction_type == EOR || instruction_type == BIC) {
    current = 3707; // uA
  } else if (instruction_type == LSL || instruction_type == LSR || instruction_type == ROR) {
    current = 3693; // uA
  } else if (instruction_type == ASR) {
    current = 3826; // uA
  } else if (instruction_type == PUSH || instruction_type == STMIA) {
    current = 4133 - ((cycles - 2) * 240); // uA
  } else if (instruction_type == LDMIA) { // @TODO handle the case when we load from flash with LDMIA?
    current = 5760 - ((cycles - 2) * 240); // uA
  } else if (instruction_type == POP || instruction_type == LDMIA) { // @TODO handle the case when we load from flash with LDMIA?
    current = 3520 - ((cycles - 2) * 240); // uA
  } else if (instruction_type == _32BIT) { // no clue
    current = 4000; // uA
  } else if (instruction_type == BNE) {
    current = 4867; // uA
  } else if (instruction_type == BEQ) {
    current = 4947; // uA
  } else if (instruction_type == B) {
    current = 4773; // uA
  } else if (instruction_type == BRANCH || instruction_type == BL32) {
    current = 4933; // uA
  } else if (instruction_type == WF_ || instruction_type == SEV) { // no clue
    current = 4000; // uA
  } else if (instruction_type == EXTENSION) { // extension of half-words to words etc.
    current = 3867; // uA
  } else if (instruction_type == UNKNOWN) {
    // return some default value
    current = 4000; // uA
  } else {
    // instruction in RAM or non-mapped instruction
    icmPrintf("BAD!!! instruction type - %d\n", instruction_type);
    current = 4000; // uA
  }
  
  return current;
}

Uns32 flashCurrent(Uns32 prevAddress, Uns32 address) {
  
  Uns32 delta = prevAddress ^ address;
  Uns32 distance = 0;
  if (delta != 0) {
    distance = (Uns32)(log((double)delta) / log(2.0));
  } else {
    return 0;
  }
  
  Uns32 current = 0;
  if (distance <= 3) {
    current = 0;
  } else if (distance <= 5) {
    current = 67; // uA
  } else if (distance <= 7) {
    current = 233; // uA
  } else if (distance <= 9) {
    current = 867; // uA
  } else if (distance <= 11) {
    current = 967; // uA
  } else {
    current = 1067; // uA
  }

  return current;
}

static Uns32 memoryAccessAddress(icmProcessorP processor, Uns16 data) {
  
  if (data == 0xffff) {
    icmPrintf("INVALID meta data\n");
  }
  
  Uns32 registerRegisterFormat = ((data & 0x8000) != 0);
  Uns32 regId = ((data >> 11) & 0xf);
  
  if (regId == 0xf) {
    // memory location in the flash
    return 0x20000000 - 1;
  } else if (regId == 0xe) {
    // memory location in the RAM (sp or some other register)
    return 0x20000000;
  }
  
  Uns32 addressBase;
  Uns32 addressOffset;
  char buffer[10];
  sprintf(buffer, "r%d", regId);
  icmReadReg(processor, buffer, &addressBase);
  
  if (registerRegisterFormat == 0) { // format 1 [reg, #offset]
    addressOffset = (data & 0x7ff);
    //icmPrintf("format 1");
  } else { // format 2 [reg, reg]
    Uns32 regId2 = (data & 0x7ff);
    sprintf(buffer, "r%d", regId2);
    icmReadReg(processor, buffer, &addressOffset);
    //icmPrintf("format 2(%d %d)", regId, regId2);
  }
  
  Uns32 memoryLocation = addressBase + addressOffset;

  return memoryLocation;
}

Uns32 ldrInstructionCurrent(icmProcessorP processor, Uns16 data) {
  
  Uns32 memoryLocation = memoryAccessAddress(processor, data);

  if (memoryLocation < 0x20000000) {
    // memory location in the flash
    return 5760; // uA
  } else if (memoryLocation < 0x40000000) {
    // memory location in the RAM
    return 3520; // uA
  } else {
    // memory location in the peripherals
    return peripheralReadCurrent(memoryLocation); // uA
  }
}

Uns32 strInstructionCurrent(icmProcessorP processor, Uns16 data) {
  
  Uns32 memoryLocation = memoryAccessAddress(processor, data);
  if (memoryLocation >= 0x20000000 && memoryLocation < 0x40000000) {
    // memory location in the RAM
    return 4133; // uA
  } else if (memoryLocation >= 0x40000000) {
    // memory location in the peripherals
    return peripheralWriteCurrent(memoryLocation); // uA
  } else {
    // error - can not store in the flash
    icmPrintf("ERROR: can not str into flash 0x%x 0x%x\n", memoryLocation, data);
    //return 0;
    return 4133; // uA
  }
}

Uns32 persistantPeripheralCurrent(icmProcessorP processor) {
  return 0;
}

Uns32 peripheralReadCurrent(Uns32 address) {
  icmPrintf("peripheralReadCurrent\n");
  return 3000; // put real value here
}

Uns32 peripheralWriteCurrent(Uns32 address) {
  icmPrintf("peripheralWriteCurrent\n");
  return 3000; // put real value here
}

void printAverageCurrentPerTimeSlot() {
  int i;
  for (i = 0; i < 1000; i++) {
    if (currentPerTimeSlot[i] == 0 || cyclesPerTimeSlot[i] == 0) break;
    icmPrintf("%f: %f (%d; %f)\n", (double)i * CURRENT_TIME_SLICE, (double)currentPerTimeSlot[i] / (double)cyclesPerTimeSlot[i], cyclesPerTimeSlot[i], (double)currentPerTimeSlot[i]);
  }
  
  Uns32 instructionsCount = 0;
  for (i = 0; i < 36; i++) {
    instructionsCount += instructionsUsageStats[i];
  }
  
  for (i = 0; i < 36; i++) {
    icmPrintf("%d - %f\n", i, ((double)instructionsUsageStats[i] / instructionsCount) * 100);
  }
  
}
