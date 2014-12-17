#ifndef _PLATFORM_H
#define _PLATFORM_H

typedef Uns32 uint32_t;

typedef struct {
  uint32_t  EN;                                /*!< Enable channel group.                                                 */
  uint32_t  DIS;                               /*!< Disable channel group.                                                */
} PPI_TASKS_CHG_Type;

typedef struct {
  uint32_t  EEP;                               /*!< Channel event end-point.                                              */
  uint32_t  TEP;                               /*!< Channel task end-point.                                               */
} PPI_CH_Type;


typedef struct {                                    /*!< PPI Structure                                                         */
  PPI_TASKS_CHG_Type TASKS_CHG[4];                  /*!< Channel group tasks.                                                  */
  uint32_t  CHEN;                              /*!< Channel enable.                                                       */
  uint32_t  CHENSET;                           /*!< Channel enable set.                                                   */
  uint32_t  CHENCLR;                           /*!< Channel enable clear.                                                 */
  PPI_CH_Type CH[16];                               /*!< PPI Channel.                                                          */
  uint32_t  CHG[4];                            /*!< Channel group configuration.                                          */
} NRF_PPI_Type;

#endif //_PLATFORM_H