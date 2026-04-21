#ifndef CONTROLLER_H
#define CONTROLLER_H

void controller_init(int w, int h);
void controller_update(float deltaTime);
void controller_render(void);
void controller_key_down(unsigned char key, int x, int y);
void controller_mouse_click(int button, int state, int x, int y);
void controller_mouse_drag(int x, int y);
void controller_passive_motion(int x, int y);
void controller_resize(int w, int h);

#endif
