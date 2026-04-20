#ifndef SIMULATION_H
#define SIMULATION_H

#include "models.h"

void sim_init(GameWorld* world, SimConfig config);
int  sim_update(GameWorld* world, float deltaTime);
void sim_reset(GameWorld* world);

void sim_toggle_school(GameWorld* world, int grade, int open);
void sim_toggle_going_out(GameWorld* world, int allowed);
void sim_toggle_sports(GameWorld* world, int allowed);
void sim_set_hand_sanitization(GameWorld* world, float level);
void sim_set_time_scale(GameWorld* world, float scale);
void sim_skip_days(GameWorld* world, int days);

#endif
