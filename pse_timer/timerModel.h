#ifndef _TIMER_H
#define _TIMER_H

#include "peripheral/impTypes.h"
#include "peripheral/bhm.h"
#include "peripheral/ppm.h"

typedef struct {

  Uns32 TASKS_START;                        /*!< Start Timer.                                                          */
  Uns32 TASKS_STOP;                         /*!< Stop Timer.                                                           */
  Uns32 TASKS_COUNT;                        /*!< Increment Timer (In counter mode).                                    */
  Uns32 TASKS_CLEAR;                        /*!< Clear timer.                                                          */
  Uns32 TASKS_SHUTDOWN;                     /*!< Shutdown timer.                                                       */
  Uns32 TASKS_CAPTURE0;                     /*!< Capture Timer value to CC[n] registers.                               */
  Uns32 TASKS_CAPTURE1;
  Uns32 TASKS_CAPTURE2;
  Uns32 TASKS_CAPTURE3;
  Uns32 EVENTS_COMPARE0;                    /*!< Compare event on CC[n] match.                                         */
  Uns32 EVENTS_COMPARE1;
  Uns32 EVENTS_COMPARE2;
  Uns32 EVENTS_COMPARE3;
  Uns32 SHORTS;                             /*!< Shortcuts for Timer.                                                  */
  Uns32 INTENSET;                           /*!< Interrupt enable set register.                                        */
  Uns32 INTENCLR;                           /*!< Interrupt enable clear register.                                      */
  Uns32 MODE;                               /*!< Timer Mode selection.                                                 */
  Uns32 BITMODE;                            /*!< Sets timer behaviour.                                                 */
  Uns32 PRESCALER;                          /*!< 4-bit prescaler to source clock frequency (max value 9). Source
                                                         clock frequency is divided by 2^SCALE.                                */
  Uns32 CC0;                                /*!< Capture/compare registers.                                            */
  Uns32 CC1;
  Uns32 CC2;
  Uns32 CC3;
  Uns32 POWER;
} timerRegs;

PPM_CONSTRUCTOR_CB(init);
PPM_CONSTRUCTOR_CB(periphConstructor);

void loop();

// Bus and net port declarations
ppmNetHandle irqHandle;
ppmNetHandle timer0NotificationHandle;

timerRegs regs;
void* timerWindow;

bhmEventHandle startEventHandle;

//
// prototypes
//
PPM_VIEW_CB(viewReg32);
PPM_REG_READ_CB(regRd32);
PPM_REG_WRITE_CB(regWr32);

#endif
