#ifndef QUEST_H
#define QUEST_H

#include "types.h"

Quest* quest_create(GameContext *game, const char *id, const char *name, 
                    const char *desc, const char *required_item, int count);
Quest* quest_find(GameContext *game, const char *id);
void quest_display(GameContext *game);
void quest_check_completion(GameContext *game);
void quest_complete(GameContext *game, const char *id);
void quest_activate(GameContext *game, const char *id);

#endif
