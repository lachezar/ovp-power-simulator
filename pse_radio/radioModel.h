#ifndef _RADIO_H
#define _RADIO_H

#include "peripheral/impTypes.h"
#include "peripheral/bhm.h"
#include "peripheral/ppm.h"

#define __O
#define __I
#define __IO

typedef struct { /*!< RADIO Structure                                                       */
  __O Uns32 TASKS_TXEN;                          /*!< Enable radio in TX mode.                                              */
  __O Uns32 TASKS_RXEN;                          /*!< Enable radio in RX mode.                                              */
  __O Uns32 TASKS_START;                         /*!< Start radio.                                                          */
  __O Uns32 TASKS_STOP;                          /*!< Stop radio.                                                           */
  __O Uns32 TASKS_DISABLE;                       /*!< Disable radio.                                                        */
  __O Uns32 TASKS_RSSISTART;                     /*!< Start the RSSI and take one sample of the receive signal strength.    */
  __O Uns32 TASKS_RSSISTOP;                      /*!< Stop the RSSI measurement.                                            */
  __O Uns32 TASKS_BCSTART;                       /*!< Start the bit counter.                                                */
  __O Uns32 TASKS_BCSTOP;                        /*!< Stop the bit counter.                                                 */
  __IO Uns32 EVENTS_READY;                       /*!< Ready event.                                                          */
  __IO Uns32 EVENTS_ADDRESS;                     /*!< Address event.                                                        */
  __IO Uns32 EVENTS_PAYLOAD;                     /*!< Payload event.                                                        */
  __IO Uns32 EVENTS_END;                         /*!< End event.                                                            */
  __IO Uns32 EVENTS_DISABLED;                    /*!< Disable event.                                                        */
  __IO Uns32 EVENTS_DEVMATCH;                    /*!< A device address match occurred on the last received packet.          */
  __IO Uns32 EVENTS_DEVMISS;                     /*!< No device address match occurred on the last received packet.         */
  __IO Uns32 EVENTS_RSSIEND;                     /*!< Sampling of the receive signal strength complete. A new RSSI
                                                         sample is ready for readout at the RSSISAMPLE register.               */
  __IO Uns32 EVENTS_BCMATCH;                     /*!< Bit counter reached bit count value specified in BC register.         */
  __IO Uns32 SHORTS;                             /*!< Shortcuts for the radio.                                              */
  __IO Uns32 INTENSET;                           /*!< Interrupt enable set register.                                        */
  __IO Uns32 INTENCLR;                           /*!< Interrupt enable clear register.                                      */
  __I Uns32 CRCSTATUS;                           /*!< CRC status of received packet.                                        */
  __I Uns32 CD;                                  /*!< Carrier detect.                                                       */
  __I Uns32 RXMATCH;                             /*!< Received address.                                                     */
  __I Uns32 RXCRC;                               /*!< Received CRC.                                                         */
  __I Uns32 DAI;                                 /*!< Device address match index.                                           */
  __IO Uns32 PACKETPTR;                          /*!< Packet pointer. Decision point: START task.                           */
  __IO Uns32 FREQUENCY;                          /*!< Frequency.                                                            */
  __IO Uns32 TXPOWER;                            /*!< Output power.                                                         */
  __IO Uns32 MODE;                               /*!< Data rate and modulation.                                             */
  __IO Uns32 PCNF0;                              /*!< Packet configuration 0.                                               */
  __IO Uns32 PCNF1;                              /*!< Packet configuration 1.                                               */
  __IO Uns32 BASE0;                              /*!< Radio base address 0. Decision point: START task.                     */
  __IO Uns32 BASE1;                              /*!< Radio base address 1. Decision point: START task.                     */
  __IO Uns32 PREFIX0;                            /*!< Prefixes bytes for logical addresses 0 to 3.                          */
  __IO Uns32 PREFIX1;                            /*!< Prefixes bytes for logical addresses 4 to 7.                          */
  __IO Uns32 TXADDRESS;                          /*!< Transmit address select.                                              */
  __IO Uns32 RXADDRESSES;                        /*!< Receive address select.                                               */
  __IO Uns32 CRCCNF;                             /*!< CRC configuration.                                                    */
  __IO Uns32 CRCPOLY;                            /*!< CRC polynomial.                                                       */
  __IO Uns32 CRCINIT;                            /*!< CRC initial value.                                                    */
  __IO Uns32 TEST;                               /*!< Test features enable register.                                        */
  __IO Uns32 TIFS;                               /*!< Inter Frame Spacing in microseconds.                                  */
  __I Uns32 RSSISAMPLE;                          /*!< RSSI sample.                                                          */
  __I Uns32 STATE;                               /*!< Current radio state.                                                  */
  __IO Uns32 DATAWHITEIV;                        /*!< Data whitening initial value.                                         */
  __IO Uns32 BCC;                                /*!< Bit counter compare.                                                  */
  __IO Uns32 DAB[8];                             /*!< Device address base segment.                                          */
  __IO Uns32 DAP[8];                             /*!< Device address prefix.                                                */
  __IO Uns32 DACNF;                              /*!< Device address match configuration.                                   */
  __IO Uns32 OVERRIDE0;                          /*!< Trim value override register 0.                                       */
  __IO Uns32 OVERRIDE1;                          /*!< Trim value override register 1.                                       */
  __IO Uns32 OVERRIDE2;                          /*!< Trim value override register 2.                                       */
  __IO Uns32 OVERRIDE3;                          /*!< Trim value override register 3.                                       */
  __IO Uns32 OVERRIDE4;                          /*!< Trim value override register 4.                                       */
  __IO Uns32 POWER;                              /*!< Peripheral power control.                                             */
} radioRegs;

PPM_CONSTRUCTOR_CB(init);
PPM_CONSTRUCTOR_CB(periphConstructor);

void loop();

// Bus and net port declarations
ppmNetHandle irqHandle;
ppmNetHandle ppiNotificationHandle;

radioRegs regs;

void* radioWindow;

bhmEventHandle txenEventHandle;

//
// prototypes
//
PPM_VIEW_CB(viewReg32);
PPM_REG_READ_CB(regRd32);
PPM_REG_WRITE_CB(regWr32);

#endif
