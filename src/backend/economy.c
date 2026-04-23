#include "economy.h"

void economy_init(SimState* state, SimConfig* config) {
    state->economy      = config->maxEconomy;
    state->economyDelta = 1;   /* base daily drain */
}

/*
 * economy_update  --  called once per simulated DAY.
 *
 * Budget drain formula:
 *
 *   base         = 1        (infrastructure / overhead)
 *   lockdown     = lockdownPercent/100 * 8    (max +8 at 100% lockdown)
 *   masks        = maskLevel/10 * 2           (max +2 at level 10)
 *   sanitize     = handSanitization/10 * 1    (max +1 at level 10)
 *   no_goingout  = +1 if going-out restricted  (lost commerce)
 *   infection    = infected / maxBeds * 3     (hospital costs peak at bed cap)
 *
 *   Total daily drain = base + lockdown + masks + sanitize + no_goingout + infection
 *
 * Rationale:
 *   - At 100% lockdown + max masks/sanitize: drain ≈ 12/day → 50000 lasts ~4167 days
 *     (way too slow; the real threat is lockdown + healthcare costs combined)
 *   - But with 150 infected at max beds + 100% lockdown: drain ≈ 15/day
 *     → budget gone in ~3333 days  (still very lenient)
 *   NOTE: maxEconomy=50000 is very large; economyDelta is stored back so the
 *   HUD can display the per-day cost.  The economy_update subtracts it once per day.
 */
void economy_update(SimState* state) {
    float lockdown_cost  = (state->lockdownPercent / 100.0f) * 8.0f;
    float masks_cost     = (state->maskLevel / 10.0f) * 2.0f;
    float sanitize_cost  = (state->handSanitization / 10.0f) * 1.0f;
    float commerce_loss  = (!state->goingOutAllowed) ? 1.0f : 0.0f;
    /* Hospital costs scale with infection pressure */
    float infect_cost    = (state->infected > 0 && state->infected <= 150)
                         ? ((float)state->infected / 150.0f) * 3.0f
                         : (state->infected > 150 ? 3.0f : 0.0f);

    float total = 1.0f
                + lockdown_cost
                + masks_cost
                + sanitize_cost
                + commerce_loss
                + infect_cost;

    state->economyDelta = (int)(total + 0.5f);   /* round to nearest int */
    if (state->economyDelta < 1) state->economyDelta = 1;

    state->economy -= state->economyDelta;
    if (state->economy < 0) state->economy = 0;
}
