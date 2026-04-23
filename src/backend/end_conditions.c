#include "end_conditions.h"

/*
 * end_check  --  called once per simulated DAY.
 *
 * Conditions (checked in priority order):
 *
 * 1. POPULATION COLLAPSE (Catastrophic Fatalities)
 *    Trigger: dead > 50% of original population, after day 5.
 *    Rationale: virus has decimated society.
 *
 * 2. HOSPITAL SYSTEM FAILURE
 *    Trigger: hospital has been at 100% capacity for 10+ consecutive days.
 *    Rationale: sustained overwhelm → total healthcare collapse.
 *
 * 3. ECONOMIC COLLAPSE
 *    Trigger: budget ≤ 0.
 *
 * 4. SOCIETAL / MENTAL HEALTH COLLAPSE
 *    Trigger: mentalHealth ≤ 0.
 *
 * 5. VICTORY — Pandemic Over
 *    Trigger: after day 15, active (infected + exposed) < 1% of population
 *    AND (recovered + susceptible) ≥ 80% of population.
 *    Rationale: disease has been eradicated.
 *
 * 6. MAX DAYS REACHED
 *    Trigger: currentDay ≥ maxDays.
 */
EndCondition end_check(const SimState* state, const SimConfig* config) {
    float pop    = config->population;
    int   iThresh_1pct  = (int)(pop * 0.01f);
    int   iThresh_50pct = (int)(pop * 0.50f);
    int   iThresh_80pct = (int)(pop * 0.80f);

    /* 1. Population collapse */
    if (state->currentDay > 5 && state->dead > iThresh_50pct) {
        return END_PANDEMIC_OVER;
    }

    /* 2. Hospital system failure (10 consecutive overwhelmed days) */
    if (state->hospitalOverwhelmedDays >= 10) {
        return END_HOSPITAL_COLLAPSE;
    }

    /* 3. Economic collapse */
    if (state->economy <= 0) {
        return END_BUDGET_DEPLETED;
    }

    /* 4. Mental health / societal collapse */
    if (state->mentalHealth <= 0.0f) {
        return END_MENTAL_CRISIS;
    }

    /* 5. Victory: pandemic eradicated */
    if (state->currentDay > 15 &&
        (state->infected + state->exposed) < iThresh_1pct &&
        (state->recovered + state->susceptible) >= iThresh_80pct) {
        return END_PANDEMIC_OVER;
    }

    /* 6. Max days */
    if (state->currentDay >= config->maxDays) {
        return END_MAX_DAYS;
    }

    return END_NONE;
}
