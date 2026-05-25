#include "quest.h"
#include "inventory.h"
#include <stdio.h>
#include <string.h>

Quest* quest_create(GameContext *game, const char *id, const char *name, 
                    const char *desc, const char *required_item, int count) {
    if (!game || game->quest_count >= MAX_QUESTS) return NULL;

    Quest *quest = &game->quests[game->quest_count++];
    strncpy(quest->id, id, MAX_NAME_LEN - 1);
    strncpy(quest->name, name, MAX_NAME_LEN - 1);
    strncpy(quest->description, desc, MAX_DESC_LEN - 1);
    quest->completed = false;
    quest->active = true;
    strncpy(quest->required_item_id, required_item ? required_item : "", MAX_NAME_LEN - 1);
    quest->required_item_count = count;

    return quest;
}

Quest* quest_find(GameContext *game, const char *id) {
    if (!game || !id) return NULL;

    for (int i = 0; i < game->quest_count; i++) {
        if (strcmp(game->quests[i].id, id) == 0) {
            return &game->quests[i];
        }
    }
    return NULL;
}

void quest_display(GameContext *game) {
    if (!game) return;

    printf("\n========== 任务列表 ==========\n");
    
    int active_count = 0;
    for (int i = 0; i < game->quest_count; i++) {
        Quest *quest = &game->quests[i];
        if (quest->active) {
            printf("\n[%s] %s\n", quest->completed ? "已完成" : "进行中", quest->name);
            printf("  %s\n", quest->description);
            
            if (quest->required_item_id[0] != '\0' && !quest->completed) {
                int current_count = inventory_count_item(&game->inventory, quest->required_item_id);
                printf("  需要：%s %d/%d\n", quest->required_item_id, current_count, quest->required_item_count);
            }
            active_count++;
        }
    }

    if (active_count == 0) {
        printf("  当前没有进行中的任务\n");
    }
    printf("================================\n\n");
}

void quest_check_completion(GameContext *game) {
    if (!game) return;

    for (int i = 0; i < game->quest_count; i++) {
        Quest *quest = &game->quests[i];
        if (quest->active && !quest->completed && quest->required_item_id[0] != '\0') {
            int current_count = inventory_count_item(&game->inventory, quest->required_item_id);
            if (current_count >= quest->required_item_count) {
                quest_complete(game, quest->id);
            }
        }
    }
}

void quest_complete(GameContext *game, const char *id) {
    if (!game || !id) return;

    Quest *quest = quest_find(game, id);
    if (!quest) return;

    quest->completed = true;
    quest->active = false;
    
    printf("\n★ 任务完成 ★\n");
    printf("  %s\n", quest->name);
    printf("  奖励: 金币 +100\n\n");
    
    game->inventory.gold += 100;
}

void quest_activate(GameContext *game, const char *id) {
    if (!game || !id) return;

    Quest *quest = quest_find(game, id);
    if (!quest) return;

    quest->active = true;
    printf("激活任务：%s\n", quest->name);
}
