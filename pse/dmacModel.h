/*
 * Copyright (c) 2005-2014 Imperas Software Ltd., www.imperas.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied.
 *
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef _DMAC_H
#define _DMAC_H

#include "peripheral/impTypes.h"
#include "peripheral/bhm.h"
#include "peripheral/ppm.h"

typedef Uns32 uint32_t;

typedef struct {

  uint32_t  TASKS_START;                       /*!< Start Timer.                                                          */
  uint32_t  TASKS_STOP;                        /*!< Stop Timer.                                                           */
  uint32_t  TASKS_COUNT;                       /*!< Increment Timer (In counter mode).                                    */
  uint32_t  TASKS_CLEAR;                       /*!< Clear timer.                                                          */
  uint32_t  TASKS_SHUTDOWN;                    /*!< Shutdown timer.                                                       */
  uint32_t  TASKS_CAPTURE0;                  /*!< Capture Timer value to CC[n] registers.                               */
  uint32_t  TASKS_CAPTURE1;
  uint32_t  TASKS_CAPTURE2;
  uint32_t  TASKS_CAPTURE3;
  uint32_t  EVENTS_COMPARE0;                 /*!< Compare event on CC[n] match.                                         */
  uint32_t  EVENTS_COMPARE1;
  uint32_t  EVENTS_COMPARE2;
  uint32_t  EVENTS_COMPARE3;
  uint32_t  SHORTS;                            /*!< Shortcuts for Timer.                                                  */
  uint32_t  INTENSET;                          /*!< Interrupt enable set register.                                        */
  uint32_t  INTENCLR;                          /*!< Interrupt enable clear register.                                      */
  uint32_t  MODE;                              /*!< Timer Mode selection.                                                 */
  uint32_t  BITMODE;                           /*!< Sets timer behaviour.                                                 */
  uint32_t  PRESCALER;                         /*!< 4-bit prescaler to source clock frequency (max value 9). Source
                                                         clock frequency is divided by 2^SCALE.                                */
  uint32_t  CC0;                             /*!< Capture/compare registers.                                            */
  uint32_t  CC1;
  uint32_t  CC2;
  uint32_t  CC3;
  uint32_t  POWER;  
} timerRegs;

// Net callbacks

PPM_CONSTRUCTOR_CB(init);
PPM_CONSTRUCTOR_CB(periphConstructor);

void userMainLoop(void);
void userReset(Uns32 v);

// Bus and net port declarations
extern ppmNetHandle      irq_handle;

extern Uns32 diag;

extern timerRegs regs;

void* timer_window;

//
// prototypes
//
PPM_VIEW_CB(viewReg32);
PPM_REG_READ_CB(regRd32);
PPM_REG_WRITE_CB(regWr32);

#endif
