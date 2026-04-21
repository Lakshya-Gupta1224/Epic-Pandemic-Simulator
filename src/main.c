/*
 * Epidemic Choices — Pandemic Simulation
 * ═══════════════════════════════════════
 * A CGV project integrating P2's SEIR epidemiological model
 * with a 3D OpenGL/GLUT visualization engine.
 *
 * Architecture: Backend (SEIR, no GL) → Controller → Frontend (OpenGL)
 * Language: C99 | Graphics: OpenGL 2.1 + GLUT
 */

#include <GL/glut.h>
#include "controller.h"
#include "constants.h"

static int lastTime = 0;

void display(void) {
    controller_render();
}

void idle(void) {
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (now - lastTime) / 1000.0f;
    if (dt > 0.1f) dt = 0.1f;  /* Cap to avoid physics explosion */
    lastTime = now;
    controller_update(dt);
}

void keyboard(unsigned char k, int x, int y) {
    controller_key_down(k, x, y);
}

void mouse(int b, int s, int x, int y) {
    controller_mouse_click(b, s, x, y);
}

void motion(int x, int y) {
    controller_mouse_drag(x, y);
}

void passive_motion(int x, int y) {
    controller_passive_motion(x, y);
}

void reshape(int w, int h) {
    controller_resize(w, h);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_W, WINDOW_H);
    glutInitWindowPosition(100, 50);
    glutCreateWindow(WINDOW_TITLE);

    controller_init(WINDOW_W, WINDOW_H);

    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(passive_motion);
    glutReshapeFunc(reshape);

    lastTime = glutGet(GLUT_ELAPSED_TIME);
    glutMainLoop();

    return 0;
}
