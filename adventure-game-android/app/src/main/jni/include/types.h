#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_NAME_LEN 64
#define MAX_DESC_LEN 512
#define MAX_MESSAGE_LEN 1024
#define MAX_ITEMS 100
#define MAX_NPCS 50
#define MAX_QUESTS 30
#define MAX_SCENES 100
#define MAX_DIALOGUE_LEN 4096
#define MAX_SAVE_SLOTS 5
#define MAX_SCENE_CONNECTIONS 10
#define MAX_SCENE_NPCS 10
#define MAX_SCENE_ITEMS 10
#define MAX_NPC_DIALOGUES 20
#define MAX_NPC_SHOP_ITEMS 20

typedef struct Item {
    char id[MAX_NAME_LEN];
    char name[MAX_NAME_LEN];
    char description[MAX_DESC_LEN];
    int value;
    int quantity;
    bool quest_item;
} Item;

typedef struct Inventory {
    Item items[MAX_ITEMS];
    int count;
    int gold;
} Inventory;

typedef struct Quest {
    char id[MAX_NAME_LEN];
    char name[MAX_NAME_LEN];
    char description[MAX_DESC_LEN];
    bool completed;
    bool active;
    int required_item_count;
    char required_item_id[MAX_NAME_LEN];
} Quest;

typedef struct NPC {
    char id[MAX_NAME_LEN];
    char name[MAX_NAME_LEN];
    char description[MAX_DESC_LEN];
    int dialogue_count;
    char **dialogues;
    bool has_shop;
    int shop_items_count;
    Item *shop_items;
} NPC;

typedef struct Scene {
    char id[MAX_NAME_LEN];
    char name[MAX_NAME_LEN];
    char description[MAX_DESC_LEN];
    char connected_scenes[MAX_SCENE_CONNECTIONS][MAX_NAME_LEN];
    int connected_count;
    char npcs[MAX_SCENE_NPCS][MAX_NAME_LEN];
    int npc_count;
    char items[MAX_SCENE_ITEMS][MAX_NAME_LEN];
    int item_count;
} Scene;

typedef struct GameContext GameContext;

struct GameContext {
    Scene *current_scene;
    Inventory inventory;
    Quest quests[MAX_QUESTS];
    int quest_count;
    NPC npcs[MAX_NPCS];
    int npc_count;
    Scene scenes[MAX_SCENES];
    int scene_count;
    bool running;
    char player_name[MAX_NAME_LEN];
    int current_npc_index;
    bool in_dialogue;
    bool in_shop;
};

typedef struct AIContext {
    void *model;
    void *context;
    char model_path[512];
    int64_t n_ctx;
    bool loaded;
} AIContext;

#endif
