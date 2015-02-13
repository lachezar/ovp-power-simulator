#ifndef _CURRENT_USAGE_H
#define _CURRENT_USAGE_H

#include "icm/icmCpuManager.h"
#include "instructions_analyser.h"

// This module uses OVP functions, so we keep it camelcased.
// The current values are in uA
// Time slice for average current usage calculation (in seconds)
#define CURRENT_TIME_SLICE 0.004 // 4ms
#define CURRENT_CYCLES_SLICE (16000000 * CURRENT_TIME_SLICE) // count of cycles per time slice

Uns32 calculateAverageCurrent(icmProcessorP processor, Uns32 tick, Uns32 address, instruction_type_t instruction_type, Uns32 cycles, Uns16 data);

Uns32 instructionCurrent(icmProcessorP processor, instruction_type_t instruction_type, Uns32 cycles, Uns16 data);

Uns32 ldrInstructionCurrent(icmProcessorP processor, Uns16 data); // flash or RAM or peripheral

Uns32 strInstructionCurrent(icmProcessorP processor, Uns16 data); // RAM or peripheral

Uns32 persistantPeripheralCurrent(icmProcessorP processor); // @TODO implement this

Uns32 flashCurrent(Uns32 prevAddress, Uns32 address);

Uns32 peripheralReadCurrent(Uns32 address);

Uns32 peripheralWriteCurrent(Uns32 address);

void printAverageCurrentPerTimeSlot();


#endif //_CURRENT_USAGE_H
