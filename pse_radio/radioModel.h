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

#ifndef _RADIO_H
#define _RADIO_H

#include "peripheral/impTypes.h"
#include "peripheral/bhm.h"
#include "peripheral/ppm.h"

#define __O 
#define __I
#define __IO  

typedef Uns32 uint32_t;

typedef struct { /*!< RADIO Structure                                                       */
  __O  uint32_t  TASKS_TXEN;                        /*!< Enable radio in TX mode.                                              */
  __O  uint32_t  TASKS_RXEN;                        /*!< Enable radio in RX mode.                                              */
  __O  uint32_t  TASKS_START;                       /*!< Start radio.                                                          */
  __O  uint32_t  TASKS_STOP;                        /*!< Stop radio.                                                           */
  __O  uint32_t  TASKS_DISABLE;                     /*!< Disable radio.                                                        */
  __O  uint32_t  TASKS_RSSISTART;                   /*!< Start the RSSI and take one sample of the receive signal strength.    */
  __O  uint32_t  TASKS_RSSISTOP;                    /*!< Stop the RSSI measurement.                                            */
  __O  uint32_t  TASKS_BCSTART;                     /*!< Start the bit counter.                                                */
  __O  uint32_t  TASKS_BCSTOP;                      /*!< Stop the bit counter.                                                 */
  __IO uint32_t  EVENTS_READY;                      /*!< Ready event.                                                          */
  __IO uint32_t  EVENTS_ADDRESS;                    /*!< Address event.                                                        */
  __IO uint32_t  EVENTS_PAYLOAD;                    /*!< Payload event.                                                        */
  __IO uint32_t  EVENTS_END;                        /*!< End event.                                                            */
  __IO uint32_t  EVENTS_DISABLED;                   /*!< Disable event.                                                        */
  __IO uint32_t  EVENTS_DEVMATCH;                   /*!< A device address match occurred on the last received packet.          */
  __IO uint32_t  EVENTS_DEVMISS;                    /*!< No device address match occurred on the last received packet.         */
  __IO uint32_t  EVENTS_RSSIEND;                    /*!< Sampling of the receive signal strength complete. A new RSSI
                                                         sample is ready for readout at the RSSISAMPLE register.               */
  __IO uint32_t  EVENTS_BCMATCH;                    /*!< Bit counter reached bit count value specified in BC register.         */
  __IO uint32_t  SHORTS;                            /*!< Shortcuts for the radio.                                              */
  __IO uint32_t  INTENSET;                          /*!< Interrupt enable set register.                                        */
  __IO uint32_t  INTENCLR;                          /*!< Interrupt enable clear register.                                      */
  __I  uint32_t  CRCSTATUS;                         /*!< CRC status of received packet.                                        */
  __I  uint32_t  CD;                                /*!< Carrier detect.                                                       */
  __I  uint32_t  RXMATCH;                           /*!< Received address.                                                     */
  __I  uint32_t  RXCRC;                             /*!< Received CRC.                                                         */
  __I  uint32_t  DAI;                               /*!< Device address match index.                                           */
  __IO uint32_t  PACKETPTR;                         /*!< Packet pointer. Decision point: START task.                           */
  __IO uint32_t  FREQUENCY;                         /*!< Frequency.                                                            */
  __IO uint32_t  TXPOWER;                           /*!< Output power.                                                         */
  __IO uint32_t  MODE;                              /*!< Data rate and modulation.                                             */
  __IO uint32_t  PCNF0;                             /*!< Packet configuration 0.                                               */
  __IO uint32_t  PCNF1;                             /*!< Packet configuration 1.                                               */
  __IO uint32_t  BASE0;                             /*!< Radio base address 0. Decision point: START task.                     */
  __IO uint32_t  BASE1;                             /*!< Radio base address 1. Decision point: START task.                     */
  __IO uint32_t  PREFIX0;                           /*!< Prefixes bytes for logical addresses 0 to 3.                          */
  __IO uint32_t  PREFIX1;                           /*!< Prefixes bytes for logical addresses 4 to 7.                          */
  __IO uint32_t  TXADDRESS;                         /*!< Transmit address select.                                              */
  __IO uint32_t  RXADDRESSES;                       /*!< Receive address select.                                               */
  __IO uint32_t  CRCCNF;                            /*!< CRC configuration.                                                    */
  __IO uint32_t  CRCPOLY;                           /*!< CRC polynomial.                                                       */
  __IO uint32_t  CRCINIT;                           /*!< CRC initial value.                                                    */
  __IO uint32_t  TEST;                              /*!< Test features enable register.                                        */
  __IO uint32_t  TIFS;                              /*!< Inter Frame Spacing in microseconds.                                  */
  __I  uint32_t  RSSISAMPLE;                        /*!< RSSI sample.                                                          */
  __I  uint32_t  STATE;                             /*!< Current radio state.                                                  */
  __IO uint32_t  DATAWHITEIV;                       /*!< Data whitening initial value.                                         */
  __IO uint32_t  BCC;                               /*!< Bit counter compare.                                                  */
  __IO uint32_t  DAB[8];                            /*!< Device address base segment.                                          */
  __IO uint32_t  DAP[8];                            /*!< Device address prefix.                                                */
  __IO uint32_t  DACNF;                             /*!< Device address match configuration.                                   */
  __IO uint32_t  OVERRIDE0;                         /*!< Trim value override register 0.                                       */
  __IO uint32_t  OVERRIDE1;                         /*!< Trim value override register 1.                                       */
  __IO uint32_t  OVERRIDE2;                         /*!< Trim value override register 2.                                       */
  __IO uint32_t  OVERRIDE3;                         /*!< Trim value override register 3.                                       */
  __IO uint32_t  OVERRIDE4;                         /*!< Trim value override register 4.                                       */
  __IO uint32_t  POWER;                             /*!< Peripheral power control.                                             */
} radioRegs;


// Net callbacks

PPM_CONSTRUCTOR_CB(init);
PPM_CONSTRUCTOR_CB(periphConstructor);
void loop();

void userMainLoop(void);
void userReset(Uns32 v);

// Bus and net port declarations
extern ppmNetHandle      irq_handle;
extern ppmNetHandle      ppi_notification_handle;

extern Uns32 diag;

extern radioRegs regs;

void* radio_window;

bhmEventHandle start_eh;

//
// prototypes
//
PPM_VIEW_CB(viewReg32);
PPM_REG_READ_CB(regRd32);
PPM_REG_WRITE_CB(regWr32);

#endif
