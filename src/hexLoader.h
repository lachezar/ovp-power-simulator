#ifndef _HEX_LOADER_H
#define _HEX_LOADER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "icm/icmCpuManager.h"

#define MAX_LINE_LENGTH 64

Int32 loadHexFile(icmProcessorP processor, char *fileName);

#endif // HEX_LOADER