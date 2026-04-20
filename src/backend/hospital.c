#include "hospital.h"
#include <stdlib.h>

void hospital_update_persons(GameWorld* world) {
    SimState* s = &world->state;

    /* Count current person-level states */
    int countS = 0, countE = 0, countI = 0, countR = 0, countD = 0;
    int i;
    for (i = 0; i < world->personCount; i++) {
        switch (world->persons[i].state) {
            case STATE_SUSCEPTIBLE: countS++; break;
            case STATE_EXPOSED:     countE++; break;
            case STATE_INFECTED:    countI++; break;
            case STATE_RECOVERED:   countR++; break;
            case STATE_DEAD:        countD++; break;
        }
    }

    /* S -> E transitions */
    int needE = s->exposed - countE;
    if (needE > 0) {
        for (i = 0; i < world->personCount && needE > 0; i++) {
            if (world->persons[i].state == STATE_SUSCEPTIBLE) {
                if ((rand() % (countS > 0 ? countS : 1)) < needE) {
                    world->persons[i].state = STATE_EXPOSED;
                    needE--;
                    countS--;
                }
            }
        }
    }

    /* E -> I transitions */
    int needI = s->infected - countI;
    if (needI > 0) {
        for (i = 0; i < world->personCount && needI > 0; i++) {
            if (world->persons[i].state == STATE_EXPOSED) {
                world->persons[i].state = STATE_INFECTED;
                /* Move to hospital */
                world->persons[i].currentPlace = PLACE_HOSPITAL;
                Building* hosp = &world->buildings[world->persons[i].hospitalId];
                world->persons[i].startPosition = world->persons[i].position;
                world->persons[i].targetPosition = hosp->position;
                world->persons[i].targetPosition.y = (world->persons[i].type == PERSON_CHILD)
                    ? CHILD_RADIUS : ADULT_RADIUS;
                world->persons[i].moveProgress = 0.0f;
                world->persons[i].isMoving = 1;
                needI--;
            }
        }
    }

    /* I -> R transitions */
    int needR = s->recovered - countR;
    if (needR > 0) {
        for (i = 0; i < world->personCount && needR > 0; i++) {
            if (world->persons[i].state == STATE_INFECTED) {
                world->persons[i].state = STATE_RECOVERED;
                /* Move back home */
                world->persons[i].currentPlace = PLACE_HOME;
                Building* home = &world->buildings[world->persons[i].homeId];
                world->persons[i].startPosition = world->persons[i].position;
                world->persons[i].targetPosition = home->position;
                world->persons[i].targetPosition.y = (world->persons[i].type == PERSON_CHILD)
                    ? CHILD_RADIUS : ADULT_RADIUS;
                world->persons[i].moveProgress = 0.0f;
                world->persons[i].isMoving = 1;
                needR--;
            }
        }
    }

    /* Overflow -> Dead transitions */
    int needD = s->dead - countD;
    if (needD > 0) {
        for (i = 0; i < world->personCount && needD > 0; i++) {
            if (world->persons[i].state == STATE_INFECTED) {
                world->persons[i].state = STATE_DEAD;
                world->persons[i].currentPlace = PLACE_ASCENDED;
                /* Death animation: float upward */
                world->persons[i].startPosition = world->persons[i].position;
                world->persons[i].targetPosition = world->persons[i].position;
                world->persons[i].targetPosition.y = 40.0f;
                world->persons[i].moveProgress = 0.0f;
                world->persons[i].isMoving = 1;
                needD--;
            }
        }
    }
}
