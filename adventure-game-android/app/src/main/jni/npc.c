#include "npc.h"
#include "inventory.h"
#include "scene.h"
#include "ai.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static NPC static_npcs[MAX_NPCS];
static int static_npc_count = 0;

NPC* create_npc(GameContext *game, const char *id, const char *name, const char *desc) {
    if (!game || static_npc_count >= MAX_NPCS) return NULL;

    NPC *npc = &static_npcs[static_npc_count++];
    strncpy(npc->id, id, MAX_NAME_LEN - 1);
    strncpy(npc->name, name, MAX_NAME_LEN - 1);
    strncpy(npc->description, desc, MAX_DESC_LEN - 1);
    npc->dialogue_count = 0;
    npc->dialogues = NULL;
    npc->has_shop = false;
    npc->shop_items = NULL;
    npc->shop_items_count = 0;

    game->npcs[game->npc_count++] = *npc;
    return npc;
}

void npc_add_dialogue(NPC *npc, const char *dialogue) {
    if (!npc) return;
    npc->dialogues = realloc(npc->dialogues, (npc->dialogue_count + 1) * sizeof(char*));
    npc->dialogues[npc->dialogue_count] = strdup(dialogue);
    npc->dialogue_count++;
}

void npc_enable_shop(NPC *npc) {
    if (!npc) return;
    npc->has_shop = true;
    npc->shop_items = malloc(MAX_NPC_SHOP_ITEMS * sizeof(Item));
    npc->shop_items_count = 0;
}

void npc_add_shop_item(NPC *npc, Item *item) {
    if (!npc || !npc->shop_items || npc->shop_items_count >= MAX_NPC_SHOP_ITEMS) return;
    npc->shop_items[npc->shop_items_count++] = *item;
}

void display_npc(NPC *npc) {
    if (!npc) return;
    printf("\n%s\n", npc->name);
    printf("─────────────────────────────\n");
    printf("%s\n", npc->description);
    printf("─────────────────────────────\n\n");
}

void start_dialogue(GameContext *game, AIContext *ai, const char *npc_id) {
    if (!game || !npc_id) return;

    NPC *npc = NULL;
    for (int i = 0; i < game->npc_count; i++) {
        if (strcmp(game->npcs[i].id, npc_id) == 0) {
            npc = &game->npcs[i];
            break;
        }
    }

    if (!npc) {
        printf("没找到这个 NPC。\n");
        return;
    }

    game->in_dialogue = true;
    game->current_npc_index = -1;
    for (int i = 0; i < game->npc_count; i++) {
        if (strcmp(game->npcs[i].id, npc_id) == 0) {
            game->current_npc_index = i;
            break;
        }
    }

    printf("\n========== 与 %s 对话 ==========\n", npc->name);
    if (npc->dialogue_count > 0) {
        printf("预设对话:\n");
        for (int i = 0; i < npc->dialogue_count && i < 5; i++) {
            printf("  [%d] %s\n", i + 1, npc->dialogues[i]);
        }
    }
    printf("\n输入对话内容，或输入 'bye' 结束对话\n");
    printf("==================================\n\n");

    if (ai && ai_is_loaded(ai)) {
        char prompt[512];
        snprintf(prompt, sizeof(prompt), 
                 "玩家与 NPC %s 对话。背景：%s。生成简短问候语 (50 字内)",
                 npc->name, npc->description);
        
        char *response = ai_generate_response(ai, prompt, 100);
        if (response) {
            printf("%s: %s\n\n", npc->name, response);
            free(response);
        }
    } else {
        printf("%s: 欢迎来到这里，冒险者！有什么我可以帮你的吗？\n\n", npc->name);
    }
}

void end_dialogue(GameContext *game) {
    if (!game) return;
    game->in_dialogue = false;
    game->current_npc_index = -1;
    printf("结束了对话。\n");
}

void handle_dialogue_input(GameContext *game, AIContext *ai, const char *input) {
    if (!game || !input) return;

    if (strcasecmp(input, "bye") == 0 || strcasecmp(input, "exit") == 0 ||
        strcasecmp(input, "quit") == 0 || strcasecmp(input, "离开") == 0) {
        end_dialogue(game);
        return;
    }

    NPC *npc = &game->npcs[game->current_npc_index];
    if (!npc) return;

    printf("你：%s\n", input);

    if (ai && ai_is_loaded(ai)) {
        char prompt[1024];
        snprintf(prompt, sizeof(prompt),
                 "玩家对 NPC %s 说：%s。背景：%s。生成 NPC 简短回复 (100 字内)",
                 npc->name, input, npc->description);

        char *response = ai_generate_response(ai, prompt, 150);
        if (response) {
            printf("%s: %s\n\n", npc->name, response);
            free(response);
        }
    } else {
        printf("%s: 嗯，我明白了。在这里冒险要小心谨慎。\n\n", npc->name);
    }
}

void open_shop(GameContext *game, const char *npc_id) {
    if (!game || !npc_id) return;

    NPC *npc = NULL;
    for (int i = 0; i < game->npc_count; i++) {
        if (strcmp(game->npcs[i].id, npc_id) == 0) {
            npc = &game->npcs[i];
            break;
        }
    }

    if (!npc || !npc->has_shop) {
        printf("这个 NPC 没有商店。\n");
        return;
    }

    game->in_shop = true;
    game->current_npc_index = -1;
    for (int i = 0; i < game->npc_count; i++) {
        if (strcmp(game->npcs[i].id, npc_id) == 0) {
            game->current_npc_index = i;
            break;
        }
    }

    printf("\n========== %s 的商店 ==========\n", npc->name);
    display_shop(game);
}

void close_shop(GameContext *game) {
    if (!game) return;
    game->in_shop = false;
    printf("离开了商店。\n");
}

void display_shop(GameContext *game) {
    if (!game) return;

    NPC *npc = &game->npcs[game->current_npc_index];
    if (!npc || !npc->has_shop) return;

    printf("\n商品列表:\n");
    printf("─────────────────────────────\n");
    for (int i = 0; i < npc->shop_items_count; i++) {
        Item *item = &npc->shop_items[i];
        printf("  [%d] %-20s %5d 金币\n", i + 1, item->name, item->value);
    }
    printf("─────────────────────────────\n");
    printf("当前金币：%d\n", game->inventory.gold);
    printf("命令：buy [编号], sell [物品 ID], exit\n");
    printf("=================================\n\n");
}

void buy_item(GameContext *game, int item_index) {
    if (!game) return;

    NPC *npc = &game->npcs[game->current_npc_index];
    if (!npc || !npc->has_shop) return;

    if (item_index < 1 || item_index > npc->shop_items_count) {
        printf("无效的商品编号。\n");
        return;
    }

    Item *shop_item = &npc->shop_items[item_index - 1];
    if (game->inventory.gold < shop_item->value) {
        printf("金币不足！\n");
        return;
    }

    game->inventory.gold -= shop_item->value;
    Item *inv_item = inventory_find(&game->inventory, shop_item->id);
    if (inv_item) {
        inv_item->quantity++;
    } else {
        inventory_add(&game->inventory, shop_item->id, shop_item->name,
                     shop_item->description, shop_item->value, 1, shop_item->quest_item);
    }

    printf("购买了 %s x1，花费 %d 金币。\n", shop_item->name, shop_item->value);
    printf("剩余金币：%d\n", game->inventory.gold);
}

void sell_item(GameContext *game, const char *item_id) {
    if (!game || !item_id) return;

    NPC *npc = &game->npcs[game->current_npc_index];
    if (!npc || !npc->has_shop) return;

    Item *inv_item = inventory_find(&game->inventory, item_id);
    if (!inv_item || inv_item->quantity <= 0) {
        printf("没有这个物品。\n");
        return;
    }

    if (inv_item->quest_item) {
        printf("任务物品无法出售。\n");
        return;
    }

    int sell_value = inv_item->value / 2;
    game->inventory.gold += sell_value;
    inventory_remove(&game->inventory, item_id, 1);

    printf("出售了 %s，获得 %d 金币。\n", inv_item->name, sell_value);
    printf("当前金币：%d\n", game->inventory.gold);
}
