#include "timer.h"

void timer_init(SimState* state) {
    state->currentDay  = 1;
    state->currentHour = 1;
    state->dayTimer    = 0.0f;
    state->hourLength  = 0.5f;   /* 0.5 real seconds per sim-hour */
    state->timeScale   = 1.0f;
}

int timer_update(SimState* state, float deltaTime) {
    if (state->endCondition != END_NONE) return 0;

    int flags = 0;
    float scaledDt = deltaTime * state->timeScale;
    state->dayTimer += scaledDt;

    float dayLength = state->hourLength * 24.0f;

    /* Hour boundary */
    float hourTarget = state->hourLength * (float)state->currentHour;
    if (state->dayTimer >= hourTarget) {
        state->currentHour++;
        flags |= 1;
    }

    /* Day boundary */
    if (state->dayTimer >= dayLength) {
        state->dayTimer -= dayLength;
        state->currentDay++;
        state->currentHour = 1;
        flags |= 2;
    }

    return flags;
}
