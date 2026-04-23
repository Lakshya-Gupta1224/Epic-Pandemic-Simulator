#include "hud.h"
#include <GL/glut.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

static int screenW = 1280, screenH = 720;

#define NUM_SLIDERS 5
static float sliderRects[NUM_SLIDERS][4];
static float sliderMin[NUM_SLIDERS];
static float sliderMax[NUM_SLIDERS];
static int   activeSlider = -1;

#define NUM_CHECKBOXES 16
static float checkboxRects[NUM_CHECKBOXES][4];

void hud_set_screen(int w, int h) { screenW = w; screenH = h; }

static float slider_value_from_mouse(int idx, int mouseX) {
    float sx = sliderRects[idx][0];
    float sw = sliderRects[idx][2];
    float t = ((float)mouseX - sx) / sw;
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    return sliderMin[idx] + t * (sliderMax[idx] - sliderMin[idx]);
}

static void apply_slider_value(int idx, float val, GameWorld* world) {
    switch (idx) {
        case 0: world->state.handSanitization = val; break;
        case 1: world->state.maskLevel = val; break;
        case 2: world->state.lockdownPercent = val; break;
        case 3: 
            world->state.infectionRate = val; 
            world->config.R0 = val;
            if (world->state.gamma > 0.0f) 
                world->state.beta = val * world->state.gamma;
            break;
        case 4: world->state.populationDensity = val; break;
    }
}

static void hud_show_toast(GameWorld* world, const char* msg, float r, float g, float b) {
    int i, slot = 0;
    float minTimer = 9999.0f;
    for (i = 0; i < MAX_TOASTS; i++) {
        if (!world->toasts[i].active) { slot = i; break; }
        if (world->toasts[i].timer < minTimer) { minTimer = world->toasts[i].timer; slot = i; }
    }
    snprintf(world->toasts[slot].message, 64, "%s", msg);
    world->toasts[slot].timer = TOAST_DURATION;
    world->toasts[slot].r = r;
    world->toasts[slot].g = g;
    world->toasts[slot].b = b;
    world->toasts[slot].active = 1;
}

int hud_handle_click(int mouseX, int mouseY, GameWorld* world) {
    int i;
    int glY = screenH - mouseY;

    for (i = 0; i < NUM_SLIDERS; i++) {
        float sx = sliderRects[i][0];
        float sy = sliderRects[i][1];
        float sw = sliderRects[i][2];
        float sh = sliderRects[i][3];

        if ((float)mouseX >= sx - 5 && (float)mouseX <= sx + sw + 5 &&
            (float)glY >= sy - 5 && (float)glY <= sy + sh + 5) {
            activeSlider = i;
            float val = slider_value_from_mouse(i, mouseX);
            apply_slider_value(i, val, world);
            return 1;
        }
    }

    for (i = 0; i < NUM_CHECKBOXES; i++) {
        float cx = checkboxRects[i][0];
        float cy = checkboxRects[i][1];
        float cw = checkboxRects[i][2];
        float ch = checkboxRects[i][3];

        float click_w = cw;
        if (i < 14) click_w = cw + 80;

        if ((float)mouseX >= cx - 2 && (float)mouseX <= cx + click_w &&
            (float)glY >= cy - 2 && (float)glY <= cy + ch + 4) {
            
            char tmsg[64];
            if (i < 12) {
                world->state.schoolOpen[i] = !world->state.schoolOpen[i];
                snprintf(tmsg, 64, "Grade %d: %s", i + 1, world->state.schoolOpen[i] ? "Opened" : "Closed");
                hud_show_toast(world, tmsg, 0.3f, 0.7f, 1.0f);
            } else if (i == 12) {
                world->state.goingOutAllowed = !world->state.goingOutAllowed;
                hud_show_toast(world, world->state.goingOutAllowed ? "Going out ALLOWED" : "Going out RESTRICTED", 0.8f, 0.5f, 1.0f);
            } else if (i == 13) {
                world->state.sportsAllowed = !world->state.sportsAllowed;
                hud_show_toast(world, world->state.sportsAllowed ? "Sports ALLOWED" : "Sports RESTRICTED", 1.0f, 0.4f, 0.2f);
            }
            return 1;
        }
    }
    return 0;
}

int hud_handle_drag(int mouseX, int mouseY, GameWorld* world) {
    (void)mouseY;
    if (activeSlider >= 0 && activeSlider < NUM_SLIDERS) {
        float val = slider_value_from_mouse(activeSlider, mouseX);
        apply_slider_value(activeSlider, val, world);
        return 1;
    }
    return 0;
}

void hud_release_slider(void) {
    activeSlider = -1;
}

static void enter_2d(void) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, screenW, 0, screenH);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
}

static void exit_2d(void) {
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void hud_draw_text(float x, float y, const char* text, Color4f c, void* font) {
    int i;
    if (!font) font = GLUT_BITMAP_HELVETICA_12;
    glColor4f(c.r, c.g, c.b, c.a);
    glRasterPos2f(x, y);
    for (i = 0; text[i] != '\0'; i++) {
        glutBitmapCharacter(font, text[i]);
    }
}

static void hud_draw_checkbox(float x, float y, float size, int checked, const char* label, int cbIndex) {
    Color4f white = {1.0f, 1.0f, 1.0f, 1.0f};
    Color4f green = {0.3f, 0.9f, 0.4f, 1.0f};

    if (cbIndex >= 0 && cbIndex < NUM_CHECKBOXES) {
        checkboxRects[cbIndex][0] = x;
        checkboxRects[cbIndex][1] = y;
        checkboxRects[cbIndex][2] = size;
        checkboxRects[cbIndex][3] = size;
    }

    glColor4f(0.5f, 0.5f, 0.6f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y); glVertex2f(x+size, y);
    glVertex2f(x+size, y+size); glVertex2f(x, y+size);
    glEnd();

    if (checked) {
        glColor4f(green.r, green.g, green.b, 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(x+2, y+2); glVertex2f(x+size-2, y+2);
        glVertex2f(x+size-2, y+size-2); glVertex2f(x+2, y+size-2);
        glEnd();
    }

    hud_draw_text(x + size + 6, y + 1, label, white, GLUT_BITMAP_HELVETICA_10);
}

static void hud_draw_button(float x, float y, float w, float h, const char* label, int btnIndex) {
    Color4f white = {1.0f, 1.0f, 1.0f, 1.0f};
    if (btnIndex >= 0 && btnIndex < NUM_CHECKBOXES) {
        checkboxRects[btnIndex][0] = x;
        checkboxRects[btnIndex][1] = y;
        checkboxRects[btnIndex][2] = w;
        checkboxRects[btnIndex][3] = h;
    }
    glColor4f(0.2f, 0.2f, 0.3f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x+w, y);
    glVertex2f(x+w, y+h); glVertex2f(x, y+h);
    glEnd();
    
    glColor4f(0.4f, 0.4f, 0.5f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y); glVertex2f(x+w, y);
    glVertex2f(x+w, y+h); glVertex2f(x, y+h);
    glEnd();
    
    hud_draw_text(x + w/2 - 3, y + h/2 - 4, label, white, GLUT_BITMAP_HELVETICA_10);
}

static void hud_draw_bar(float x, float y, float w, float h,
                         float value, float maxValue,
                         float cr, float cg, float cb,
                         const char* label, float gameTime) {
    float t = (maxValue > 0) ? value / maxValue : 0;
    char buf[64];
    Color4f white = {1,1,1,1};

    if (t < 0) t = 0;
    if (t > 1) t = 1;

    /* Outer frame */
    glColor4f(0.25f, 0.25f, 0.30f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(x-1, y-1); glVertex2f(x+w+1, y-1);
    glVertex2f(x+w+1, y+h+1); glVertex2f(x, y+h+1);
    glEnd();

    /* Background */
    glColor4f(0.06f, 0.06f, 0.08f, 0.85f);
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x+w, y);
    glVertex2f(x+w, y+h); glVertex2f(x, y+h);
    glEnd();

    /* Fill with vertical gradient */
    glBegin(GL_QUADS);
    glColor4f(cr * 0.5f, cg * 0.5f, cb * 0.5f, 0.9f);
    glVertex2f(x, y);
    glVertex2f(x + w * t, y);
    glColor4f(cr, cg, cb, 0.95f);
    glVertex2f(x + w * t, y + h);
    glVertex2f(x, y + h);
    glEnd();

    /* Critical glow */
    if (t < 0.25f) {
        float glow = 0.3f + 0.2f * (float)sin(gameTime * 5.0);
        glColor4f(1.0f, 0.3f, 0.3f, glow);
        glBegin(GL_QUADS);
        glVertex2f(x + w*t - 2, y);  glVertex2f(x + w*t + 2, y);
        glVertex2f(x + w*t + 2, y+h); glVertex2f(x + w*t - 2, y+h);
        glEnd();
    }

    /* Label */
    snprintf(buf, 64, "%s: %.0f / %.0f", label, value, maxValue);
    hud_draw_text(x + 4, y + 4, buf, white, GLUT_BITMAP_HELVETICA_10);
}

static void hud_draw_graph(float x, float y, float w, float h,
                           const SimState* st, float population, int maxBeds, int maxDays) {
    int i;
    float maxPop, xScale, hospY;

    /* Background panel */
    glColor4f(0.05f, 0.05f, 0.07f, 0.85f);
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x+w, y);
    glVertex2f(x+w, y+h); glVertex2f(x, y+h);
    glEnd();

    /* Border */
    glColor4f(0.22f, 0.22f, 0.28f, 0.8f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y); glVertex2f(x+w, y);
    glVertex2f(x+w, y+h); glVertex2f(x, y+h);
    glEnd();

    if (st->historyCount < 2) return;

    maxPop = population;
    if (maxPop < 1) maxPop = 1;
    
    /* Lock the X scale to maxDays so the graph draws incrementally */
    xScale = w / (float)(maxDays > 1 ? maxDays : 1);

    /* Grid lines */
    glColor4f(0.12f, 0.12f, 0.15f, 0.5f);
    for (i = 1; i < 4; i++) {
        float gy = y + h * (i / 4.0f);
        glBegin(GL_LINES);
        glVertex2f(x, gy); glVertex2f(x + w, gy);
        glEnd();
    }

    /* Hospital capacity line (blue dashed) */
    hospY = y + ((float)maxBeds / maxPop) * h;
    glColor4f(0.3f, 0.5f, 1.0f, 0.7f);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(2, 0xAAAA);
    glBegin(GL_LINES);
    glVertex2f(x, hospY); glVertex2f(x + w, hospY);
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    glLineWidth(2.0f);

    /* S curve (gray) */
    glColor3f(0.50f, 0.50f, 0.60f);
    glBegin(GL_LINE_STRIP);
    for (i = 0; i < st->historyCount; i++)
        glVertex2f(x + i * xScale, y + (st->S_history[i] / maxPop) * h);
    glEnd();

    /* E curve (orange) */
    glColor3f(1.00f, 0.64f, 0.00f);
    glBegin(GL_LINE_STRIP);
    for (i = 0; i < st->historyCount; i++)
        glVertex2f(x + i * xScale, y + (st->E_history[i] / maxPop) * h);
    glEnd();

    /* I curve (red) */
    glColor3f(0.94f, 0.14f, 0.24f);
    glBegin(GL_LINE_STRIP);
    for (i = 0; i < st->historyCount; i++)
        glVertex2f(x + i * xScale, y + (st->I_history[i] / maxPop) * h);
    glEnd();

    /* R curve (green) */
    glColor3f(0.05f, 0.68f, 0.38f);
    glBegin(GL_LINE_STRIP);
    for (i = 0; i < st->historyCount; i++)
        glVertex2f(x + i * xScale, y + (st->R_history[i] / maxPop) * h);
    glEnd();

    glLineWidth(1.0f);

    /* Legend */
    {
        float lx = x + 8, ly = y + h - 14;
        const char* labels[] = {"S", "E", "I", "R", "Hosp"};
        float colors[5][3] = {
            {0.5f,0.5f,0.6f}, {1.0f,0.64f,0.0f}, {0.94f,0.14f,0.24f},
            {0.05f,0.68f,0.38f}, {0.3f,0.5f,1.0f}
        };
        int j;
        for (j = 0; j < 5; j++) {
            Color4f tc = {0.7f, 0.7f, 0.75f, 1.0f};
            glColor3fv(colors[j]);
            glBegin(GL_QUADS);
            glVertex2f(lx, ly); glVertex2f(lx+8, ly);
            glVertex2f(lx+8, ly+8); glVertex2f(lx, ly+8);
            glEnd();
            hud_draw_text(lx + 11, ly + 1, labels[j], tc, GLUT_BITMAP_HELVETICA_10);
            lx += 40;
        }
    }
}

void render_hud(const GameWorld* world) {
    const SimState* s = &world->state;
    float barW = 250, barH = 20, gap = 7;
    float bx, by;
    char buf[128];
    Color4f white = {1.0f, 1.0f, 1.0f, 1.0f};
    Color4f red   = {1.0f, 0.35f, 0.35f, 1.0f};
    Color4f gray  = {0.55f, 0.55f, 0.65f, 1.0f};
    Color4f cyan  = {0.4f, 0.8f, 1.0f, 1.0f};
    int g;

    enter_2d();

    /* ─── Top-left info panel background ─── */
    glColor4f(0.04f, 0.04f, 0.06f, 0.65f);
    glBegin(GL_QUADS);
    glVertex2f(0, (float)screenH - 85); glVertex2f(420, (float)screenH - 85);
    glVertex2f(420, (float)screenH); glVertex2f(0, (float)screenH);
    glEnd();

    /* ─── Day / Hour / Speed (top-left) ─── */
    snprintf(buf, 128, "Day %d / %d  |  Hour %d:00  |  Speed: %.1fx",
             s->currentDay, world->config.maxDays, s->currentHour, s->timeScale);
    hud_draw_text(15, (float)screenH - 22, buf, white, GLUT_BITMAP_HELVETICA_18);

    /* Deaths counter + rho */
    snprintf(buf, 64, "Deaths: %d", s->dead);
    hud_draw_text(15, (float)screenH - 46, buf, red, GLUT_BITMAP_HELVETICA_18);

    snprintf(buf, 64, "Rho: %.2f", s->rho);
    hud_draw_text(180, (float)screenH - 46, buf, cyan, GLUT_BITMAP_HELVETICA_12);

    /* Population counts */
    snprintf(buf, 128, "S:%d  E:%d  I:%d  R:%d",
             s->susceptible, s->exposed, s->infected, s->recovered);
    hud_draw_text(15, (float)screenH - 68, buf, gray, GLUT_BITMAP_HELVETICA_12);

    /* ─── Resource Bars (top-right) with background ─── */
    bx = (float)screenW - barW - 20;
    by = (float)screenH - 30;

    /* Bars panel background */
    glColor4f(0.04f, 0.04f, 0.06f, 0.65f);
    glBegin(GL_QUADS);
    glVertex2f(bx - 8, by - (barH + gap) * 4 + 5);
    glVertex2f((float)screenW, by - (barH + gap) * 4 + 5);
    glVertex2f((float)screenW, (float)screenH);
    glVertex2f(bx - 8, (float)screenH);
    glEnd();

    int actualHospitalOccupancy = 0;
    for (g = 0; g < world->buildingCount; g++) {
        if (world->buildings[g].type == BUILDING_HOSPITAL) {
            actualHospitalOccupancy += world->buildings[g].currCapacity;
        }
    }

    hud_draw_bar(bx, by, barW, barH,
                 (float)actualHospitalOccupancy, (float)world->config.maxHospitalBeds,
                 0.9f, 0.25f, 0.25f, "Hospital", world->gameTime);
    by -= barH + gap;

    hud_draw_bar(bx, by, barW, barH,
                 (float)s->economy, (float)world->config.maxEconomy,
                 0.25f, 0.75f, 0.35f, "Budget", world->gameTime);
    by -= barH + gap;

    hud_draw_bar(bx, by, barW, barH,
                 s->mentalHealth, world->config.maxMentalHealth,
                 0.85f, 0.75f, 0.20f, "Mental Health", world->gameTime);
    by -= barH + gap;

    /* Alive = sum of all living compartments (exact, no rounding drift) */
    float currentPop = (float)(s->susceptible + s->exposed + s->infected + s->recovered);
    hud_draw_bar(bx, by, barW, barH,
                 currentPop, world->config.population,
                 0.4f, 0.6f, 0.9f, "Population", world->gameTime);

    /* ─── SEIR Graph (bottom-left) ─── */
    hud_draw_graph(15, 15, 380, 180, s,
                   world->config.population, world->config.maxHospitalBeds, world->config.maxDays);

    /* ─── Decision Panel (right side, with panel background) ─── */
    /* panelTop must be below the 4 resource bars.
       Bars start at screenH-30 and occupy 4*(barH+gap)=4*(20+7)=108px.
       So bars end at screenH-138; we add 17px clearance -> screenH-155. */
    {
        float panelW = 220;
        float panelH = 510;
        float px = (float)screenW - panelW - 10;
        float panelTop = (float)screenH - 155;
        /* py starts 14px below panelTop so the CONTROLS text baseline+ascent
           sits fully inside the box (box top is panelTop+18). */
        float py = panelTop - 14;

        /* Panel background */
        glColor4f(0.04f, 0.04f, 0.06f, 0.70f);
        glBegin(GL_QUADS);
        glVertex2f(px - 8, panelTop - panelH);
        glVertex2f((float)screenW, panelTop - panelH);
        glVertex2f((float)screenW, panelTop + 18);   /* box top: 18px above panelTop */
        glVertex2f(px - 8, panelTop + 18);
        glEnd();

        /* Panel border */
        glColor4f(0.20f, 0.25f, 0.35f, 0.5f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(px - 8, panelTop - panelH);
        glVertex2f((float)screenW - 2, panelTop - panelH);
        glVertex2f((float)screenW - 2, panelTop + 18);
        glVertex2f(px - 8, panelTop + 18);
        glEnd();

        hud_draw_text(px + panelW/2 - 42, py, "CONTROLS", white, GLUT_BITMAP_HELVETICA_12);
        py -= 6;

        /* Separator */
        glColor4f(0.3f, 0.3f, 0.4f, 0.5f);
        glBegin(GL_LINES);
        glVertex2f(px, py); glVertex2f(px + panelW - 10, py);
        glEnd();
        py -= 14;

        /* Separator */
        glColor4f(0.3f, 0.3f, 0.4f, 0.4f);
        glBegin(GL_LINES);
        glVertex2f(px, py + 4); glVertex2f(px + panelW - 10, py + 4);
        glEnd();
        py -= 10;

        snprintf(buf, 64, "Going Out [%c]", 'G');
        hud_draw_checkbox(px, py + 2, 12.0f, s->goingOutAllowed, buf, 12);

        snprintf(buf, 64, "Sports [%c]", 'S');
        hud_draw_checkbox(px + 100, py + 2, 12.0f, s->sportsAllowed, buf, 13);
        py -= 16;

        /* ── Interactive Sliders ── */
        {
            Color4f lblC = {0.6f, 0.6f, 0.7f, 1.0f};
            hud_draw_text(px, py, "SLIDERS (click & drag)", lblC, GLUT_BITMAP_HELVETICA_10);
        }
        py -= 14;

#define DRAW_SLIDER(idx, label, val, minV, maxV, cr, cg, cb) \
        do { \
            float slW = panelW - 10, slH = 14; \
            float fill = ((val) - (minV)) / ((maxV) - (minV)); \
            float knobX; \
            if (fill < 0) fill = 0; \
            if (fill > 1) fill = 1; \
            snprintf(buf, 64, "%s: %.1f", label, (float)(val)); \
            hud_draw_text(px, py, buf, white, GLUT_BITMAP_HELVETICA_10); \
            py -= 15; \
            sliderRects[idx][0] = px; sliderRects[idx][1] = py; \
            sliderRects[idx][2] = slW; sliderRects[idx][3] = slH; \
            sliderMin[idx] = (minV); sliderMax[idx] = (maxV); \
            glColor4f(0.10f, 0.10f, 0.13f, 0.8f); \
            glBegin(GL_QUADS); \
            glVertex2f(px, py); glVertex2f(px+slW, py); \
            glVertex2f(px+slW, py+slH); glVertex2f(px, py+slH); \
            glEnd(); \
            glColor4f(0.25f, 0.25f, 0.30f, 0.7f); \
            glBegin(GL_LINE_LOOP); \
            glVertex2f(px, py); glVertex2f(px+slW, py); \
            glVertex2f(px+slW, py+slH); glVertex2f(px, py+slH); \
            glEnd(); \
            glBegin(GL_QUADS); \
            glColor4f((cr)*0.4f, (cg)*0.4f, (cb)*0.4f, 0.7f); \
            glVertex2f(px, py); glVertex2f(px + slW * fill, py); \
            glColor4f(cr, cg, cb, 0.9f); \
            glVertex2f(px + slW * fill, py + slH); glVertex2f(px, py + slH); \
            glEnd(); \
            knobX = px + slW * fill; \
            glColor4f(0.95f, 0.95f, 0.98f, 1.0f); \
            glBegin(GL_QUADS); \
            glVertex2f(knobX - 3, py - 1); glVertex2f(knobX + 3, py - 1); \
            glVertex2f(knobX + 3, py + slH + 1); glVertex2f(knobX - 3, py + slH + 1); \
            glEnd(); \
            if (activeSlider == idx) { \
                glColor4f(0.4f, 0.8f, 1.0f, 0.9f); \
                glLineWidth(2.0f); \
                glBegin(GL_LINE_LOOP); \
                glVertex2f(px-1, py-1); glVertex2f(px+slW+1, py-1); \
                glVertex2f(px+slW+1, py+slH+1); glVertex2f(px-1, py+slH+1); \
                glEnd(); \
                glLineWidth(1.0f); \
            } \
            py -= 20; \
        } while(0)

        DRAW_SLIDER(0, "Sanitize", s->handSanitization, 0.0f, 10.0f, 0.3f, 0.6f, 1.0f);
        DRAW_SLIDER(1, "Masks",    s->maskLevel,         0.0f, 10.0f, 0.8f, 0.5f, 1.0f);
        DRAW_SLIDER(2, "Lockdown", s->lockdownPercent,   0.0f, 100.0f, 1.0f, 0.4f, 0.2f);
        DRAW_SLIDER(3, "Infectn Rate", s->infectionRate, 0.5f, 5.0f,  0.9f, 0.2f, 0.2f);
        DRAW_SLIDER(4, "Density",  s->populationDensity, 0.5f, 3.0f,  0.3f, 0.85f, 0.6f);

#undef DRAW_SLIDER

        /* Separator */
        glColor4f(0.3f, 0.3f, 0.4f, 0.3f);
        glBegin(GL_LINES);
        glVertex2f(px, py + 6); glVertex2f(px + panelW - 10, py + 6);
        glEnd();
        py -= 6;

        hud_draw_text(px, py, "[H] Help  [P] Pause", gray,
                      GLUT_BITMAP_HELVETICA_10);
        py -= 14;
        hud_draw_text(px, py, "[F] Skip  [D] Debug", gray,
                      GLUT_BITMAP_HELVETICA_10);
        py -= 14;
        hud_draw_text(px, py, "[B] Back to World Map", gray,
                      GLUT_BITMAP_HELVETICA_10);
    }

    /* ─── Toast Notifications ─── */
    {
        float ty = (float)screenH * 0.70f;
        int t;
        for (t = 0; t < MAX_TOASTS; t++) {
            if (world->toasts[t].active) {
                float alpha = (world->toasts[t].timer > 0.5f)
                    ? 1.0f : world->toasts[t].timer * 2.0f;
                float tw = 280, th = 22;
                float tx_pos = 20;

                /* Background */
                glColor4f(0.08f, 0.08f, 0.10f, 0.8f * alpha);
                glBegin(GL_QUADS);
                glVertex2f(tx_pos, ty); glVertex2f(tx_pos + tw, ty);
                glVertex2f(tx_pos + tw, ty + th); glVertex2f(tx_pos, ty + th);
                glEnd();

                /* Left accent */
                glColor4f(world->toasts[t].r, world->toasts[t].g,
                          world->toasts[t].b, alpha);
                glBegin(GL_QUADS);
                glVertex2f(tx_pos, ty); glVertex2f(tx_pos + 3, ty);
                glVertex2f(tx_pos + 3, ty + th); glVertex2f(tx_pos, ty + th);
                glEnd();

                /* Text */
                {
                    Color4f tc = {0.9f, 0.9f, 0.95f, alpha};
                    hud_draw_text(tx_pos + 8, ty + 6,
                                  world->toasts[t].message, tc,
                                  GLUT_BITMAP_HELVETICA_10);
                }

                ty -= th + 4;
            }
        }
    }

    /* ─── Help Overlay ─── */
    if (world->showHelp) {
        float pw = 340, ph = 310;
        float px = ((float)screenW - pw) / 2;
        float py = ((float)screenH - ph) / 2;
        float ty2;

        const char* lines[] = {
            "G             : Toggle going out",
            "S             : Toggle outdoor sports",
            "+             : Increase sanitization",
            "Backspace     : Decrease sanitization",
            "[ / ]         : Decrease / Increase speed",
            "F             : Fast forward 5 days",
            "P / ESC       : Pause / Resume",
            "D             : Debug output (console)",
            "H             : Toggle this help",
            "",
            "Mouse Drag    : Rotate camera",
            "Right Drag    : Pan camera",
            "Scroll        : Zoom in/out",
            NULL
        };

        glColor4f(0.04f, 0.04f, 0.06f, 0.94f);
        glBegin(GL_QUADS);
        glVertex2f(px, py); glVertex2f(px+pw, py);
        glVertex2f(px+pw, py+ph); glVertex2f(px, py+ph);
        glEnd();

        glColor4f(0.3f, 0.5f, 1.0f, 0.6f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(px, py); glVertex2f(px+pw, py);
        glVertex2f(px+pw, py+ph); glVertex2f(px, py+ph);
        glEnd();

        ty2 = py + ph - 30;
        hud_draw_text(px + pw/2 - 80, ty2, "KEYBOARD SHORTCUTS", white,
                      GLUT_BITMAP_HELVETICA_18);
        ty2 -= 25;

        {
            int li;
            for (li = 0; lines[li]; li++) {
                hud_draw_text(px + 20, ty2, lines[li], gray, GLUT_BITMAP_HELVETICA_10);
                ty2 -= 16;
            }
        }
    }

    exit_2d();
}
