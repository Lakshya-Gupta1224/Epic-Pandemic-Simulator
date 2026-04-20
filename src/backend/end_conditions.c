#include "end_conditions.h"

EndCondition end_check(const SimState* state, const SimConfig* config) {
    float pop = config->population;

    /* Pandemic ended naturally (after day 15, infection < 1%, recovery high) */
    if (state->currentDay > 15 &&
        (state->infected + state->exposed) < 0.01f * pop &&
        (state->recovered + state->susceptible) >= 0.95f * pop) {
        return END_PANDEMIC_OVER;
    }

    /* Budget depleted */
    if (state->economy <= 0) {
        return END_BUDGET_DEPLETED;
    }

    /* Mental health crisis */
    if (state->mentalHealth <= 0.0f) {
        return END_MENTAL_CRISIS;
    }

    /* Reached maximum days */
    if (state->currentDay >= config->maxDays) {
        return END_MAX_DAYS;
    }

    return END_NONE;
}
