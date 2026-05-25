#include "game.h"
#include "scene.h"
#include "inventory.h"
#include "npc.h"
#include "quest.h"
#include "savegame.h"
#include "ai.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* str_trim(char *str) {
    char *end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

void print_help(void) {
    printf("\n");
    printf("╔═══════════════════════════════════════════╗\n");
    printf("║            游戏命令帮助                   ║\n");
    printf("╠═══════════════════════════════════════════╣\n");
    printf("║ 移动：go [方向] / n,s,e,w,u,d           ║\n");
    printf("║ 查看：look / l                           ║\n");
    printf("║ 物品：take [物品] / drop [物品]          ║\n");
    printf("║ 背包：inventory / i                      ║\n");
    printf("║ NPC:   talk [NPC] / shop [NPC]            ║\n");
    printf("║ 任务：quest / quests                     ║\n");
    printf("║ 系统：save [槽位], load [槽位], help      ║\n");
    printf("║ AI:    ask [问题] (需要加载模型)          ║\n");
    printf("║ 退出：quit / exit                         ║\n");
    printf("╚═══════════════════════════════════════════╝\n\n");
}

void init_game_world(GameContext *game) {
    // 创建场景
    Scene *village = create_scene(game, "village", "新手村广场", 
                 "你站在一个宁静的小村庄广场中央。四周是古朴的木屋，村民们忙碌地走动。\n"
                 "北方是村庄的铁匠铺，东方有通往森林的小路。");
    
    Scene *blacksmith_room = create_scene(game, "blacksmith", "铁匠铺",
                 "铁匠铺内炉火熊熊。铁匠正在敲打着一把剑，墙上挂满了各式武器和护甲。\n"
                 "南方是村庄广场。");
    
    Scene *forest = create_scene(game, "forest", "迷雾森林入口",
                 "茂密的树木遮天蔽日，薄雾在林间飘荡。一条小径通向森林深处。\n"
                 "西方是村庄广场，北方似乎有一些奇怪的光芒。");
    
    Scene *cave = create_scene(game, "cave", "神秘洞穴",
                 "这是一个阴暗潮湿的洞穴。水滴从钟乳石上滴落，深处有微弱的光芒闪烁。\n"
                 "南方是迷雾森林。\n"
                 "地上散落着一些发光的矿石...");

    // 设置场景连接
    add_scene_connection(village, "blacksmith");
    add_scene_connection(village, "forest");
    add_scene_connection(blacksmith_room, "village");
    add_scene_connection(forest, "village");
    add_scene_connection(forest, "cave");
    add_scene_connection(cave, "forest");

    // 创建 NPC
    NPC *blacksmith = create_npc(game, "blacksmith", "铁匠老王", 
                                  "一个强壮的中年铁匠，手臂上肌肉隆起，眼神专注。");
    npc_add_dialogue(blacksmith, "你好，冒险者！需要武器或护甲吗？");
    npc_add_dialogue(blacksmith, "最近森林里的哥布林越来越猖狂了...");
    npc_add_dialogue(blacksmith, "听说洞穴里有珍贵的魔法矿石。");
    npc_enable_shop(blacksmith);
    Item sword = {"iron_sword", "铁剑", "一把普通的铁剑", 50, 1, false};
    Item shield = {"wood_shield", "木盾", "简陋的木制盾牌", 30, 1, false};
    Item potion = {"health_potion", "生命药水", "恢复少量生命值", 20, 1, false};
    npc_add_shop_item(blacksmith, &sword);
    npc_add_shop_item(blacksmith, &shield);
    npc_add_shop_item(blacksmith, &potion);
    add_scene_npc(blacksmith_room, "blacksmith");

    NPC *elder = create_npc(game, "elder", "村长",
                            "一位白发苍苍的老人，眼神中透着智慧。");
    npc_add_dialogue(elder, "欢迎来到我们的村庄，年轻人。");
    npc_add_dialogue(elder, "村庄最近不太平，希望你能帮忙。");
    add_scene_npc(village, "elder");

    // 创建任务
    quest_create(game, "kill_goblins", "清剿哥布林",
                 "村庄附近的哥布林越来越多，需要教训它们。", NULL, 0);
    quest_create(game, "collect_crystals", "收集水晶矿",
                 "铁匠需要水晶矿来锻造武器，帮他收集 3 块。", "crystal_ore", 3);

    // 初始场景
    game->current_scene = village;

    // 初始化背包
    inventory_init(&game->inventory);
    
    // 添加一些初始物品
    inventory_add(&game->inventory, "bread", "面包", "普通的面包，可以食用", 5, 2, false);

    game->running = true;
    game->in_dialogue = false;
    game->in_shop = false;
    game->current_npc_index = -1;
}

void game_start(GameContext *game, AIContext *ai) {
    (void)ai;  // Suppress unused parameter warning
    
    printf("\n");
    printf("╔═══════════════════════════════════════════════════╗\n");
    printf("║                                                   ║\n");
    printf("║          █████╗ ██╗  ██╗███████╗███╗   ███╗      ║\n");
    printf("║         ██╔══██╗██║  ██║██╔════╝████╗ ████║      ║\n");
    printf("║         ███████║███████║█████╗  ██╔████╔██║      ║\n");
    printf("║         ██╔══██║██╔══██║██╔══╝  ██║╚██╔╝██║      ║\n");
    printf("║         ██║  ██║██║  ██║███████╗██║ ╚═╝ ██║      ║\n");
    printf("║         ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝     ╚═╝      ║\n");
    printf("║                                                   ║\n");
    printf("║              文字冒险游戏 v1.0                    ║\n");
    printf("║              Powered by llama.cpp                 ║\n");
    printf("║                                                   ║\n");
    printf("╚═══════════════════════════════════════════════════╝\n\n");

    printf("初始化游戏世界...\n");
    init_game_world(game);
    
    printf("欢迎来到冒险世界！\n");
    printf("输入 'help' 查看游戏命令。\n");
    printf("输入 'ask [问题]' 使用 AI 对话（需要先加载模型）。\n\n");

    display_scene(game);
}

void handle_take_item(GameContext *game, const char *item_name) {
    if (!game || !item_name) return;

    if (game->in_dialogue || game->in_shop) {
        printf("请先结束对话或离开商店！\n");
        return;
    }

    if (game->current_scene && strcmp(game->current_scene->id, "cave") == 0) {
        if (strstr(item_name, "水晶") || strstr(item_name, "ore") || 
            strstr(item_name, "crystal") || strstr(item_name, "矿")) {
            inventory_add(&game->inventory, "crystal_ore", "水晶矿",
                         "闪烁着光芒的神秘矿石", 100, 1, true);
            printf("你拾取了：水晶矿 x1\n");
            
            quest_check_completion(game);
            return;
        }
    }

    printf("这里没有这个物品可供拾取。\n");
}

void handle_drop_item(GameContext *game, const char *item_id) {
    if (!game || !item_id) return;

    if (game->in_dialogue || game->in_shop) {
        printf("请先结束对话或离开商店！\n");
        return;
    }

    Item *item = inventory_find(&game->inventory, item_id);
    if (!item) {
        for (int i = 0; i < game->inventory.count; i++) {
            if (strstr(game->inventory.items[i].name, item_id)) {
                item = &game->inventory.items[i];
                break;
            }
        }
    }

    if (!item) {
        printf("背包里没有这个物品。\n");
        return;
    }

    if (item->quest_item) {
        printf("任务物品无法丢弃！\n");
        return;
    }

    inventory_remove(&game->inventory, item->id, 1);
    printf("丢弃了：%s\n", item->name);
}

void handle_ask_ai(GameContext *game, AIContext *ai, const char *question) {
    (void)game;  // Suppress unused parameter warning
    
    if (!ai || !question) {
        printf("AI 未初始化或问题为空。\n");
        return;
    }

    if (!ai_is_loaded(ai)) {
        printf("AI 模型未加载。请使用 GGUF 模型文件启动游戏。\n");
        printf("示例：./adventure-game models/your-model.gguf\n");
        return;
    }

    printf("\n思考中...\n\n");
    char *response = ai_generate_response(ai, question, 300);
    if (response) {
        printf("AI: %s\n\n", response);
        free(response);
    }
}

void game_handle_command(GameContext *game, AIContext *ai, const char *input) {
    if (!game || !input) return;

    char cmd[256];
    strncpy(cmd, input, sizeof(cmd) - 1);
    cmd[sizeof(cmd) - 1] = '\0';
    
    char *trimmed = str_trim(cmd);
    if (strlen(trimmed) == 0) return;

    char *space = strchr(trimmed, ' ');
    char command[64] = {0};
    char parameter[192] = {0};

    if (space) {
        size_t cmd_len = (size_t)(space - trimmed);
        if (cmd_len >= sizeof(command)) cmd_len = sizeof(command) - 1;
        strncpy(command, trimmed, cmd_len);
        command[cmd_len] = '\0';
        strncpy(parameter, space + 1, sizeof(parameter) - 1);
        parameter[sizeof(parameter) - 1] = '\0';
    } else {
        strncpy(command, trimmed, sizeof(command) - 1);
    }

    if (game->in_dialogue) {
        handle_dialogue_input(game, ai, input);
        return;
    }

    if (game->in_shop) {
        if (strcasecmp(command, "exit") == 0 || strcasecmp(command, "离开") == 0 ||
            strcasecmp(command, "quit") == 0) {
            close_shop(game);
            return;
        } else if (strcasecmp(command, "buy") == 0) {
            int index = atoi(parameter);
            buy_item(game, index);
            display_shop(game);
            return;
        } else if (strcasecmp(command, "sell") == 0) {
            sell_item(game, parameter);
            display_shop(game);
            return;
        } else if (strcasecmp(command, "look") == 0 || strcasecmp(command, "l") == 0) {
            display_shop(game);
            return;
        } else {
            printf("商店模式下只支持：buy [编号], sell [物品 ID], look, exit\n");
            return;
        }
    }

    if (strcasecmp(command, "help") == 0 || strcasecmp(command, "h") == 0) {
        print_help();
    }
    else if (strcasecmp(command, "look") == 0 || strcasecmp(command, "l") == 0) {
        display_scene(game);
    }
    else if (strcasecmp(command, "go") == 0 || strcasecmp(command, "move") == 0) {
        move_to_scene(game, parameter);
    }
    else if (strcasecmp(command, "n") == 0 || strcasecmp(command, "north") == 0) {
        printf("请使用 'go [方向]' 命令，如：go 铁匠铺\n");
    }
    else if (strcasecmp(command, "s") == 0 || strcasecmp(command, "south") == 0) {
        printf("请使用 'go [方向]' 命令，如：go 新手村广场\n");
    }
    else if (strcasecmp(command, "e") == 0 || strcasecmp(command, "east") == 0) {
        printf("请使用 'go [方向]' 命令，如：go 迷雾森林入口\n");
    }
    else if (strcasecmp(command, "w") == 0 || strcasecmp(command, "west") == 0) {
        printf("请使用 'go [方向]' 命令，如：go 新手村广场\n");
    }
    else if (strcasecmp(command, "take") == 0 || strcasecmp(command, "get") == 0 ||
             strcasecmp(command, "拾取") == 0) {
        handle_take_item(game, parameter);
    }
    else if (strcasecmp(command, "drop") == 0 || strcasecmp(command, "丢弃") == 0) {
        handle_drop_item(game, parameter);
    }
    else if (strcasecmp(command, "inventory") == 0 || strcasecmp(command, "i") == 0 ||
             strcasecmp(command, "inv") == 0 || strcasecmp(command, "背包") == 0) {
        inventory_display(&game->inventory);
    }
    else if (strcasecmp(command, "talk") == 0 || strcasecmp(command, "speak") == 0 ||
             strcasecmp(command, "对话") == 0) {
        start_dialogue(game, ai, parameter);
    }
    else if (strcasecmp(command, "shop") == 0 || strcasecmp(command, "store") == 0 ||
             strcasecmp(command, "商店") == 0) {
        open_shop(game, parameter);
    }
    else if (strcasecmp(command, "quest") == 0 || strcasecmp(command, "quests") == 0 ||
             strcasecmp(command, "任务") == 0) {
        quest_display(game);
    }
    else if (strcasecmp(command, "save") == 0) {
        int slot = atoi(parameter);
        if (slot == 0) slot = 1;
        savegame_save(game, slot);
    }
    else if (strcasecmp(command, "load") == 0) {
        int slot = atoi(parameter);
        if (slot == 0) slot = 1;
        savegame_load(game, slot);
    }
    else if (strcasecmp(command, "saves") == 0) {
        savegame_list_slots();
    }
    else if (strcasecmp(command, "ask") == 0 || strcasecmp(command, "ai") == 0) {
        handle_ask_ai(game, ai, parameter);
    }
    else if (strcasecmp(command, "quit") == 0 || strcasecmp(command, "exit") == 0 ||
             strcasecmp(command, "q") == 0 || strcasecmp(command, "退出") == 0) {
        printf("感谢您的游玩，再见！\n");
        game->running = false;
    }
    else if (strcasecmp(command, "铁匠铺") == 0 || strcasecmp(command, "north") == 0) {
        move_to_scene(game, "铁匠铺");
    }
    else if (strcasecmp(command, "森林") == 0 || strcasecmp(command, "east") == 0) {
        move_to_scene(game, "森林");
    }
    else {
        printf("未知的命令：%s (输入 'help' 查看帮助)\n", command);
    }
}

void game_loop(GameContext *game, AIContext *ai) {
    char input[1024];

    while (game->running) {
        printf("> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n输入错误，游戏结束。\n");
            break;
        }

        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

        if (strlen(str_trim(input)) == 0) {
            continue;
        }

        game_handle_command(game, ai, input);
    }
}

void game_cleanup(GameContext *game, AIContext *ai) {
    if (!game) return;
    game->running = false;
    
    if (ai) {
        ai_cleanup(ai);
    }
    
    printf("游戏已清理。\n");
}
