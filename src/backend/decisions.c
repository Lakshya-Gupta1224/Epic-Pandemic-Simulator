#include "decisions.h"

static const DecisionEffect SCHOOL_EFFECT    = { 0.1f,  0.0f, -1 };
static const DecisionEffect GOING_OUT_EFFECT = { 0.1f,  1.0f, -1 };
static const DecisionEffect SPORTS_EFFECT    = { 0.1f,  1.0f, -1 };

void decision_toggle_school(SimState* state, int grade, int open) {
    if (grade < 1 || grade > MAX_GRADES) return;
    int idx = grade - 1;

    if (open && !state->schoolOpen[idx]) {
        state->rho          += SCHOOL_EFFECT.rhoEffect;
        state->economyDelta += SCHOOL_EFFECT.economyEffect;
        state->schoolOpen[idx] = 1;
    } else if (!open && state->schoolOpen[idx]) {
        state->rho          -= SCHOOL_EFFECT.rhoEffect;
        state->economyDelta -= SCHOOL_EFFECT.economyEffect;
        state->schoolOpen[idx] = 0;
    }
}

void decision_toggle_going_out(SimState* state, int allowed) {
    if (allowed && !state->goingOutAllowed) {
        state->rho              += GOING_OUT_EFFECT.rhoEffect;
        state->mentalHealthDelta += GOING_OUT_EFFECT.mentalEffect;
        state->economyDelta     += GOING_OUT_EFFECT.economyEffect;
        state->goingOutAllowed = 1;
    } else if (!allowed && state->goingOutAllowed) {
        state->rho              -= GOING_OUT_EFFECT.rhoEffect;
        state->mentalHealthDelta -= GOING_OUT_EFFECT.mentalEffect;
        state->economyDelta     -= GOING_OUT_EFFECT.economyEffect;
        state->goingOutAllowed = 0;
    }
}

void decision_toggle_sports(SimState* state, int allowed) {
    if (allowed && !state->sportsAllowed) {
        state->rho              += SPORTS_EFFECT.rhoEffect;
        state->mentalHealthDelta += SPORTS_EFFECT.mentalEffect;
        state->economyDelta     += SPORTS_EFFECT.economyEffect;
        state->sportsAllowed = 1;
    } else if (!allowed && state->sportsAllowed) {
        state->rho              -= SPORTS_EFFECT.rhoEffect;
        state->mentalHealthDelta -= SPORTS_EFFECT.mentalEffect;
        state->economyDelta     -= SPORTS_EFFECT.economyEffect;
        state->sportsAllowed = 0;
    }
}

void decision_set_sanitization(SimState* state, float newLevel, float maxLevel) {
    float oldLevel = state->handSanitization;
    if (newLevel < 0.0f) newLevel = 0.0f;
    if (newLevel > maxLevel) newLevel = maxLevel;

    if (newLevel > oldLevel) {
        state->rho -= 0.1f / maxLevel;
        if (newLevel / maxLevel > 0.9f) state->economyDelta += 1;
        if (newLevel / maxLevel > 0.6f) state->mentalHealthDelta -= 0.5f;
    } else if (newLevel < oldLevel) {
        state->rho += 0.1f / maxLevel;
        if (oldLevel / maxLevel > 0.8f) state->economyDelta -= 1;
        if (oldLevel / maxLevel >= 0.6f) state->mentalHealthDelta += 0.5f;
    }

    state->handSanitization = newLevel;
}
