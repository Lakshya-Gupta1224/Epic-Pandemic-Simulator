#include "seir.h"
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
}

void seir_step(SimState* state, SimConfig* config) {
    if (state->currentDay >= config->maxDays) return;

    /* Clamp rho */
    if (state->rho < 0.0f) state->rho = 0.0f;
    if (state->rho > 1.0f) state->rho = 1.0f;

    float dt = 1.0f;

    float prev_S = state->S;
    float prev_E = state->E;
    float prev_I = state->I;
    float prev_R = state->R;

    /* SEIR Differential Equations (Euler method) */
    float infection_force = state->rho * state->beta * prev_S * prev_I;

    float next_S = prev_S - infection_force * dt;
    float next_E = prev_E + (infection_force - state->alpha * prev_E) * dt;
    float next_I = prev_I + (state->alpha * prev_E - state->gamma * prev_I) * dt;
    float next_R = prev_R + (state->gamma * prev_I) * dt;

    /* Clamp to valid range */
    if (next_S < 0.0f) next_S = 0.0f;
    if (next_E < 0.0f) next_E = 0.0f;
    if (next_I < 0.0f) next_I = 0.0f;
    if (next_R < 0.0f) next_R = 0.0f;

    state->S = next_S;
    state->E = next_E;
    state->I = next_I;
    state->R = next_R;

    /* Population counts */
    int newInfected = (int)roundf(next_I * config->population);
    int prevDead = state->dead;

    state->susceptible = (int)roundf(next_S * config->population);
    state->exposed     = (int)roundf(next_E * config->population);
    state->recovered   = (int)roundf(next_R * config->population);

    /* Hospital overflow deaths */
    if (newInfected > config->maxHospitalBeds) {
        int overflow = newInfected - config->maxHospitalBeds;
        state->dead += overflow;
        state->infected = config->maxHospitalBeds;
    } else {
        state->infected = newInfected;
    }

    /* Apply death-based mental health degradation */
    int newDeaths = state->dead - prevDead;
    if (newDeaths > 0) {
        state->mentalHealthBase -= (float)newDeaths;
    }

    /* Record history */
    int idx = state->historyCount;
    if (idx < MAX_HISTORY) {
        state->S_history[idx] = (float)state->susceptible;
        state->E_history[idx] = (float)state->exposed;
        state->I_history[idx] = (float)state->infected;
        state->R_history[idx] = (float)state->recovered;
        state->historyCount++;
    }
}
