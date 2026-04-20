#include "world_gen.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* 5-region map layout (inspired by P2's Denmark regions) */
static const float regCenters[5][3] = {
    { -120.0f, 0.0f, -100.0f },   /* North */
    {   10.0f, 0.0f,  -30.0f },   /* Central */
    { -120.0f, 0.0f,   80.0f },   /* South */
    {   60.0f, 0.0f,   90.0f },   /* East */
    {  160.0f, 0.0f,  -10.0f },   /* Capital */
};
static const float regSizes[5][3] = {
    { 110.0f, 1.0f,  90.0f },
    { 120.0f, 1.0f, 100.0f },
    { 110.0f, 1.0f,  80.0f },
    { 100.0f, 1.0f,  90.0f },
    {  80.0f, 1.0f,  70.0f },
};
static const char* regNames[5] = {
    "North", "Central", "South", "East", "Capital"
};

static float randf(float lo, float hi) {
    return lo + (float)rand() / (float)RAND_MAX * (hi - lo);
}

void world_generate(GameWorld* world) {
    int bIdx = 0;
    int pIdx = 0;
    int r, h;

    world->regionCount = NUM_REGIONS;

    for (r = 0; r < NUM_REGIONS; r++) {
        Region* reg = &world->regions[r];
        float halfW, halfD;

        reg->id = r;
        strncpy(reg->name, regNames[r], 31);
        reg->name[31] = '\0';
        reg->center.x = regCenters[r][0];
        reg->center.y = regCenters[r][1];
        reg->center.z = regCenters[r][2];
        reg->size.x = regSizes[r][0];
        reg->size.y = regSizes[r][1];
        reg->size.z = regSizes[r][2];
        reg->houseCount = 0;

        halfW = reg->size.x * 0.5f;
        halfD = reg->size.z * 0.5f;

        /* Hospital (1 per region) */
        {
            Building* hosp = &world->buildings[bIdx];
            hosp->id = bIdx;
            hosp->type = BUILDING_HOSPITAL;
            hosp->regionIndex = r;
            hosp->position.x = reg->center.x + halfW * 0.55f;
            hosp->position.y = 0.0f;
            hosp->position.z = reg->center.z - halfD * 0.45f;
            hosp->maxCapacity = world->config.maxHospitalBeds;
            hosp->currCapacity = 0;
            hosp->residentCount = 0;
            reg->hospitalId = bIdx;
            bIdx++;
        }

        /* School (1 per region) */
        {
            Building* sch = &world->buildings[bIdx];
            sch->id = bIdx;
            sch->type = BUILDING_SCHOOL;
            sch->regionIndex = r;
            sch->position.x = reg->center.x - halfW * 0.45f;
            sch->position.y = 0.0f;
            sch->position.z = reg->center.z - halfD * 0.55f;
            sch->maxCapacity = 200;
            sch->currCapacity = 0;
            sch->residentCount = 0;
            reg->schoolId = bIdx;
            bIdx++;
        }

        /* Houses */
        for (h = 0; h < HOUSES_PER_REGION; h++) {
            Building* house;
            Person* child;
            Person* adult;

            house = &world->buildings[bIdx];
            house->id = bIdx;
            house->type = BUILDING_HOUSE;
            house->regionIndex = r;
            house->position.x = reg->center.x + randf(-halfW * 0.85f, halfW * 0.85f);
            house->position.y = 0.0f;
            house->position.z = reg->center.z + randf(-halfD * 0.85f, halfD * 0.85f);
            house->maxCapacity = PERSONS_PER_HOUSE;
            house->currCapacity = 0;
            house->residentCount = 0;

            reg->houseIds[reg->houseCount++] = bIdx;

            /* Child */
            child = &world->persons[pIdx];
            child->id = pIdx;
            child->type = PERSON_CHILD;
            child->state = STATE_SUSCEPTIBLE;
            child->currentPlace = PLACE_HOME;
            child->homeId = bIdx;
            child->workOrSchoolId = reg->schoolId;
            child->hospitalId = reg->hospitalId;
            child->regionIndex = r;
            child->age = 6 + rand() % 12;
            child->grade = 1 + rand() % 12;
            child->position = house->position;
            child->position.y = CHILD_RADIUS;
            child->position.x += randf(-0.8f, 0.8f);
            child->startPosition = child->position;
            child->targetPosition = child->position;
            child->moveProgress = 1.0f;
            child->isMoving = 0;
            house->residentIds[house->residentCount++] = pIdx;
            pIdx++;

            /* Adult */
            adult = &world->persons[pIdx];
            adult->id = pIdx;
            adult->type = PERSON_ADULT;
            adult->state = STATE_SUSCEPTIBLE;
            adult->currentPlace = PLACE_HOME;
            adult->homeId = bIdx;
            adult->workOrSchoolId = -1;
            adult->hospitalId = reg->hospitalId;
            adult->regionIndex = r;
            adult->age = 25 + rand() % 40;
            adult->grade = 0;
            adult->position = house->position;
            adult->position.y = ADULT_RADIUS;
            adult->position.x += randf(0.8f, 1.6f);
            adult->startPosition = adult->position;
            adult->targetPosition = adult->position;
            adult->moveProgress = 1.0f;
            adult->isMoving = 0;
            house->residentIds[house->residentCount++] = pIdx;
            pIdx++;

            bIdx++;
        }
    }

    world->buildingCount = bIdx;
    world->personCount   = pIdx;

    /* Mark initial exposed persons randomly */
    {
        int toExpose = (int)world->config.initialExposed;
        int i;
        for (i = 0; i < world->personCount && toExpose > 0; i++) {
            if ((rand() % world->personCount) < toExpose * 2) {
                world->persons[i].state = STATE_EXPOSED;
                toExpose--;
            }
        }
        /* If any remaining, force them */
        for (i = 0; i < world->personCount && toExpose > 0; i++) {
            if (world->persons[i].state == STATE_SUSCEPTIBLE) {
                world->persons[i].state = STATE_EXPOSED;
                toExpose--;
            }
        }
    }
}
