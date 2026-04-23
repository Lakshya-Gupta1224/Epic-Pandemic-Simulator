#include "world_map.h"
#include "hud.h"
#include "texture_loader.h"
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ═══════════════════════════════════════════
   COORDINATE HELPERS
   ═══════════════════════════════════════════ */

/*
 * Geographic (lat, lon) -> world-space position.
 *
 * The gluSphere is rendered with glRotatef(90,X) then glRotatef(180,Z)
 * plus a texture-matrix glScalef(-1,1,1).  Tracing those transforms:
 *
 *   x_world =  sin(lon_rad) * cos(lat_rad)
 *   y_world =  sin(lat_rad)               <- latitude lives on Y; north = +Y
 *   z_world = -cos(lon_rad) * cos(lat_rad)
 *
 * Camera is Y-up; eye orbits in the XZ equatorial plane.
 * Inverse (hit-test):
 *   lat = asin(hy / R)
 *   lon = atan2(hx, -hz)
 */
static Vec3 latlon_to_sphere(float lat, float lon, float radius) {
    float la = (float)(lat * M_PI / 180.0);
    float lo = (float)(lon * M_PI / 180.0);
    Vec3 p;
    p.x =  radius * sinf(lo) * cosf(la);
    p.y =  radius * sinf(la);           /* north pole -> +Y  */
    p.z = -radius * cosf(lo) * cosf(la);
    return p;
}

static void draw_text_3d(const char* text, float x, float y, float z, void* font) {
    int i;
    glRasterPos3f(x, y, z);
    for (i = 0; text[i]; i++)
        glutBitmapCharacter(font, text[i]);
}

/* ═══════════════════════════════════════════
   INIT — Populate countries, cities, routes
   ═══════════════════════════════════════════ */

void world_map_init(GameWorld* world) {
    WorldMapState* wm = &world->worldMap;
    int i;
    memset(wm, 0, sizeof(WorldMapState));

    wm->globeAzimuth   = 0.0f;
    wm->globeElevation  = 15.0f;
    wm->globeZoom       = 250.0f;
    wm->hoveredCountry  = -1;
    wm->selectedCountry = -1;
    wm->hoveredVirus    = -1;
    wm->autoRotate      = 0.0f;

    /* Load Earth texture */
    wm->earthTexture = load_texture("assets/earth_texture.jpg");

    /* Countries */
    struct { const char* n; float la, lo; float min_la, max_la; float min_lo, max_lo; } cdata[] = {
        {"United States",  40.0f, -100.0f, 24.0f, 49.0f, -125.0f, -66.0f},
        {"Brazil",        -10.0f,  -55.0f, -33.0f, 5.0f, -74.0f, -34.0f},
        {"United Kingdom", 54.0f,   -2.0f, 50.0f, 60.0f, -8.0f, 2.0f},
        {"France",         46.0f,    2.0f, 41.0f, 51.0f, -5.0f, 9.0f},
        {"Germany",        51.0f,   10.0f, 47.0f, 55.0f, 5.0f, 15.0f},
        {"India",          22.0f,   78.0f, 8.0f, 35.0f, 68.0f, 97.0f},
        {"China",          35.0f,  105.0f, 18.0f, 53.0f, 73.0f, 134.0f},
        {"Japan",          36.0f,  138.0f, 31.0f, 45.0f, 129.0f, 146.0f},
        {"Australia",     -25.0f,  135.0f, -43.0f, -10.0f, 113.0f, 153.0f},
        {"South Africa",  -30.0f,   25.0f, -35.0f, -22.0f, 16.0f, 33.0f},
        {"Nigeria",         9.0f,    8.0f, 4.0f, 14.0f, 2.0f, 14.0f},
        {"Russia",         60.0f,   90.0f, 41.0f, 82.0f, 19.0f, 180.0f},
        {"Egypt",          27.0f,   30.0f, 22.0f, 31.0f, 25.0f, 35.0f},
        {"Mexico",         23.0f, -102.0f, 14.0f, 32.0f, -118.0f, -86.0f},
        {"Argentina",     -35.0f,  -65.0f, -55.0f, -21.0f, -73.0f, -53.0f},
        {"Antarctica",    -80.0f,   0.0f, -90.0f, -60.0f, -180.0f, 180.0f}
    };
    wm->countryCount = 16;
    for (i = 0; i < 16; i++) {
        strncpy(wm->countries[i].name, cdata[i].n, 31);
        wm->countries[i].lat = cdata[i].la;
        wm->countries[i].lon = cdata[i].lo;
        wm->countries[i].minLat = cdata[i].min_la;
        wm->countries[i].maxLat = cdata[i].max_la;
        wm->countries[i].minLon = cdata[i].min_lo;
        wm->countries[i].maxLon = cdata[i].max_lo;
        wm->countries[i].infectionLevel = 0.0f;
        wm->countries[i].selected = 0;
    }

    /* Cities */
    struct { const char* n; float la, lo; int pop; int hub; int ci; } citydata[] = {
        {"New York",      40.7f,  -74.0f,  8, 1, 0},
        {"Los Angeles",   34.0f, -118.2f,  4, 0, 0},
        {"Chicago",       41.9f,  -87.6f,  3, 0, 0},
        {"Sao Paulo",    -23.5f,  -46.6f, 12, 1, 1},
        {"Rio de Janeiro",-22.9f, -43.2f,  7, 0, 1},
        {"London",        51.5f,   -0.1f,  9, 1, 2},
        {"Paris",         48.9f,    2.3f,  2, 0, 3},
        {"Berlin",        52.5f,   13.4f,  4, 0, 4},
        {"Mumbai",        19.1f,   72.9f, 20, 1, 5},
        {"Delhi",         28.6f,   77.2f, 19, 0, 5},
        {"Beijing",       39.9f,  116.4f, 21, 1, 6},
        {"Shanghai",      31.2f,  121.5f, 24, 0, 6},
        {"Tokyo",         35.7f,  139.7f, 14, 1, 7},
        {"Sydney",       -33.9f,  151.2f,  5, 0, 8},
        {"Melbourne",    -37.8f,  144.9f,  5, 0, 8},
        {"Cape Town",    -33.9f,   18.4f,  4, 0, 9},
        {"Johannesburg",  -26.2f,  28.0f,  6, 0, 9},
        {"Lagos",          6.5f,    3.4f, 15, 0, 10},
        {"Moscow",        55.8f,   37.6f, 12, 0, 11},
        {"Cairo",         30.0f,   31.2f, 10, 0, 12},
        {"Mexico City",   19.4f,  -99.1f,  9, 0, 13},
        {"Buenos Aires",  -34.6f, -58.4f,  3, 0, 14},
        {"Singapore",      1.3f,  103.8f,  6, 0, 6},
        {"Dubai",         25.2f,   55.3f,  3, 0, 5},
        {"Toronto",       43.7f,  -79.4f,  3, 0, 0}
    };
    wm->cityCount = 25;
    for (i = 0; i < 25; i++) {
        strncpy(wm->cities[i].name, citydata[i].n, 31);
        wm->cities[i].lat = citydata[i].la;
        wm->cities[i].lon = citydata[i].lo;
        wm->cities[i].population = citydata[i].pop;
        wm->cities[i].isSuperSpreadHub = citydata[i].hub;
        wm->cities[i].countryIndex = citydata[i].ci;
    }

    /* Routes */
    struct { int f, t; RouteType rt; } rdata[] = {
        { 0,  5, ROUTE_AIR},  /* NYC - London */
        { 5,  6, ROUTE_AIR},  /* London - Paris */
        { 5,  7, ROUTE_AIR},  /* London - Berlin */
        { 0, 10, ROUTE_AIR},  /* NYC - Beijing */
        {10, 12, ROUTE_AIR},  /* Beijing - Tokyo */
        { 8,  3, ROUTE_AIR},  /* Mumbai - Sao Paulo */
        { 3,  0, ROUTE_AIR},  /* Sao Paulo - NYC */
        {12, 13, ROUTE_AIR},  /* Tokyo - Sydney */
        { 8, 23, ROUTE_AIR},  /* Mumbai - Dubai */
        {23,  5, ROUTE_AIR},  /* Dubai - London */
        { 0,  3, ROUTE_SEA},  /* NYC - Sao Paulo (sea) */
        { 5, 17, ROUTE_SEA},  /* London - Lagos (sea) */
        {13, 14, ROUTE_SEA},  /* Sydney - Melbourne (sea) */
        {15, 17, ROUTE_SEA},  /* Cape Town - Lagos (sea) */
        {22, 12, ROUTE_SEA}   /* Singapore - Tokyo (sea) */
    };
    wm->routeCount = 15;
    for (i = 0; i < 15; i++) {
        wm->routes[i].fromCity = rdata[i].f;
        wm->routes[i].toCity   = rdata[i].t;
        wm->routes[i].type     = rdata[i].rt;
        wm->routes[i].animProgress = (float)(i * 37 % 100) / 100.0f;
    }
}

/* ═══════════════════════════════════════════
   UPDATE
   ═══════════════════════════════════════════ */

void world_map_update(GameWorld* world, float dt) {
    WorldMapState* wm = &world->worldMap;
    int i;

    /* Auto-rotation when not dragging */
    if (!wm->dragActive) {
        wm->autoRotate += dt * 3.0f;
    }

    /* Animate route dots */
    for (i = 0; i < wm->routeCount; i++) {
        wm->routes[i].animProgress += dt * 0.08f;
        if (wm->routes[i].animProgress > 1.0f)
            wm->routes[i].animProgress -= 1.0f;
    }
}

/* ═══════════════════════════════════════════
   CONTINENT GEOMETRY DATA
   ═══════════════════════════════════════════ */

/* Simplified continent outlines as lat/lon vertex arrays */

static const float north_america[][2] = {
    {50,  -130}, {55,  -130}, {60,  -140}, {65,  -168},
    {72,  -160}, {72,  -140}, {68,  -100}, {62,   -75},
    {55,   -60}, {48,   -55}, {45,   -65}, {43,   -70},
    {40,   -75}, {30,   -82}, {25,   -80}, {20,   -87},
    {18,   -95}, {15,  -105}, {20,  -105}, {25,  -110},
    {30,  -115}, {35,  -120}, {40,  -125}, {48,  -125}
};
#define NA_COUNT (sizeof(north_america)/sizeof(north_america[0]))

static const float south_america[][2] = {
    {12,   -70}, {10,   -62}, { 5,   -52}, { 0,   -50},
    {-5,   -35}, {-10,  -37}, {-15,  -39}, {-23,  -43},
    {-30,  -50}, {-35,  -57}, {-40,  -63}, {-45,  -68},
    {-55,  -68}, {-55,  -72}, {-50,  -75}, {-42,  -73},
    {-35,  -72}, {-28,  -70}, {-18,  -70}, {-10,  -75},
    { -5,  -77}, {  0,  -80}, { 5,   -77}, { 8,   -72}
};
#define SA_COUNT (sizeof(south_america)/sizeof(south_america[0]))

static const float europe[][2] = {
    {36,   -10}, {38,    -5}, {43,    -8}, {48,    -5},
    {50,     2}, {51,     5}, {54,    10}, {55,    12},
    {58,    18}, {60,    25}, {65,    25}, {70,    30},
    {72,    28}, {70,    20}, {68,    15}, {64,    10},
    {58,     8}, {55,     5}, {52,     3}, {50,    -2},
    {47,    -3}, {44,    -2}, {40,    -4}, {37,    -6}
};
#define EU_COUNT (sizeof(europe)/sizeof(europe[0]))

static const float africa[][2] = {
    {37,   -10}, {37,    10}, {33,    12}, {30,    32},
    {22,    36}, {15,    42}, {12,    44}, {10,    50},
    { 3,    42}, {-5,    40}, {-10,   40}, {-15,   35},
    {-20,   35}, {-25,   33}, {-30,   30}, {-35,   20},
    {-35,   18}, {-30,   16}, {-22,   14}, {-15,   12},
    {-7,     9}, { 0,     9}, { 5,    -5}, {10,    -8},
    {15,   -17}, {22,   -17}, {30,   -10}, {35,   -10}
};
#define AF_COUNT (sizeof(africa)/sizeof(africa[0]))

static const float asia[][2] = {
    {42,    26}, {45,    35}, {40,    44}, {38,    48},
    {35,    52}, {30,    48}, {25,    55}, {25,    65},
    {20,    72}, {15,    75}, {10,    77}, { 8,    80},
    { 5,    95}, { 1,   104}, {-8,   110}, {-8,   115},
    {-5,   120}, { 0,   128}, { 5,   120}, {10,   108},
    {18,   107}, {22,   114}, {30,   122}, {35,   128},
    {40,   130}, {45,   135}, {50,   140}, {55,   135},
    {60,   130}, {65,   120}, {68,   100}, {70,    80},
    {72,    60}, {70,    50}, {65,    40}, {55,    38},
    {48,    30}
};
#define AS_COUNT (sizeof(asia)/sizeof(asia[0]))

static const float oceania[][2] = {
    {-12,  130}, {-12,  142}, {-18,  148}, {-25,  153},
    {-30,  153}, {-35,  150}, {-38,  146}, {-39,  144},
    {-35,  136}, {-32,  132}, {-30,  128}, {-25,  114},
    {-22,  114}, {-15,  122}, {-12,  127}
};
#define OC_COUNT (sizeof(oceania)/sizeof(oceania[0]))

static void draw_continent(const float verts[][2], int count, float R,
                           float cr, float cg, float cb) {
    int i;
    float sr = R + 1.0f; /* elevated well above ocean to prevent z-fighting */
    glColor4f(cr, cg, cb, 0.92f);
    glBegin(GL_TRIANGLE_FAN);
    /* Center for fan */
    {
        float cla = 0, clo = 0;
        for (i = 0; i < count; i++) { cla += verts[i][0]; clo += verts[i][1]; }
        cla /= count; clo /= count;
        Vec3 c = latlon_to_sphere(cla, clo, sr);
        glVertex3f(c.x, c.y, c.z);
    }
    for (i = 0; i < count; i++) {
        Vec3 p = latlon_to_sphere(verts[i][0], verts[i][1], sr);
        glVertex3f(p.x, p.y, p.z);
    }
    /* Close the fan */
    {
        Vec3 p = latlon_to_sphere(verts[0][0], verts[0][1], sr);
        glVertex3f(p.x, p.y, p.z);
    }
    glEnd();

    /* Outline — bold edges */
    glColor4f(cr * 0.4f, cg * 0.4f, cb * 0.4f, 0.9f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    for (i = 0; i < count; i++) {
        Vec3 p = latlon_to_sphere(verts[i][0], verts[i][1], sr + 0.2f);
        glVertex3f(p.x, p.y, p.z);
    }
    glEnd();
    glLineWidth(1.0f);
}

/* ═══════════════════════════════════════════
   RENDERING
   ═══════════════════════════════════════════ */

static void setup_globe_camera(WorldMapState* wm) {
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    /* Reserve right side for sidebar */
    int sideW = 220;
    int globeW = (w > sideW + 100) ? w - sideW : w;

    glViewport(0, 0, globeW, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)globeW / (double)(h > 0 ? h : 1), 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float camDist = wm->globeZoom;
    /* Standard Y-up orbit: elevation above XZ plane, azimuth in XZ plane. */
    float elev = (float)(wm->globeElevation * M_PI / 180.0);
    float azim = (float)((wm->globeAzimuth + wm->autoRotate) * M_PI / 180.0);

    float eyeX = camDist * cosf(elev) * sinf(azim);
    float eyeY = camDist * sinf(elev);       /* elevation lifts in Y (north) */
    float eyeZ = camDist * cosf(elev) * cosf(azim);

    gluLookAt(eyeX, eyeY, eyeZ, 0, 0, 0, 0, 1, 0);  /* up = +Y = north pole */
}

static void render_globe_sphere(WorldMapState* wm, float gameTime) {
    GLUquadric* q = gluNewQuadric();
    gluQuadricNormals(q, GLU_SMOOTH);
    gluQuadricTexture(q, GL_TRUE);

    /* Disable lighting to see texture clearly */
    glDisable(GL_LIGHTING);

    /* Enable texturing and bind Earth texture */
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, wm->earthTexture);
    
    /* Set color to white for full texture visibility */
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    
    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
    
    /* Flip texture horizontally to fix left-to-right mirroring */
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glScalef(-1.0f, 1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    
    gluSphere(q, GLOBE_RADIUS, GLOBE_SLICES, GLOBE_STACKS);
    
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    gluDeleteQuadric(q);

    (void)gameTime;
}

static void render_atmosphere(void) {
    GLUquadric* q = gluNewQuadric();
    gluQuadricNormals(q, GLU_SMOOTH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT); /* Draw back faces only for glow */

    glColor4f(0.3f, 0.5f, 0.9f, 0.08f);
    glDisable(GL_LIGHTING);
    gluSphere(q, GLOBE_RADIUS * 1.06f, 32, 24);

    glDisable(GL_CULL_FACE);
    gluDeleteQuadric(q);
}

static void draw_all_continents(void) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    /* Keep depth test ON so far-side continents are hidden by the sphere */
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE); /* Don't write to depth buffer (overlay on sphere) */

    draw_continent(north_america, NA_COUNT, GLOBE_RADIUS, 0.20f, 0.45f, 0.18f);
    draw_continent(south_america, SA_COUNT, GLOBE_RADIUS, 0.25f, 0.50f, 0.20f);
    draw_continent(europe,        EU_COUNT, GLOBE_RADIUS, 0.30f, 0.48f, 0.22f);
    draw_continent(africa,        AF_COUNT, GLOBE_RADIUS, 0.45f, 0.40f, 0.20f);
    draw_continent(asia,          AS_COUNT, GLOBE_RADIUS, 0.35f, 0.45f, 0.18f);
    draw_continent(oceania,       OC_COUNT, GLOBE_RADIUS, 0.38f, 0.50f, 0.22f);

    glDepthMask(GL_TRUE);
}

static void draw_cities(WorldMapState* wm, float gameTime) {
    int i;
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);

    for (i = 0; i < wm->cityCount; i++) {
        CityNode* c = &wm->cities[i];
        Vec3 p = latlon_to_sphere(c->lat, c->lon, GLOBE_RADIUS + 0.6f);
        float sz = c->isSuperSpreadHub ? 4.0f : 2.5f;

        if (c->isSuperSpreadHub) {
            /* Pulsing red glow */
            float pulse = 0.5f + 0.5f * sinf(gameTime * 4.0f + i * 1.5f);
            glPointSize(sz + 3.0f);
            glBegin(GL_POINTS);
            glColor4f(1.0f, 0.2f, 0.15f, 0.3f * pulse);
            glVertex3f(p.x, p.y, p.z);
            glEnd();
        }

        glPointSize(sz);
        glBegin(GL_POINTS);
        if (c->isSuperSpreadHub)
            glColor4f(1.0f, 0.3f, 0.2f, 0.9f);
        else
            glColor4f(1.0f, 0.85f, 0.4f, 0.8f);
        glVertex3f(p.x, p.y, p.z);
        glEnd();
    }
    glPointSize(1.0f);
}

static void draw_routes(WorldMapState* wm, float gameTime) {
    int i, j;
    int segments = 30;
    (void)gameTime;

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);

    for (i = 0; i < wm->routeCount; i++) {
        FlightRoute* r = &wm->routes[i];
        CityNode* c1 = &wm->cities[r->fromCity];
        CityNode* c2 = &wm->cities[r->toCity];
        float arcHeight = (r->type == ROUTE_AIR) ? 12.0f : 3.0f;

        if (r->type == ROUTE_AIR) {
            glColor4f(0.8f, 0.8f, 0.9f, 0.25f);
            glEnable(GL_LINE_STIPPLE);
            glLineStipple(3, 0xAAAA);
        } else {
            glColor4f(0.2f, 0.7f, 0.9f, 0.3f);
        }

        glLineWidth(1.0f);
        glBegin(GL_LINE_STRIP);
        for (j = 0; j <= segments; j++) {
            float t = (float)j / (float)segments;
            float lat = c1->lat + (c2->lat - c1->lat) * t;
            float lon = c1->lon + (c2->lon - c1->lon) * t;
            float height_offset = sinf(t * (float)M_PI) * arcHeight;
            Vec3 p = latlon_to_sphere(lat, lon, GLOBE_RADIUS + 1.0f + height_offset);
            glVertex3f(p.x, p.y, p.z);
        }
        glEnd();

        if (r->type == ROUTE_AIR)
            glDisable(GL_LINE_STIPPLE);

        /* Animated traveling dot */
        {
            float at = r->animProgress;
            float alat = c1->lat + (c2->lat - c1->lat) * at;
            float alon = c1->lon + (c2->lon - c1->lon) * at;
            float ah   = sinf(at * (float)M_PI) * arcHeight;
            Vec3 dp = latlon_to_sphere(alat, alon, GLOBE_RADIUS + 1.2f + ah);

            glPointSize(4.0f);
            glBegin(GL_POINTS);
            if (r->type == ROUTE_AIR)
                glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
            else
                glColor4f(0.3f, 0.9f, 1.0f, 0.9f);
            glVertex3f(dp.x, dp.y, dp.z);
            glEnd();
            glPointSize(1.0f);
        }
    }
    glLineWidth(1.0f);
}

static void draw_country_labels(WorldMapState* wm) {
    int i;
    glDisable(GL_LIGHTING);
    glColor4f(0.85f, 0.85f, 0.9f, 0.7f);
    for (i = 0; i < wm->countryCount; i++) {
        Country* c = &wm->countries[i];
        Vec3 p = latlon_to_sphere(c->lat, c->lon, GLOBE_RADIUS + 2.5f);
        draw_text_3d(c->name, p.x, p.y, p.z, GLUT_BITMAP_HELVETICA_10);
    }
}

static void draw_country_dots(WorldMapState* wm, float gameTime) {
    int i;
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);

    for (i = 0; i < wm->countryCount; i++) {
        Country* c = &wm->countries[i];
        Vec3 p = latlon_to_sphere(c->lat, c->lon, GLOBE_RADIUS + 1.0f);
        float sz = 6.0f;
        float r = 0.3f, g = 0.9f, b = 0.4f;

        if (c->infectionLevel > 0.0f) {
            r = 0.3f + 0.7f * c->infectionLevel;
            g = 0.9f - 0.7f * c->infectionLevel;
            b = 0.4f - 0.3f * c->infectionLevel;
        }

        if (i == wm->hoveredCountry) {
            sz = 9.0f;
            float pulse = 0.7f + 0.3f * sinf(gameTime * 5.0f);
            glPointSize(sz + 4.0f);
            glBegin(GL_POINTS);
            glColor4f(1.0f, 1.0f, 1.0f, 0.3f * pulse);
            glVertex3f(p.x, p.y, p.z);
            glEnd();
        }

        glPointSize(sz);
        glBegin(GL_POINTS);
        glColor4f(r, g, b, 0.9f);
        glVertex3f(p.x, p.y, p.z);
        glEnd();
    }
    glPointSize(1.0f);
}

/* ═══════════════════════════════════════════
   HIT TESTING
   ═══════════════════════════════════════════ */

static int globe_hit_test(WorldMapState* wm, int mouseX, int mouseY) {
    GLdouble modelview[16], projection[16];
    GLint viewport[4];
    GLdouble nearX, nearY, nearZ, farX, farY, farZ;
    int i;

    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    int glY = viewport[3] - mouseY;

    gluUnProject(mouseX, glY, 0.0, modelview, projection, viewport,
                 &nearX, &nearY, &nearZ);
    gluUnProject(mouseX, glY, 1.0, modelview, projection, viewport,
                 &farX, &farY, &farZ);

    /* Ray: origin=near, direction=far-near */
    double dx = farX - nearX, dy = farY - nearY, dz = farZ - nearZ;
    double a = dx*dx + dy*dy + dz*dz;
    double b = 2.0 * (nearX*dx + nearY*dy + nearZ*dz);
    double c = nearX*nearX + nearY*nearY + nearZ*nearZ - (double)(GLOBE_RADIUS * GLOBE_RADIUS);
    double disc = b*b - 4*a*c;

    if (disc < 0) return -1;

    double t = (-b - sqrt(disc)) / (2.0 * a);
    if (t < 0) t = (-b + sqrt(disc)) / (2.0 * a);
    if (t < 0) return -1;

    /* Hit point on sphere */
    double hx = nearX + t * dx;
    double hy = nearY + t * dy;
    double hz = nearZ + t * dz;

    /* Inverse of latlon_to_sphere:
       x =  sin(lon)*cos(lat)
       y =  sin(lat)           <- lat on Y axis
       z = -cos(lon)*cos(lat)
       => lat = asin(hy / R)
       => lon = atan2(hx, -hz)
    */
    double R = GLOBE_RADIUS;
    double lat = asin(hy / R) * 180.0 / M_PI;
    double lon = atan2(hx, -hz) * 180.0 / M_PI;

    /* Find closest country using bounding boxes and center distance */
    int bestIdx = -1;
    float bestDist = 9999.0f;
    for (i = 0; i < wm->countryCount; i++) {
        float minLat = wm->countries[i].minLat;
        float maxLat = wm->countries[i].maxLat;
        float minLon = wm->countries[i].minLon;
        float maxLon = wm->countries[i].maxLon;
        
        if (lat >= minLat && lat <= maxLat && lon >= minLon && lon <= maxLon) {
            float dlat = wm->countries[i].lat - (float)lat;
            float dlon = wm->countries[i].lon - (float)lon;
            if (dlon > 180.0f) dlon -= 360.0f;
            if (dlon < -180.0f) dlon += 360.0f;
            
            float d = sqrtf(dlat*dlat + dlon*dlon);
            if (d < bestDist) {
                bestDist = d;
                bestIdx = i;
            }
        }
    }
    return bestIdx;
}

/* ═══════════════════════════════════════════
   2D HUD ELEMENTS
   ═══════════════════════════════════════════ */

static void draw_globe_hud(WorldMapState* wm, float gameTime) {
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    Color4f white = {1,1,1,1};
    Color4f gray  = {0.6f, 0.6f, 0.7f, 1.0f};

    /* Reset viewport to full window for 2D overlay + sidebar */
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Title bar */
    glColor4f(0.04f, 0.04f, 0.06f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(0, (float)h - 45); glVertex2f((float)(w - 220), (float)h - 45);
    glVertex2f((float)(w - 220), (float)h); glVertex2f(0, (float)h);
    glEnd();

    /* Show selected country name or default message */
    if (wm->selectedCountry >= 0 && wm->selectedCountry < wm->countryCount) {
        Country* c = &wm->countries[wm->selectedCountry];
        char title[128];
        int i;
        float titleWidth = 0;
        snprintf(title, 128, "SELECTED: %s  --  Click virus in sidebar to begin", c->name);
        for (i = 0; title[i]; i++) titleWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, title[i]);
        hud_draw_text(((float)(w - 220) - titleWidth) / 2, (float)h - 30, title, white, GLUT_BITMAP_HELVETICA_18);
    } else {
        hud_draw_text(30, (float)h - 30, "SELECT A COUNTRY FROM THE SIDEBAR TO INFECT",
                      white, GLUT_BITMAP_HELVETICA_18);
    }

    /* Bottom info bar */
    glColor4f(0.04f, 0.04f, 0.06f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f((float)(w - 220), 0);
    glVertex2f((float)(w - 220), 35); glVertex2f(0, 35);
    glEnd();
    hud_draw_text(15, 12, "Scroll to zoom | Drag to rotate | ESC to return",
                  gray, GLUT_BITMAP_HELVETICA_10);

    /* ── SIDEBAR ─────────────────────────────────────────────────── */
    {
        int i;
        int sideW = 220;
        float sx    = (float)(w - sideW);
        float sw    = (float)sideW;
        float rowH  = 30.0f;
        float hdrH  = 44.0f;
        float topY  = (float)h;

        /* Background */
        glColor4f(0.04f, 0.05f, 0.10f, 0.97f);
        glBegin(GL_QUADS);
        glVertex2f(sx, 0);    glVertex2f(sx + sw, 0);
        glVertex2f(sx + sw, topY); glVertex2f(sx, topY);
        glEnd();

        /* Left accent border */
        glColor4f(0.28f, 0.48f, 1.0f, 0.9f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(sx, 0); glVertex2f(sx, topY);
        glEnd();
        glLineWidth(1.0f);

        /* Header strip */
        glColor4f(0.09f, 0.12f, 0.22f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(sx, topY - hdrH);  glVertex2f(sx + sw, topY - hdrH);
        glVertex2f(sx + sw, topY);    glVertex2f(sx, topY);
        glEnd();
        {
            Color4f accent = {0.55f, 0.78f, 1.0f, 1.0f};
            hud_draw_text(sx + 12, topY - 24, "COUNTRIES", accent, GLUT_BITMAP_HELVETICA_12);
            hud_draw_text(sx + 12, topY - 38, "Click a name to select", gray, GLUT_BITMAP_HELVETICA_10);
        }

        /* Country rows */
        for (i = 0; i < wm->countryCount; i++) {
            Country* c = &wm->countries[i];
            float rowY = topY - hdrH - (float)(i + 1) * rowH;
            if (rowY < 36.0f) break;  /* stop before bottom bar */

            int isSelected = (i == wm->selectedCountry);
            float inf = c->infectionLevel;

            /* Row bg */
            if (isSelected)
                glColor4f(0.15f, 0.30f, 0.65f, 0.98f);
            else
                glColor4f(i % 2 == 0 ? 0.06f : 0.08f,
                          i % 2 == 0 ? 0.07f : 0.09f,
                          i % 2 == 0 ? 0.12f : 0.14f, 0.95f);
            glBegin(GL_QUADS);
            glVertex2f(sx + 2, rowY);
            glVertex2f(sx + sw - 2, rowY);
            glVertex2f(sx + sw - 2, rowY + rowH - 1);
            glVertex2f(sx + 2, rowY + rowH - 1);
            glEnd();

            /* Thin infection bar at bottom of row */
            if (inf > 0.001f) {
                float bw = (sw - 4) * inf;
                glColor4f(0.9f, 0.3f + 0.3f*(1-inf), 0.1f, 0.7f);
                glBegin(GL_QUADS);
                glVertex2f(sx + 2, rowY);
                glVertex2f(sx + 2 + bw, rowY);
                glVertex2f(sx + 2 + bw, rowY + 4);
                glVertex2f(sx + 2, rowY + 4);
                glEnd();
            }

            /* Country name text */
            {
                Color4f nc = isSelected
                    ? (Color4f){1.0f, 1.0f, 1.0f, 1.0f}
                    : (inf > 0.25f
                        ? (Color4f){1.0f, 0.55f + 0.35f*(1-inf), 0.25f, 1.0f}
                        : (Color4f){0.80f, 0.84f, 0.94f, 1.0f});
                hud_draw_text(sx + 10, rowY + 9, c->name, nc, GLUT_BITMAP_HELVETICA_12);
            }

            /* Infection pct badge on right */
            if (inf > 0.001f) {
                char pct[10];
                snprintf(pct, sizeof(pct), "%.0f%%", inf * 100.0f);
                Color4f rc = {1.0f, 0.45f, 0.2f, 1.0f};
                hud_draw_text(sx + sw - 36, rowY + 9, pct, rc, GLUT_BITMAP_HELVETICA_10);
            }

            /* Selected outline */
            if (isSelected) {
                glColor4f(0.45f, 0.72f, 1.0f, 0.95f);
                glLineWidth(1.5f);
                glBegin(GL_LINE_LOOP);
                glVertex2f(sx + 2, rowY);
                glVertex2f(sx + sw - 2, rowY);
                glVertex2f(sx + sw - 2, rowY + rowH - 1);
                glVertex2f(sx + 2, rowY + rowH - 1);
                glEnd();
                glLineWidth(1.0f);
            }
        }
    }

    (void)gameTime;

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW); glPopMatrix();
}

/* ═══════════════════════════════════════════
   RENDER WORLD MAP
   ═══════════════════════════════════════════ */

void render_world_map(GameWorld* world) {
    WorldMapState* wm = &world->worldMap;
    float gt = world->gameTime;

    /* Dark space background */
    glClearColor(0.02f, 0.02f, 0.04f, 1.0f);

    /* Draw background stars */
    {
        int i;
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glPointSize(1.5f);
        glBegin(GL_POINTS);
        for (i = 0; i < 200; i++) {
            float sx = sinf(i * 45.7f) * 400.0f;
            float sy = cosf(i * 23.1f) * 400.0f;
            float sz = sinf(i * 67.3f) * 400.0f;
            float brightness = 0.3f + 0.3f * sinf(gt * 0.5f + i * 0.7f);
            glColor4f(brightness, brightness, brightness + 0.1f, 0.6f);
            glVertex3f(sx, sy, sz);
        }
        glEnd();
        glPointSize(1.0f);
        glEnable(GL_DEPTH_TEST);
    }

    setup_globe_camera(wm);

    /* Atmosphere glow (behind globe) */
    render_atmosphere();

    /* Ocean sphere */
    render_globe_sphere(wm, gt);

    /* Continents - DISABLED - Earth texture already contains continents */
    /* draw_all_continents(); */

    /* Country dots */
    draw_country_dots(wm, gt);

    /* Cities */
    draw_cities(wm, gt);

    /* Routes */
    draw_routes(wm, gt);

    /* Country labels removed from globe -- shown in sidebar instead */

    /* 2D HUD overlay + sidebar */
    draw_globe_hud(wm, gt);
}

/* ═══════════════════════════════════════════
   VIRUS SELECTION OVERLAY
   ═══════════════════════════════════════════ */

void render_virus_select(GameWorld* world) {
    WorldMapState* wm = &world->worldMap;
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    Color4f white = {1,1,1,1};
    Color4f gray  = {0.7f, 0.7f, 0.8f, 1.0f};
    int i;

    float cardW = 220, cardH = 260, gap = 30;
    float totalW = 3 * cardW + 2 * gap;
    float startX = ((float)w - totalW) / 2.0f;
    float cardY  = ((float)h - cardH) / 2.0f;

    const char* names[] = {"STEALTH", "AGGRESSIVE", "ENVIRONMENTAL"};
    const char* desc1[] = {"Slow spread", "Fast spread", "Climate-dependent"};
    const char* desc2[] = {"Hard to detect", "High mortality", "Moderate spread"};
    const char* r0_str[] = {"R0: 1.8", "R0: 4.0", "R0: 2.5"};
    const char* mort_str[] = {"Mortality: 1%", "Mortality: 5%", "Mortality: 2.5%"};
    const char* inc_str[] = {"Incubation: 14d", "Incubation: 3d", "Incubation: 7d"};
    float card_colors[][3] = {
        {0.2f, 0.5f, 0.8f},   /* blue */
        {0.8f, 0.2f, 0.2f},   /* red */
        {0.2f, 0.7f, 0.4f}    /* green */
    };

    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);

    /* Darken overlay */
    glColor4f(0.0f, 0.0f, 0.02f, 0.75f);
    glBegin(GL_QUADS);
    glVertex2f(0,0); glVertex2f((float)w,0);
    glVertex2f((float)w,(float)h); glVertex2f(0,(float)h);
    glEnd();

    /* Title */
    {
        char title[64];
        if (wm->selectedCountry >= 0)
            snprintf(title, 64, "Choose virus for %s", wm->countries[wm->selectedCountry].name);
        else
            snprintf(title, 64, "Choose your virus");
        float tw_calc = 0;
        for (i = 0; title[i]; i++) tw_calc += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, title[i]);
        hud_draw_text(((float)w - tw_calc)/2, cardY + cardH + 40, title, white, GLUT_BITMAP_HELVETICA_18);
    }

    /* Draw 3 virus cards */
    for (i = 0; i < 3; i++) {
        float cx = startX + i * (cardW + gap);
        float cy = cardY;
        int hovered = (i == wm->hoveredVirus);
        float cr = card_colors[i][0], cg = card_colors[i][1], cb = card_colors[i][2];

        /* Card background */
        if (hovered) {
            glColor4f(cr * 0.3f, cg * 0.3f, cb * 0.3f, 0.95f);
        } else {
            glColor4f(0.06f, 0.06f, 0.10f, 0.92f);
        }
        glBegin(GL_QUADS);
        glVertex2f(cx, cy); glVertex2f(cx+cardW, cy);
        glVertex2f(cx+cardW, cy+cardH); glVertex2f(cx, cy+cardH);
        glEnd();

        /* Border */
        float alpha = hovered ? 0.9f : 0.4f;
        glColor4f(cr, cg, cb, alpha);
        glLineWidth(hovered ? 2.5f : 1.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(cx, cy); glVertex2f(cx+cardW, cy);
        glVertex2f(cx+cardW, cy+cardH); glVertex2f(cx, cy+cardH);
        glEnd();
        glLineWidth(1.0f);

        /* Top accent bar */
        glColor4f(cr, cg, cb, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(cx, cy+cardH-4); glVertex2f(cx+cardW, cy+cardH-4);
        glVertex2f(cx+cardW, cy+cardH); glVertex2f(cx, cy+cardH);
        glEnd();

        /* Virus icon (simple circle) */
        {
            int j;
            float ix = cx + cardW/2, iy = cy + cardH - 45;
            float ir = 18.0f;
            glColor4f(cr, cg, cb, 0.7f);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(ix, iy);
            for (j = 0; j <= 20; j++) {
                float a2 = (float)j / 20.0f * 2.0f * (float)M_PI;
                glVertex2f(ix + ir * cosf(a2), iy + ir * sinf(a2));
            }
            glEnd();
            /* Virus spikes */
            glColor4f(cr*1.2f, cg*1.2f, cb*1.2f, 0.9f);
            glLineWidth(2.0f);
            for (j = 0; j < 8; j++) {
                float a2 = (float)j / 8.0f * 2.0f * (float)M_PI;
                glBegin(GL_LINES);
                glVertex2f(ix + ir * cosf(a2), iy + ir * sinf(a2));
                glVertex2f(ix + (ir+8) * cosf(a2), iy + (ir+8) * sinf(a2));
                glEnd();
            }
            glLineWidth(1.0f);
        }

        /* Name */
        {
            Color4f nc = {cr*1.3f, cg*1.3f, cb*1.3f, 1.0f};
            float nw = 0;
            int k;
            for(k=0;names[i][k];k++) nw+=glutBitmapWidth(GLUT_BITMAP_HELVETICA_12,names[i][k]);
            hud_draw_text(cx + (cardW-nw)/2, cy+cardH-80, names[i], nc, GLUT_BITMAP_HELVETICA_12);
        }

        /* Description */
        hud_draw_text(cx+15, cy+cardH-105, desc1[i], gray, GLUT_BITMAP_HELVETICA_10);
        hud_draw_text(cx+15, cy+cardH-120, desc2[i], gray, GLUT_BITMAP_HELVETICA_10);

        /* Stats */
        {
            Color4f sc = {0.8f, 0.8f, 0.9f, 1.0f};
            hud_draw_text(cx+15, cy+55, r0_str[i], sc, GLUT_BITMAP_HELVETICA_10);
            hud_draw_text(cx+15, cy+40, mort_str[i], sc, GLUT_BITMAP_HELVETICA_10);
            hud_draw_text(cx+15, cy+25, inc_str[i], sc, GLUT_BITMAP_HELVETICA_10);
        }

        /* Click prompt */
        if (hovered) {
            Color4f hc = {1.0f, 1.0f, 1.0f, 0.8f};
            hud_draw_text(cx+55, cy+8, "CLICK TO SELECT", hc, GLUT_BITMAP_HELVETICA_10);
        }
    }

    /* Back hint */
    hud_draw_text(15, 15, "Press ESC to go back", gray, GLUT_BITMAP_HELVETICA_10);

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW); glPopMatrix();
}

/* ═══════════════════════════════════════════
   INTERACTION
   ═══════════════════════════════════════════ */

/* Helper: given screen pixel (mx, my), return the sidebar country index or -1.
   Constants must match draw_globe_hud sidebar section exactly. */
static int sidebar_hit(WorldMapState* wm, int mx, int my) {
    int w  = glutGet(GLUT_WINDOW_WIDTH);
    int h  = glutGet(GLUT_WINDOW_HEIGHT);
    float sx   = (float)(w - 220);
    float rowH = 30.0f;    /* must match draw code */
    float hdrH = 44.0f;   /* must match draw code */
    float topY = (float)h;
    int   glY  = h - my;  /* window Y → ortho Y (0=bottom) */

    if ((float)mx < sx) return -1;  /* left of sidebar */

    int i;
    for (i = 0; i < wm->countryCount; i++) {
        float rowY = topY - hdrH - (float)(i + 1) * rowH;
        if (rowY < 36.0f) break;
        if ((float)glY >= rowY && (float)glY < rowY + rowH)
            return i;
    }
    return -1;
}

int world_map_click(GameWorld* world, int mouseX, int mouseY) {
    WorldMapState* wm = &world->worldMap;

    /* Sidebar only -- clicking the globe no longer selects countries */
    int sidebarIdx = sidebar_hit(wm, mouseX, mouseY);
    if (sidebarIdx >= 0) {
        wm->selectedCountry = sidebarIdx;
        wm->hoveredVirus = -1;
        return 1;
    }
    return 0;
}

int virus_select_click(GameWorld* world, int mouseX, int mouseY) {
    WorldMapState* wm = &world->worldMap;
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    int i;
    int glY = h - mouseY;

    float cardW = 220, cardH = 260, gap = 30;
    float totalW = 3 * cardW + 2 * gap;
    float startX = ((float)w - totalW) / 2.0f;
    float cardY  = ((float)h - cardH) / 2.0f;

    for (i = 0; i < 3; i++) {
        float cx = startX + i * (cardW + gap);
        if ((float)mouseX >= cx && (float)mouseX <= cx + cardW &&
            (float)glY >= cardY && (float)glY <= cardY + cardH) {
            wm->selectedVirus = (VirusType)i;
            return 1;
        }
    }
    return 0;
}

void world_map_passive_motion(GameWorld* world, int x, int y) {
    WorldMapState* wm = &world->worldMap;
    /* Only track mouse position; no hover highlighting */
    wm->mouseX = x;
    wm->mouseY = y;

    if (world->appState == APP_VIRUS_SELECT) {
        int w = glutGet(GLUT_WINDOW_WIDTH);
        int h = glutGet(GLUT_WINDOW_HEIGHT);
        int glY = h - y;
        int i;

        float cardW = 220, cardH = 260, gap = 30;
        float totalW = 3 * cardW + 2 * gap;
        float startX = ((float)w - totalW) / 2.0f;
        float cardY  = ((float)h - cardH) / 2.0f;

        wm->hoveredVirus = -1;
        for (i = 0; i < 3; i++) {
            float cx = startX + i * (cardW + gap);
            if ((float)x >= cx && (float)x <= cx + cardW &&
                (float)glY >= cardY && (float)glY <= cardY + cardH) {
                wm->hoveredVirus = i;
                break;
            }
        }
    }
}

void world_map_drag_start(GameWorld* world, int x, int y) {
    WorldMapState* wm = &world->worldMap;
    wm->dragActive = 1;
    wm->lastMouseX = x;
    wm->lastMouseY = y;
}

void world_map_drag(GameWorld* world, int x, int y) {
    WorldMapState* wm = &world->worldMap;
    if (wm->dragActive) {
        float dx = (float)(x - wm->lastMouseX);
        float dy = (float)(y - wm->lastMouseY);
        /* drag right -> globe surface moves right -> azimuth decreases */
        wm->globeAzimuth   -= dx * 0.4f;
        /* drag down  -> globe surface moves down  -> elevation increases */
        wm->globeElevation += dy * 0.3f;
        if (wm->globeElevation >  80.0f) wm->globeElevation =  80.0f;
        if (wm->globeElevation < -80.0f) wm->globeElevation = -80.0f;
        wm->lastMouseX = x;
        wm->lastMouseY = y;
    }
}

void world_map_drag_end(GameWorld* world) {
    world->worldMap.dragActive = 0;
}

void world_map_scroll(GameWorld* world, int direction) {
    WorldMapState* wm = &world->worldMap;
    if (direction > 0) {
        wm->globeZoom -= 15.0f;
    } else {
        wm->globeZoom += 15.0f;
    }
    if (wm->globeZoom < 120.0f) wm->globeZoom = 120.0f;
    if (wm->globeZoom > 600.0f) wm->globeZoom = 600.0f;
}
