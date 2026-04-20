#ifndef MODELS_H
#define MODELS_H

#include "constants.h"

/* ═══════════════════════════════════════════
   ENUMERATIONS
   ═══════════════════════════════════════════ */

typedef enum {
    STATE_SUSCEPTIBLE = 0,
    STATE_EXPOSED     = 1,
    STATE_INFECTED    = 2,
    STATE_RECOVERED   = 3,
    STATE_DEAD        = 4
} PersonState;

typedef enum {
    PERSON_CHILD = 0,
    PERSON_ADULT = 1
} PersonType;

typedef enum {
    PLACE_HOME     = 0,
    PLACE_HOSPITAL = 1,
    PLACE_SCHOOL   = 2,
    PLACE_WORK     = 3,
    PLACE_ASCENDED = 4
} PersonPlace;

typedef enum {
    BUILDING_HOUSE    = 0,
    BUILDING_HOSPITAL = 1,
    BUILDING_SCHOOL   = 2
} BuildingType;

typedef enum {
    APP_MENU    = 0,
    APP_RUNNING = 1,
    APP_PAUSED  = 2,
    APP_RESULT  = 3
} AppState;

typedef enum {
    END_NONE            = -1,
    END_PANDEMIC_OVER   =  0,
    END_BUDGET_DEPLETED =  1,
    END_MAX_DAYS        =  2,
    END_MENTAL_CRISIS   =  3
} EndCondition;

/* ═══════════════════════════════════════════
   DATA STRUCTURES
   ═══════════════════════════════════════════ */

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    float r, g, b, a;
} Color4f;

typedef struct {
    int          id;
    BuildingType type;
    Vec3         position;
    int          regionIndex;
    int          maxCapacity;
    int          currCapacity;
    int          residentIds[MAX_RESIDENTS];
    int          residentCount;
} Building;

typedef struct {
    int          id;
    PersonType   type;
    PersonState  state;
    PersonPlace  currentPlace;
    int          homeId;
    int          workOrSchoolId;
    int          hospitalId;
    int          regionIndex;
    int          age;
    int          grade;
    Vec3         position;
    Vec3         startPosition;
    Vec3         targetPosition;
    float        moveProgress;
    int          isMoving;
} Person;

typedef struct {
    int     id;
    char    name[32];
    Vec3    center;
    Vec3    size;
    int     houseIds[512];
    int     houseCount;
    int     hospitalId;
    int     schoolId;
} Region;

typedef struct {
    float  population;
    float  incubationPeriod;
    float  infectionPeriod;
    float  R0;
    float  initialExposed;
    int    maxDays;
    int    maxHospitalBeds;
    int    maxEconomy;
    float  maxMentalHealth;
} SimConfig;

typedef struct {
    /* SEIR compartments (proportions) */
    float S, E, I, R;

    /* Population counts */
    int   susceptible;
    int   exposed;
    int   infected;
    int   recovered;
    int   dead;

    /* History for graph */
    float S_history[MAX_HISTORY];
    float E_history[MAX_HISTORY];
    float I_history[MAX_HISTORY];
    float R_history[MAX_HISTORY];
    int   historyCount;

    /* SEIR parameters */
    float rho;
    float alpha, beta, gamma;

    /* Resources */
    int   economy;
    int   economyDelta;
    float mentalHealth;
    float mentalHealthDelta;
    float mentalHealthBase;

    /* Time */
    int   currentDay;
    int   currentHour;
    float dayTimer;
    float hourLength;

    /* User decisions */
    int   schoolOpen[MAX_GRADES];
    int   goingOutAllowed;
    int   sportsAllowed;
    float handSanitization;

    /* End state */
    EndCondition endCondition;

    /* Speed */
    float timeScale;
} SimState;

typedef struct {
    float rhoEffect;
    float mentalEffect;
    int   economyEffect;
} DecisionEffect;

typedef struct {
    float maxExposed;
    float maxInfected;
    float maxDead;
    float sickPercent;
    float hospitalizedPercent;
    float deadPercent;
    int   pandemicDayEnd;
    int   daysSchoolClosed[MAX_GRADES];
    int   daysNoGoingOut;
    int   daysNoSports;
    float mentalHealthHistory[MAX_HISTORY];
    float handSanitizationHistory[MAX_HISTORY];
    int   historyCount;
} DebriefingStats;

typedef struct {
    float distance;
    float azimuth;
    float elevation;
    Vec3  target;
    float panSpeed;
    float rotateSpeed;
    float zoomSpeed;
    float minDist, maxDist;
    float minElev, maxElev;
} CameraState;

typedef struct {
    char    message[64];
    float   timer;
    float   r, g, b;
    int     active;
} Toast;

typedef struct {
    Person      persons[MAX_PERSONS];
    int         personCount;
    Building    buildings[MAX_BUILDINGS];
    int         buildingCount;
    Region      regions[MAX_REGIONS];
    int         regionCount;
    SimConfig   config;
    SimState    state;
    DebriefingStats stats;
    CameraState camera;
    AppState    appState;
    Toast       toasts[MAX_TOASTS];
    float       gameTime;
    int         showHelp;
} GameWorld;

#endif /* MODELS_H */
