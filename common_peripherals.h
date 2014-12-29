#ifndef _COMMON_PERIPHERALS_H
#define _COMMON_PERIPHERALS_H

#include <stdlib.h>
#include <time.h>

#include "icm/icmCpuManager.h"

void init_rng();
void start_pending_rng_irq(icmNetP rngNet);
void stop_pending_rng_irq(icmNetP rngNet);

ICM_MEM_READ_FN(extMemReadRNGCB);
ICM_MEM_WRITE_FN(extMemWriteRNGCB);
ICM_MEM_READ_FN(extMemReadGPIOCB);
ICM_MEM_WRITE_FN(extMemWriteGPIOCB);
ICM_MEM_READ_FN(extMemReadUICRCB);
ICM_MEM_WRITE_FN(extMemWriteUICRCB);
ICM_MEM_READ_FN(extMemReadFICRCB);
ICM_MEM_WRITE_FN(extMemWriteFICRCB);
ICM_MEM_READ_FN(extMemReadNVMCCB);
ICM_MEM_WRITE_FN(extMemWriteNVMCCB);
ICM_MEM_READ_FN(extMemReadClockCB);
ICM_MEM_WRITE_FN(extMemWriteClockCB);

#endif // _COMMON_PERIPHERALS_H
