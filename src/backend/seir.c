#include "seir.h"
#include "mental_health.h"
#include <math.h>

void seir_init(SimState* state, SimConfig* config) {
    state->alpha = 1.0f / config->incubationPeriod;
    state->gamma = 1.0f / config->infectionPeriod;
    state->beta  = config->R0 * state->gamma;
    state->rho   = 1.0f;

    state->I = 0.0f;
    state->E = config->initialExposed / config->population;
    state->S = 1.0f - state->E;
    state->R = 0.0f;
    state->D = 0.0f;

    state->susceptible = (int)(state->S * config->population);
    state->exposed     = (int)config->initialExposed;
    state->infected    = 0;
    state->recovered   = 0;
    state->dead        = 0;

    state->S_history[0] = (float)state->susceptible;
    state->E_history[0] = (float)state->exposed;
    state->I_history[0] = (float)state->infected;
    state->R_history[0] = (float)state->recovered;
    state->historyCount = 1;

    state->mentalHealthBase       = 0.0f;
    state->hospitalOverwhelmedDays = 0;
}

/*
 * seir_step — called once per simulated DAY.
 *
 * Infection spread: force = rho * beta * S * I
 *   rho encodes all control decisions (school, going-out, masks, lockdown…).
 *   rho > 1 means super-spreading (high density/infectionRate slider).
 *
 * Hospital capacity:
 *   Infected with a bed  → base mortality 5%
 *   Overflow (no bed)    → mortality spikes to 40%
 *   If ALL beds are full (hospitalOverwhelmedDays ≥ 10) → spike to 60%
 *
 * Mental health accumulator (mentalHealthBase):
 *   Negative: death shock      (-2 × maxMH per 1% of pop dead in one day)
 *   Positive: recovery boost   (+0.5 × maxMH per 1% of pop newly recovered)
 *   mental_health_update() is called at the end of each day to consume it.
 */
void seir_step(SimState* state, SimConfig* config) {
    if (state->currentDay >= config->maxDays) return;

    /* Clamp rho */
    if (state->rho < 0.0f) state->rho = 0.0f;
    if (state->rho > 2.0f) state->rho = 2.0f;

    float dt  = 1.0f;
    float pop = config->population;

    float prev_S = state->S;
    float prev_E = state->E;
    float prev_I = state->I;
    float prev_R = state->R;
    int   prevDead      = state->dead;
    int   prevRecovered = state->recovered;

    /* Hospital capacity (as fraction of original population) */
    float beds_frac   = (float)config->maxHospitalBeds / pop;
    float I_with_bed  = (prev_I < beds_frac) ? prev_I : beds_frac;
    float I_no_bed    = (prev_I > beds_frac) ? prev_I - beds_frac : 0.0f;

    /* Mortality rates — spike if hospital has been overwhelmed ≥10 days */
    int overwhelmed = (state->infected >= config->maxHospitalBeds);
    if (overwhelmed) {
        state->hospitalOverwhelmedDays++;
    } else {
        state->hospitalOverwhelmedDays = 0;
    }

    float baseMort   = 0.05f;
    float noBedMort  = (state->hospitalOverwhelmedDays >= 10) ? 0.60f : 0.40f;

    /* Infection force */
    float infection_force = state->rho * state->beta * prev_S * prev_I;

    /* Daily deaths and recoveries (as proportions) */
    float deaths_prop   = (baseMort  * state->gamma * I_with_bed
                         + noBedMort * state->gamma * I_no_bed) * dt;
    float recovers_prop = ((1.0f - baseMort)  * state->gamma * I_with_bed
                         + (1.0f - noBedMort) * state->gamma * I_no_bed) * dt;

    /* Euler integration */
    float next_S = prev_S - infection_force * dt;
    float next_E = prev_E + (infection_force - state->alpha * prev_E) * dt;
    float next_I = prev_I + (state->alpha * prev_E - deaths_prop - recovers_prop) * dt;
    float next_R = prev_R + recovers_prop;
    float next_D = state->D + deaths_prop;

    /* Clamp */
    if (next_S < 0.0f) next_S = 0.0f;
    if (next_E < 0.0f) next_E = 0.0f;
    if (next_I < 0.0f) next_I = 0.0f;
    if (next_R < 0.0f) next_R = 0.0f;
    if (next_D > 1.0f) next_D = 1.0f;

    state->S = next_S;
    state->E = next_E;
    state->I = next_I;
    state->R = next_R;
    state->D = next_D;

    /* Integer population counts (dead derived, no drift) */
    state->susceptible = (int)roundf(next_S * pop);
    state->exposed     = (int)roundf(next_E * pop);
    state->infected    = (int)roundf(next_I * pop);
    state->recovered   = (int)roundf(next_R * pop);

    int alive  = state->susceptible + state->exposed
               + state->infected    + state->recovered;
    state->dead = (int)pop - alive;
    if (state->dead < 0) state->dead = 0;

    /* ── Mental health accumulator ─────────────────────────────────────
     * Death shock: each new death as a fraction of pop gives -2×maxMH.
     * Recovery boost: each new recovery as a fraction of pop gives +0.5×maxMH.
     * Both accumulate in mentalHealthBase; mental_health_update consumes it.
     */
    int newDeaths     = state->dead      - prevDead;
    int newRecovered  = state->recovered - prevRecovered;

    if (newDeaths > 0) {
        float deathFrac = (float)newDeaths / pop;
        state->mentalHealthBase -= deathFrac * 2.0f * config->maxMentalHealth;
    }
    if (newRecovered > 0) {
        float recFrac = (float)newRecovered / pop;
        /* Recovery boost: +0.5×maxMH per 1% of pop newly recovered.
           Scaled so that 10 recoveries out of 1080 (≈0.9%) gives +0.45 */
        state->mentalHealthBase += recFrac * 0.5f * config->maxMentalHealth;
    }

    /* ── Update mental health once per day ── */
    mental_health_update(state, config);

    /* ── Record history ── */
    int idx = state->historyCount;
    if (idx < MAX_HISTORY) {
        state->S_history[idx] = (float)state->susceptible;
        state->E_history[idx] = (float)state->exposed;
        state->I_history[idx] = (float)state->infected;
        state->R_history[idx] = (float)state->recovered;
        state->historyCount++;
    }
}
