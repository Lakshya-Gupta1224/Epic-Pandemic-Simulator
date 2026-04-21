#ifndef WORLD_MAP_H
#define WORLD_MAP_H

#include "models.h"

void world_map_init(GameWorld* world);
void render_world_map(GameWorld* world);
void render_virus_select(GameWorld* world);
int  world_map_click(GameWorld* world, int mouseX, int mouseY);
int  virus_select_click(GameWorld* world, int mouseX, int mouseY);
void world_map_passive_motion(GameWorld* world, int x, int y);
void world_map_drag(GameWorld* world, int x, int y);
void world_map_drag_start(GameWorld* world, int x, int y);
void world_map_drag_end(GameWorld* world);
void world_map_scroll(GameWorld* world, int direction);
void world_map_update(GameWorld* world, float dt);

#endif /* WORLD_MAP_H */
