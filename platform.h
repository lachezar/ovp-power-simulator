#ifndef _PLATFORM_H
#define _PLATFORM_H

static void simulate_custom_platform(icmProcessorP processor);
static void parseArgs(int argc, char ** argv);

static const char *application;
static const char *model;
static const char *semihosting;
static icmNetP timer0Net, rtcNet, rngNet, radioNet, radioPPINet, rtcPPINet, timer0PPINet;

#define CYCLES_TABLE_SIZE 0xC000
static unsigned char cycles_table[CYCLES_TABLE_SIZE];


#endif //_PLATFORM_H