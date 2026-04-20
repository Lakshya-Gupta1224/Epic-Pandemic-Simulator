#ifndef TIMER_H
#define TIMER_H

#include "models.h"

void timer_init(SimState* state);
/* Returns flags: bit 0 = new hour, bit 1 = new day */
int  timer_update(SimState* state, float deltaTime);

#endif
