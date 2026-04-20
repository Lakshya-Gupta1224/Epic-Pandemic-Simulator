#include "renderer.h"
#include "city_renderer.h"
#include "entity_renderer.h"
#include <GL/glut.h>
#include <math.h>

static int winW, winH;

void render_init(int w, int h) {
    winW = w; winH = h;

    glClearColor(0.08f, 0.09f, 0.13f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    /* Main light */
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    {
        /* Key light (warm white from above-left) */
        GLfloat ambient0[]  = {0.35f, 0.35f, 0.38f, 1.0f};
        GLfloat diffuse0[]  = {0.95f, 0.92f, 0.85f, 1.0f};
        GLfloat lightPos0[] = {-200.0f, 500.0f, -150.0f, 1.0f};
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);

        /* Fill light (cool blue from opposite side) */
        GLfloat ambient1[]  = {0.08f, 0.10f, 0.15f, 1.0f};
        GLfloat diffuse1[]  = {0.25f, 0.30f, 0.40f, 1.0f};
        GLfloat lightPos1[] = {200.0f, 300.0f, 200.0f, 1.0f};
        glLightfv(GL_LIGHT1, GL_AMBIENT, ambient1);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse1);
        glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);
    }

    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Fog for depth cue */
    {
        GLfloat fogColor[] = {0.08f, 0.09f, 0.13f, 1.0f};
        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE, GL_LINEAR);
        glFogfv(GL_FOG_COLOR, fogColor);
        glFogf(GL_FOG_START, 400.0f);
        glFogf(GL_FOG_END, 1200.0f);
    }

    render_resize(w, h);
}

void render_resize(int w, int h) {
    winW = w; winH = h;
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(55.0, (double)w / (double)h, 1.0, 3000.0);
    glMatrixMode(GL_MODELVIEW);
}

static void draw_sun_and_moon(const GameWorld* world) {
    float hourNorm = world->state.currentHour / 24.0f;
    /* Angle: 12 noon = top (90 deg), 0 midnight = bottom (-90 deg) */
    float angle = (hourNorm * 2.0f * 3.14159f) - 1.5708f; /* shift by 90 degrees */
    float radius = 900.0f;

    float sx = cosf(angle) * radius;
    float sy = sinf(angle) * radius;
    float mx = cosf(angle + 3.14159f) * radius;
    float my = sinf(angle + 3.14159f) * radius;

    glDisable(GL_LIGHTING);

    /* Draw Sun */
    if (sy > -100.0f) {
        glPushMatrix();
        glTranslatef(sx, sy, -400.0f);
        glColor3f(1.0f, 0.95f, 0.7f);
        glutSolidSphere(60.0f, 24, 24);
        /* Sun Glow */
        glEnable(GL_BLEND);
        glColor4f(1.0f, 0.8f, 0.2f, 0.3f);
        glutSolidSphere(85.0f, 16, 16);
        glDisable(GL_BLEND);
        glPopMatrix();
    }

    /* Draw Moon */
    if (my > -100.0f) {
        glPushMatrix();
        glTranslatef(mx, my, -400.0f);
        glColor3f(0.8f, 0.85f, 0.95f);
        glutSolidSphere(40.0f, 16, 16);
        glPopMatrix();
    }

    glEnable(GL_LIGHTING);
}

void render_scene(const GameWorld* world) {
    draw_sun_and_moon(world);
    draw_city(world);
    draw_entities(world);
}

void update_lighting(int currentHour) {
    float hourNorm = currentHour / 24.0f;
    float intensity;

    if (hourNorm < 0.25f || hourNorm > 0.75f) {
        intensity = 0.30f;
    } else if (hourNorm < 0.35f) {
        intensity = 0.30f + (hourNorm - 0.25f) * 7.0f;
    } else if (hourNorm > 0.65f) {
        intensity = 0.30f + (0.75f - hourNorm) * 7.0f;
    } else {
        intensity = 1.0f;
    }

    {
        GLfloat ambient[] = {
            0.20f * intensity + 0.10f,
            0.20f * intensity + 0.08f,
            0.25f * intensity + 0.12f,
            1.0f
        };
        GLfloat diffuse[] = {
            0.95f * intensity,
            0.90f * intensity,
            0.80f * intensity,
            1.0f
        };
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    }

    {
        float skyR = 0.06f + 0.10f * intensity;
        float skyG = 0.07f + 0.12f * intensity;
        float skyB = 0.10f + 0.18f * intensity;
        GLfloat fogColor[] = {skyR, skyG, skyB, 1.0f};
        glClearColor(skyR, skyG, skyB, 1.0f);
        glFogfv(GL_FOG_COLOR, fogColor);
    }
}
