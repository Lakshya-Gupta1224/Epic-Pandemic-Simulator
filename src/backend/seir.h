#ifndef SEIR_H
#define SEIR_H

#include "models.h"

void seir_init(SimState* state, SimConfig* config);
void seir_step(SimState* state, SimConfig* config);

#endif
