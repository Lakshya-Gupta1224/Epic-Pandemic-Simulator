#include "simulation.h"
#include "seir.h"
#include "timer.h"
#include "economy.h"
#include "mental_health.h"
#include "decisions.h"
#include "end_conditions.h"
#include "debriefing.h"
#include "hospital.h"

void sim_init(GameWorld* world, SimConfig config) {
    int i;
    world->config = config;

    timer_init(&world->state);
    seir_init(&world->state, &world->config);
    economy_init(&world->state, &world->config);
    mental_health_init(&world->state, &world->config);
    debriefing_init(&world->stats);

    for (i = 0; i < MAX_GRADES; i++) world->state.schoolOpen[i] = 1;
    world->state.goingOutAllowed = 1;
    world->state.sportsAllowed   = 1;
    world->state.handSanitization = 0.0f;
    world->state.endCondition    = END_NONE;
    world->state.timeScale       = 1.0f;
}

int sim_update(GameWorld* world, float deltaTime) {
    if (world->state.endCondition != END_NONE) return 0;

    int flags = timer_update(&world->state, deltaTime);

    if (flags & 2) {  /* DAY_PASSED */
        seir_step(&world->state, &world->config);
        economy_update(&world->state);
        mental_health_update(&world->state, &world->config);
        hospital_update_persons(world);
        debriefing_gather(&world->stats, &world->state, &world->config);

        EndCondition ec = end_check(&world->state, &world->config);
        if (ec != END_NONE) {
            world->state.endCondition = ec;
        }

        return 1;
    }

    return 0;
}

void sim_reset(GameWorld* world) {
    SimConfig cfg = world->config;
    sim_init(world, cfg);
}

void sim_toggle_school(GameWorld* world, int grade, int open) {
    decision_toggle_school(&world->state, grade, open);
}

void sim_toggle_going_out(GameWorld* world, int allowed) {
    decision_toggle_going_out(&world->state, allowed);
}

void sim_toggle_sports(GameWorld* world, int allowed) {
    decision_toggle_sports(&world->state, allowed);
}

void sim_set_hand_sanitization(GameWorld* world, float level) {
    decision_set_sanitization(&world->state, level, 10.0f);
}

void sim_set_time_scale(GameWorld* world, float scale) {
    if (scale < 0.5f) scale = 0.5f;
    if (scale > 10.0f) scale = 10.0f;
    world->state.timeScale = scale;
}

void sim_skip_days(GameWorld* world, int days) {
    int d;
    for (d = 0; d < days; d++) {
        if (world->state.endCondition != END_NONE) break;
        seir_step(&world->state, &world->config);
        economy_update(&world->state);
        mental_health_update(&world->state, &world->config);
        hospital_update_persons(world);
        debriefing_gather(&world->stats, &world->state, &world->config);
        world->state.currentDay++;

        EndCondition ec = end_check(&world->state, &world->config);
        if (ec != END_NONE) {
            world->state.endCondition = ec;
            break;
        }
    }
}
