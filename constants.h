#ifndef CONSTANTS_H
#define CONSTANTS_H

/* ─── World Limits ─── */
#define MAX_PERSONS     8192
#define MAX_BUILDINGS   2048
#define MAX_REGIONS     12
#define MAX_HISTORY     256
#define MAX_GRADES      12
#define MAX_RESIDENTS   16
#define MAX_COUNTRIES   12
#define MAX_CONTINENTS  6
#define MAX_COUNTRY_VERTS 24
#define MAX_CITIES      30
#define MAX_ROUTES      20
#define MAX_TOASTS      5
#define TOAST_DURATION  3.0f

/* ─── Globe Rendering ─── */
#define GLOBE_RADIUS    100.0f
#define GLOBE_SLICES    64
#define GLOBE_STACKS    48
#define GLOBE_ATM_SCALE 1.06f

/* ─── Default Simulation Config ─── */
#define DEFAULT_POPULATION        3000
#define DEFAULT_INCUBATION        5.0f
#define DEFAULT_INFECTION_PERIOD  10.0f
#define DEFAULT_R0                2.5f
#define DEFAULT_INITIAL_EXPOSED   30
#define DEFAULT_MAX_DAYS          365
#define DEFAULT_HOSPITAL_BEDS     800
#define DEFAULT_MAX_ECONOMY       50000
#define DEFAULT_MAX_MENTAL_HEALTH 100.0f

/* ─── Region Layout ─── */
#define NUM_REGIONS             9
#define HOUSES_PER_REGION       60
#define PERSONS_PER_HOUSE       2       /* 1 child + 1 adult */

/* ─── Window ─── */
#define WINDOW_W  1280
#define WINDOW_H  720
#define WINDOW_TITLE "Epidemic Choices - Pandemic Simulation"

/* ─── Camera Defaults ─── */
#define CAM_DEFAULT_DIST      350.0f
#define CAM_DEFAULT_AZIMUTH   45.0f
#define CAM_DEFAULT_ELEVATION 55.0f
#define CAM_MIN_DIST          80.0f
#define CAM_MAX_DIST          1500.0f
#define CAM_MIN_ELEV          10.0f
#define CAM_MAX_ELEV          85.0f

/* ─── Rendering ─── */
#define CHILD_RADIUS   0.4f
#define ADULT_RADIUS   0.6f

/* ─── Math ─── */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define DEG_TO_RAD (M_PI / 180.0)

#endif /* CONSTANTS_H */
