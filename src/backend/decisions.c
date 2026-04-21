#include "decisions.h"
#include <math.h>

/* Recalculate rho from all contributing factors */
void decision_recalc_rho(SimState* state) {
    float rho = 1.0f;
    int i;

    /* School closures reduce rho */
    for (i = 0; i < MAX_GRADES; i++) {
        if (!state->schoolOpen[i]) rho -= 0.02f;
    }

    /* Going out restriction */
    if (!state->goingOutAllowed) rho -= 0.10f;

    /* Sports restriction */
    if (!state->sportsAllowed) rho -= 0.08f;

    /* Hand sanitization (0-10 scale) */
    rho -= state->handSanitization * 0.03f;

    /* Masks (0-10 scale, very effective) */
    rho -= state->maskLevel * 0.05f;

    /* Lockdown percentage (0-100) */
    rho -= state->lockdownPercent * 0.005f;

    /* Population density multiplier */
    rho *= state->populationDensity;

    /* Clamp */
    if (rho < 0.0f) rho = 0.0f;
    if (rho > 2.0f) rho = 2.0f;

    state->rho = rho;
}

void decision_toggle_school(SimState* state, int grade, int open) {
    if (grade < 1 || grade > MAX_GRADES) return;
    int idx = grade - 1;
    state->schoolOpen[idx] = open ? 1 : 0;
    decision_recalc_rho(state);
}

void decision_toggle_going_out(SimState* state, int allowed) {
    state->goingOutAllowed = allowed ? 1 : 0;

    /* Mental health impact */
    if (!allowed) {
        state->mentalHealthDelta -= 0.2f;
        state->economyDelta -= 1;
    } else {
        state->mentalHealthDelta += 0.2f;
        state->economyDelta += 1;
    }

    decision_recalc_rho(state);
}

void decision_toggle_sports(SimState* state, int allowed) {
    state->sportsAllowed = allowed ? 1 : 0;

    if (!allowed) {
        state->mentalHealthDelta -= 0.1f;
    } else {
        state->mentalHealthDelta += 0.1f;
    }

    decision_recalc_rho(state);
}

void decision_set_sanitization(SimState* state, float newLevel, float maxLevel) {
    if (newLevel < 0.0f) newLevel = 0.0f;
    if (newLevel > maxLevel) newLevel = maxLevel;
    state->handSanitization = newLevel;
    decision_recalc_rho(state);
}

void decision_set_masks(SimState* state, float newLevel) {
    if (newLevel < 0.0f) newLevel = 0.0f;
    if (newLevel > 10.0f) newLevel = 10.0f;
    state->maskLevel = newLevel;
    decision_recalc_rho(state);
}

void decision_set_lockdown(SimState* state, float percent) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    state->lockdownPercent = percent;

    /* Lockdown costs mental health and economy */
    state->mentalHealthDelta = -(percent / 100.0f) * 0.4f;
    state->economyDelta = 1 + (int)(percent / 20.0f);

    decision_recalc_rho(state);
}

void decision_set_density(SimState* state, float density) {
    if (density < 0.5f) density = 0.5f;
    if (density > 3.0f) density = 3.0f;
    state->populationDensity = density;
    decision_recalc_rho(state);
}

void decision_set_infection_rate(SimState* state, float rate, SimConfig* config) {
    if (rate < 0.5f) rate = 0.5f;
    if (rate > 5.0f) rate = 5.0f;
    state->infectionRate = rate;
    config->R0 = rate;
    state->beta = rate * state->gamma;
}
