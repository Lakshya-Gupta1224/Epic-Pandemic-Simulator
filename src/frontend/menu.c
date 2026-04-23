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

    /* Dark slate blue gradient background */
    glBegin(GL_QUADS);
    glColor3f(0.04f, 0.05f, 0.08f); glVertex2f(0, 0); glVertex2f((float)w, 0);
    glColor3f(0.08f, 0.10f, 0.14f); glVertex2f((float)w, (float)h); glVertex2f(0, (float)h);
    glEnd();

    /* Animated Network Background (Modern / Sleek) */
    glEnable(GL_BLEND);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (i = 0; i < 20; i++) {
        float cx1 = w * (0.1f + 0.8f * sinf(i * 1.3f + gameTime * 0.1f));
        float cy1 = h * (0.1f + 0.8f * cosf(i * 0.7f + gameTime * 0.15f));
        int j;
        for (j = i + 1; j < 20; j++) {
            float cx2 = w * (0.1f + 0.8f * sinf(j * 1.3f + gameTime * 0.1f));
            float cy2 = h * (0.1f + 0.8f * cosf(j * 0.7f + gameTime * 0.15f));
            float dist = sqrtf((cx2-cx1)*(cx2-cx1) + (cy2-cy1)*(cy2-cy1));
            if (dist < 250.0f) {
                float alpha = 1.0f - (dist / 250.0f);
                if (alpha > 0.4f) alpha = 0.4f;
                glColor4f(0.3f, 0.5f, 0.8f, alpha * 0.5f);
                glVertex2f(cx1, cy1);
                glVertex2f(cx2, cy2);
            }
        }
    }
    glEnd();

    /* Nodes on the network */
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    for (i = 0; i < 20; i++) {
        float cx1 = w * (0.1f + 0.8f * sinf(i * 1.3f + gameTime * 0.1f));
        float cy1 = h * (0.1f + 0.8f * cosf(i * 0.7f + gameTime * 0.15f));
        float r = 0.3f, g = 0.5f, b = 0.8f;
        if (i % 3 == 0) { r = 0.9f; g = 0.2f; b = 0.3f; } /* Occasional infected node */
        glColor4f(r, g, b, 0.8f);
        glVertex2f(cx1, cy1);
    }
    glEnd();
    glPointSize(1.0f);

    /* Decorative glass panel behind title */
    titleY = h * 0.70f;
    {
        float pWidth = 600, pHeight = 250;
        float px = (w - pWidth) / 2.0f;
        float py = titleY - 140;

        glColor4f(0.06f, 0.08f, 0.12f, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(px, py); glVertex2f(px+pWidth, py);
        glVertex2f(px+pWidth, py+pHeight); glVertex2f(px, py+pHeight);
        glEnd();

        /* Sleek borders */
        glColor4f(0.3f, 0.5f, 0.8f, 0.3f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(px, py); glVertex2f(px+pWidth, py);
        glVertex2f(px+pWidth, py+pHeight); glVertex2f(px, py+pHeight);
        glEnd();

        /* Left/Right accentbars */
        glColor4f(0.3f, 0.7f, 1.0f, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(px, py); glVertex2f(px+4, py);
        glVertex2f(px+4, py+pHeight); glVertex2f(px, py+pHeight);
        glEnd();
        glColor4f(1.0f, 0.3f, 0.3f, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(px+pWidth-4, py); glVertex2f(px+pWidth, py);
        glVertex2f(px+pWidth, py+pHeight); glVertex2f(px+pWidth-4, py+pHeight);
        glEnd();
    }

    /* Title */
    glColor3f(1.0f, 1.0f, 1.0f);
    draw_centered_text(titleY, "E P I D E M I C   C H O I C E S", GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(0.5f, 0.6f, 0.8f);
    draw_centered_text(titleY - 30, "A Modern SEIR Simulation", GLUT_BITMAP_HELVETICA_18);

    /* Description */
    glColor3f(0.7f, 0.7f, 0.7f);
    draw_centered_text(titleY - 80, "Manage an outbreak across 5 connected regions.", GLUT_BITMAP_HELVETICA_12);
    draw_centered_text(titleY - 100, "Balance public health, economic stability, and mental wellbeing.", GLUT_BITMAP_HELVETICA_12);

    /* Start prompt (pulsing, highly visible) */
    pulse = 0.6f + 0.4f * sinf(gameTime * 4.0f);
    {
        float btnW = 300, btnH = 50;
        float bx = (w - btnW) / 2.0f;
        float by = h * 0.35f;

        /* Glowing button backdrop */
        glColor4f(0.2f, 0.8f, 0.4f, pulse * 0.3f);
        glBegin(GL_QUADS);
        glVertex2f(bx - 10, by - 10); glVertex2f(bx+btnW + 10, by - 10);
        glVertex2f(bx+btnW + 10, by+btnH + 10); glVertex2f(bx - 10, by+btnH + 10);
        glEnd();

        /* Button border */
        glColor4f(0.2f, 0.9f, 0.4f, pulse);
        glBegin(GL_LINE_LOOP);
        glVertex2f(bx, by); glVertex2f(bx+btnW, by);
        glVertex2f(bx+btnW, by+btnH); glVertex2f(bx, by+btnH);
        glEnd();

        glColor4f(0.9f, 1.0f, 0.9f, pulse);
        draw_centered_text(by + 18, "P R E S S   E N T E R   T O   S T A R T", GLUT_BITMAP_HELVETICA_12);
    }

    glColor3f(0.4f, 0.4f, 0.5f);
    draw_centered_text(h * 0.28f, "Press Q to Quit", GLUT_BITMAP_HELVETICA_12);

    /* Controls reference */
    {
        float ctrlY = h * 0.15f;
        const char* controls[] = {
            "CONTROLS: Drag Mouse: Rotate/Pan | Scroll: Zoom | S/G: Restrictions | +/-/M/N/L/K: Sliders",
            NULL
        };
        int ci;
        glColor3f(0.4f, 0.5f, 0.6f);
        for (ci = 0; controls[ci]; ci++) {
            draw_centered_text(ctrlY, controls[ci], GLUT_BITMAP_HELVETICA_10);
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

    const char* headline = "Simulation Ended";
    Color4f hc = gray;

    switch (s->endCondition) {
        case END_VICTORY:
            headline = "PANDEMIC ERADICATED: VICTORY!";
            hc = greenC;
            break;
        case END_BUDGET_DEPLETED:
            headline = "ECONOMIC COLLAPSE: BUDGET EXHAUSTED!";
            hc = redC;
            break;
        case END_MENTAL_CRISIS:
            headline = "SOCIETAL BREAKDOWN: MENTAL HEALTH ZERO!";
            hc = redC;
            break;
        case END_POPULATION_ZERO:
            headline = "EXTINCTION: POPULATION REACHED ZERO!";
            hc = redC;
            break;
        default:
            headline = "SIMULATION TERMINATED";
            hc = gray;
            break;
    }

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
    float tw = 0;
    for (int i=0; headline[i]; i++) tw += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, headline[i]);
    hud_draw_text((float)w/2 - tw/2, cy, headline, hc, GLUT_BITMAP_TIMES_ROMAN_24);

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
