#include "savegame.h"
#include "scene.h"
#include "inventory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SAVE_DIR "data/saves"

bool ensure_save_dir(void) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", SAVE_DIR);
    return system(cmd) == 0;
}

char* get_save_path(int slot) {
    static char path[256];
    snprintf(path, sizeof(path), "%s/save_%d.dat", SAVE_DIR, slot);
    return path;
}

bool savegame_exists(int slot) {
    FILE *f = fopen(get_save_path(slot), "r");
    if (f) {
        fclose(f);
        return true;
    }
    return false;
}

bool savegame_save(GameContext *game, int slot) {
    if (!game || slot < 1 || slot > MAX_SAVE_SLOTS) {
        printf("无效的存档槽位。\n");
        return false;
    }

    if (!ensure_save_dir()) {
        printf("无法创建存档目录。\n");
        return false;
    }

    FILE *f = fopen(get_save_path(slot), "wb");
    if (!f) {
        printf("无法打开存档文件。\n");
        return false;
    }

    fwrite(&game->inventory, sizeof(Inventory), 1, f);
    fwrite(&game->quest_count, sizeof(int), 1, f);
    fwrite(game->quests, sizeof(Quest), game->quest_count, f);
    fwrite(&game->scene_count, sizeof(int), 1, f);
    
    if (game->current_scene) {
        fwrite(game->current_scene->id, MAX_NAME_LEN, 1, f);
    } else {
        char empty_id[MAX_NAME_LEN] = {0};
        fwrite(empty_id, MAX_NAME_LEN, 1, f);
    }
    fwrite(&game->in_dialogue, sizeof(bool), 1, f);
    fwrite(&game->in_shop, sizeof(bool), 1, f);

    fclose(f);
    printf("游戏已保存到槽位 %d\n", slot);
    return true;
}

bool savegame_load(GameContext *game, int slot) {
    if (!game || slot < 1 || slot > MAX_SAVE_SLOTS) {
        printf("无效的存档槽位。\n");
        return false;
    }

    FILE *f = fopen(get_save_path(slot), "rb");
    if (!f) {
        printf("存档文件不存在。\n");
        return false;
    }

    fread(&game->inventory, sizeof(Inventory), 1, f);
    fread(&game->quest_count, sizeof(int), 1, f);
    fread(game->quests, sizeof(Quest), game->quest_count, f);
    fread(&game->scene_count, sizeof(int), 1, f);
    
    char scene_id[MAX_NAME_LEN];
    fread(scene_id, MAX_NAME_LEN, 1, f);
    
    game->current_scene = NULL;
    for (int i = 0; i < game->scene_count; i++) {
        if (strcmp(game->scenes[i].id, scene_id) == 0) {
            game->current_scene = &game->scenes[i];
            break;
        }
    }

    fread(&game->in_dialogue, sizeof(bool), 1, f);
    fread(&game->in_shop, sizeof(bool), 1, f);

    fclose(f);
    printf("游戏已从槽位 %d 加载\n", slot);
    
    if (game->current_scene) {
        display_scene(game);
    }
    
    return true;
}

void savegame_list_slots(void) {
    printf("\n========== 存档列表 ==========\n");
    for (int i = 1; i <= MAX_SAVE_SLOTS; i++) {
        if (savegame_exists(i)) {
            printf("  [%d] 有存档\n", i);
        } else {
            printf("  [%d] 空\n", i);
        }
    }
    printf("================================\n\n");
}

void savegame_display_info(int slot) {
    if (!savegame_exists(slot)) {
        printf("槽位 %d: 空\n", slot);
    } else {
        printf("槽位 %d: 有存档\n", slot);
    }
}
