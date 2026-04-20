#ifndef HUD_H
#define HUD_H

#include "models.h"

void hud_set_screen(int w, int h);
void render_hud(const GameWorld* world);
void hud_draw_text(float x, float y, const char* text, Color4f c, void* font);

#endif
