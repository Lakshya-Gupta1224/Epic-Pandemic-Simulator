#include "debriefing.h"
#include <string.h>

void debriefing_init(DebriefingStats* stats) {
    memset(stats, 0, sizeof(DebriefingStats));
    stats->pandemicDayEnd = -1;
}

void debriefing_gather(DebriefingStats* stats, const SimState* st, const SimConfig* cfg) {
    /* Track peaks */
    if ((float)st->exposed  > stats->maxExposed)  stats->maxExposed  = (float)st->exposed;
    if ((float)st->infected > stats->maxInfected)  stats->maxInfected = (float)st->infected;
    if ((float)st->dead     > stats->maxDead)      stats->maxDead     = (float)st->dead;

    /* Track pandemic natural end */
    if (stats->pandemicDayEnd < 0 && st->currentDay > 15) {
        float pop = cfg->population;
        if ((st->infected + st->exposed) < 0.01f * pop &&
            (st->recovered + st->susceptible) >= 0.95f * pop) {
            stats->pandemicDayEnd = st->currentDay;
        }
    }

    /* Track school closures */
    {
        int i;
        for (i = 0; i < MAX_GRADES; i++) {
            if (!st->schoolOpen[i]) stats->daysSchoolClosed[i]++;
        }
    }

    /* Track restrictions */
    if (!st->goingOutAllowed) stats->daysNoGoingOut++;
    if (!st->sportsAllowed)   stats->daysNoSports++;

    /* Per-day logs */
    {
        int idx = stats->historyCount;
        if (idx < MAX_HISTORY) {
            stats->mentalHealthHistory[idx]     = st->mentalHealth;
            stats->handSanitizationHistory[idx] = st->handSanitization;
            stats->historyCount++;
        }
    }

    /* Compute percentages */
    if (cfg->population > 0) {
        stats->sickPercent         = ((stats->maxExposed + stats->maxInfected) / cfg->population) * 100.0f;
        stats->hospitalizedPercent = (stats->maxInfected / cfg->population) * 100.0f;
        stats->deadPercent         = (stats->maxDead / cfg->population) * 100.0f;
    }
}
