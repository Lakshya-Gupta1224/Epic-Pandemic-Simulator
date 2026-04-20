#ifndef HOSPITAL_H
#define HOSPITAL_H

#include "models.h"

/* Sync individual person states with population-level SEIR counts */
void hospital_update_persons(GameWorld* world);

#endif
