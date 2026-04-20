#include "controller.h"
#include "models.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "hud.h"
#include "menu.h"
#include "simulation.h"
#include "world_gen.h"
#include <GL/glut.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

static GameWorld world;

/* ─── Smoothstep easing ─── */
static float smoothstep(float t) {
    return t * t * (3.0f - 2.0f * t);
}

/* ─── Toast helper ─── */
static void toast_show(const char* msg, float r, float g, float b) {
    int slot = 0, i;
    float minTimer = 999.0f;
    for (i = 0; i < MAX_TOASTS; i++) {
        if (!world.toasts[i].active) { slot = i; break; }
        if (world.toasts[i].timer < minTimer) { minTimer = world.toasts[i].timer; slot = i; }
    }
    strncpy(world.toasts[slot].message, msg, 63);
    world.toasts[slot].message[63] = '\0';
    world.toasts[slot].timer = TOAST_DURATION;
    world.toasts[slot].r = r;
    world.toasts[slot].g = g;
    world.toasts[slot].b = b;
    world.toasts[slot].active = 1;
}

/* ═══════════════════════════════════════════
   INIT
   ═══════════════════════════════════════════ */
void controller_init(int w, int h) {
    memset(&world, 0, sizeof(GameWorld));

    render_init(w, h);
    hud_set_screen(w, h);
    camera_init(&world.camera);

    world.config.population       = DEFAULT_POPULATION;
    world.config.incubationPeriod = DEFAULT_INCUBATION;
    world.config.infectionPeriod  = DEFAULT_INFECTION_PERIOD;
    world.config.R0               = DEFAULT_R0;
    world.config.initialExposed   = DEFAULT_INITIAL_EXPOSED;
    world.config.maxDays          = DEFAULT_MAX_DAYS;
    world.config.maxHospitalBeds  = DEFAULT_HOSPITAL_BEDS;
    world.config.maxEconomy       = DEFAULT_MAX_ECONOMY;
    world.config.maxMentalHealth  = DEFAULT_MAX_MENTAL_HEALTH;

    world.appState = APP_MENU;
    world.gameTime = 0.0f;
    world.showHelp = 0;
}

/* ═══════════════════════════════════════════
   UPDATE
   ═══════════════════════════════════════════ */
void controller_update(float dt) {
    int i;
    world.gameTime += dt;

    /* Update toasts */
    for (i = 0; i < MAX_TOASTS; i++) {
        if (world.toasts[i].active) {
            world.toasts[i].timer -= dt;
            if (world.toasts[i].timer <= 0) world.toasts[i].active = 0;
        }
    }

    switch (world.appState) {
        case APP_MENU:
            break;

        case APP_RUNNING: {
            /* Update simulation */
            sim_update(&world, dt);

            /* Update lighting based on hour */
            update_lighting(world.state.currentHour);

            /* Check for game over */
            if (world.state.endCondition != END_NONE) {
                world.appState = APP_RESULT;
                break;
            }

            /* Update person movement interpolation */
            for (i = 0; i < world.personCount; i++) {
                Person* p = &world.persons[i];
                if (p->isMoving && p->moveProgress < 1.0f) {
                    p->moveProgress += dt * 1.5f;
                    if (p->moveProgress > 1.0f) p->moveProgress = 1.0f;

                    {
                        float t = smoothstep(p->moveProgress);
                        p->position.x = p->startPosition.x + (p->targetPosition.x - p->startPosition.x) * t;
                        p->position.y = p->startPosition.y + (p->targetPosition.y - p->startPosition.y) * t;
                        p->position.z = p->startPosition.z + (p->targetPosition.z - p->startPosition.z) * t;
                    }

                    if (p->moveProgress >= 1.0f) {
                        p->isMoving = 0;
                        p->position = p->targetPosition;
                    }
                }
            }
            break;
        }

        case APP_PAUSED:
            break;

        case APP_RESULT:
            break;
    }

    glutPostRedisplay();
}

/* ═══════════════════════════════════════════
   RENDER
   ═══════════════════════════════════════════ */
void controller_render(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    switch (world.appState) {
        case APP_MENU:
            render_menu(world.gameTime);
            break;

        case APP_RUNNING:
        case APP_PAUSED:
            camera_apply(&world.camera);
            render_scene(&world);
            render_hud(&world);

            if (world.appState == APP_PAUSED) {
                render_pause();
            }
            break;

        case APP_RESULT:
            render_results(&world, world.gameTime);
            break;
    }

    glutSwapBuffers();
}

/* ═══════════════════════════════════════════
   INPUT
   ═══════════════════════════════════════════ */
void controller_key_down(unsigned char key, int x, int y) {
    (void)x; (void)y;

    switch (world.appState) {
        case APP_MENU:
            if (key == 13) {  /* ENTER */
                world_generate(&world);
                sim_init(&world, world.config);
                world.appState = APP_RUNNING;
                toast_show("Simulation started! Good luck.", 0.3f, 0.9f, 0.4f);
            }
            if (key == 'q' || key == 'Q' || key == 27) exit(0);
            break;

        case APP_RUNNING:
            /* Pause */
            if (key == 'p' || key == 'P' || key == 27) {
                world.appState = APP_PAUSED;
                break;
            }

            /* School grades 1-9 */
            if (key >= '1' && key <= '9') {
                int g = key - '1';
                int newState = !world.state.schoolOpen[g];
                char msg[64];
                sim_toggle_school(&world, g + 1, newState);
                snprintf(msg, 64, "Grade %d %s", g + 1, newState ? "OPENED" : "CLOSED");
                toast_show(msg, 0.3f, 0.7f, 1.0f);
            }
            /* Grade 10 */
            if (key == '0') {
                int newState = !world.state.schoolOpen[9];
                char msg[64];
                sim_toggle_school(&world, 10, newState);
                snprintf(msg, 64, "Grade 10 %s", newState ? "OPENED" : "CLOSED");
                toast_show(msg, 0.3f, 0.7f, 1.0f);
            }
            /* Grade 11 */
            if (key == '-') {
                int newState = !world.state.schoolOpen[10];
                char msg[64];
                sim_toggle_school(&world, 11, newState);
                snprintf(msg, 64, "Grade 11 %s", newState ? "OPENED" : "CLOSED");
                toast_show(msg, 0.3f, 0.7f, 1.0f);
            }
            /* Grade 12 */
            if (key == '=') {
                int newState = !world.state.schoolOpen[11];
                char msg[64];
                sim_toggle_school(&world, 12, newState);
                snprintf(msg, 64, "Grade 12 %s", newState ? "OPENED" : "CLOSED");
                toast_show(msg, 0.3f, 0.7f, 1.0f);
            }

            /* Going out */
            if (key == 'g' || key == 'G') {
                int newState = !world.state.goingOutAllowed;
                sim_toggle_going_out(&world, newState);
                toast_show(newState ? "Going out ALLOWED" : "Going out RESTRICTED",
                           newState ? 0.3f : 0.9f,
                           newState ? 0.9f : 0.3f,
                           newState ? 0.4f : 0.3f);
            }

            /* Sports */
            if (key == 's' || key == 'S') {
                int newState = !world.state.sportsAllowed;
                sim_toggle_sports(&world, newState);
                toast_show(newState ? "Sports ALLOWED" : "Sports RESTRICTED",
                           newState ? 0.3f : 0.9f,
                           newState ? 0.9f : 0.3f,
                           newState ? 0.4f : 0.3f);
            }

            /* Sanitization */
            if (key == '+' || key == '=') {
                sim_set_hand_sanitization(&world, world.state.handSanitization + 1.0f);
                {
                    char msg[32];
                    snprintf(msg, 32, "Sanitization: %.0f/10", world.state.handSanitization);
                    toast_show(msg, 0.3f, 0.5f, 1.0f);
                }
            }
            if (key == 8) { /* Backspace */
                sim_set_hand_sanitization(&world, world.state.handSanitization - 1.0f);
                {
                    char msg[32];
                    snprintf(msg, 32, "Sanitization: %.0f/10", world.state.handSanitization);
                    toast_show(msg, 0.3f, 0.5f, 1.0f);
                }
            }

            /* Speed control */
            if (key == ']') {
                sim_set_time_scale(&world, world.state.timeScale + 0.5f);
                {
                    char msg[32];
                    snprintf(msg, 32, "Speed: %.1fx", world.state.timeScale);
                    toast_show(msg, 0.8f, 0.8f, 0.3f);
                }
            }
            if (key == '[') {
                sim_set_time_scale(&world, world.state.timeScale - 0.5f);
                {
                    char msg[32];
                    snprintf(msg, 32, "Speed: %.1fx", world.state.timeScale);
                    toast_show(msg, 0.8f, 0.8f, 0.3f);
                }
            }

            /* Fast forward */
            if (key == 'f' || key == 'F') {
                sim_skip_days(&world, 5);
                toast_show("Skipped 5 days!", 0.9f, 0.7f, 0.2f);
            }

            /* Debug */
            if (key == 'd' || key == 'D') {
                SimState* st = &world.state;
                printf("Day %d: S=%d E=%d I=%d R=%d Dead=%d | Econ=%d MH=%.1f Rho=%.3f\n",
                       st->currentDay, st->susceptible, st->exposed, st->infected,
                       st->recovered, st->dead, st->economy, st->mentalHealth, st->rho);
            }

            /* Help toggle */
            if (key == 'h' || key == 'H') {
                world.showHelp = !world.showHelp;
            }

            /* Quit */
            if (key == 'q' || key == 'Q') exit(0);
            break;

        case APP_PAUSED:
            if (key == 'p' || key == 'P' || key == 27) {
                world.appState = APP_RUNNING;
            }
            if (key == 'q' || key == 'Q') exit(0);
            break;

        case APP_RESULT:
            if (key == 'r' || key == 'R') {
                memset(&world.state, 0, sizeof(SimState));
                memset(&world.stats, 0, sizeof(DebriefingStats));
                world.personCount = 0;
                world.buildingCount = 0;
                world.regionCount = 0;
                world.appState = APP_MENU;
            }
            if (key == 'q' || key == 'Q') exit(0);
            break;
    }
}

void controller_mouse_click(int button, int state, int x, int y) {
    if (world.appState == APP_RUNNING || world.appState == APP_PAUSED) {
        input_mouse_click(button, state, x, y, &world.camera);
    }
}

void controller_mouse_drag(int x, int y) {
    if (world.appState == APP_RUNNING || world.appState == APP_PAUSED) {
        input_mouse_drag(x, y, &world.camera);
    }
}

void controller_resize(int w, int h) {
    render_resize(w, h);
    hud_set_screen(w, h);
}
