#ifndef DEBRIEFING_H
#define DEBRIEFING_H

#include "models.h"

void debriefing_init(DebriefingStats* stats);
void debriefing_gather(DebriefingStats* stats, const SimState* st, const SimConfig* cfg);

#endif
