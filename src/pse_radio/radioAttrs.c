#include "peripheral/impTypes.h"
#include "peripheral/bhm.h"
#include "peripheral/ppm.h"

static ppmBusPort busPorts[] = {
  {
    .name            = "RADIO",
    .type            = PPM_SLAVE_PORT,
    .addrHi          = 0xfffLL,
    .mustBeConnected = 1,
    .remappable      = 0,
    .description     = 0,
  },
  { 0 }
};

static PPM_BUS_PORT_FN(nextBusPort) {
  if(!busPort) {
    return busPorts;
  } else {
    busPort++;
  }
  return busPort->name ? busPort : 0;
}

static ppmNetPort netPorts[] = {
  {
    .name            = "radio_irq",
    .type            = PPM_OUTPUT_PORT,
    .mustBeConnected = 0,
    .description     = 0
  },
  {
    .name            = "radio_ppi",
    .type            = PPM_OUTPUT_PORT,
    .mustBeConnected = 0,
    .description     = 0
  },
  { 0 }
};

static PPM_NET_PORT_FN(nextNetPort) {
  if(!netPort) {
    return netPorts;
  } else {
    netPort++;
  }
  return netPort->name ? netPort : 0;
}

static ppmParameter parameters[] = {
  { 0 }
};

static PPM_PARAMETER_FN(nextParameter) {
  if(!parameter) {
    return parameters;
  } else {
    parameter++;
  }
  return parameter->name ? parameter : 0;
}

ppmModelAttr modelAttrs = {

  .versionString = PPM_VERSION_STRING,
  .type          = PPM_MT_PERIPHERAL,

  .busPortsCB    = nextBusPort,
  .netPortsCB    = nextNetPort,
  .paramSpecCB   = nextParameter,

  .vlnv          = {
    .vendor  = "ovpworld.org",
    .library = "peripheral",
    .name    = "Radio",
    .version = "1.0"
  },

  .family    = "ovpworld.org",

};
