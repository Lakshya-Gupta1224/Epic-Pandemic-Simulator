#ifndef RENDERER_H
#define RENDERER_H

#include "models.h"

void render_init(int windowWidth, int windowHeight);
void render_resize(int width, int height);
void render_scene(const GameWorld* world);
void update_lighting(int currentHour);

#endif
