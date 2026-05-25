#ifndef NPC_H
#define NPC_H

#include "types.h"

NPC* create_npc(GameContext *game, const char *id, const char *name, const char *desc);
void npc_add_dialogue(NPC *npc, const char *dialogue);
void npc_enable_shop(NPC *npc);
void npc_add_shop_item(NPC *npc, Item *item);
void display_npc(NPC *npc);
void start_dialogue(GameContext *game, AIContext *ai, const char *npc_id);
void end_dialogue(GameContext *game);
void handle_dialogue_input(GameContext *game, AIContext *ai, const char *input);
void open_shop(GameContext *game, const char *npc_id);
void close_shop(GameContext *game);
void display_shop(GameContext *game);
void buy_item(GameContext *game, int item_index);
void sell_item(GameContext *game, const char *item_id);

#endif
