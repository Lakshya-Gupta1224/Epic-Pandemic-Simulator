#include "world_gen.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

static const char* regNames[9] = {
    "North-West", "North", "North-East",
    "West", "Central", "East",
    "South-West", "South", "South-East"
};

static float randf(float lo, float hi) {
    float r = (float)rand() / (float)RAND_MAX;
    return lo + r * (hi - lo);
}

void world_generate(GameWorld* world) {
    int bIdx = 0;
    int pIdx = 0;
    int r, h;

    world->regionCount = NUM_REGIONS;

    for (r = 0; r < NUM_REGIONS; r++) {
        Region* reg = &world->regions[r];
        int gx = r % 3;
        int gz = r / 3;
        float halfW, halfD;

        reg->id = r;
        strncpy(reg->name, regNames[r], 31);
        reg->name[31] = '\0';
        reg->center.x = (gx - 1) * 220.0f;
        reg->center.y = 0.0f;
        reg->center.z = (gz - 1) * 220.0f;
        reg->size.x = 140.0f + randf(-20.0f, 20.0f);
        reg->size.y = 1.0f;
        reg->size.z = 140.0f + randf(-20.0f, 20.0f);
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

    /* Map configuration to actual simulation entities created */
    world->config.population = (float)world->personCount;

    /* Initialize Vehicles for road travel between regions */
    world->vehicleCount = 40;  
    for (int v = 0; v < world->vehicleCount; v++) {
        Vehicle* veh = &world->vehicles[v];
        int rStart = rand() % NUM_REGIONS;
        int rEnd = rand() % NUM_REGIONS;
        while (rEnd == rStart && NUM_REGIONS > 1) rEnd = rand() % NUM_REGIONS;

        veh->startPos = world->regions[rStart].center;
        veh->targetPos = world->regions[rEnd].center;
        
        veh->progress = (rand() % 100) / 100.0f;
        veh->position.x = veh->startPos.x + (veh->targetPos.x - veh->startPos.x) * veh->progress;
        veh->position.y = 0.5f; 
        veh->position.z = veh->startPos.z + (veh->targetPos.z - veh->startPos.z) * veh->progress;
        
        veh->speed = 50.0f + (rand() % 30); 
        
        veh->r = 0.2f + (rand() % 80) / 100.0f;
        veh->g = 0.2f + (rand() % 80) / 100.0f;
        veh->b = 0.2f + (rand() % 80) / 100.0f;
    }
}
