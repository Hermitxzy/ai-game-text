#ifndef SCENE_H
#define SCENE_H

#include "types.h"

Scene* create_scene(GameContext *game, const char *id, const char *name, const char *desc);
void add_scene_connection(Scene *scene, const char *target_id);
void add_scene_npc(Scene *scene, const char *npc_id);
void add_scene_item(Scene *scene, const char *item_id);
Scene* find_scene_by_id(GameContext *game, const char *id);
void display_scene(GameContext *game);
void move_to_scene(GameContext *game, const char *direction);

#endif
