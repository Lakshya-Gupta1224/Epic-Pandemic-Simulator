#ifndef HUD_H
#define HUD_H

#include "models.h"

void hud_set_screen(int w, int h);
void render_hud(const GameWorld* world);
void hud_draw_text(float x, float y, const char* text, Color4f c, void* font);

/* Returns 1 if a slider was clicked and the value was changed */
int  hud_handle_click(int mouseX, int mouseY, GameWorld* world);
/* Returns 1 if currently dragging a slider */
int  hud_handle_drag(int mouseX, int mouseY, GameWorld* world);
void hud_release_slider(void);

#endif
