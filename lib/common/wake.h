#ifndef _WAKE_H
#define _WAKE_H

#include "stdint.h"

void logWakeReason();
void goSleep(uint64_t usecDelay);
#endif
