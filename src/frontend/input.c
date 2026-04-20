#include "input.h"
#include "camera.h"
#include <GL/glut.h>

static int mouseLastX = 0, mouseLastY = 0;
static int mouseLeftDown = 0, mouseRightDown = 0;

void input_mouse_click(int button, int bstate, int x, int y, CameraState* cam) {
    if (button == GLUT_LEFT_BUTTON) {
        mouseLeftDown = (bstate == GLUT_DOWN);
    } else if (button == GLUT_RIGHT_BUTTON) {
        mouseRightDown = (bstate == GLUT_DOWN);
    }

    /* Scroll wheel zoom */
    if (button == 3) camera_zoom(cam,  1.0f);
    if (button == 4) camera_zoom(cam, -1.0f);

    mouseLastX = x;
    mouseLastY = y;
}

void input_mouse_drag(int x, int y, CameraState* cam) {
    int dx = x - mouseLastX;
    int dy = y - mouseLastY;

    if (mouseLeftDown) {
        camera_rotate(cam, (float)dx, (float)dy);
    }
    if (mouseRightDown) {
        camera_pan(cam, (float)dx * 0.5f, (float)dy * 0.5f);
    }

    mouseLastX = x;
    mouseLastY = y;
}
