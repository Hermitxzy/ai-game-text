#ifndef GAME_H
#define GAME_H

#include "types.h"

void game_init(GameContext *game);
void game_start(GameContext *game, AIContext *ai);
void game_loop(GameContext *game, AIContext *ai);
void game_cleanup(GameContext *game, AIContext *ai);
void game_handle_command(GameContext *game, AIContext *ai, const char *command);
void print_help(void);

#endif
