#include "entity_renderer.h"
#include <GL/glut.h>
#include <math.h>

/* SEIR state color mapping — more vibrant */
static const float stateColors[5][3] = {
    {0.35f, 0.75f, 1.00f},   /* Susceptible: bright cyan-blue  */
    {1.00f, 0.70f, 0.10f},   /* Exposed:     vivid orange      */
    {0.96f, 0.12f, 0.20f},   /* Infected:    bright red        */
    {0.10f, 0.85f, 0.45f},   /* Recovered:   bright green      */
    {0.25f, 0.25f, 0.28f},   /* Dead:        dark gray         */
};

static void draw_person(const Person* p) {
    float alpha;
    float radius;

    /* Hide dead persons that finished ascending */
    if (p->state == STATE_DEAD && p->moveProgress >= 1.0f) return;

    glPushMatrix();
    glTranslatef(p->position.x, p->position.y, p->position.z);

    /* Dead persons fade out */
    alpha = 1.0f;
    if (p->state == STATE_DEAD) {
        alpha = 1.0f - p->moveProgress;
    }

    glColor4f(stateColors[p->state][0],
              stateColors[p->state][1],
              stateColors[p->state][2],
              alpha);

    if (p->type == PERSON_CHILD) {
        /* Children: smaller sphere */
        radius = 0.6f;
        glutSolidSphere(radius, 8, 8);
    } else {
        /* Adults: larger sphere with better detail */
        radius = 0.9f;
        glutSolidSphere(radius, 10, 10);
        /* Small "head" on top to distinguish from children */
        glTranslatef(0, radius * 0.9f, 0);
        glutSolidSphere(radius * 0.45f, 8, 8);
    }

    glPopMatrix();
}

void draw_entities(const GameWorld* world) {
    int i;
    for (i = 0; i < world->personCount; i++) {
        draw_person(&world->persons[i]);
    }
}
