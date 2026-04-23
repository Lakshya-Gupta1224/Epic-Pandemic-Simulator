#include "end_conditions.h"

/*
 * end_check  --  called once per simulated DAY.
 *
 * The simulation ends ONLY IFF:
 * 1. Population reaches zero (S+E+I+R == 0)
 * 2. Budget gets exhausted (budget <= 0)
 * 3. Mental health declines to zero (mentalHealth <= 0)
 *
 * (We also keep a Victory condition for when the pandemic is eradicated).
 */
EndCondition end_check(const SimState* state, const SimConfig* config) {
    (void)config;
    /* 1. Population reaches zero */
    int alive = state->susceptible + state->exposed + state->infected + state->recovered;
    if (alive <= 0 && state->currentDay > 1) {
        return END_POPULATION_ZERO;
    }

    /* 2. Budget exhausted */
    if (state->economy <= 0) {
        return END_BUDGET_DEPLETED;
    }

    /* 3. Mental health collapse */
    if (state->mentalHealth <= 0.0f) {
        return END_MENTAL_CRISIS;
    }

    /* 4. Victory: pandemic eradicated (all infected and exposed are gone) */
    /* Threshold: infected + exposed < 1 and some time has passed */
    if (state->currentDay > 15 && (state->infected + state->exposed) <= 0) {
        return END_VICTORY;
    }

    return END_NONE;
}
