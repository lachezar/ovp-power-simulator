#ifndef _PPI_H
#define _PPI_H

#include "icm/icmCpuManager.h"

#define PPI_CHANNELS_COUNT       16
#define PPI_CHANNEL_GROUPS_COUNT 4

typedef struct {
  Uns32  EN;                                /*!< Enable channel group.                                                 */
  Uns32  DIS;                               /*!< Disable channel group.                                                */
} PPI_TASKS_CHG_Type;

typedef struct {
  Uns32  EEP;                               /*!< Channel event end-point.                                              */
  Uns32  TEP;                               /*!< Channel task end-point.                                               */
} PPI_CH_Type;

typedef struct {                            /*!< PPI Structure                                                         */
  PPI_TASKS_CHG_Type TASKS_CHG[PPI_CHANNEL_GROUPS_COUNT]; /*!< Channel group tasks.                                    */
  Uns32  CHEN;                              /*!< Channel enable.                                                       */
  Uns32  CHENSET;                           /*!< Channel enable set.                                                   */
  Uns32  CHENCLR;                           /*!< Channel enable clear.                                                 */
  PPI_CH_Type CH[PPI_CHANNELS_COUNT];       /*!< PPI Channel.                                                          */
  Uns32  CHG[PPI_CHANNEL_GROUPS_COUNT];     /*!< Channel group configuration.                                          */
} NRF_PPI_Type;

ICM_MEM_READ_FN(extMemReadPPICB);
ICM_MEM_WRITE_FN(extMemWritePPICB);
NET_WRITE_FN(intPPINetWritten);

void runPPI(icmProcessorP processor);

#endif // _PPI_H