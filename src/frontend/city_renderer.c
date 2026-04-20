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
            /* Main house body — larger, warmer color */
            glColor3f(0.75f, 0.62f, 0.45f);
            glPushMatrix();
            glTranslatef(0, 1.8f, 0);
            glScalef(2.5f, 3.6f, 2.5f);
            glutSolidCube(1.0f);
            glPopMatrix();
            /* Dark roof */
            glColor3f(0.48f, 0.25f, 0.12f);
            glPushMatrix();
            glTranslatef(0, 3.8f, 0);
            glScalef(2.8f, 0.6f, 2.8f);
            glutSolidCube(1.0f);
            glPopMatrix();
            /* Door */
            glColor3f(0.35f, 0.20f, 0.10f);
            glPushMatrix();
            glTranslatef(0, 0.6f, 1.28f);
            glScalef(0.6f, 1.2f, 0.1f);
            glutSolidCube(1.0f);
            glPopMatrix();
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

void draw_city(const GameWorld* world) {
    int i;
    draw_ground(world);
    for (i = 0; i < world->buildingCount; i++) {
        draw_building(&world->buildings[i]);
    }
}
