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

#ifndef _RTC_H
#define _RTC_H

#include "peripheral/impTypes.h"
#include "peripheral/bhm.h"
#include "peripheral/ppm.h"

typedef Uns32 uint32_t;

typedef struct {                                    /*!< RTC Structure                                                         */
  uint32_t  TASKS_START;                       /*!< Start RTC Counter.                                                    */
  uint32_t  TASKS_STOP;                        /*!< Stop RTC Counter.                                                     */
  uint32_t  TASKS_CLEAR;                       /*!< Clear RTC Counter.                                                    */
  uint32_t  TASKS_TRIGOVRFLW;                  /*!< Set COUNTER to 0xFFFFFFF0.                                            */
  uint32_t  EVENTS_TICK;                       /*!< Event on COUNTER increment.                                           */
  uint32_t  EVENTS_OVRFLW;                     /*!< Event on COUNTER overflow.                                            */
  uint32_t  EVENTS_COMPARE0;                 /*!< Compare event on CC[n] match.                                         */
  uint32_t  EVENTS_COMPARE1;                 /*!< Compare event on CC[n] match.                                         */
  uint32_t  EVENTS_COMPARE2;                 /*!< Compare event on CC[n] match.                                         */
  uint32_t  EVENTS_COMPARE3;                 /*!< Compare event on CC[n] match.                                         */
  uint32_t  INTENSET;                          /*!< Interrupt enable set register.                                        */
  uint32_t  INTENCLR;                          /*!< Interrupt enable clear register.                                      */
  uint32_t  EVTEN;                             /*!< Configures event enable routing to PPI for each RTC event.            */
  uint32_t  EVTENSET;                          /*!< Enable events routing to PPI. The reading of this register gives
                                                         the value of EVTEN.                                                   */
  uint32_t  EVTENCLR;                          /*!< Disable events routing to PPI. The reading of this register
                                                         gives the value of EVTEN.                                             */
  uint32_t  COUNTER;                           /*!< Current COUNTER value.                                                */
  uint32_t  PRESCALER;                         /*!< 12-bit prescaler for COUNTER frequency (32768/(PRESCALER+1)).
                                                         Must be written when RTC is STOPed.                                   */
  uint32_t  CC0;                             /*!< Capture/compare registers.                                            */
  uint32_t  CC1;                             /*!< Capture/compare registers.                                            */
  uint32_t  CC2;                             /*!< Capture/compare registers.                                            */
  uint32_t  CC3;                             /*!< Capture/compare registers.                                            */
  uint32_t  POWER;                             /*!< Peripheral power control.                                             */
} rtcRegs;


// Net callbacks

PPM_CONSTRUCTOR_CB(init);
PPM_CONSTRUCTOR_CB(periphConstructor);
void loop();

void userMainLoop(void);
void userReset(Uns32 v);

// Bus and net port declarations
extern ppmNetHandle      irq_handle;
extern ppmNetHandle      rtc_notification_handle;

extern Uns32 diag;

extern rtcRegs regs;

void* rtc_window;

bhmEventHandle start_eh;

//
// prototypes
//
PPM_VIEW_CB(viewReg32);
PPM_REG_READ_CB(regRd32);
PPM_REG_WRITE_CB(regWr32);

#endif
