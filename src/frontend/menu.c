#include "menu.h"
#include "hud.h"
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void draw_centered_text(float y, const char* text, void* font) {
    int i;
    float totalWidth = 0;
    float x;
    int w = glutGet(GLUT_WINDOW_WIDTH);

    for (i = 0; text[i]; i++)
        totalWidth += glutBitmapWidth(font, text[i]);
    x = (w - totalWidth) / 2.0f;
    glRasterPos2f(x, y);
    for (i = 0; text[i]; i++)
        glutBitmapCharacter(font, text[i]);
}

/* ═══════════════════════════════════════════
   MENU SCREEN
   ═══════════════════════════════════════════ */
void render_menu(float gameTime) {
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    float titleY, pulse;
    int i;

    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    glDisable(GL_DEPTH_TEST); glDisable(GL_LIGHTING);

    /* Gradient background */
    glBegin(GL_QUADS);
    glColor3f(0.04f, 0.04f, 0.08f); glVertex2f(0, 0); glVertex2f((float)w, 0);
    glColor3f(0.08f, 0.06f, 0.14f); glVertex2f((float)w, (float)h); glVertex2f(0, (float)h);
    glEnd();

    /* Animated decorative circles */
    glEnable(GL_BLEND);
    for (i = 0; i < 10; i++) {
        float cx = w * 0.08f + i * w * 0.09f;
        float cy = h * 0.35f + sinf(gameTime * 0.4f + i * 0.8f) * 50.0f;
        float radius = 18.0f + sinf(gameTime * 0.6f + i) * 8.0f;
        float alpha = 0.04f + 0.02f * sinf(gameTime * 0.7f + i);
        int j;

        if (i < 3) glColor4f(0.94f, 0.14f, 0.24f, alpha);
        else if (i < 6) glColor4f(1.0f, 0.64f, 0.0f, alpha);
        else glColor4f(0.05f, 0.68f, 0.38f, alpha);

        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (j = 0; j <= 24; j++) {
            float angle = j * 2.0f * (float)M_PI / 24.0f;
            glVertex2f(cx + cosf(angle) * radius, cy + sinf(angle) * radius);
        }
        glEnd();
    }

    /* Title */
    titleY = h * 0.72f;
    glColor3f(0.95f, 0.95f, 0.97f);
    draw_centered_text(titleY, "EPIDEMIC CHOICES", GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(0.55f, 0.55f, 0.65f);
    draw_centered_text(titleY - 32, "Pandemic Simulation", GLUT_BITMAP_HELVETICA_18);

    /* Separator */
    glColor4f(0.35f, 0.35f, 0.45f, 0.8f);
    glBegin(GL_LINES);
    glVertex2f(w * 0.3f, titleY - 52); glVertex2f(w * 0.7f, titleY - 52);
    glEnd();

    /* Description */
    glColor3f(0.45f, 0.45f, 0.55f);
    draw_centered_text(titleY - 75, "Manage a pandemic using the SEIR model.", GLUT_BITMAP_HELVETICA_12);
    draw_centered_text(titleY - 92, "Balance infection control, economy, and mental health.", GLUT_BITMAP_HELVETICA_12);

    /* Start prompt (pulsing) */
    pulse = 0.55f + 0.45f * sinf(gameTime * 3.0f);
    glColor4f(0.3f, 0.9f, 0.4f, pulse);
    draw_centered_text(h * 0.42f, "Press ENTER to Start", GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.4f, 0.4f, 0.5f);
    draw_centered_text(h * 0.38f, "Press Q to Quit", GLUT_BITMAP_HELVETICA_12);

    /* Controls reference */
    {
        float ctrlY = h * 0.28f;
        const char* controls[] = {
            "-- In-Game Controls --",
            "1-0,-,= : Toggle school grades   |   G : Going out   |   S : Sports",
            "+/- : Sanitization   |   P/ESC : Pause   |   [/] : Speed   |   F : Skip",
            "Mouse Drag : Rotate   |   Right Drag : Pan   |   Scroll : Zoom",
            NULL
        };
        int ci;
        glColor3f(0.38f, 0.38f, 0.48f);
        for (ci = 0; controls[ci]; ci++) {
            draw_centered_text(ctrlY, controls[ci],
                               ci == 0 ? GLUT_BITMAP_HELVETICA_12 : GLUT_BITMAP_HELVETICA_10);
            ctrlY -= 16;
        }
    }

    /* Version */
    glColor3f(0.22f, 0.22f, 0.28f);
    draw_centered_text(16, "v1.0 | OpenGL/GLUT | C99 | SEIR Model", GLUT_BITMAP_HELVETICA_10);

    glEnable(GL_LIGHTING); glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW); glPopMatrix();
}

/* ═══════════════════════════════════════════
   PAUSE SCREEN
   ═══════════════════════════════════════════ */
void render_pause(void) {
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    float pw = 300, ph = 160;
    float px = (w - pw) / 2.0f, py = (h - ph) / 2.0f;
    Color4f gray  = {0.55f, 0.55f, 0.65f, 1.0f};

    /* Darken overlay */
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    glDisable(GL_DEPTH_TEST); glDisable(GL_LIGHTING);

    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(0,0); glVertex2f((float)w,0);
    glVertex2f((float)w,(float)h); glVertex2f(0,(float)h);
    glEnd();

    /* Panel */
    glColor4f(0.06f, 0.06f, 0.10f, 0.92f);
    glBegin(GL_QUADS);
    glVertex2f(px, py); glVertex2f(px+pw, py);
    glVertex2f(px+pw, py+ph); glVertex2f(px, py+ph);
    glEnd();

    glColor4f(0.3f, 0.5f, 1.0f, 0.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(px, py); glVertex2f(px+pw, py);
    glVertex2f(px+pw, py+ph); glVertex2f(px, py+ph);
    glEnd();

    draw_centered_text(py + ph - 45, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);
    hud_draw_text(px + 55, py + ph - 80, "Press P or ESC to resume", gray,
                  GLUT_BITMAP_HELVETICA_12);
    hud_draw_text(px + 90, py + ph - 100, "Press Q to quit", gray,
                  GLUT_BITMAP_HELVETICA_12);

    glEnable(GL_LIGHTING); glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW); glPopMatrix();
}

/* ═══════════════════════════════════════════
   RESULTS / DEBRIEFING SCREEN
   ═══════════════════════════════════════════ */
void render_results(const GameWorld* world, float gameTime) {
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    const SimState* s = &world->state;
    const DebriefingStats* d = &world->stats;
    float cy, pulse;
    char buf[128];
    Color4f gray  = {0.60f, 0.60f, 0.70f, 1.0f};
    Color4f redC  = {0.9f, 0.3f, 0.3f, 1.0f};
    Color4f greenC = {0.3f, 0.9f, 0.4f, 1.0f};

    const char* headlines[] = {
        "Pandemic Ended Successfully!",
        "Prevention Budget Depleted!",
        "Pandemic Continues After Max Days...",
        "Mental Health Crisis!"
    };

    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    glDisable(GL_DEPTH_TEST); glDisable(GL_LIGHTING);

    /* Full dark background */
    glBegin(GL_QUADS);
    glColor3f(0.04f, 0.04f, 0.06f); glVertex2f(0, 0); glVertex2f((float)w, 0);
    glColor3f(0.06f, 0.05f, 0.10f); glVertex2f((float)w, (float)h); glVertex2f(0, (float)h);
    glEnd();

    cy = h * 0.88f;

    /* Headline */
    if (s->endCondition >= 0 && s->endCondition <= 3) {
        Color4f hc = (s->endCondition == END_PANDEMIC_OVER) ? greenC : redC;
        hud_draw_text((float)w/2 - 130, cy, headlines[s->endCondition], hc,
                      GLUT_BITMAP_TIMES_ROMAN_24);
    }
    cy -= 35;

    snprintf(buf, 128, "Simulation ended on Day %d", s->currentDay);
    hud_draw_text((float)w/2 - 90, cy, buf, gray, GLUT_BITMAP_HELVETICA_18);
    cy -= 50;

    /* Statistics */
    {
        char stats[8][80];
        int si;
        snprintf(stats[0], 80, "Population: %.0f", world->config.population);
        snprintf(stats[1], 80, "Peak Exposed: %.0f", d->maxExposed);
        snprintf(stats[2], 80, "Peak Infected: %.0f", d->maxInfected);
        snprintf(stats[3], 80, "Total Deaths: %.0f  (%.1f%%)", d->maxDead, d->deadPercent);
        snprintf(stats[4], 80, "Sick: %.1f%%  |  Hospitalized: %.1f%%",
                 d->sickPercent, d->hospitalizedPercent);
        snprintf(stats[5], 80, "Budget Remaining: %d / %d", s->economy, world->config.maxEconomy);
        snprintf(stats[6], 80, "Mental Health: %.0f / %.0f", s->mentalHealth,
                 world->config.maxMentalHealth);
        snprintf(stats[7], 80, "Days no going out: %d  |  Days no sports: %d",
                 d->daysNoGoingOut, d->daysNoSports);

        for (si = 0; si < 8; si++) {
            Color4f sc;
            if (si == 3 && d->deadPercent > 5.0f) sc = redC;
            else if (si == 3) sc = greenC;
            else sc = gray;
            hud_draw_text((float)w/2 - 160, cy, stats[si], sc, GLUT_BITMAP_HELVETICA_12);
            cy -= 22;
        }
    }

    cy -= 15;

    /* Final SEIR Graph (wide) */
    {
        float gx = w * 0.15f;
        float gw = w * 0.70f;
        float gy = cy - 140;
        float gh = 130;

        glColor4f(0.05f, 0.05f, 0.07f, 0.85f);
        glBegin(GL_QUADS);
        glVertex2f(gx, gy); glVertex2f(gx+gw, gy);
        glVertex2f(gx+gw, gy+gh); glVertex2f(gx, gy+gh);
        glEnd();

        if (s->historyCount >= 2) {
            float maxPop = world->config.population;
            float xScale = gw / (float)(s->historyCount - 1);
            int j;

            glLineWidth(2.0f);
            /* S */
            glColor3f(0.5f, 0.5f, 0.6f);
            glBegin(GL_LINE_STRIP);
            for (j = 0; j < s->historyCount; j++)
                glVertex2f(gx + j * xScale, gy + (s->S_history[j] / maxPop) * gh);
            glEnd();
            /* E */
            glColor3f(1.0f, 0.64f, 0.0f);
            glBegin(GL_LINE_STRIP);
            for (j = 0; j < s->historyCount; j++)
                glVertex2f(gx + j * xScale, gy + (s->E_history[j] / maxPop) * gh);
            glEnd();
            /* I */
            glColor3f(0.94f, 0.14f, 0.24f);
            glBegin(GL_LINE_STRIP);
            for (j = 0; j < s->historyCount; j++)
                glVertex2f(gx + j * xScale, gy + (s->I_history[j] / maxPop) * gh);
            glEnd();
            /* R */
            glColor3f(0.05f, 0.68f, 0.38f);
            glBegin(GL_LINE_STRIP);
            for (j = 0; j < s->historyCount; j++)
                glVertex2f(gx + j * xScale, gy + (s->R_history[j] / maxPop) * gh);
            glEnd();
            glLineWidth(1.0f);
        }
    }

    /* Restart prompt */
    pulse = 0.55f + 0.45f * sinf(gameTime * 3.0f);
    glColor4f(0.3f, 0.9f, 0.4f, pulse);
    draw_centered_text(35, "Press R to Restart   |   Press Q to Quit",
                       GLUT_BITMAP_HELVETICA_18);

    glEnable(GL_LIGHTING); glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW); glPopMatrix();
}
