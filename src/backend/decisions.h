#ifndef DECISIONS_H
#define DECISIONS_H

#include "models.h"

void decision_toggle_school(SimState* state, int grade, int open);
void decision_toggle_going_out(SimState* state, int allowed);
void decision_toggle_sports(SimState* state, int allowed);
void decision_set_sanitization(SimState* state, float newLevel, float maxLevel);

#endif
