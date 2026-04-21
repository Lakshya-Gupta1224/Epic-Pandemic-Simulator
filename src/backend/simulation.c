#include "simulation.h"
#include "seir.h"
#include "timer.h"
#include "economy.h"
#include "mental_health.h"
#include "decisions.h"
#include "end_conditions.h"
#include "debriefing.h"
#include "hospital.h"
#include <stdlib.h>
#include <math.h>

/* Schedule random movement of people between buildings */
static void schedule_movements(GameWorld* world) {
    int i;
    SimState* s = &world->state;

    for (i = 0; i < world->personCount; i++) {
        Person* p = &world->persons[i];

        /* Don't move dead or already moving persons */
        if (p->state == STATE_DEAD || p->isMoving) continue;

        /* Infected persons should be at hospital */
        if (p->state == STATE_INFECTED && p->currentPlace != PLACE_HOSPITAL) {
            Building* hosp = &world->buildings[p->hospitalId];
            p->startPosition = p->position;
            p->targetPosition = hosp->position;
            p->targetPosition.y = (p->type == PERSON_CHILD) ? 0.6f : 0.9f;
            p->targetPosition.x += ((float)(rand() % 40) / 10.0f) - 2.0f;
            p->targetPosition.z += ((float)(rand() % 40) / 10.0f) - 2.0f;
            p->moveProgress = 0.0f;
            p->isMoving = 1;
            p->currentPlace = PLACE_HOSPITAL;
            continue;
        }

        /* Going out restriction: most non-infected people stay home */
        if (!s->goingOutAllowed && p->state != STATE_INFECTED) {
            /* 85% of people obey the restriction */
            if ((rand() % 100) < 85) {
                if (p->currentPlace != PLACE_HOME) {
                    Building* home = &world->buildings[p->homeId];
                    p->startPosition = p->position;
                    p->targetPosition = home->position;
                    p->targetPosition.y = (p->type == PERSON_CHILD) ? 0.6f : 0.9f;
                    p->targetPosition.x += ((float)(rand() % 20) / 10.0f) - 1.0f;
                    p->targetPosition.z += ((float)(rand() % 20) / 10.0f) - 1.0f;
                    p->moveProgress = 0.0f;
                    p->isMoving = 1;
                    p->currentPlace = PLACE_HOME;
                }
                continue;
            }
        }

        /* Lockdown: percentage of people stay home */
        if (s->lockdownPercent > 0 && (rand() % 100) < (int)s->lockdownPercent) {
            /* Stay home or go home if away */
            if (p->currentPlace != PLACE_HOME) {
                Building* home = &world->buildings[p->homeId];
                p->startPosition = p->position;
                p->targetPosition = home->position;
                p->targetPosition.y = (p->type == PERSON_CHILD) ? 0.6f : 0.9f;
                p->targetPosition.x += ((float)(rand() % 20) / 10.0f) - 1.0f;
                p->targetPosition.z += ((float)(rand() % 20) / 10.0f) - 1.0f;
                p->moveProgress = 0.0f;
                p->isMoving = 1;
                p->currentPlace = PLACE_HOME;
            }
            continue;
        }

        /* Random chance to move (30% per cycle) */
        if ((rand() % 100) > 30) continue;

        /* Children go to school (if open, during school hours 8-15) */
        if (p->type == PERSON_CHILD && p->grade > 0 && p->grade <= MAX_GRADES) {
            if (s->schoolOpen[p->grade - 1] && s->currentHour >= 8 && s->currentHour <= 15) {
                if (p->currentPlace != PLACE_SCHOOL) {
                    Building* sch = &world->buildings[p->workOrSchoolId];
                    p->startPosition = p->position;
                    p->targetPosition = sch->position;
                    p->targetPosition.y = 0.6f;
                    p->targetPosition.x += ((float)(rand() % 60) / 10.0f) - 3.0f;
                    p->targetPosition.z += ((float)(rand() % 60) / 10.0f) - 3.0f;
                    p->moveProgress = 0.0f;
                    p->isMoving = 1;
                    p->currentPlace = PLACE_SCHOOL;
                }
            } else if (p->currentPlace == PLACE_SCHOOL) {
                /* School closed or after hours: go home */
                Building* home = &world->buildings[p->homeId];
                p->startPosition = p->position;
                p->targetPosition = home->position;
                p->targetPosition.y = 0.6f;
                p->targetPosition.x += ((float)(rand() % 20) / 10.0f) - 1.0f;
                p->targetPosition.z += ((float)(rand() % 20) / 10.0f) - 1.0f;
                p->moveProgress = 0.0f;
                p->isMoving = 1;
                p->currentPlace = PLACE_HOME;
            }
        }

        /* Adults wander if going out is allowed */
        if (p->type == PERSON_ADULT && s->goingOutAllowed && p->state != STATE_INFECTED) {
            if ((rand() % 100) < 20) {
                /* Visit a random house in same region */
                Region* reg = &world->regions[p->regionIndex];
                if (reg->houseCount > 0) {
                    int rh = rand() % reg->houseCount;
                    Building* dest = &world->buildings[reg->houseIds[rh]];
                    p->startPosition = p->position;
                    p->targetPosition = dest->position;
                    p->targetPosition.y = 0.9f;
                    p->targetPosition.x += ((float)(rand() % 20) / 10.0f) - 1.0f;
                    p->targetPosition.z += ((float)(rand() % 20) / 10.0f) - 1.0f;
                    p->moveProgress = 0.0f;
                    p->isMoving = 1;
                    p->currentPlace = PLACE_WORK;
                }
            }
        }
    }
}

void sim_init(GameWorld* world, SimConfig config) {
    int i;
    world->config = config;

    timer_init(&world->state);
    seir_init(&world->state, &world->config);
    economy_init(&world->state, &world->config);
    mental_health_init(&world->state, &world->config);
    debriefing_init(&world->stats);

    for (i = 0; i < MAX_GRADES; i++) world->state.schoolOpen[i] = 1;
    world->state.goingOutAllowed    = 1;
    world->state.sportsAllowed      = 1;
    world->state.handSanitization   = 0.0f;
    world->state.maskLevel          = 0.0f;
    world->state.lockdownPercent    = 0.0f;
    world->state.populationDensity  = 1.0f;
    world->state.infectionRate      = config.R0;
    world->state.movementTimer      = 0.0f;
    world->state.endCondition       = END_NONE;
    world->state.timeScale          = 1.0f;
}

static void update_vehicles(GameWorld* world, float dt) {
    for (int v = 0; v < world->vehicleCount; v++) {
        Vehicle* veh = &world->vehicles[v];
        float dx = veh->targetPos.x - veh->startPos.x;
        float dz = veh->targetPos.z - veh->startPos.z;
        float dist = sqrtf(dx*dx + dz*dz) * 1.414f; 
        if (dist < 1.0f) dist = 1.0f;

        veh->progress += (veh->speed * dt * world->state.timeScale) / dist;

        if (veh->progress >= 1.0f) {
            veh->startPos = veh->targetPos;
            int rNext = rand() % world->regionCount;
            veh->targetPos = world->regions[rNext].center;
            veh->progress = 0.0f;
        }

        float t = veh->progress;
        int xFirst = (v % 2 == 0);
        float midX = xFirst ? veh->targetPos.x : veh->startPos.x;
        float midZ = xFirst ? veh->startPos.z : veh->targetPos.z;

        if (t <= 0.5f) {
            float pt = t * 2.0f;
            veh->position.x = veh->startPos.x + (midX - veh->startPos.x) * pt;
            veh->position.z = veh->startPos.z + (midZ - veh->startPos.z) * pt;
        } else {
            float pt = (t - 0.5f) * 2.0f;
            veh->position.x = midX + (veh->targetPos.x - midX) * pt;
            veh->position.z = midZ + (veh->targetPos.z - midZ) * pt;
        }
    }
}

int sim_update(GameWorld* world, float deltaTime) {
    if (world->state.endCondition != END_NONE) return 0;

    int flags = timer_update(&world->state, deltaTime);

    /* Schedule movements every 2 seconds of game time */
    world->state.movementTimer += deltaTime * world->state.timeScale;
    if (world->state.movementTimer >= 2.0f) {
        world->state.movementTimer -= 2.0f;
        schedule_movements(world);
    }
    
    update_vehicles(world, deltaTime);

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

void sim_set_masks(GameWorld* world, float level) {
    decision_set_masks(&world->state, level);
}

void sim_set_lockdown(GameWorld* world, float percent) {
    decision_set_lockdown(&world->state, percent);
}

void sim_set_density(GameWorld* world, float density) {
    decision_set_density(&world->state, density);
}

void sim_set_infection_rate(GameWorld* world, float rate) {
    decision_set_infection_rate(&world->state, rate, &world->config);
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
