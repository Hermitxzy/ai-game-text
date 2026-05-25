#include "scene.h"
#include <stdio.h>
#include <string.h>

Scene* create_scene(GameContext *game, const char *id, const char *name, const char *desc) {
    if (!game || game->scene_count >= MAX_SCENES) return NULL;

    Scene *scene = &game->scenes[game->scene_count++];
    strncpy(scene->id, id, MAX_NAME_LEN - 1);
    strncpy(scene->name, name, MAX_NAME_LEN - 1);
    strncpy(scene->description, desc, MAX_DESC_LEN - 1);
    scene->connected_count = 0;
    scene->npc_count = 0;
    scene->item_count = 0;

    return scene;
}

void add_scene_connection(Scene *scene, const char *target_id) {
    if (!scene || scene->connected_count >= MAX_SCENE_CONNECTIONS) return;
    strncpy(scene->connected_scenes[scene->connected_count++], target_id, MAX_NAME_LEN - 1);
}

void add_scene_npc(Scene *scene, const char *npc_id) {
    if (!scene || scene->npc_count >= MAX_SCENE_NPCS) return;
    strncpy(scene->npcs[scene->npc_count++], npc_id, MAX_NAME_LEN - 1);
}

void add_scene_item(Scene *scene, const char *item_id) {
    if (!scene || scene->item_count >= MAX_SCENE_ITEMS) return;
    strncpy(scene->items[scene->item_count++], item_id, MAX_NAME_LEN - 1);
}

Scene* find_scene_by_id(GameContext *game, const char *id) {
    if (!game || !id) return NULL;

    for (int i = 0; i < game->scene_count; i++) {
        if (strcmp(game->scenes[i].id, id) == 0) {
            return &game->scenes[i];
        }
    }
    return NULL;
}

void display_scene(GameContext *game) {
    if (!game || !game->current_scene) return;

    Scene *scene = game->current_scene;

    printf("\n");
    printf("┌──────────────────────────────────────────────┐\n");
    printf("│ %-44s │\n", scene->name);
    printf("├──────────────────────────────────────────────┤\n");
    printf("%s\n", scene->description);

    if (scene->connected_count > 0) {
        printf("\n可前往：");
        for (int i = 0; i < scene->connected_count; i++) {
            Scene *conn = find_scene_by_id(game, scene->connected_scenes[i]);
            if (conn) {
                printf("[%s] ", conn->name);
            }
        }
        printf("\n");
    }

    if (scene->npc_count > 0) {
        printf("NPC: ");
        for (int i = 0; i < scene->npc_count; i++) {
            for (int j = 0; j < game->npc_count; j++) {
                if (strcmp(game->npcs[j].id, scene->npcs[i]) == 0) {
                    printf("[%s] ", game->npcs[j].name);
                    break;
                }
            }
        }
        printf("\n");
    }

    printf("└──────────────────────────────────────────────┘\n\n");
}

void move_to_scene(GameContext *game, const char *direction) {
    if (!game || !game->current_scene || !direction) return;

    if (game->in_dialogue || game->in_shop) {
        printf("请先结束对话或离开商店！\n");
        return;
    }

    Scene *target = NULL;
    for (int i = 0; i < game->current_scene->connected_count; i++) {
        Scene *conn = find_scene_by_id(game, game->current_scene->connected_scenes[i]);
        if (conn && (strstr(conn->name, direction) || strstr(conn->id, direction))) {
            target = conn;
            break;
        }
    }

    if (target) {
        game->current_scene = target;
        display_scene(game);
    } else {
        printf("无法向那个方向移动！使用 'look' 查看可用出口。\n");
    }
}
