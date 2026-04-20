#ifndef INPUT_H
#define INPUT_H

#include "models.h"

void input_mouse_click(int button, int state, int x, int y, CameraState* cam);
void input_mouse_drag(int x, int y, CameraState* cam);

#endif
