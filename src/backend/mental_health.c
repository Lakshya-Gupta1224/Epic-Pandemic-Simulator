#include "mental_health.h"

void mental_health_init(SimState* state, SimConfig* config) {
    state->mentalHealth      = config->maxMentalHealth;
    state->mentalHealthDelta = 0.0f;
    state->mentalHealthBase  = 0.0f;
}

void mental_health_update(SimState* state, SimConfig* config) {
    state->mentalHealth += state->mentalHealthDelta + state->mentalHealthBase;

    if (state->mentalHealth < 0.0f) state->mentalHealth = 0.0f;
    if (state->mentalHealth > config->maxMentalHealth)
        state->mentalHealth = config->maxMentalHealth;
}
