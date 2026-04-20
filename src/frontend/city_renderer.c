#include "city_renderer.h"
#include <GL/glut.h>
#include <math.h>
#include <string.h>

/* Distinct ground colors per region — high contrast */
static const float regionColors[5][3] = {
    {0.22f, 0.45f, 0.25f},   /* North: deep green    */
    {0.30f, 0.48f, 0.28f},   /* Central: meadow      */
    {0.38f, 0.42f, 0.22f},   /* South: olive brown   */
    {0.25f, 0.50f, 0.35f},   /* East: teal           */
    {0.40f, 0.44f, 0.30f},   /* Capital: sandy green */
};

/* Lighter border colors per region */
static const float borderColors[5][3] = {
    {0.35f, 0.60f, 0.38f},
    {0.45f, 0.62f, 0.40f},
    {0.50f, 0.55f, 0.32f},
    {0.38f, 0.65f, 0.48f},
    {0.55f, 0.58f, 0.42f},
};

static void draw_ground(const GameWorld* world) {
    int r;

    /* Global base ground */
    glPushMatrix();
    glColor3f(0.18f, 0.35f, 0.20f); /* Darker grassy green */
    glTranslatef(0.0f, -0.6f, 0.0f);
    glScalef(1500.0f, 0.5f, 1500.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    for (r = 0; r < world->regionCount; r++) {
        const Region* reg = &world->regions[r];
        float hx = reg->size.x * 0.5f;
        float hz = reg->size.z * 0.5f;

        /* Main ground quad (slightly above y=0 for depth clarity) */
        glPushMatrix();
        glColor3f(regionColors[r][0], regionColors[r][1], regionColors[r][2]);
        glTranslatef(reg->center.x, -0.3f, reg->center.z);
        glScalef(reg->size.x, 0.6f, reg->size.z);
        glutSolidCube(1.0f);
        glPopMatrix();

        /* Decorative edge strip (lighter border ring) */
        glDisable(GL_LIGHTING);
        glLineWidth(2.5f);
        glColor3f(borderColors[r][0], borderColors[r][1], borderColors[r][2]);
        glBegin(GL_LINE_LOOP);
        glVertex3f(reg->center.x - hx, 0.05f, reg->center.z - hz);
        glVertex3f(reg->center.x + hx, 0.05f, reg->center.z - hz);
        glVertex3f(reg->center.x + hx, 0.05f, reg->center.z + hz);
        glVertex3f(reg->center.x - hx, 0.05f, reg->center.z + hz);
        glEnd();
        glLineWidth(1.0f);
        glEnable(GL_LIGHTING);

        /* Road grid lines inside region */
        glDisable(GL_LIGHTING);
        glColor4f(0.18f, 0.28f, 0.18f, 0.4f);
        glLineWidth(1.0f);
        {
            float step = reg->size.x / 5.0f;
            int i;
            for (i = 1; i < 5; i++) {
                float lx = reg->center.x - hx + step * i;
                glBegin(GL_LINES);
                glVertex3f(lx, 0.06f, reg->center.z - hz);
                glVertex3f(lx, 0.06f, reg->center.z + hz);
                glEnd();
            }
            step = reg->size.z / 5.0f;
            for (i = 1; i < 5; i++) {
                float lz = reg->center.z - hz + step * i;
                glBegin(GL_LINES);
                glVertex3f(reg->center.x - hx, 0.06f, lz);
                glVertex3f(reg->center.x + hx, 0.06f, lz);
                glEnd();
            }
        }
        glEnable(GL_LIGHTING);
    }

    /* Region name labels */
    glDisable(GL_LIGHTING);
    glColor3f(0.92f, 0.92f, 0.96f);
    for (r = 0; r < world->regionCount; r++) {
        const Region* reg = &world->regions[r];
        int c;
        float textWidth = 0;
        for (c = 0; reg->name[c]; c++)
            textWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, reg->name[c]);
        glRasterPos3f(reg->center.x - textWidth * 0.04f, 14.0f, reg->center.z);
        for (c = 0; reg->name[c] != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, reg->name[c]);
        }
    }
    glEnable(GL_LIGHTING);
}

static void draw_building(const Building* b) {
    glPushMatrix();
    glTranslatef(b->position.x, 0.0f, b->position.z);

    switch (b->type) {
        case BUILDING_HOUSE:
            /* Different types of houses based on ID */
            int variant = b->id % 4;
            if (variant == 0) {
                glColor3f(0.75f, 0.62f, 0.45f);
                glPushMatrix(); glTranslatef(0, 1.8f, 0); glScalef(2.5f, 3.6f, 2.5f); glutSolidCube(1.0f); glPopMatrix();
                glColor3f(0.48f, 0.25f, 0.12f); /* Roof */
                glPushMatrix(); glTranslatef(0, 3.8f, 0); glScalef(2.8f, 0.6f, 2.8f); glutSolidCube(1.0f); glPopMatrix();
                glColor3f(0.35f, 0.20f, 0.10f); /* Door */
                glPushMatrix(); glTranslatef(0, 0.6f, 1.28f); glScalef(0.6f, 1.2f, 0.1f); glutSolidCube(1.0f); glPopMatrix();
            } else if (variant == 1) {
                glColor3f(0.55f, 0.65f, 0.60f); /* Modern */
                glPushMatrix(); glTranslatef(0, 2.5f, 0); glScalef(3.0f, 5.0f, 2.0f); glutSolidCube(1.0f); glPopMatrix();
                glColor3f(0.25f, 0.25f, 0.30f); /* Roof */
                glPushMatrix(); glTranslatef(0, 5.2f, 0); glScalef(3.2f, 0.4f, 2.2f); glutSolidCube(1.0f); glPopMatrix();
            } else if (variant == 2) {
                glColor3f(0.85f, 0.82f, 0.75f); /* Office/Apartment */
                glPushMatrix(); glTranslatef(0, 4.0f, 0); glScalef(2.0f, 8.0f, 2.0f); glutSolidCube(1.0f); glPopMatrix();
                glColor3f(0.35f, 0.50f, 0.65f); /* Glass */
                glPushMatrix(); glTranslatef(0, 4.0f, 1.05f); glScalef(1.5f, 7.0f, 0.1f); glutSolidCube(1.0f); glPopMatrix();
            } else {
                glColor3f(0.65f, 0.45f, 0.45f); /* Brick warehouse */
                glPushMatrix(); glTranslatef(0, 1.5f, 0); glScalef(4.0f, 3.0f, 4.0f); glutSolidCube(1.0f); glPopMatrix();
            }
            break;

        case BUILDING_HOSPITAL:
            /* Main body — white with red accent */
            glColor3f(0.92f, 0.92f, 0.92f);
            glPushMatrix();
            glTranslatef(0, 4.0f, 0);
            glScalef(7.0f, 8.0f, 7.0f);
            glutSolidCube(1.0f);
            glPopMatrix();
            /* Red stripe band */
            glColor3f(0.85f, 0.15f, 0.15f);
            glPushMatrix();
            glTranslatef(0, 7.5f, 0);
            glScalef(7.2f, 1.0f, 7.2f);
            glutSolidCube(1.0f);
            glPopMatrix();
            /* Red cross — horizontal */
            glColor3f(0.9f, 0.1f, 0.1f);
            glPushMatrix();
            glTranslatef(0, 8.5f, 0);
            glScalef(3.5f, 0.5f, 0.8f);
            glutSolidCube(1.0f);
            glPopMatrix();
            /* Red cross — vertical */
            glPushMatrix();
            glTranslatef(0, 8.5f, 0);
            glScalef(0.8f, 0.5f, 3.5f);
            glutSolidCube(1.0f);
            glPopMatrix();
            break;

        case BUILDING_SCHOOL:
            /* Main body — blue */
            glColor3f(0.25f, 0.40f, 0.80f);
            glPushMatrix();
            glTranslatef(0, 3.0f, 0);
            glScalef(8.0f, 6.0f, 6.0f);
            glutSolidCube(1.0f);
            glPopMatrix();
            /* Lighter upper section */
            glColor3f(0.35f, 0.50f, 0.88f);
            glPushMatrix();
            glTranslatef(0, 6.3f, 0);
            glScalef(8.3f, 0.8f, 6.3f);
            glutSolidCube(1.0f);
            glPopMatrix();
            /* Flag pole */
            glColor3f(0.6f, 0.6f, 0.6f);
            glPushMatrix();
            glTranslatef(3.5f, 8.0f, 0);
            glScalef(0.15f, 4.0f, 0.15f);
            glutSolidCube(1.0f);
            glPopMatrix();
            /* Flag */
            glColor3f(0.9f, 0.2f, 0.2f);
            glPushMatrix();
            glTranslatef(4.3f, 9.5f, 0);
            glScalef(1.5f, 0.8f, 0.05f);
            glutSolidCube(1.0f);
            glPopMatrix();
            break;
    }

    glPopMatrix();
}

/* Road connections for 3x3 grid * 9 regions */
static const int roadPairs[][2] = {
    {0, 1}, {1, 2},
    {3, 4}, {4, 5},
    {6, 7}, {7, 8},
    {0, 3}, {1, 4}, {2, 5},
    {3, 6}, {4, 7}, {5, 8}
};
#define NUM_ROADS 12

static void draw_roads(const GameWorld* world) {
    int r;
    float roadWidth = 3.5f;

    glDisable(GL_LIGHTING);

    for (r = 0; r < NUM_ROADS; r++) {
        int a = roadPairs[r][0];
        int b = roadPairs[r][1];
        const Region* ra = &world->regions[a];
        const Region* rb = &world->regions[b];

        float ax = ra->center.x, az = ra->center.z;
        float bx = rb->center.x, bz = rb->center.z;

        /* Direction vector */
        float dx = bx - ax;
        float dz = bz - az;
        float len = (float)sqrt(dx * dx + dz * dz);
        if (len < 0.001f) continue;
        float nx = -dz / len * roadWidth;
        float nz =  dx / len * roadWidth;

        /* Road quad (gray asphalt) */
        glColor4f(0.28f, 0.28f, 0.30f, 0.85f);
        glBegin(GL_QUADS);
        glVertex3f(ax + nx, 0.02f, az + nz);
        glVertex3f(ax - nx, 0.02f, az - nz);
        glVertex3f(bx - nx, 0.02f, bz - nz);
        glVertex3f(bx + nx, 0.02f, bz + nz);
        glEnd();

        /* Road edges */
        glColor4f(0.45f, 0.45f, 0.48f, 0.6f);
        glLineWidth(1.5f);
        glBegin(GL_LINES);
        glVertex3f(ax + nx, 0.04f, az + nz);
        glVertex3f(bx + nx, 0.04f, bz + nz);
        glVertex3f(ax - nx, 0.04f, az - nz);
        glVertex3f(bx - nx, 0.04f, bz - nz);
        glEnd();

        /* Center dashed line (yellow) */
        glColor4f(0.85f, 0.75f, 0.20f, 0.7f);
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(3, 0xF0F0);
        glBegin(GL_LINES);
        glVertex3f(ax, 0.05f, az);
        glVertex3f(bx, 0.05f, bz);
        glEnd();
        glDisable(GL_LINE_STIPPLE);
        glLineWidth(1.0f);
    }

    glEnable(GL_LIGHTING);
}

static void draw_vehicles(const GameWorld* world) {
    int v;
    int isNight = (world->state.currentHour < 6 || world->state.currentHour > 18);

    for (v = 0; v < world->vehicleCount; v++) {
        const Vehicle* veh = &world->vehicles[v];
        
        glPushMatrix();
        glTranslatef(veh->position.x, veh->position.y, veh->position.z);
        
        /* Compute rotation so car points towards target */
        float dx = veh->targetPos.x - veh->startPos.x;
        float dz = veh->targetPos.z - veh->startPos.z;
        float angle = atan2f(dx, dz) * 180.0f / 3.14159f;
        glRotatef(angle, 0.0f, 1.0f, 0.0f);
        
        /* Body */
        glColor3f(veh->r, veh->g, veh->b);
        glPushMatrix();
        glTranslatef(0.0f, 0.6f, 0.0f);
        glScalef(1.2f, 0.8f, 2.4f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        /* Cabin */
        glColor3f(0.8f, 0.8f, 0.9f); /* Windows */
        glPushMatrix();
        glTranslatef(0.0f, 1.2f, -0.2f);
        glScalef(1.0f, 0.6f, 1.2f);
        glutSolidCube(1.0f);
        glPopMatrix();

        if (isNight) {
            glDisable(GL_LIGHTING);
            /* Headlights */
            glColor3f(1.0f, 0.95f, 0.8f);
            glPushMatrix();
            glTranslatef(-0.4f, 0.6f, 1.21f);
            glutSolidSphere(0.15f, 8, 8);
            glPopMatrix();
            glPushMatrix();
            glTranslatef(0.4f, 0.6f, 1.21f);
            glutSolidSphere(0.15f, 8, 8);
            glPopMatrix();
            
            /* Taillights */
            glColor3f(1.0f, 0.1f, 0.1f);
            glPushMatrix();
            glTranslatef(-0.4f, 0.6f, -1.21f);
            glutSolidSphere(0.15f, 8, 8);
            glPopMatrix();
            glPushMatrix();
            glTranslatef(0.4f, 0.6f, -1.21f);
            glutSolidSphere(0.15f, 8, 8);
            glPopMatrix();
            glEnable(GL_LIGHTING);
        }

        glPopMatrix();
    }
}

void draw_city(const GameWorld* world) {
    int i;
    draw_roads(world);
    draw_ground(world);
    for (i = 0; i < world->buildingCount; i++) {
        draw_building(&world->buildings[i]);
    }
    draw_vehicles(world);
}
