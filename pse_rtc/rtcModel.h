#ifndef _RTC_H
#define _RTC_H

#include "peripheral/impTypes.h"
#include "peripheral/bhm.h"
#include "peripheral/ppm.h"

#define RTC_PERIPHERAL_ID 0xB

typedef struct {                            /*!< RTC Structure                                                         */
  Uns32 TASKS_START;                        /*!< Start RTC Counter.                                                    */
  Uns32 TASKS_STOP;                         /*!< Stop RTC Counter.                                                     */
  Uns32 TASKS_CLEAR;                        /*!< Clear RTC Counter.                                                    */
  Uns32 TASKS_TRIGOVRFLW;                   /*!< Set COUNTER to 0xFFFFFFF0.                                            */
  Uns32 EVENTS_TICK;                        /*!< Event on COUNTER increment.                                           */
  Uns32 EVENTS_OVRFLW;                      /*!< Event on COUNTER overflow.                                            */
  Uns32 EVENTS_COMPARE0;                    /*!< Compare event on CC[n] match.                                         */
  Uns32 EVENTS_COMPARE1;                    /*!< Compare event on CC[n] match.                                         */
  Uns32 EVENTS_COMPARE2;                    /*!< Compare event on CC[n] match.                                         */
  Uns32 EVENTS_COMPARE3;                    /*!< Compare event on CC[n] match.                                         */
  Uns32 INTENSET;                           /*!< Interrupt enable set register.                                        */
  Uns32 INTENCLR;                           /*!< Interrupt enable clear register.                                      */
  Uns32 EVTEN;                              /*!< Configures event enable routing to PPI for each RTC event.            */
  Uns32 EVTENSET;                           /*!< Enable events routing to PPI. The reading of this register gives
                                                         the value of EVTEN.                                                   */
  Uns32 EVTENCLR;                           /*!< Disable events routing to PPI. The reading of this register
                                                         gives the value of EVTEN.                                             */
  Uns32 COUNTER;                            /*!< Current COUNTER value.                                                */
  Uns32 PRESCALER;                          /*!< 12-bit prescaler for COUNTER frequency (32768/(PRESCALER+1)).
                                                         Must be written when RTC is STOPed.                                   */
  Uns32 CC0;                                /*!< Capture/compare registers.                                            */
  Uns32 CC1;                                /*!< Capture/compare registers.                                            */
  Uns32 CC2;                                /*!< Capture/compare registers.                                            */
  Uns32 CC3;                                /*!< Capture/compare registers.                                            */
  Uns32 POWER;                              /*!< Peripheral power control.                                             */
} rtcRegs;

PPM_CONSTRUCTOR_CB(init);
PPM_CONSTRUCTOR_CB(periphConstructor);

void loop();

// Bus and net port declarations
ppmNetHandle irqHandle;
ppmNetHandle rtcNotificationHandle;

rtcRegs regs;
void* rtcWindow;

bhmEventHandle startEventHandle;

//
// prototypes
//
PPM_VIEW_CB(viewReg32);
PPM_REG_READ_CB(regRd32);
PPM_REG_WRITE_CB(regWr32);

#endif
