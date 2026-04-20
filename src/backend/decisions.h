#ifndef DECISIONS_H
#define DECISIONS_H

#include "models.h"

void decision_toggle_school(SimState* state, int grade, int open);
void decision_toggle_going_out(SimState* state, int allowed);
void decision_toggle_sports(SimState* state, int allowed);
void decision_set_sanitization(SimState* state, float newLevel, float maxLevel);
void decision_set_masks(SimState* state, float newLevel);
void decision_set_lockdown(SimState* state, float percent);
void decision_set_density(SimState* state, float density);
void decision_set_infection_rate(SimState* state, float rate, SimConfig* config);
void decision_recalc_rho(SimState* state);

#endif
