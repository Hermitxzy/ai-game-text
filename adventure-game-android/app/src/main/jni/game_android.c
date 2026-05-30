// Android 游戏入口（HTTP API 版）
#include <jni.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <android/log.h>

#define LOG_TAG "AdventureGame"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// 简化版场景定义
typedef struct Scene {
    char id[64];
    char name[64];
    char description[512];
    char connections[256];  // 可到达的场景列表
    char npcs[256];         // 当前场景的 NPC
} Scene;

// 简化版任务定义
typedef struct Quest {
    char id[64];
    char name[64];
    char description[512];
    int completed;
    int active;
} Quest;

// 简化版背包定义
typedef struct Inventory {
    int gold;
    int item_count;
} Inventory;

// 游戏状态
typedef struct GameContext {
    Scene scenes[10];
    int scene_count;
    Scene *current_scene;
    Inventory inventory;
    Quest quests[30];
    int quest_count;
    int running;
} GameContext;

static GameContext g_game;
static int g_initialized = 0;

// 初始化游戏
JNIEXPORT jboolean JNICALL Java_com_adventure_game_GameActivity_initGame(
    JNIEnv *env, jobject thiz, jstring modelPath) {
    (void)thiz; (void)modelPath;
    
    if (g_initialized) return JNI_TRUE;
    
    LOGI("=== 初始化游戏 ===");
    memset(&g_game, 0, sizeof(GameContext));
    
    // 创建场景
    strcpy(g_game.scenes[0].id, "village");
    strcpy(g_game.scenes[0].name, "新手村广场");
    strcpy(g_game.scenes[0].description, "你站在一个宁静的小村庄广场中央。四周是古朴的木屋，村民们忙碌地走动。北方是铁匠铺，东方有通往森林的小路。");
    strcpy(g_game.scenes[0].connections, "铁匠铺，迷雾森林入口");
    strcpy(g_game.scenes[0].npcs, "村长，村民");
    
    strcpy(g_game.scenes[1].id, "blacksmith");
    strcpy(g_game.scenes[1].name, "铁匠铺");
    strcpy(g_game.scenes[1].description, "铁匠铺内炉火熊熊。墙上挂满了各式武器和护甲。");
    strcpy(g_game.scenes[1].connections, "新手村广场");
    strcpy(g_game.scenes[1].npcs, "铁匠老王");
    
    strcpy(g_game.scenes[2].id, "forest");
    strcpy(g_game.scenes[2].name, "迷雾森林入口");
    strcpy(g_game.scenes[2].description, "茂密的树木遮天蔽日，薄雾在林间飘荡。一条小径通向森林深处。");
    strcpy(g_game.scenes[2].connections, "新手村广场");
    strcpy(g_game.scenes[2].npcs, "");
    
    g_game.scene_count = 3;
    g_game.current_scene = &g_game.scenes[0];
    
    // 初始化背包
    g_game.inventory.gold = 50;
    g_game.inventory.item_count = 1;
    
    // 创建任务
    strcpy(g_game.quests[0].id, "kill_goblins");
    strcpy(g_game.quests[0].name, "清剿哥布林");
    strcpy(g_game.quests[0].description, "村庄附近的哥布林越来越多，需要教训它们。");
    g_game.quests[0].active = 1;
    g_game.quest_count = 1;
    
    g_game.running = 1;
    g_initialized = 1;
    
    LOGI("游戏初始化完成");
    return JNI_TRUE;
}

// 处理输入（游戏命令）
JNIEXPORT jstring JNICALL Java_com_adventure_game_GameActivity_processInput(
    JNIEnv *env, jobject thiz, jstring input) {
    (void)thiz;
    
    const char *inputText = (*env)->GetStringUTFChars(env, input, NULL);
    LOGI("输入：%s", inputText);
    
    char response[4096] = {0};
    
    // 简单命令处理
    if (strcmp(inputText, "help") == 0 || strcmp(inputText, "h") == 0) {
        strcpy(response, 
            "命令帮助:\n"
            "  look - 查看当前场景\n"
            "  map - 查看完整地图\n"
            "  go [地点] - 移动\n"
            "  inventory - 查看背包\n"
            "  take [物品] - 拾取物品\n"
            "  talk [NPC] - 与 NPC 对话\n"
            "  quest - 查看任务\n"
            "  ask [问题] - AI 对话（需要 API 服务器）\n"
            "  exit - 退出");
    }
    else if (strcmp(inputText, "map") == 0 || strcmp(inputText, "m") == 0) {
        char map_text[4096] = "=== 世界地图 ===\n\n";
        for (int i = 0; i < g_game.scene_count; i++) {
            Scene *s = &g_game.scenes[i];
            char line[512];
            snprintf(line, sizeof(line), "%s", s->name);
            if (strcmp(g_game.current_scene->id, s->id) == 0) {
                strcat(line, " [你在这里]");
            }
            strcat(map_text, line);
            strcat(map_text, "\n  可前往：");
            strcat(map_text, s->connections);
            if (strlen(s->npcs) > 0) {
                strcat(map_text, "\n  NPC: ");
                strcat(map_text, s->npcs);
            }
            strcat(map_text, "\n\n");
        }
        strcat(map_text, "■ 图例：[你在这里] = 当前位置");
        strcpy(response, map_text);
    }
    else if (strcmp(inputText, "look") == 0 || strcmp(inputText, "l") == 0) {
        snprintf(response, sizeof(response), "%s", g_game.current_scene->description);
        if (strcmp(g_game.current_scene->id, "village") == 0) {
            strcat(response, "\n\n可前往：铁匠铺、迷雾森林入口");
        } else if (strcmp(g_game.current_scene->id, "blacksmith") == 0) {
            strcat(response, "\n\n可前往：新手村广场");
        } else if (strcmp(g_game.current_scene->id, "forest") == 0) {
            strcat(response, "\n\n可前往：新手村广场");
        }
    }
    else if (strcmp(inputText, "inventory") == 0 || strcmp(inputText, "i") == 0) {
        snprintf(response, sizeof(response), "背包:\n  金币：%d\n  物品数量：%d", 
                 g_game.inventory.gold, g_game.inventory.item_count);
    }
    else if (strcmp(inputText, "quest") == 0) {
        if (g_game.quest_count > 0) {
            Quest *q = &g_game.quests[0];
            snprintf(response, sizeof(response), "任务 [%s]: %s\n状态：%s",
                     q->name, q->description, q->active ? "进行中" : "已完成");
        } else {
            strcpy(response, "当前没有任务");
        }
    }
    else if (strncmp(inputText, "go ", 3) == 0) {
        const char *dest = inputText + 3;
        int moved = 0;
        int found_scene = -1;
        
        for (int i = 0; i < g_game.scene_count; i++) {
            if (strcmp(g_game.scenes[i].name, dest) == 0 || 
                strcmp(g_game.scenes[i].id, dest) == 0) {
                found_scene = i;
                break;
            }
        }
        
        if (found_scene < 0) {
            snprintf(response, sizeof(response), "找不到地点：%s\n使用 'map' 查看所有地点。", dest);
        } else {
            char current_id[64];
            strcpy(current_id, g_game.current_scene->id);
            
            if (strcmp(current_id, "village") == 0 && found_scene == 1) {
                g_game.current_scene = &g_game.scenes[1];
                moved = 1;
            } else if (strcmp(current_id, "village") == 0 && found_scene == 2) {
                g_game.current_scene = &g_game.scenes[2];
                moved = 1;
            } else if ((strcmp(current_id, "blacksmith") == 0 || strcmp(current_id, "forest") == 0) && found_scene == 0) {
                g_game.current_scene = &g_game.scenes[0];
                moved = 1;
            }
            
            if (moved) {
                snprintf(response, sizeof(response), "你来到了 %s。\n\n%s\n\n可前往：%s",
                         g_game.current_scene->name,
                         g_game.current_scene->description,
                         g_game.current_scene->connections);
            } else {
                strcpy(response, "无法直接前往，请先查看地图确认路线。");
            }
        }
    }
    else if (strcmp(inputText, "exit") == 0 || strcmp(inputText, "quit") == 0) {
        g_game.running = 0;
        strcpy(response, "游戏结束，再见！");
    }
    else if (strncmp(inputText, "talk ", 5) == 0) {
        const char *npc_name = inputText + 5;
        int found = 0;
        
        if (strlen(g_game.current_scene->npcs) > 0) {
            char *npc_list = strdup(g_game.current_scene->npcs);
            char *npc = strtok(npc_list, "，");
            while (npc != NULL) {
                char trimmed[64];
                int len = strlen(npc);
                int start = 0, end = len - 1;
                while (start < len && npc[start] == ' ') start++;
                while (end >= 0 && npc[end] == ' ') end--;
                int i = 0;
                for (int j = start; j <= end; j++) {
                    trimmed[i++] = npc[j];
                }
                trimmed[i] = '\0';
                
                if (strcmp(trimmed, npc_name) == 0) {
                    found = 1;
                    break;
                }
                npc = strtok(NULL, "，");
            }
            free(npc_list);
        }
        
        if (found) {
            if (strcmp(npc_name, "铁匠老王") == 0) {
                strcpy(response, "铁匠老王：欢迎来到这里，冒险者！需要武器或护甲吗？");
            } else if (strcmp(npc_name, "村长") == 0) {
                strcpy(response, "村长：欢迎你，年轻的冒险者！村庄最近的哥布林越来越多，你能帮帮我们吗？");
            } else if (strcmp(npc_name, "村民") == 0) {
                strcpy(response, "村民：今天天气真好，适合出门冒险！");
            } else {
                snprintf(response, sizeof(response), "%s：你好，冒险者！", npc_name);
            }
        } else {
            snprintf(response, sizeof(response), "这里没有 %s。\n使用 'map' 查看 NPC 位置。", npc_name);
        }
    }
    else {
        snprintf(response, sizeof(response), "未知命令：%s\n输入 'help' 查看帮助。", inputText);
    }
    
    (*env)->ReleaseStringUTFChars(env, input, inputText);
    return (*env)->NewStringUTF(env, response);
}

JNIEXPORT void JNICALL Java_com_adventure_game_GameActivity_cleanupGame(
    JNIEnv *env, jobject thiz) {
    (void)env; (void)thiz;
    LOGI("游戏清理");
    g_initialized = 0;
    g_game.running = 0;
}

JNIEXPORT jboolean JNICALL Java_com_adventure_game_GameActivity_isGameRunning(
    JNIEnv *env, jobject thiz) {
    (void)env; (void)thiz;
    return g_initialized && g_game.running ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jstring JNICALL Java_com_adventure_game_GameActivity_getCurrentScene(
    JNIEnv *env, jobject thiz) {
    (void)env; (void)thiz;
    if (!g_initialized || !g_game.current_scene) return NULL;
    return (*env)->NewStringUTF(env, g_game.current_scene->name);
}

JNIEXPORT jint JNICALL Java_com_adventure_game_GameActivity_getGold(
    JNIEnv *env, jobject thiz) {
    (void)env; (void)thiz;
    if (!g_initialized) return 0;
    return g_game.inventory.gold;
}

JNIEXPORT jstring JNICALL Java_com_adventure_game_GameActivity_getAvailableCommands(
    JNIEnv *env, jobject thiz) {
    (void)env; (void)thiz;
    if (!g_initialized) return (*env)->NewStringUTF(env, "");
    
    char commands[1024] = "";
    strcat(commands, "look");
    strcat(commands, "|map");
    strcat(commands, "|inventory");
    strcat(commands, "|quest");
    strcat(commands, "|go");
    strcat(commands, "|talk");
    
    return (*env)->NewStringUTF(env, commands);
}

JNIEXPORT jstring JNICALL Java_com_adventure_game_GameActivity_getCommandTargets(
    JNIEnv *env, jobject thiz, jstring command) {
    (void)env; (void)thiz;

    if (g_game.current_scene == NULL) {
        const char *cmd = (*env)->GetStringUTFChars(env, command, NULL);
        (*env)->ReleaseStringUTFChars(env, command, cmd);
        return (*env)->NewStringUTF(env, "");
    }
    if (!g_initialized) return (*env)->NewStringUTF(env, "");
    
    const char *cmd = (*env)->GetStringUTFChars(env, command, NULL);
    char targets[1024] = "";
    
    if (strcmp(cmd, "go") == 0) {
        for (int i = 0; i < g_game.scene_count; i++) {
            if (i > 0) strcat(targets, "|");
            strcat(targets, g_game.scenes[i].name);
        }
    }
    else if (strcmp(cmd, "talk") == 0) {
        if (g_game.current_scene != NULL && strlen(g_game.current_scene->npcs) > 0) {
            char *npc_list = strdup(g_game.current_scene->npcs);
            char *npc = strtok(npc_list, "，");
            int first = 1;
            while (npc != NULL) {
                if (!first) strcat(targets, "|");
                strcat(targets, npc);
                first = 0;
                npc = strtok(NULL, "，");
            }
            free(npc_list);
        }
    }
    
    (*env)->ReleaseStringUTFChars(env, command, cmd);
    return (*env)->NewStringUTF(env, targets);
}
