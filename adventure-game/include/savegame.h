#ifndef SAVEGAME_H
#define SAVEGAME_H

#include "types.h"

// 前向声明
struct GameContext;

bool savegame_save(struct GameContext *game, int slot);
bool savegame_load(struct GameContext *game, int slot);
void savegame_list_slots(void);
void savegame_display_info(int slot);
bool savegame_exists(int slot);

#endif
