// ============================================================================
// Android 游戏入口（HTTP API 版）
// 功能：处理游戏核心逻辑，包括场景、NPC、物品、任务系统
// 编译：通过 CMake 编译为 JNI 库
// ============================================================================

#include <jni.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <android/log.h>

// Android 日志标签
#define LOG_TAG "AdventureGame"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// ============================================================================
// 数据结构定义
// ============================================================================

// 场景结构：定义游戏中的一个地点
typedef struct Scene {
    char id[64];          // 场景唯一标识符（英文 ID，用于代码判断）
    char name[64];        // 场景显示名称（中文，用于 UI 显示）
    char description[512];// 场景描述文本
    char connections[256];// 可前往的场景列表，中文逗号分隔
    char npcs[256];       // 当前场景的 NPC 列表，中文逗号分隔
} Scene;

// 任务结构：定义游戏中的一个任务
typedef struct Quest {
    char id[64];          // 任务唯一标识符
    char name[64];        // 任务显示名称
    char description[512];// 任务描述
    int completed;        // 是否已完成 (0=未完成，1=已完成)
    int active;           // 是否激活 (0=未激活，1=激活中)
} Quest;

// 背包结构：玩家物品管理
typedef struct Inventory {
    int gold;             // 金币数量
    int item_count;       // 物品数量
} Inventory;

// 游戏全局状态
typedef struct GameContext {
    Scene scenes[10];     // 场景数组，最大 10 个场景
    int scene_count;      // 实际场景数量
    Scene *current_scene; // 当前所在场景指针
    Inventory inventory;  // 玩家背包
    Quest quests[30];     // 任务数组，最大 30 个任务
    int quest_count;      // 实际任务数量
    int running;          // 游戏运行状态 (0=停止，1=运行中)
} GameContext;

// ============================================================================
// 全局变量
// ============================================================================
static GameContext g_game;       // 游戏全局状态
static int g_initialized = 0;    // 初始化标志 (0=未初始化，1=已初始化)

// ============================================================================
// 辅助函数：解析 NPC 列表
// 参数：npcs - NPC 列表字符串（中文逗号分隔）
//      target - 要查找的目标 NPC 名称
// 返回：1=找到，0=未找到
// ============================================================================
static int find_npc_in_list(const char *npcs, const char *target) {
    if (npcs == NULL || target == NULL) return 0;
    
    int len = strlen(npcs);
    char current[128] = {0};  // 当前解析的 NPC 名称
    int ci = 0;               // current 数组索引
    
    for (int i = 0; i <= len; i++) {
        unsigned char c = (unsigned char)npcs[i];
        
        // 检测中文逗号（UTF-8 编码：0xE5 0xBC 0x8C）或字符串结束
        if ((c == 0xE5 && i + 2 < len && 
             (unsigned char)npcs[i+1] == 0xBC && 
             (unsigned char)npcs[i+2] == 0x8C) || c == '\0') {
            
            current[ci] = '\0';
            
            // 去除前后空格
            int start = 0, end = ci - 1;
            while (start < ci && current[start] == ' ') start++;
            while (end >= 0 && current[end] == ' ') end--;
            
            if (end >= start) {
                char trimmed[128] = {0};
                int ti = 0;
                for (int j = start; j <= end; j++) {
                    trimmed[ti++] = current[j];
                }
                trimmed[ti] = '\0';
                
                // 比较 NPC 名称
                if (strcmp(trimmed, target) == 0) {
                    return 1;
                }
            }
            
            ci = 0;  // 重置索引
            if (c == 0xE5) i += 2;  // 跳过中文逗号的后续 2 个字节
        } else {
            if (ci < 127) current[ci++] = c;
        }
    }
    
    return 0;
}

// ============================================================================
// 辅助函数：分割字符串获取目标列表（用于 getCommandTargets）
// 参数：str - 输入字符串（中文逗号分隔）
//      targets - 输出缓冲区（用 | 分隔）
// ============================================================================
static void parse_targets_from_string(const char *str, char *targets) {
    if (str == NULL || targets == NULL) return;
    
    int len = strlen(str);
    char current[128] = {0};
    int ci = 0;
    int first = 1;
    
    for (int i = 0; i <= len; i++) {
        unsigned char c = (unsigned char)str[i];
        
        // 检测中文逗号或字符串结束
        if ((c == 0xE5 && i + 2 < len && 
             (unsigned char)str[i+1] == 0xBC && 
             (unsigned char)str[i+2] == 0x8C) || c == '\0') {
            
            current[ci] = '\0';
            
            // 去除前后空格
            int start = 0, end = ci - 1;
            while (start < ci && current[start] == ' ') start++;
            while (end >= 0 && current[end] == ' ') end--;
            
            if (end >= start) {
                char trimmed[128] = {0};
                int ti = 0;
                for (int j = start; j <= end; j++) {
                    trimmed[ti++] = current[j];
                }
                trimmed[ti] = '\0';
                
                if (ti > 0) {
                    if (!first) strcat(targets, "|");
                    strcat(targets, trimmed);
                    first = 0;
                }
            }
            
            ci = 0;
            if (c == 0xE5) i += 2;
        } else {
            if (ci < 127) current[ci++] = c;
        }
    }
}

// ============================================================================
// JNI 函数：初始化游戏
// 参数：modelPath - 模型路径（HTTP API 版本不使用）
// 返回：JNI_TRUE=成功，JNI_FALSE=失败
// ============================================================================
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
    // talk 命令：与 NPC 对话
    // 格式：talk [NPC 名称]
    // 例：talk 村长、talk 铁匠老王
    else if (strncmp(inputText, "talk ", 5) == 0) {
        const char *npc_name = inputText + 5;
        
        // 使用辅助函数查找 NPC（统一解析逻辑）
        int found = find_npc_in_list(g_game.current_scene->npcs, npc_name);
        
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
    
    if (!g_initialized) return (*env)->NewStringUTF(env, "");
    if (g_game.current_scene == NULL) return (*env)->NewStringUTF(env, "");
    
    const char *cmd = (*env)->GetStringUTFChars(env, command, NULL);
    if (cmd == NULL) return (*env)->NewStringUTF(env, "");
    
    char targets[1024] = "";
    
    if (strcmp(cmd, "go") == 0) {
        for (int i = 0; i < g_game.scene_count; i++) {
            if (i > 0) strcat(targets, "|");
            strcat(targets, g_game.scenes[i].name);
        }
    }
    // talk 命令：获取当前场景 NPC 列表
    else if (strcmp(cmd, "talk") == 0) {
        // 从当前场景的 npcs 字段解析 NPC 名称
        parse_targets_from_string(g_game.current_scene->npcs, targets);
    }
    
    (*env)->ReleaseStringUTFChars(env, command, cmd);
    return (*env)->NewStringUTF(env, targets);
}
