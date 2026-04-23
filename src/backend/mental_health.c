#include "mental_health.h"

void mental_health_init(SimState* state, SimConfig* config) {
    state->mentalHealth      = config->maxMentalHealth;
    state->mentalHealthDelta = 0.0f;
    state->mentalHealthBase  = 0.0f;
}

/*
 * mental_health_update  --  called once per simulated DAY from seir_step.
 *
 * Calibrated targets (maxMH=100, maxBeds=150, pop=1080):
 *
 *   At 70% bed occupancy (105/150 beds, ~10% pop infected):
 *     hosp_penalty ≈ 0.49 * 3.5 = 1.71 / day
 *     infection_burden ≈ 0.10 * 2.5 = 0.25 / day
 *     → Total drain ≈ -2.0 / day → reaches 50 in ~25 days.
 *
 *   At 100% bed occupancy (all 150 beds, ~14% infected):
 *     hosp_penalty = 1.0 * 3.5 = 3.5 / day
 *     → Total drain ≈ -3.85 / day → reaches 50 in ~13 days.
 *
 *   Recovery boost: every 1% of pop newly recovered gives +0.5 MH (from seir.c).
 *
 *   Natural recovery: when infection < 5%, deficit drifts back at 1.5%/day.
 *
 *   Decision delta: lockdowns push MH down, going-out / sports push it up.
 *
 * Components:
 *   1. mentalHealthBase  — net of (death shock - recovery boost) from seir.c
 *   2. hosp_penalty      — quadratic on bed occupancy
 *   3. infection_burden  — linear on active infected fraction
 *   4. natural_recovery  — slow heal when infection low
 *   5. decision_delta    — from decisions.c (persists until changed)
 */
void mental_health_update(SimState* state, SimConfig* config) {
    float pop   = config->population;
    float maxMH = config->maxMentalHealth;
    if (pop <= 0.0f || maxMH <= 0.0f) return;

    /* 1. Net shock/boost from seir.c (deaths negative, recoveries positive) */
    float base_signal = state->mentalHealthBase;   /* reset below */

    /* 2. Hospital pressure (quadratic: 0 at 0%, -3.5 at 100%) */
    int beds_used = (state->infected < config->maxHospitalBeds)
                  ? state->infected : config->maxHospitalBeds;
    float hosp_occ     = (config->maxHospitalBeds > 0)
                       ? (float)beds_used / (float)config->maxHospitalBeds : 0.0f;
    float hosp_penalty = hosp_occ * hosp_occ * 3.5f;

    /* Extra penalty when fully overwhelmed (10+ days) — societal panic */
    if (state->hospitalOverwhelmedDays >= 10)
        hosp_penalty += 1.5f;

    /* 3. Infection burden (linear) */
    float infected_frac      = (float)state->infected / pop;
    float infection_penalty  = infected_frac * 2.5f;

    /* 4. Natural recovery when pandemic is under control */
    float natural_recovery = 0.0f;
    if (infected_frac < 0.05f && state->currentDay > 5) {
        float deficit = maxMH - state->mentalHealth;
        if (deficit > 0.0f)
            natural_recovery = deficit * 0.015f;
    }

    /* 5. Decision delta (persistent, set by decisions.c) */
    float decision_delta = state->mentalHealthDelta;

    /* Apply all components */
    state->mentalHealth += base_signal
                         - hosp_penalty
                         - infection_penalty
                         + natural_recovery
                         + decision_delta;

    /* Reset daily accumulator */
    state->mentalHealthBase = 0.0f;

    /* Clamp */
    if (state->mentalHealth < 0.0f)  state->mentalHealth = 0.0f;
    if (state->mentalHealth > maxMH) state->mentalHealth = maxMH;
}
