#ifndef _PLATFORM_H
#define _PLATFORM_H

static void simulate_custom_platform(icmProcessorP processor);
static void parseArgs(int argc, char ** argv);

static const char* application;
static const char* assemblerFile;
static const char* model;
static const char* semihosting;
static icmNetP timer0Net, rtcNet, rngNet, radioNet, radioPPINet, rtcPPINet, timer0PPINet;

#define TABLE_SIZE 0x10000
static Uns32 table[TABLE_SIZE];

#endif //_PLATFORM_H
