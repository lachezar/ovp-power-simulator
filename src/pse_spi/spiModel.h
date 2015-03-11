#ifndef _SPI_H
#define _SPI_H

#include "peripheral/impTypes.h"
#include "peripheral/bhm.h"
#include "peripheral/ppm.h"

#define __O
#define __I
#define __IO

typedef Uns32 uint32_t;

typedef struct {                            /*!< SPI Structure                                                         */
  __IO uint32_t EVENTS_READY;                       /*!< TXD byte sent and RXD byte received.                                  */
  __IO uint32_t INTENSET;                           /*!< Interrupt enable set register.                                        */
  __IO uint32_t INTENCLR;                           /*!< Interrupt enable clear register.                                      */
  __IO uint32_t ENABLE;                             /*!< Enable SPI.                                                           */
  __IO uint32_t PSELSCK;                            /*!< Pin select for SCK.                                                   */
  __IO uint32_t PSELMOSI;                           /*!< Pin select for MOSI.                                                  */
  __IO uint32_t PSELMISO;                           /*!< Pin select for MISO.                                                  */
  __I uint32_t RXD;                                 /*!< RX data.                                                              */
  __IO uint32_t TXD;                                /*!< TX data.                                                              */
  __IO uint32_t FREQUENCY;                          /*!< SPI frequency                                                         */
  __IO uint32_t CONFIG;                             /*!< Configuration register.                                               */
  __IO uint32_t POWER;                              /*!< Peripheral power control.                                             */
} spiRegs;

PPM_CONSTRUCTOR_CB(init);
PPM_CONSTRUCTOR_CB(periphConstructor);

void loop();

// Bus and net port declarations
ppmNetHandle irqHandle;
ppmNetHandle spiNotificationHandle;

spiRegs regs;
void* spiWindow;

bhmEventHandle startEventHandle;
bhmEventHandle txdEventHandle;

//
// prototypes
//
PPM_VIEW_CB(viewReg32);
PPM_REG_READ_CB(regRd32);
PPM_REG_WRITE_CB(regWr32);

#endif
