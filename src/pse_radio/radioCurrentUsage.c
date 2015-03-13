#include <stdio.h>

#include "peripheral/bhm.h"

#include "radioCurrentUsage.h"
#include "radio_state_machine.h"

struct StateLogEntry {
  double transitionTime;
  Int32 state;
} stateLog[RADIO_LOG_SIZE];

Uns32 stateLogSize = 0;

static Uns32 avgCurrentByState(Uns32 state) {
  if (state == RADIO_STATE_STATE_Disabled || state == RADIO_STATE_STATE_TxDisable) return 0; // uA
  else if (state == RADIO_STATE_STATE_TxRu || state == RADIO_STATE_STATE_TxIdle) return 6500; // uA
  else if (state == RADIO_STATE_STATE_Tx) return 11000; // uA
  else {
    bhmPrintf("ERROR in avgCurrentByState - not handled state!!!\n\n");
    return 0;
  }
}

void addStateLogEntry(Uns32 state) {
  static Uns32 prevState = -1;

  if (prevState == state) {
    // same state, nothing to do here
    return;
  }

  prevState = state;

  stateLog[stateLogSize].transitionTime = bhmGetCurrentTime();
  stateLog[stateLogSize].state = state;
  stateLogSize++;
}

void printAvgCurrent() {
  bhmPrintf("RADIO AVERAGE CURRENT LOG:\n\n");

  FILE* fp = fopen("radio_avg_current.csv", "w+");

  Uns32 i;
  for (i = 0; i < stateLogSize-1; i++) {
    Uns32 current = avgCurrentByState(stateLog[i].state);
    if (current == 0 || stateLog[i].transitionTime == stateLog[i+1].transitionTime) {
      // don't report recorded states that lasted for 0 seconds or brought no change in current
    } else {
      bhmPrintf("%f,%f - %d\n", stateLog[i].transitionTime / 1000000.0, stateLog[i+1].transitionTime / 1000000.0, current);
      fprintf(fp, "%f,%f,%d\n", stateLog[i].transitionTime / 1000000.0, stateLog[i+1].transitionTime / 1000000.0, current);
    }
  }

  fclose(fp);
}
