#include "world_map.h"
#include "hud.h"
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define DEG_TO_RAD (M_PI / 180.0)

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static GLuint earthTextureID = 0;

/* ═══════════════════════════════════════════
   DATA INITIALIZATION
   ═══════════════════════════════════════════ */

static void add_country(WorldMap* wm, const char* name, int continent,
                         float lat, float lon, float pop, float r, float g, float b,
                         const float verts[][2], int nv) {
    Country* c = &wm->countries[wm->countryCount];
    int i;
    strncpy(c->name, name, 31); c->name[31] = '\0';
    c->continentIndex = continent;
    c->lat = lat; c->lon = lon;
    
    /* 2D Fallback for UI positioning */
    c->centerX = (lon + 180.0f) / 360.0f;
    c->centerY = (lat + 90.0f) / 180.0f;
    
    c->population = pop;
    c->susceptible = pop;
    c->exposed = 0; c->infected = 0; c->recovered = 0; c->dead = 0;
    c->infectionRate = 2.0f + (rand() % 20) / 10.0f;
    c->r = r; c->g = g; c->b = b;
    c->vertexCount = nv;
    for (i = 0; i < nv && i < MAX_COUNTRY_VERTS; i++) {
        c->vertices[i][0] = verts[i][0]; /* lon */
        c->vertices[i][1] = verts[i][1]; /* lat */
    }
    wm->countryCount++;
}

static void add_city(WorldMap* wm, const char* name, float lat, float lon, float pop, int isSuperSpread, int countryIdx) {
    CityNode* c = &wm->cities[wm->cityCount];
    strncpy(c->name, name, 23); c->name[23] = '\0';
    c->lat = lat; c->lon = lon;
    c->population = pop;
    c->isSuperSpreadHub = isSuperSpread;
    c->countryIndex = countryIdx;
    wm->cityCount++;
}

static void add_route(WorldMap* wm, int from, int to, int isAir) {
    FlightRoute* r = &wm->routes[wm->routeCount];
    r->fromCity = from;
    r->toCity = to;
    r->isAirRoute = isAir;
    r->animProgress = (float)(rand() % 100) / 100.0f;
    wm->routeCount++;
}

void world_map_init(WorldMap* wm) {
    memset(wm, 0, sizeof(WorldMap));
    wm->hoveredCountry = -1;
    wm->selectedCountry = -1;
    wm->hoveredVirus = -1;
    wm->globeAzimuth = 45.0f;
    wm->globeElevation = 20.0f;
    wm->globeZoom = 300.0f;

    /* ─── Load Earth Texture ─── */
    if (earthTextureID == 0) {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(1);
        unsigned char *data = stbi_load("earth.jpg", &width, &height, &channels, 3);
        if (data) {
            glGenTextures(1, &earthTextureID);
            glBindTexture(GL_TEXTURE_2D, earthTextureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            /* Important for spherical mapping borders */
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            printf("Failed to load earth.jpg!\n");
        }
    }

    /* ─── Continents Data (Lat/Lon) ─── */
    /* North America */
    {
        const float v[][2] = { {-160, 70}, {-120, 80}, {-60, 80}, {-50, 50}, {-80, 20}, {-100, 15}, {-120, 30}, {-130, 50} };
        add_country(wm, "North America", 0, 45, -100, 590000, 0.25f, 0.45f, 0.65f, v, 8);
    }
    /* South America */
    {
        const float v[][2] = { {-80, 15}, {-40, -5}, {-35, -20}, {-50, -50}, {-75, -50}, {-80, -20} };
        add_country(wm, "South America", 1, -15, -60, 430000, 0.25f, 0.60f, 0.30f, v, 6);
    }
    /* Europe */
    {
        const float v[][2] = { {-10, 60}, {10, 70}, {40, 70}, {40, 45}, {20, 40}, {0, 40} };
        add_country(wm, "Europe", 2, 50, 15, 746000, 0.55f, 0.50f, 0.35f, v, 6);
    }
    /* Africa */
    {
        const float v[][2] = { {-15, 35}, {15, 35}, {40, 30}, {50, 10}, {40, -30}, {15, -35}, {5, -10}, {-15, 10} };
        add_country(wm, "Africa", 3, 0, 20, 1300000, 0.45f, 0.55f, 0.30f, v, 8);
    }
    /* Asia */
    {
        const float v[][2] = { {40, 70}, {100, 75}, {160, 65}, {140, 30}, {100, 10}, {70, 10}, {40, 30} };
        add_country(wm, "Asia", 4, 40, 90, 4600000, 0.65f, 0.35f, 0.30f, v, 7);
    }
    /* Oceania */
    {
        const float v[][2] = { {110, -10}, {150, -10}, {155, -40}, {115, -35} };
        add_country(wm, "Oceania", 5, -25, 135, 43000, 0.55f, 0.45f, 0.25f, v, 4);
    }

    /* ─── Cities ─── */
    add_city(wm, "New York", 40.7, -74.0, 8.4, 1, 0);   /* North America */
    add_city(wm, "Los Angeles", 34.0, -118.2, 3.9, 1, 0); 
    add_city(wm, "São Paulo", -23.5, -46.6, 12.3, 1, 1); /* South America */
    add_city(wm, "London", 51.5, -0.1, 8.9, 1, 2);      /* Europe */
    add_city(wm, "Paris", 48.8, 2.3, 2.1, 0, 2);
    add_city(wm, "Lagos", 6.5, 3.3, 14.8, 1, 3);        /* Africa */
    add_city(wm, "Johannesburg", -26.2, 28.0, 5.6, 0, 3);
    add_city(wm, "Mumbai", 19.0, 72.8, 20.4, 1, 4);     /* Asia */
    add_city(wm, "Beijing", 39.9, 116.4, 21.5, 1, 4);
    add_city(wm, "Tokyo", 35.6, 139.6, 13.9, 1, 4);
    add_city(wm, "Sydney", -33.8, 151.2, 5.3, 1, 5);    /* Oceania */

    /* ─── Routes ─── */
    add_route(wm, 0, 3, 1); /* NY -> London (Air) */
    add_route(wm, 3, 9, 1); /* London -> Tokyo (Air) */
    add_route(wm, 9, 1, 1); /* Tokyo -> LA (Air) */
    add_route(wm, 1, 0, 1); /* LA -> NY (Air) */
    add_route(wm, 0, 2, 0); /* NY -> Sao Paulo (Sea) */
    add_route(wm, 2, 5, 0); /* Sao Paulo -> Lagos (Sea) */
    add_route(wm, 3, 5, 1); /* London -> Lagos (Air) */
    add_route(wm, 5, 7, 1); /* Lagos -> Mumbai (Air) */
    add_route(wm, 7, 8, 1); /* Mumbai -> Beijing (Air) */
    add_route(wm, 8, 10, 1); /* Beijing -> Sydney (Air) */

    /* Infection will now be seeded upon virus selection via Sidebar interaction */
}

/* ═══════════════════════════════════════════
   UPDATE LOGIC
   ═══════════════════════════════════════════ */

void world_map_update(WorldMap* wm, float dt) {
    int i;
    float alpha = 1.0f / 5.0f;
    float gammaR = 1.0f / 10.0f;
    float mortality = 0.02f;

    wm->globalTimer += dt;
    wm->spreadTimer += dt;

    if (!wm->mouseDragging) {
        wm->globeAzimuth -= dt * 5.0f; /* auto rotate */
    }

    /* SEIR Update */
    for (i = 0; i < wm->countryCount; i++) {
        Country* c = &wm->countries[i];
        float N = c->population;
        if (N < 1.0f) continue;

        float beta = c->infectionRate * gammaR;
        float dS = -beta * c->susceptible * c->infected / N;
        float dE =  beta * c->susceptible * c->infected / N - alpha * c->exposed;
        float dI =  alpha * c->exposed - gammaR * c->infected;
        float dR =  gammaR * c->infected * (1.0f - mortality);
        float dD =  gammaR * c->infected * mortality;

        c->susceptible += dS * dt * 2.0f;
        c->exposed     += dE * dt * 2.0f;
        c->infected    += dI * dt * 2.0f;
        c->recovered   += dR * dt * 2.0f;
        c->dead        += dD * dt * 2.0f;

        if (c->susceptible < 0) c->susceptible = 0;
        if (c->exposed < 0)     c->exposed = 0;
        if (c->infected < 0)    c->infected = 0;
        if (c->recovered < 0)   c->recovered = 0;
        if (c->dead < 0)        c->dead = 0;
    }

    /* Inter-country spread */
    if (wm->spreadTimer > 2.0f) {
        wm->spreadTimer = 0;
        for (i = 0; i < wm->countryCount; i++) {
            Country* c = &wm->countries[i];
            if (c->infected > 10.0f) {
                int target = rand() % wm->countryCount;
                if (target != i && wm->countries[target].susceptible > 100.0f) {
                    float leak = c->infected * 0.01f;
                    wm->countries[target].susceptible -= leak;
                    wm->countries[target].exposed += leak;
                    if (wm->countries[target].susceptible < 0)
                        wm->countries[target].susceptible = 0;
                }
            }
        }
    }

    /* Update routes */
    for (i = 0; i < wm->routeCount; i++) {
        wm->routes[i].animProgress += dt * (wm->routes[i].isAirRoute ? 0.3f : 0.05f);
        if (wm->routes[i].animProgress > 1.0f) wm->routes[i].animProgress = 0.0f;
    }
}

void world_map_mouse_drag(WorldMap* wm, int x, int y) {
    int dx = x - wm->mouseLastX;
    int dy = y - wm->mouseLastY;
    wm->globeAzimuth -= dx * 0.5f;
    wm->globeElevation -= dy * 0.5f;
    
    if (wm->globeElevation > 80.0f) wm->globeElevation = 80.0f;
    if (wm->globeElevation < -80.0f) wm->globeElevation = -80.0f;
    
    wm->mouseLastX = x;
    wm->mouseLastY = y;
}

void world_map_scroll(WorldMap* wm, int delta) {
    wm->globeZoom -= delta * 15.0f;
    if (wm->globeZoom < 150.0f) wm->globeZoom = 150.0f;
    if (wm->globeZoom > 600.0f) wm->globeZoom = 600.0f;
}

void world_map_scroll_ui(WorldMap* wm, int delta) {
    int maxScroll = (wm->countryCount * 65) - 300; 
    if (maxScroll < 0) maxScroll = 0;
    wm->sidebarScrollY -= delta * 40.0f;
    if (wm->sidebarScrollY < 0.0f) wm->sidebarScrollY = 0.0f;
    if (wm->sidebarScrollY > maxScroll) wm->sidebarScrollY = maxScroll;
}

/* ═══════════════════════════════════════════
   3D MATH & RENDERING UTILS
   ═══════════════════════════════════════════ */

static void latlon_to_cartesian(float lat, float lon, float radius, float* x, float* y, float* z) {
    float latRad = lat * DEG_TO_RAD;
    float lonRad = lon * DEG_TO_RAD;
    *x = radius * cosf(latRad) * sinf(lonRad);
    *y = radius * sinf(latRad);
    *z = radius * cosf(latRad) * cosf(lonRad);
}

static void draw_text_centered(float x, float y, const char* text, void* font) {
    int i; float totalW = 0;
    for (i = 0; text[i]; i++) totalW += glutBitmapWidth(font, text[i]);
    glRasterPos2f(x - totalW * 0.5f, y);
    for (i = 0; text[i]; i++) glutBitmapCharacter(font, text[i]);
}

static void draw_text_multiline_centered(float x, float y, const char* text, void* font, float lineSpacing) {
    char buf[512];
    strncpy(buf, text, 511);
    buf[511] = '\0';
    char* line = strtok(buf, "\n");
    float currentY = y;
    while(line) {
        draw_text_centered(x, currentY, line, font);
        currentY -= lineSpacing;
        line = strtok(NULL, "\n");
    }
}

static void draw_text(float x, float y, const char* text, void* font) {
    int i;
    glRasterPos2f(x, y);
    for (i = 0; text[i]; i++) glutBitmapCharacter(font, text[i]);
}

/* Get infection severity color (green → yellow → red) */
static void infection_color(const Country* c, float* cr, float* cg, float* cb) {
    float ratio;
    if (c->population < 1.0f) { *cr = c->r; *cg = c->g; *cb = c->b; return; }
    ratio = (c->infected + c->exposed) / c->population;
    if (ratio > 1.0f) ratio = 1.0f;

    if (ratio < 0.001f) {
        *cr = c->r; *cg = c->g; *cb = c->b;
    } else if (ratio < 0.05f) {
        float t = ratio / 0.05f;
        *cr = c->r * (1-t) + 0.7f * t;
        *cg = c->g * (1-t) + 0.7f * t;
        *cb = c->b * (1-t) + 0.1f * t;
    } else if (ratio < 0.2f) {
        float t = (ratio - 0.05f) / 0.15f;
        *cr = 0.7f * (1-t) + 0.9f * t;
        *cg = 0.7f * (1-t) + 0.4f * t;
        *cb = 0.1f * (1-t) + 0.1f * t;
    } else {
        float t = (ratio - 0.2f) / 0.8f;
        if (t > 1.0f) t = 1.0f;
        *cr = 0.9f * (1-t) + 0.8f * t;
        *cg = 0.4f * (1-t) + 0.1f * t;
        *cb = 0.1f * (1-t) + 0.1f * t;
    }
}

/* ═══════════════════════════════════════════
   MAIN 3D GLOBE RENDERER
   ═══════════════════════════════════════════ */

void render_world_map(WorldMap* wm, float gameTime) {
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    int i, j;
    GLUquadric* quad;
    GLfloat lightPos[] = { 1.0f, 0.5f, -1.0f, 0.0f }; /* directional light from right */
    
    /* Setup 3D Camera */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 1000.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    /* Background space dark blue */
    glClearColor(0.01f, 0.02f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    /* Move camera away */
    glTranslatef(0.0f, 0.0f, -wm->globeZoom);
    
    /* Rotate globe based on input */
    glRotatef(wm->globeElevation, 1.0f, 0.0f, 0.0f);
    glRotatef(wm->globeAzimuth, 0.0f, 1.0f, 0.0f);

    /* ─── 1. Ocean/Earth Sphere ─── */
    glColor3f(1.0f, 1.0f, 1.0f);
    quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);
    
    if (earthTextureID) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, earthTextureID);
        gluQuadricTexture(quad, GL_TRUE);
        /* Orient the texture correctly so North is up, and Prime Meridian matches */
        glPushMatrix();
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); /* gluSphere standard poles are Z */
        glRotatef(90.0f, 0.0f, 0.0f, 1.0f);  /* Adjust prime meridian alignment if needed depending on texture */
        gluSphere(quad, GLOBE_RADIUS, GLOBE_SLICES, GLOBE_STACKS);
        glPopMatrix();
        glDisable(GL_TEXTURE_2D);
    } else {
        glColor3f(0.04f, 0.1f, 0.25f);
        gluSphere(quad, GLOBE_RADIUS, GLOBE_SLICES, GLOBE_STACKS);
    }
    
    /* Polygons removed, globe remains as aesthetic backdrop */

    glEnable(GL_LIGHTING);

    /* ─── 3. Flight & Ship Routes ─── */
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    for (i = 0; i < wm->routeCount; i++) {
        FlightRoute* r = &wm->routes[i];
        CityNode* cA = &wm->cities[r->fromCity];
        CityNode* cB = &wm->cities[r->toCity];
        float xA, yA, zA, xB, yB, zB;
        float arcHeight = r->isAirRoute ? 15.0f : 1.0f;
        int segs = 20;

        latlon_to_cartesian(cA->lat, cA->lon, GLOBE_RADIUS, &xA, &yA, &zA);
        latlon_to_cartesian(cB->lat, cB->lon, GLOBE_RADIUS, &xB, &yB, &zB);
        
        /* Draw route arc */
        if (r->isAirRoute) glColor4f(0.8f, 0.8f, 1.0f, 0.3f);
        else glColor4f(0.2f, 0.8f, 0.8f, 0.4f);
        
        glBegin(GL_LINE_STRIP);
        for (j = 0; j <= segs; j++) {
            float t = (float)j / segs;
            float rT = GLOBE_RADIUS + arcHeight * sinf(t * M_PI);
            float lat = cA->lat + (cB->lat - cA->lat) * t;
            float lon = cA->lon + (cB->lon - cA->lon) * t;
            float px, py, pz;
            latlon_to_cartesian(lat, lon, rT, &px, &py, &pz);
            glVertex3f(px, py, pz);
        }
        glEnd();
        
        /* Draw animated dot */
        {
            float t = r->animProgress;
            float rT = GLOBE_RADIUS + arcHeight * sinf(t * M_PI);
            float lat = cA->lat + (cB->lat - cA->lat) * t;
            float lon = cA->lon + (cB->lon - cA->lon) * t;
            float px, py, pz;
            latlon_to_cartesian(lat, lon, rT, &px, &py, &pz);
            
            glPointSize(r->isAirRoute ? 3.0f : 2.0f);
            if (r->isAirRoute) glColor3f(1.0f, 1.0f, 1.0f);
            else glColor3f(0.4f, 1.0f, 1.0f);
            glBegin(GL_POINTS);
            glVertex3f(px, py, pz);
            glEnd();
            glPointSize(1.0f);
        }
    }

    /* ─── 4. Cities ─── */
    for (i = 0; i < wm->cityCount; i++) {
        CityNode* c = &wm->cities[i];
        float cx, cy, cz;
        latlon_to_cartesian(c->lat, c->lon, GLOBE_RADIUS + 0.8f, &cx, &cy, &cz);
        
        glPointSize(c->isSuperSpreadHub ? 6.0f : 3.0f);
        if (c->isSuperSpreadHub) {
            float pulse = 0.5f + 0.5f * sinf(gameTime * 4.0f);
            glColor4f(1.0f, 0.2f, 0.2f, pulse);
        } else {
            glColor4f(0.8f, 0.8f, 0.8f, 0.8f);
        }
        glBegin(GL_POINTS);
        glVertex3f(cx, cy, cz);
        glEnd();
    }

    /* ─── 5. Atmospheric Glow ─── */
    glEnable(GL_LIGHTING);
    glCullFace(GL_FRONT); /* Render back faces for rim effect */
    glColor4f(0.3f, 0.5f, 0.9f, 0.15f);
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluSphere(quad, GLOBE_RADIUS * GLOBE_ATM_SCALE, GLOBE_SLICES, GLOBE_STACKS);
    glCullFace(GL_BACK);
    
    gluDeleteQuadric(quad);

    /* ═══════════════════════════════════════════
       2D HUD OVERLAYS
       ═══════════════════════════════════════════ */
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    /* Title Window */
    {
        float panelH = 55;
        glColor4f(0.04f, 0.05f, 0.10f, 0.85f);
        glBegin(GL_QUADS);
        glVertex2f(0, h - panelH); glVertex2f(w, h - panelH);
        glVertex2f(w, h); glVertex2f(0, h);
        glEnd();

        glColor3f(1.0f, 1.0f, 1.0f);
        draw_text_centered(w * 0.5f, h - 30, "S E L E C T   A   C O U N T R Y", GLUT_BITMAP_TIMES_ROMAN_24);
        glColor3f(0.5f, 0.6f, 0.8f);
        draw_text_centered(w * 0.5f, h - 48, "Drag to rotate globe. Scroll to zoom. Click country to mutate.", GLUT_BITMAP_HELVETICA_12);
    }

    /* Stats bar */
    {
        float panelH = 40;
        float totalPop = 0, totalInf = 0, totalDead = 0, totalRec = 0;
        char buf[128];
        for (i = 0; i < wm->countryCount; i++) {
            totalPop  += wm->countries[i].population;
            totalInf  += wm->countries[i].infected;
            totalDead += wm->countries[i].dead;
            totalRec  += wm->countries[i].recovered;
        }

        glColor4f(0.04f, 0.04f, 0.08f, 0.85f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0); glVertex2f(w, 0);
        glVertex2f(w, panelH); glVertex2f(0, panelH);
        glEnd();

        glColor3f(0.7f, 0.7f, 0.8f);
        snprintf(buf, 128, "GLOBAL  |  Population: %.0f  |  Infected: %.0f  |  Recovered: %.0f  |  Deaths: %.0f",
                 totalPop, totalInf, totalRec, totalDead);
        draw_text_centered(w * 0.5f, 14, buf, GLUT_BITMAP_HELVETICA_12);
    }

    /* Navigation */
    glColor4f(0.4f, 0.7f, 0.9f, 0.8f);
    draw_text(15, 50, "ESC: Back to Menu  |  Q: Quit", GLUT_BITMAP_HELVETICA_10);

    /* ─── Scrolling Sidebar ─── */
    {
        float sidebarW = 280;
        float sidebarX = w - sidebarW;
        
        /* Sidebar background */
        glColor4f(0.04f, 0.04f, 0.08f, 0.85f);
        glBegin(GL_QUADS);
        glVertex2f(sidebarX, 0); glVertex2f(w, 0);
        glVertex2f(w, h); glVertex2f(sidebarX, h);
        glEnd();
        
        glEnable(GL_SCISSOR_TEST);
        glScissor(sidebarX, 0, sidebarW, h);
        
        float itemH = 60;
        float currentY = h - 70 + wm->sidebarScrollY;
        
        for (i = 0; i < wm->countryCount; i++) {
            Country* c = &wm->countries[i];
            
            if (wm->hoveredCountry == i) {
                glColor4f(0.2f, 0.4f, 0.8f, 0.5f);
            } else {
                glColor4f(0.1f, 0.1f, 0.15f, 0.5f);
            }
            glBegin(GL_QUADS);
            glVertex2f(sidebarX + 5, currentY - itemH); glVertex2f(w - 5, currentY - itemH);
            glVertex2f(w - 5, currentY); glVertex2f(sidebarX + 5, currentY);
            glEnd();
            
            /* Selection Glow Outline */
            if (wm->hoveredCountry == i) {
                glColor4f(0.4f, 0.7f, 1.0f, 0.8f);
                glLineWidth(2.0f);
                glBegin(GL_LINE_LOOP);
                glVertex2f(sidebarX + 5, currentY - itemH); glVertex2f(w - 5, currentY - itemH);
                glVertex2f(w - 5, currentY); glVertex2f(sidebarX + 5, currentY);
                glEnd();
                glLineWidth(1.0f);
            }
            
            glColor3f(1.0f, 1.0f, 1.0f);
            draw_text(sidebarX + 15, currentY - 20, c->name, GLUT_BITMAP_HELVETICA_12);
            
            char buf[128];
            glColor3f(0.7f, 0.7f, 0.7f);
            snprintf(buf, 128, "Pop: %.0f  |  Inf: %.0f", c->population, c->infected);
            draw_text(sidebarX + 15, currentY - 40, buf, GLUT_BITMAP_HELVETICA_10);
            
            snprintf(buf, 128, "Dead: %.0f", c->dead);
            glColor3f(1.0f, 0.4f, 0.4f);
            draw_text(sidebarX + 15, currentY - 55, buf, GLUT_BITMAP_HELVETICA_10);
            
            currentY -= (itemH + 5);
        }
        
        glDisable(GL_SCISSOR_TEST);
        
        /* Sidebar Title Overlay */
        glColor4f(0.04f, 0.05f, 0.12f, 0.95f);
        glBegin(GL_QUADS);
        glVertex2f(sidebarX, h - 50); glVertex2f(w, h - 50);
        glVertex2f(w, h); glVertex2f(sidebarX, h);
        glEnd();
        
        glColor3f(1.0f, 1.0f, 1.0f);
        draw_text_centered(sidebarX + (sidebarW / 2), h - 30, "START LOCATIONS", GLUT_BITMAP_HELVETICA_18);
        
        glColor4f(0.3f, 0.5f, 0.8f, 0.8f);
        glBegin(GL_LINES);
        glVertex2f(sidebarX, h - 50); glVertex2f(w, h - 50);
        glEnd();
    }

    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW); glPopMatrix();
}

/* ═══════════════════════════════════════════
   GLOBE HIT TEST (3D to Lat/Lon Raycast)
   ═══════════════════════════════════════════ */

int world_map_hit_test(const WorldMap* wm, int mouseX, int mouseY) {
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    float sidebarW = 280;
    float sidebarX = w - sidebarW;
    float glY = h - mouseY;

    if (mouseX >= sidebarX && mouseX <= w) {
        float itemH = 60;
        float currentY = h - 70 + wm->sidebarScrollY;
        
        for (int i = 0; i < wm->countryCount; i++) {
            if (mouseX >= sidebarX + 5 && mouseX <= w - 5 &&
                glY >= currentY - itemH && glY <= currentY) {
                return i;
            }
            currentY -= (itemH + 5);
        }
    }
    return -1;
}

/* ═══════════════════════════════════════════
   VIRUS SELECTION OVERLAY
   ═══════════════════════════════════════════ */

void render_virus_select(WorldMap* wm, float gameTime) {
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    int i;
    
    /* Draw map behind */
    render_world_map(wm, gameTime);
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    
    /* Dark overlay */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(w, 0);
    glVertex2f(w, h); glVertex2f(0, h);
    glEnd();
    
    glColor3f(1.0f, 1.0f, 1.0f);
    draw_text_centered(w/2, h - 100, "CHOOSE VIRUS MUTATION", GLUT_BITMAP_TIMES_ROMAN_24);
    
    /* Draw 3 cards */
    const char* titles[] = { "STEALTH MUTATION", "AGGRESSIVE STRAIN", "ENVIRONMENTAL ADAPTATION" };
    const char* desc[] = { "Low initial detection. Spreads slowly\nbut deeply infiltrates the population.",
                           "Extremely contagious and lethal.\nRapid spread, fast detection.",
                           "Adapts to any climate. Moderate\nspread but highly resilient." };
    Color4f colors[] = { {0.4f, 0.8f, 1.0f, 1.0f}, {1.0f, 0.3f, 0.3f, 1.0f}, {0.3f, 0.9f, 0.4f, 1.0f} };
    
    float cardW = 300;
    float cardH = 400;
    float startX = w/2 - (cardW*1.5f + 40);
    
    for (i = 0; i < 3; i++) {
        float cx = startX + i * (cardW + 40);
        float cy = h/2 - cardH/2;
        
        float alpha = (wm->hoveredVirus == i) ? 0.9f : 0.6f;
        float lift = (wm->hoveredVirus == i) ? 10.0f : 0.0f;
        cy += lift;
        
        glColor4f(0.1f, 0.15f, 0.2f, alpha);
        glBegin(GL_QUADS);
        glVertex2f(cx, cy); glVertex2f(cx+cardW, cy);
        glVertex2f(cx+cardW, cy+cardH); glVertex2f(cx, cy+cardH);
        glEnd();
        
        glColor4f(colors[i].r, colors[i].g, colors[i].b, 1.0f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(cx, cy); glVertex2f(cx+cardW, cy);
        glVertex2f(cx+cardW, cy+cardH); glVertex2f(cx, cy+cardH);
        glEnd();
        glLineWidth(1.0f);
        
        draw_text_centered(cx + cardW/2, cy + cardH - 50, titles[i], GLUT_BITMAP_HELVETICA_18);
        draw_text_multiline_centered(cx + cardW/2, cy + cardH - 90, desc[i], GLUT_BITMAP_HELVETICA_12, 16.0f);
        
        if (wm->hoveredVirus == i) {
            draw_text_centered(cx + cardW/2, cy + 50, "CLICK TO DEPLOY", GLUT_BITMAP_HELVETICA_18);
        }
    }
    
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW); glPopMatrix();
}

int virus_select_hit_test(const WorldMap* wm, int mouseX, int mouseY) {
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    float cardW = 300, cardH = 400;
    float startX = w/2 - (cardW*1.5f + 40);
    int i;
    mouseY = h - mouseY; /* flip Y */
    
    for (i = 0; i < 3; i++) {
        float cx = startX + i * (cardW + 40);
        float cy = h/2 - cardH/2;
        
        if (mouseX >= cx && mouseX <= cx + cardW &&
            mouseY >= cy && mouseY <= cy + cardH) {
            return i;
        }
    }
    return -1;
}
