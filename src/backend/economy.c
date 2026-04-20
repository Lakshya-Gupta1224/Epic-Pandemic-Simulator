#include "economy.h"

void economy_init(SimState* state, SimConfig* config) {
    state->economy      = config->maxEconomy;
    state->economyDelta = 1;   /* Default daily drain */
}

void economy_update(SimState* state) {
    state->economy -= state->economyDelta;
    if (state->economy < 0) state->economy = 0;
}
