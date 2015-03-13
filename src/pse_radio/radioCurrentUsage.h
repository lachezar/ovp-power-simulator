#ifndef _RADIO_CURRENT_USAGE_H
#define _RADIO_CURRENT_USAGE_H

#include "peripheral/impTypes.h"

#define RADIO_LOG_SIZE 1000

void addStateLogEntry(Uns32 state);
void printAvgCurrent();

#endif // _RADIO_CURRENT_USAGE_H
