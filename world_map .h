#ifndef WORLD_MAP_H
#define WORLD_MAP_H

#include "models.h"

/* Initialize world map data (cities, routes, globe parameters) */
void world_map_init(WorldMap* wm);

/* Update background animation (SEIR, routes, globe rotation) */
void world_map_update(WorldMap* wm, float dt);

/* Input handling */
void world_map_mouse_drag(WorldMap* wm, int x, int y);
void world_map_scroll(WorldMap* wm, int delta);
void world_map_scroll_ui(WorldMap* wm, int delta);

/* Render the 3D globe world map screen */
void render_world_map(WorldMap* wm, float gameTime);

/* Hit-test: returns country index under (mouseX, mouseY), or -1 */
int world_map_hit_test(const WorldMap* wm, int mouseX, int mouseY);

/* ═══════════════════════════════════════════
   VIRUS SELECTION OVERLAY
   ═══════════════════════════════════════════ */

/* Render the virus selection overlay after a country is clicked */
void render_virus_select(WorldMap* wm, float gameTime);

/* Hit-test for virus cards: returns virus type index, or -1 */
int virus_select_hit_test(const WorldMap* wm, int mouseX, int mouseY);

#endif
