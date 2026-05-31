// ============================================================================
// Android 游戏入口（HTTP API 版）- v2.8
// 功能：游戏核心逻辑，包括场景、NPC（外貌/记忆/状态）、主角系统、交互动作、存档系统
// 编译：通过 CMake 编译为 JNI 库
// ============================================================================

#include <jni.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <android/log.h>

#define LOG_TAG "AdventureGame"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define MAX_NPCS 20
#define MAX_MEMORIES 20
#define MAX_SAVE_SLOTS 5

// ============================================================================
// 数据结构定义
// ============================================================================

// NPC 外貌描述
typedef struct NPCAppearance {
    char hair[64];        // 发型/发色
    char eyes[64];        // 眼睛描述
    char body[64];        // 身材/体型
    char clothes[128];    // 服装描述
    char features[128];   // 特征（疤痕、饰品等）
} NPCAppearance;

// NPC 记忆条目
typedef struct MemoryEntry {
    char content[256];    // 记忆内容
    int timestamp;        // 时间戳（游戏内时间）
    int relation_change;  // 关系变化值（+/-）
    char type[32];        // 记忆类型："talk", "gift", "interact", "trade"
    char speaker[64];     // 说话者（对话时使用）
} MemoryEntry;

// NPC 状态
typedef struct NPCStatus {
    int health;           // 生命值 (0-100)
    int mood;             // 心情 (0-100)
    int energy;           // 精力 (0-100)
    char occupation[64];  // 职业
    int is_alive;         // 是否存活 (0=死亡，1=存活)
} NPCStatus;

// NPC 结构体
typedef struct NPC {
    char id[64];          // NPC 唯一标识符
    char name[64];        // NPC 名称
    char description[256];// NPC 描述
    NPCAppearance appearance;  // 外貌
    NPCStatus status;     // 状态
    MemoryEntry memories[20]; // 记忆数组，最多 20 条
    int memory_count;     // 记忆数量
    int relation;         // 与主角关系 (0-100)
    char location[64];    // 所在位置 ID
    int is_custom;        // 是否玩家自定义 (0=内置，1=自定义)
} NPC;

// 主角外貌
typedef struct PlayerAppearance {
    char hair[64];
    char eyes[64];
    char body[64];
    char clothes[128];
    char features[128];
} PlayerAppearance;

// 主角状态
typedef struct PlayerStatus {
    int health;           // 生命值 (0-100)
    int max_health;       // 最大生命
    int mana;             // 法力值 (0-100)
    int max_mana;         // 最大法力
    int strength;         // 力量
    int agility;          // 敏捷
    int intelligence;     // 智力
    int level;            // 等级
    int exp;              // 经验值
    char title[64];       // 称号
} PlayerStatus;

// 主角记忆
typedef struct PlayerMemory {
    MemoryEntry entries[30]; // 记忆数组，最多 30 条
    int count;               // 记忆数量
} PlayerMemory;

// 场景结构
typedef struct Scene {
    char id[64];
    char name[64];
    char description[512];
    char connections[256]; // 中文逗号分隔
    char npcs[256];        // NPC 列表，中文逗号分隔
} Scene;

// 任务结构
typedef struct Quest {
    char id[64];
    char name[64];
    char description[512];
    int completed;
    int active;
} Quest;

// 背包物品
typedef struct Item {
    char name[64];
    char type[32];        // "weapon", "armor", "consumable", "material"
    int value;            // 价值
    int effect;           // 效果值
} Item;

// 背包结构
typedef struct Inventory {
    int gold;
    Item items[50];       // 最多 50 个物品
    int item_count;
} Inventory;

// 游戏全局状态
typedef struct GameContext {
    Scene scenes[15];     // 最多 15 个场景
    int scene_count;
    Scene *current_scene;
    NPC npcs[20];         // 最多 20 个 NPC
    int npc_count;
    int game_time;        // 游戏内时间（小时）
    int day;              // 第几天
    char player_name[64]; // 主角名字
    PlayerStatus player_status;
    PlayerAppearance player_appearance;
    PlayerMemory player_memory;
    Inventory inventory;
    Quest quests[30];
    int quest_count;
    int running;
} GameContext;

// ============================================================================
// 全局变量
// ============================================================================
static GameContext g_game;
static int g_initialized = 0;
static char g_save_dir[512] = "/data/data/com.adventure.game/files/saves";

// ============================================================================
// 辅助函数：添加主角记忆
// ============================================================================
static void add_player_memory(const char *content, int relation_change) {
    // 使用循环缓冲区：如果记忆已满，覆盖最旧的记忆
    int index = g_game.player_memory.count % 30;
    
    MemoryEntry *entry = &g_game.player_memory.entries[index];
    strncpy(entry->content, content, 255);
    entry->content[255] = '\0';
    entry->timestamp = g_game.game_time + g_game.day * 24;
    entry->relation_change = relation_change;
    
    // 只在未满时增加计数
    if (g_game.player_memory.count < 30) {
        g_game.player_memory.count++;
    }
    
    LOGI("添加记忆：%s (count=%d/%d)", content, g_game.player_memory.count, 30);
}

// ============================================================================
// 辅助函数：添加 NPC 记忆
// ============================================================================
static void add_npc_memory(NPC *npc, const char *content, int relation_change) {
    if (npc == NULL) return;
    
    // 使用循环缓冲区：如果记忆已满，覆盖最旧的记忆
    int index = npc->memory_count % 20;
    MemoryEntry *entry = &npc->memories[index];
    
    strncpy(entry->content, content, 255);
    entry->content[255] = '\0';
    entry->timestamp = g_game.game_time + g_game.day * 24;
    entry->relation_change = relation_change;
    
    // 只在未满时增加计数
    if (npc->memory_count < 20) {
        npc->memory_count++;
    }
    
    LOGI("NPC %s 添加记忆：%s (count=%d/%d)", npc->name, content, npc->memory_count, 20);
}

// ============================================================================
// 辅助函数：查找 NPC
// ============================================================================
static NPC* find_npc_by_name(const char *name) {
    for (int i = 0; i < g_game.npc_count; i++) {
        if (strcmp(g_game.npcs[i].name, name) == 0) {
            return &g_game.npcs[i];
        }
    }
    return NULL;
}

// ============================================================================
// 辅助函数：解析 NPC 列表
// ============================================================================
static int find_npc_in_list(const char *npcs, const char *target) {
    if (npcs == NULL || target == NULL) return 0;
    
    int len = strlen(npcs);
    char current[128] = {0};
    int ci = 0;
    
    for (int i = 0; i <= len; i++) {
        unsigned char c = (unsigned char)npcs[i];
        
        if ((c == 0xE5 && i + 2 < len && 
             (unsigned char)npcs[i+1] == 0xBC && 
             (unsigned char)npcs[i+2] == 0x8C) || c == '\0') {
            
            current[ci] = '\0';
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
                
                if (strcmp(trimmed, target) == 0) {
                    return 1;
                }
            }
            
            ci = 0;
            if (c == 0xE5) i += 2;
        } else {
            if (ci < 127) current[ci++] = c;
        }
    }
    return 0;
}

// ============================================================================
// 辅助函数：分割字符串
// ============================================================================
static void parse_targets_from_string(const char *str, char *targets) {
    if (str == NULL || targets == NULL) return;
    
    int len = strlen(str);
    char current[128] = {0};
    int ci = 0;
    int first = 1;
    
    for (int i = 0; i <= len; i++) {
        unsigned char c = (unsigned char)str[i];
        
        if ((c == 0xE5 && i + 2 < len && 
             (unsigned char)str[i+1] == 0xBC && 
             (unsigned char)str[i+2] == 0x8C) || c == '\0') {
            
            current[ci] = '\0';
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
// ============================================================================
JNIEXPORT jboolean JNICALL Java_com_adventure_game_GameActivity_initGame(
    JNIEnv *env, jobject thiz, jstring modelPath) {
    (void)thiz; (void)modelPath;
    
    if (g_initialized) return JNI_TRUE;
    
    LOGI("=== 初始化游戏 v2.8 ===");
    memset(&g_game, 0, sizeof(GameContext));
    
    // 设置主角默认属性
    strcpy(g_game.player_name, "冒险者");
    g_game.player_status.max_health = 100;
    g_game.player_status.health = 100;
    g_game.player_status.max_mana = 50;
    g_game.player_status.mana = 50;
    g_game.player_status.strength = 10;
    g_game.player_status.agility = 10;
    g_game.player_status.intelligence = 10;
    g_game.player_status.level = 1;
    g_game.player_status.exp = 0;
    strcpy(g_game.player_status.title, "新手冒险者");
    
    // 主角外貌
    strcpy(g_game.player_appearance.hair, "黑色短发");
    strcpy(g_game.player_appearance.eyes, "黑色眼睛");
    strcpy(g_game.player_appearance.body, "身材匀称");
    strcpy(g_game.player_appearance.clothes, "朴素的冒险者服装");
    strcpy(g_game.player_appearance.features, "无特征");
    
    // 初始化背包
    g_game.inventory.gold = 50;
    g_game.inventory.item_count = 1;
    strcpy(g_game.inventory.items[0].name, "面包");
    strcpy(g_game.inventory.items[0].type, "consumable");
    g_game.inventory.items[0].value = 5;
    g_game.inventory.items[0].effect = 10;
    
    // NPC 1: 村长
    strcpy(g_game.npcs[0].id, "elder");
    strcpy(g_game.npcs[0].name, "村长");
    strcpy(g_game.npcs[0].description, "村庄的领导者，睿智而仁慈。");
    strcpy(g_game.npcs[0].appearance.hair, "花白长发");
    strcpy(g_game.npcs[0].appearance.eyes, "深邃的灰眼睛");
    strcpy(g_game.npcs[0].appearance.body, "微驼的瘦高身材");
    strcpy(g_game.npcs[0].appearance.clothes, "深蓝色长袍，绣有金色纹路");
    strcpy(g_game.npcs[0].appearance.features, "手持橡木手杖，胡须花白");
    g_game.npcs[0].status.health = 60;
    g_game.npcs[0].status.mood = 70;
    g_game.npcs[0].status.energy = 50;
    strcpy(g_game.npcs[0].status.occupation, "村长");
    g_game.npcs[0].status.is_alive = 1;
    g_game.npcs[0].relation = 50;
    strcpy(g_game.npcs[0].location, "village");
    g_game.npcs[0].is_custom = 0;
    g_game.npcs[0].memory_count = 0;
    
    // NPC 2: 村民
    strcpy(g_game.npcs[1].id, "villager");
    strcpy(g_game.npcs[1].name, "村民");
    strcpy(g_game.npcs[1].description, "普通的村民，正在忙碌地工作。");
    strcpy(g_game.npcs[1].appearance.hair, "棕色短发");
    strcpy(g_game.npcs[1].appearance.eyes, "棕色眼睛");
    strcpy(g_game.npcs[1].appearance.body, "健壮的身材");
    strcpy(g_game.npcs[1].appearance.clothes, "粗布衣和围裙");
    strcpy(g_game.npcs[1].appearance.features, "手上布满老茧");
    g_game.npcs[1].status.health = 80;
    g_game.npcs[1].status.mood = 60;
    g_game.npcs[1].status.energy = 70;
    strcpy(g_game.npcs[1].status.occupation, "农民");
    g_game.npcs[1].status.is_alive = 1;
    g_game.npcs[1].relation = 40;
    strcpy(g_game.npcs[1].location, "village");
    g_game.npcs[1].is_custom = 0;
    g_game.npcs[1].memory_count = 0;
    
    // NPC 3: 铁匠老王
    strcpy(g_game.npcs[2].id, "blacksmith");
    strcpy(g_game.npcs[2].name, "铁匠老王");
    strcpy(g_game.npcs[2].description, "技艺精湛的铁匠，性格豪爽。");
    strcpy(g_game.npcs[2].appearance.hair, "黑色寸头");
    strcpy(g_game.npcs[2].appearance.eyes, "炯炯有神的黑眼睛");
    strcpy(g_game.npcs[2].appearance.body, "魁梧健壮，肌肉发达");
    strcpy(g_game.npcs[2].appearance.clothes, "皮质围裙，露出强壮的手臂");
    strcpy(g_game.npcs[2].appearance.features, "右臂有烧伤疤痕，戴着铁护腕");
    g_game.npcs[2].status.health = 90;
    g_game.npcs[2].status.mood = 75;
    g_game.npcs[2].status.energy = 80;
    strcpy(g_game.npcs[2].status.occupation, "铁匠");
    g_game.npcs[2].status.is_alive = 1;
    g_game.npcs[2].relation = 45;
    strcpy(g_game.npcs[2].location, "blacksmith");
    g_game.npcs[2].is_custom = 0;
    g_game.npcs[2].memory_count = 0;
    
    g_game.npc_count = 3;
    
    // 场景 1: 新手村广场
    strcpy(g_game.scenes[0].id, "village");
    strcpy(g_game.scenes[0].name, "新手村广场");
    strcpy(g_game.scenes[0].description, "你站在一个宁静的小村庄广场中央。四周是古朴的木屋，村民们忙碌地走动。北方是铁匠铺，东方有通往森林的小路。");
    strcpy(g_game.scenes[0].connections, "铁匠铺，迷雾森林入口");
    strcpy(g_game.scenes[0].npcs, "村长，村民");
    
    // 场景 2: 铁匠铺
    strcpy(g_game.scenes[1].id, "blacksmith");
    strcpy(g_game.scenes[1].name, "铁匠铺");
    strcpy(g_game.scenes[1].description, "铁匠铺内炉火熊熊。墙上挂满了各式武器和护甲，空气中弥漫着金属和煤炭的味道。");
    strcpy(g_game.scenes[1].connections, "新手村广场");
    strcpy(g_game.scenes[1].npcs, "铁匠老王");
    
    // 场景 3: 迷雾森林入口
    strcpy(g_game.scenes[2].id, "forest");
    strcpy(g_game.scenes[2].name, "迷雾森林入口");
    strcpy(g_game.scenes[2].description, "茂密的树木遮天蔽日，薄雾在林间飘荡。一条小径通向森林深处，隐约能听到鸟鸣声。");
    strcpy(g_game.scenes[2].connections, "新手村广场");
    strcpy(g_game.scenes[2].npcs, "");
    
    g_game.scene_count = 3;
    g_game.current_scene = &g_game.scenes[0];
    g_game.game_time = 8; // 早上 8 点
    g_game.day = 1;
    
    // 任务 1: 清剿哥布林
    strcpy(g_game.quests[0].id, "kill_goblins");
    strcpy(g_game.quests[0].name, "清剿哥布林");
    strcpy(g_game.quests[0].description, "村庄附近的哥布林越来越多，村长请求你帮忙教训它们。已经消灭 0/5 只哥布林。");
    g_game.quests[0].active = 1;
    g_game.quests[0].completed = 0;
    g_game.quest_count = 1;
    
    g_game.running = 1;
    g_initialized = 1;
    

// 存档系统前向声明
bool savegame_save(const struct GameContext *game, int slot);
bool savegame_load(struct GameContext *game, int slot);
bool savegame_exists(int slot);
bool savegame_delete(int slot);

    LOGI("游戏初始化完成");
    return JNI_TRUE;
}

// ============================================================================
// JNI 函数：处理输入
// ============================================================================
JNIEXPORT jstring JNICALL Java_com_adventure_game_GameActivity_processInput(
    JNIEnv *env, jobject thiz, jstring input) {
    (void)thiz;
    
    const char *inputText = (*env)->GetStringUTFChars(env, input, NULL);
    LOGI("输入：%s", inputText);
    
    char response[8192] = {0};
    
    // help 命令
    if (strcmp(inputText, "help") == 0 || strcmp(inputText, "h") == 0) {
        strcpy(response, 
            "=== 命令帮助 ===\n\n"
            "【基础命令】\n"
            "  look - 查看当前场景\n"
            "  map - 查看完整地图\n"
            "  go [地点] - 移动\n"
            "  inventory - 查看背包\n"
            "  quest - 查看任务\n"
            "  talk [NPC] - 与 NPC 对话\n\n"
            "【查看命令】\n"
            "  status - 查看主角状态\n"
            "  appearance - 查看主角外貌\n"
            "  memory - 查看主角记忆\n"
            "  npc [NPC 名] - 查看 NPC 详情\n\n"
            "【交互命令】\n"
            "  gift [NPC] - 赠送礼物\n"
            "  trade [NPC] - 交易物品\n"
            "  interact [NPC] - 互动（聊天等）\n"
            "  ask [问题] - AI 对话\n\n"
            "【自定义 NPC】\n"
            "  create_npc - 查看创建向导\n"
            "  create_npc [名] [职业] - 快速创建\n"
            "  setnpc [名] [属性] [值] - 修改 NPC\n"
            "  remove_npc [名] - 删除 NPC\n\n"
            "【存档系统】\n"
            "  saves - 查看存档列表\n"
            "  save [槽位 1-5] - 保存游戏\n"
            "  load [槽位 1-5] - 加载游戏\n"
            "  delete [槽位] - 删除存档\n\n"
            "  exit - 退出游戏");
    }
    
    // map 命令
    else if (strcmp(inputText, "map") == 0 || strcmp(inputText, "m") == 0) {
        char map_text[4096] = "=== 世界地图 ===\n\n";
        for (int i = 0; i < g_game.scene_count; i++) {
            Scene *s = &g_game.scenes[i];
            char line[512];
            snprintf(line, sizeof(line), "■ %s", s->name);
            if (strcmp(g_game.current_scene->id, s->id) == 0) {
                strcat(line, " [你在这里]");
            }
            strcat(map_text, line);
            strcat(map_text, "\n  可前往：");
            strcat(map_text, s->connections[0] ? s->connections : "无");
            if (strlen(s->npcs) > 0) {
                strcat(map_text, "\n  NPC: ");
                strcat(map_text, s->npcs);
            }
            strcat(map_text, "\n\n");
        }
        strcat(map_text, "时间：第 ");
        char day_str[32];
        snprintf(day_str, sizeof(day_str), "%d 天 %d:00", g_game.day, g_game.game_time);
        strcat(map_text, day_str);
        strcpy(response, map_text);
    }
    
    // look 命令
    else if (strcmp(inputText, "look") == 0 || strcmp(inputText, "l") == 0) {
        snprintf(response, sizeof(response), "%s\n\n", g_game.current_scene->description);
        if (strlen(g_game.current_scene->npcs) > 0) {
            strcat(response, "【在场 NPC】 ");
            strcat(response, g_game.current_scene->npcs);
            strcat(response, "\n");
        }
        strcat(response, "\n可前往：");
        strcat(response, g_game.current_scene->connections);
    }
    
    // inventory 命令
    else if (strcmp(inputText, "inventory") == 0 || strcmp(inputText, "i") == 0) {
        snprintf(response, sizeof(response), 
            "=== 背包 ===\n"
            "金币：%d\n"
            "物品数量：%d/50\n\n【物品列表】\n",
            g_game.inventory.gold, g_game.inventory.item_count);
        
        for (int j = 0; j < g_game.inventory.item_count; j++) {
            char item_line[128];
            snprintf(item_line, sizeof(item_line), "  - %s (%s) 价值：%d\n",
                     g_game.inventory.items[j].name,
                     g_game.inventory.items[j].type,
                     g_game.inventory.items[j].value);
            strcat(response, item_line);
        }
    }
    
    // quest 命令
    else if (strcmp(inputText, "quest") == 0) {
        if (g_game.quest_count > 0) {
            Quest *q = &g_game.quests[0];
            snprintf(response, sizeof(response), 
                "=== 当前任务 ===\n"
                "名称：%s\n"
                "描述：%s\n"
                "状态：%s\n"
                "完成度：%s",
                q->name, q->description,
                q->active ? "进行中" : "已完成",
                q->completed ? "已完成" : "未完成");
        } else {
            strcpy(response, "当前没有任务");
        }
    }
    
    // status 命令 - 查看主角状态
    else if (strcmp(inputText, "status") == 0) {
        snprintf(response, sizeof(response),
            "=== 主角状态 ===\n"
            "名称：%s\n"
            "称号：%s\n"
            "等级：%d (经验：%d)\n\n"
            "【生命】%d/%d\n"
            "【法力】%d/%d\n\n"
            "【属性】\n"
            "  力量：%d\n"
            "  敏捷：%d\n"
            "  智力：%d\n\n"
            "时间：第 %d 天 %d:00",
            g_game.player_name,
            g_game.player_status.title,
            g_game.player_status.level,
            g_game.player_status.exp,
            g_game.player_status.health,
            g_game.player_status.max_health,
            g_game.player_status.mana,
            g_game.player_status.max_mana,
            g_game.player_status.strength,
            g_game.player_status.agility,
            g_game.player_status.intelligence,
            g_game.day,
            g_game.game_time);
    }
    
    // appearance 命令 - 查看主角外貌
    else if (strcmp(inputText, "appearance") == 0 || strcmp(inputText, "appear") == 0) {
        snprintf(response, sizeof(response),
            "=== 主角外貌 ===\n"
            "【发型】%s\n"
            "【眼睛】%s\n"
            "【身材】%s\n"
            "【服装】%s\n"
            "【特征】%s",
            g_game.player_appearance.hair,
            g_game.player_appearance.eyes,
            g_game.player_appearance.body,
            g_game.player_appearance.clothes,
            g_game.player_appearance.features);
    }
    
    // memory 命令 - 查看主角记忆
    else if (strcmp(inputText, "memory") == 0 || strcmp(inputText, "memories") == 0) {
        if (g_game.player_memory.count == 0) {
            strcpy(response, "=== 主角记忆 ===\n你还没有什么特别的记忆。\n\n开始冒险来创造回忆吧！");
        } else {
            strcpy(response, "=== 主角记忆 ===\n");
            for (int i = g_game.player_memory.count - 1; i >= 0; i--) {
                MemoryEntry *entry = &g_game.player_memory.entries[i];
                int hour = entry->timestamp % 24;
                int day = entry->timestamp / 24;
                char line[300];
                snprintf(line, sizeof(line), "\n【第%d天%d:00】%s",
                         day, hour, entry->content);
                strcat(response, line);
            }
        }
    }
    
    // npc [NPC 名] 命令 - 查看 NPC 详情
    else if (strncmp(inputText, "npc ", 4) == 0) {
        const char *npc_name = inputText + 4;
        NPC *npc = find_npc_by_name(npc_name);
        
        if (npc == NULL) {
            snprintf(response, sizeof(response), 
                "找不到 NPC：%s\n使用 'map' 查看所有 NPC 位置。", npc_name);
        } else {
            // 检查 NPC 是否在当前场景
            if (strcmp(npc->location, g_game.current_scene->id) != 0) {
                snprintf(response, sizeof(response),
                    "【%s】不在当前场景\n"
                    "你可以在 %s 找到他/她。",
                    npc->name, npc->location);
            } else {
                char relation_str[32];
                if (npc->relation >= 80) strcpy(relation_str, "亲密");
                else if (npc->relation >= 60) strcpy(relation_str, "友好");
                else if (npc->relation >= 40) strcpy(relation_str, "普通");
                else if (npc->relation >= 20) strcpy(relation_str, "冷淡");
                else strcpy(relation_str, "敌对");
                
                snprintf(response, sizeof(response),
                    "=== %s ===\n\n"
                    "【外貌】\n"
                    "  发型：%s\n"
                    "  眼睛：%s\n"
                    "  身材：%s\n"
                    "  服装：%s\n"
                    "  特征：%s\n\n"
                    "【状态】\n"
                    "  生命：%d/100\n"
                    "  心情：%d/100\n"
                    "  精力：%d/100\n"
                    "  职业：%s\n"
                    "  关系：%s (%d/100)\n",
                    npc->name,
                    npc->appearance.hair,
                    npc->appearance.eyes,
                    npc->appearance.body,
                    npc->appearance.clothes,
                    npc->appearance.features,
                    npc->status.health,
                    npc->status.mood,
                    npc->status.energy,
                    npc->status.occupation,
                    relation_str,
                    npc->relation);
            }
        }
    }
    
    // go 命令
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
                // 时间流逝
                g_game.game_time += 1;
                if (g_game.game_time >= 24) {
                    g_game.game_time = 0;
                    g_game.day++;
                }
                
                snprintf(response, sizeof(response), 
                    "【时间流逝】1 小时过去了...\n\n"
                    "你来到了 %s。\n\n%s\n\n"
                    "可前往：%s",
                    g_game.current_scene->name,
                    g_game.current_scene->description,
                    g_game.current_scene->connections);
                
                // 检查是否有 NPC
                NPC *nearby_npc = NULL;
                for (int i = 0; i < g_game.npc_count; i++) {
                    if (strcmp(g_game.npcs[i].location, g_game.current_scene->id) == 0) {
                        nearby_npc = &g_game.npcs[i];
                        break;
                    }
                }
            } else {
                strcpy(response, "无法直接前往，请先查看地图确认路线。");
            }
        }
    }
    
    // talk 命令 - 查看 NPC 对话提示
    else if (strncmp(inputText, "talk ", 5) == 0) {
        const char *npc_name = inputText + 5;
        NPC *npc = find_npc_by_name(npc_name);
        
        // 检查 NPC 是否在当前场景
        int found = (npc != NULL && strcmp(npc->location, g_game.current_scene->id) == 0);
        
        if (found && npc != NULL) {
            // 根据关系值调整对话
            char mood_str[64];
            if (npc->status.mood >= 80) strcpy(mood_str, "心情愉快的");
            else if (npc->status.mood >= 50) strcpy(mood_str, "平静的");
            else strcpy(mood_str, "有些疲惫的");
            
            if (strcmp(npc_name, "铁匠老王") == 0) {
                if (npc->relation >= 70) {
                    strcpy(response, "铁匠老王：哈哈，老朋友！看到你我就来劲了！需要武器或护甲吗？");
                } else if (npc->relation >= 40) {
                    strcpy(response, "铁匠老王：欢迎来到这里，冒险者！需要武器或护甲吗？");
                } else {
                    strcpy(response, "铁匠老王：（头也不抬）忙着呢，没空闲聊。");
                }
            } else if (strcmp(npc_name, "村长") == 0) {
                if (npc->relation >= 70) {
                    strcpy(response, "村长：啊，我们的英雄回来了！村庄因为有你在而感到安心。");
                } else if (npc->relation >= 40) {
                    strcpy(response, "村长：欢迎你，年轻的冒险者！村庄最近的哥布林越来越多，你能帮帮我们吗？");
                } else {
                    strcpy(response, "村长：（严肃地打量着你）年轻人，希望你不是来惹麻烦的。");
                }
            } else if (strcmp(npc_name, "村民") == 0) {
                if (npc->relation >= 60) {
                    strcpy(response, "村民：嘿！又见面了！今天天气真好，适合出门冒险！");
                } else {
                    strcpy(response, "村民：今天天气真好，适合出门冒险！");
                }
            } else {
                snprintf(response, sizeof(response), "%s：%s%s", 
                         npc_name, mood_str, " 你好，有何贵干？");
            }
            
            // 添加互动记忆
            char memory_content[256];
            snprintf(memory_content, sizeof(memory_content), "%s：与其进行了交谈", npc_name);
            add_player_memory(memory_content, 0);
            
            // 提示使用 AI 对话
            strcat(response, "\n\n💡 提示：使用 'chat [NPC] [内容]' 进行 AI 对话");
            strcat(response, "\n例：chat 村长 最近的哥布林是怎么回事？");
            
        } else {
            snprintf(response, sizeof(response), "这里没有 %s。\n使用 'map' 查看 NPC 位置。", npc_name);
        }
    }
    
    // chat [NPC] [内容] - AI 对话命令
    else if (strncmp(inputText, "chat ", 5) == 0) {
        // 此命令在 Java 层处理（需要调用 AI API）
        // 返回提示信息
        strcpy(response, "正在联系 AI 服务器...\n(等待回复)");
    }
    
    // gift [NPC] 礼物 命令
    else if (strncmp(inputText, "gift ", 5) == 0) {
        const char *args = inputText + 5;
        char npc_name[64] = {0};
        char item_name[64] = {0};
        
        // 解析参数 (NPC 名 和 物品名，用空格分隔)
        int i = 0;
        while (args[i] && args[i] != ' ' && i < 63) {
            npc_name[i] = args[i];
            i++;
        }
        if (args[i] == ' ') {
            i++;
            int j = 0;
            while (args[i] && j < 63) {
                item_name[j++] = args[i++];
            }
        }
        
        if (strlen(npc_name) == 0) {
            strcpy(response, "用法：gift [NPC 名] [物品名]\n例：gift 村长 面包");
        } else {
            NPC *npc = find_npc_by_name(npc_name);
            // 检查 NPC 是否在当前场景
            int found = (npc != NULL && strcmp(npc->location, g_game.current_scene->id) == 0);
            
            if (!found) {
                snprintf(response, sizeof(response), "这里没有 %s。", npc_name);
            } else {
                NPC *npc = find_npc_by_name(npc_name);
                int relation_change = 5;
                
                // 检查是否有物品
                char memory_content[256];
                if (strlen(item_name) > 0) {
                    // 简化：只要有物品就送
                    snprintf(memory_content, sizeof(memory_content), "%s：赠送了 %s", npc_name, item_name);
                } else {
                    snprintf(memory_content, sizeof(memory_content), "%s：赠送了礼物", npc_name);
                }
                
                add_player_memory(memory_content, relation_change);
                
                if (npc != NULL) {
                    add_npc_memory(npc, memory_content, relation_change);
                    npc->relation += relation_change;
                    if (npc->relation > 100) npc->relation = 100;
                    npc->status.mood += 10;
                    if (npc->status.mood > 100) npc->status.mood = 100;
                }
                
                snprintf(response, sizeof(response),
                    "你向 %s 赠送了礼物。\n"
                    "关系提升！当前关系：%d/100\n"
                    "%s 看起来很开心。",
                    npc_name,
                    npc ? npc->relation : 0,
                    npc_name);
            }
        }
    }
    
    // trade [NPC] 命令
    else if (strncmp(inputText, "trade ", 6) == 0) {
        const char *npc_name = inputText + 6;
        NPC *npc = find_npc_by_name(npc_name);
        // 检查 NPC 是否在当前场景
        int found = (npc != NULL && strcmp(npc->location, g_game.current_scene->id) == 0);
        
        if (!found) {
            snprintf(response, sizeof(response), "这里没有 %s。", npc_name);
        } else {
            NPC *npc = find_npc_by_name(npc_name);
            snprintf(response, sizeof(response),
                "=== 交易界面 ===\n"
                "与 %s 进行交易\n\n"
                "【你的金币】%d\n"
                "【你的关系】%d/100\n\n"
                "【可购买物品】\n"
                "  1. 治疗药水 - 20 金币 (恢复 50 生命)\n"
                "  2. 铁剑 - 50 金币 (攻击力 +5)\n"
                "  3. 皮甲 - 30 金币 (防御力 +3)\n\n"
                "使用 'buy [数量]' 购买\n"
                "使用 'sell [物品]' 出售",
                npc_name,
                g_game.inventory.gold,
                npc ? npc->relation : 0);
        }
    }
    
    // interact [NPC] 命令
    else if (strncmp(inputText, "interact ", 9) == 0) {
        const char *npc_name = inputText + 9;
        NPC *npc = find_npc_by_name(npc_name);
        // 检查 NPC 是否在当前场景
        int found = (npc != NULL && strcmp(npc->location, g_game.current_scene->id) == 0);
        
        if (!found) {
            snprintf(response, sizeof(response), "这里没有 %s。", npc_name);
        } else {
            NPC *npc = find_npc_by_name(npc_name);
            int relation_change = 2;
            
            char memory_content[256];
            snprintf(memory_content, sizeof(memory_content), "%s：进行了友好互动", npc_name);
            add_player_memory(memory_content, relation_change);
            
            if (npc != NULL) {
                add_npc_memory(npc, memory_content, relation_change);
                npc->relation += relation_change;
                if (npc->relation > 100) npc->relation = 100;
                npc->status.mood += 5;
                if (npc->status.mood > 100) npc->status.mood = 100;
            }
            
            snprintf(response, sizeof(response),
                "你与 %s 进行了友好的互动。\n"
                "关系小幅提升！当前关系：%d/100\n"
                "%s 看起来心情不错。",
                npc_name,
                npc ? npc->relation : 0,
                npc_name);
        }
    }
    
    // create_npc 命令 - 创建自定义 NPC（带向导）
    else if (strcmp(inputText, "create_npc") == 0) {
        if (g_game.npc_count >= 20) {
            strcpy(response, "最多只能创建 20 个 NPC。");
        } else {
            strcpy(response,
                "╔══════════════════════════════════════════╗\n"
                "║     自定义 NPC 创建向导                   ║\n"
                "╚══════════════════════════════════════════╝\n\n"
                "【方法一】快速创建（推荐新手）\n"
                "输入：create_npc [名称] [职业]\n"
                "例：create_npc 魔法师 法师\n"
                "   → 自动创建默认外貌的 NPC\n\n"
                "【方法二】详细创建\n"
                "输入：create_npc [名称] [职业] [发型] [眼睛] [身材] [服装] [特征]\n"
                "例：create_npc 艾莉 法师 银色长发 紫色眼睛 纤细 紫色长袍 水晶法杖\n"
                "   → 创建完全自定义的 NPC\n\n"
                "【方法三】AI 创建\n"
                "输入：ask 创建一个 [描述]\n"
                "例：ask 创建一个神秘的精灵弓箭手，金色长发，绿色眼睛\n"
                "   → 使用 AI 生成 NPC 设定\n\n"
                "【编辑已创建的 NPC】\n"
                "setnpc [NPC 名] [属性] [新值]\n"
                "属性包括：name, hair, eyes, body, clothes, features, occupation\n"
                "例：setnpc 艾莉 hair 金色长发\n"
                "   → 修改艾莉的发型为金色长发\n\n"
                "【删除 NPC】\n"
                "remove_npc [NPC 名]\n"
                "例：remove_npc 艾莉\n\n"
                "提示：当前已创建 ");
            
            // 统计自定义 NPC 数量
            int custom_count = 0;
            for (int i = 0; i < g_game.npc_count; i++) {
                if (g_game.npcs[i].is_custom) custom_count++;
            }
            
            char count_str[64];
            snprintf(count_str, sizeof(count_str), "%d 个自定义 NPC，还可创建 %d 个。", 
                     custom_count, 20 - custom_count);
            strcat(response, count_str);
        }
    }
    
    // create_npc [名称] [职业] ... 快速创建
    else if (strncmp(inputText, "create_npc ", 11) == 0) {
        if (g_game.npc_count >= 20) {
            strcpy(response, "最多只能创建 20 个 NPC。");
        } else {
            const char *args = inputText + 11;
            char npc_name[64] = {0};
            char occupation[64] = {0};
            char hair[64] = "普通发型";
            char eyes[64] = "普通眼睛";
            char body[64] = "普通身材";
            char clothes[128] = "普通服装";
            char features[128] = "无明显特征";
            
            // 解析参数（空格分隔）
            int part = 0;
            int i = 0, j = 0;
            while (args[i] && part < 7) {
                if (args[i] == ' ') {
                    part++;
                    j = 0;
                    i++;
                } else {
                    switch (part) {
                        case 0: if (j < 63) npc_name[j++] = args[i]; break;
                        case 1: if (j < 63) occupation[j++] = args[i]; break;
                        case 2: if (j < 63) hair[j++] = args[i]; break;
                        case 3: if (j < 63) eyes[j++] = args[i]; break;
                        case 4: if (j < 63) body[j++] = args[i]; break;
                        case 5: if (j < 127) clothes[j++] = args[i]; break;
                        case 6: if (j < 127) features[j++] = args[i]; break;
                    }
                    i++;
                }
            }
            
            if (strlen(npc_name) == 0) {
                strcpy(response, "错误：请提供 NPC 名称\n用法：create_npc [名称] [职业] [发型] [眼睛] [身材] [服装] [特征]");
            } else {
                // 检查是否已存在同名 NPC
                NPC *existing = find_npc_by_name(npc_name);
                if (existing != NULL) {
                    snprintf(response, sizeof(response), "错误：已存在名为 '%s' 的 NPC。", npc_name);
                } else {
                    char new_npc_id[32];
                    snprintf(new_npc_id, sizeof(new_npc_id), "custom_%d", g_game.npc_count);
                    
                    NPC *new_npc = &g_game.npcs[g_game.npc_count];
                    strcpy(new_npc->id, new_npc_id);
                    strcpy(new_npc->name, npc_name);
                    
                    char desc[256];
                    if (strlen(occupation) > 0 && strcmp(occupation, "自由职业者") != 0) {
                        snprintf(desc, sizeof(desc), "%s，一位%s。", npc_name, occupation);
                        strcpy(new_npc->status.occupation, occupation);
                    } else {
                        snprintf(desc, sizeof(desc), "%s，一位自由职业者。", npc_name);
                        strcpy(new_npc->status.occupation, "自由职业者");
                    }
                    strcpy(new_npc->description, desc);
                    
                    strcpy(new_npc->appearance.hair, hair);
                    strcpy(new_npc->appearance.eyes, eyes);
                    strcpy(new_npc->appearance.body, body);
                    strcpy(new_npc->appearance.clothes, clothes);
                    strcpy(new_npc->appearance.features, features);
                    
                    new_npc->status.health = 100;
                    new_npc->status.mood = 70;
                    new_npc->status.energy = 80;
                    new_npc->status.is_alive = 1;
                    new_npc->relation = 30;
                    strcpy(new_npc->location, g_game.current_scene->id);
                    new_npc->is_custom = 1;
                    new_npc->memory_count = 0;
                    
                    // 添加到当前场景 NPC 列表
                    if (strlen(g_game.current_scene->npcs) == 0) {
                        strcpy(g_game.current_scene->npcs, npc_name);
                    } else {
                        char old_npcs[256];
                        strcpy(old_npcs, g_game.current_scene->npcs);
                        snprintf(g_game.current_scene->npcs, sizeof(g_game.current_scene->npcs), 
                                 "%s，%s", old_npcs, npc_name);
                    }
                    
                    g_game.npc_count++;
                    
                    snprintf(response, sizeof(response),
                        "【自定义 NPC 创建成功】\n\n"
                        "╔══════════════════════════════════════════╗\n"
                        "║ %-36s ║\n"
                        "╚══════════════════════════════════════════╝\n\n"
                        "【基本信息】\n"
                        "  名称：%s\n"
                        "  职业：%s\n"
                        "  位置：%s\n\n"
                        "【外貌特征】\n"
                        "  发型：%s\n"
                        "  眼睛：%s\n"
                        "  身材：%s\n"
                        "  服装：%s\n"
                        "  特征：%s\n\n"
                        "【初始状态】\n"
                        "  关系：30/100 (陌生)\n"
                        "  心情：70/100\n"
                        "  生命：100/100\n\n"
                        "💡 提示：\n"
                        "  • 使用 'setnpc %s [属性] [新值]' 修改属性\n"
                        "  • 使用 'talk %s' 与 NPC 对话\n"
                        "  • 使用 'gift %s [物品]' 提升关系\n"
                        "  • 使用 'remove_npc %s' 删除此 NPC",
                        npc_name,
                        npc_name,
                        strlen(occupation) > 0 ? occupation : "自由职业者",
                        g_game.current_scene->name,
                        hair,
                        eyes,
                        body,
                        clothes,
                        features,
                        npc_name,
                        npc_name,
                        npc_name,
                        npc_name);
                }
            }
        }
    }
    
    // setnpc [NPC 名] [属性] [新值] - 修改 NPC 属性
    else if (strncmp(inputText, "setnpc ", 7) == 0) {
        const char *args = inputText + 7;
        char npc_name[64] = {0};
        char attr[32] = {0};
        char new_value[256] = {0};
        
        // 解析参数
        int part = 0, i = 0, j = 0;
        while (args[i] && part < 3) {
            if (args[i] == ' ') {
                part++;
                j = 0;
                i++;
            } else {
                switch (part) {
                    case 0: if (j < 63) npc_name[j++] = args[i]; break;
                    case 1: if (j < 31) attr[j++] = args[i]; break;
                    case 2: if (j < 255) new_value[j++] = args[i]; break;
                }
                i++;
            }
        }
        
        if (strlen(npc_name) == 0 || strlen(attr) == 0 || strlen(new_value) == 0) {
            strcpy(response,
                "用法：setnpc [NPC 名] [属性] [新值]\n\n"
                "属性列表：\n"
                "  name - 名称\n"
                "  hair - 发型\n"
                "  eyes - 眼睛\n"
                "  body - 身材\n"
                "  clothes - 服装\n"
                "  features - 特征\n"
                "  occupation - 职业\n"
                "  desc - 描述\n\n"
                "例：setnpc 艾莉 hair 金色长发\n"
                "   setnpc 艾莉 occupation 法师");
        } else {
            NPC *npc = find_npc_by_name(npc_name);
            if (npc == NULL) {
                snprintf(response, sizeof(response), "找不到 NPC：%s", npc_name);
            } else if (!npc->is_custom) {
                strcpy(response, "只能修改自定义 NPC 的属性。\n使用 'create_npc' 创建自定义 NPC。");
            } else {
                int modified = 0;
                if (strcmp(attr, "name") == 0) {
                    // 检查新名字是否已存在
                    NPC *existing = find_npc_by_name(new_value);
                    if (existing != NULL && existing != npc) {
                        snprintf(response, sizeof(response), "错误：已存在名为 '%s' 的 NPC。", new_value);
                    } else {
                        strncpy(npc->name, new_value, 63);
                        modified = 1;
                    }
                } else if (strcmp(attr, "hair") == 0) {
                    strncpy(npc->appearance.hair, new_value, 63);
                    modified = 1;
                } else if (strcmp(attr, "eyes") == 0) {
                    strncpy(npc->appearance.eyes, new_value, 63);
                    modified = 1;
                } else if (strcmp(attr, "body") == 0) {
                    strncpy(npc->appearance.body, new_value, 63);
                    modified = 1;
                } else if (strcmp(attr, "clothes") == 0) {
                    strncpy(npc->appearance.clothes, new_value, 127);
                    modified = 1;
                } else if (strcmp(attr, "features") == 0) {
                    strncpy(npc->appearance.features, new_value, 127);
                    modified = 1;
                } else if (strcmp(attr, "occupation") == 0) {
                    strncpy(npc->status.occupation, new_value, 63);
                    modified = 1;
                } else if (strcmp(attr, "desc") == 0) {
                    strncpy(npc->description, new_value, 255);
                    modified = 1;
                }
                
                if (modified) {
                    snprintf(response, sizeof(response),
                        "【NPC 属性已修改】\n\n"
                        "  NPC：%s\n"
                        "  属性：%s → %s\n\n"
                        "使用 'npc %s' 查看完整信息。",
                        npc_name, attr, new_value, npc->name);
                }
            }
        }
    }
    
    // remove_npc [NPC 名] 命令
    else if (strncmp(inputText, "remove_npc ", 11) == 0) {
        const char *npc_name = inputText + 11;
        int found_idx = -1;
        
        for (int i = 0; i < g_game.npc_count; i++) {
            if (strcmp(g_game.npcs[i].name, npc_name) == 0) {
                if (g_game.npcs[i].is_custom) {
                    found_idx = i;
                    break;
                } else {
                    strcpy(response, "只能删除自定义 NPC。");
                    found_idx = -2;
                    break;
                }
            }
        }
        
        if (found_idx >= 0) {
            // 删除 NPC
            for (int i = found_idx; i < g_game.npc_count - 1; i++) {
                g_game.npcs[i] = g_game.npcs[i + 1];
            }
            g_game.npc_count--;
            
            // 从场景 NPC 列表中移除
            // 简化处理：不清除场景列表中的引用
            
            snprintf(response, sizeof(response), 
                "已删除自定义 NPC：%s", npc_name);
        } else if (found_idx == -1) {
            snprintf(response, sizeof(response), 
                "找不到 NPC：%s", npc_name);
        }
    }
    
    // exit 命令
    else if (strcmp(inputText, "exit") == 0 || strcmp(inputText, "quit") == 0) {
        g_game.running = 0;
        strcpy(response, "游戏结束，再见！");
    }
    
    // AI 对话 - ask 命令
    else if (strncmp(inputText, "ask ", 4) == 0) {
        // AI 对话由 Java 层处理，这里返回提示
        snprintf(response, sizeof(response),
            "【AI 对话模式】\n"
            "你的问题：%s\n\n"
            "正在连接 AI 服务器...\n"
            "(实际 AI 响应将由 API 服务器返回)",
            inputText + 4);
    }
    
    // 存档系统命令
    else if (strcmp(inputText, "saves") == 0) {
        strcat(response, "=== 存档列表 ===\n");
        for (int i = 1; i <= 5; i++) {
            if (savegame_exists(i)) {
                char fp[256];
                snprintf(fp, sizeof(fp), "%s/save_slot_%d.json", g_save_dir, i);
                struct stat st;
                if (stat(fp, &st) == 0) {
                    char tb[64];
                    struct tm *tm = localtime(&st.st_mtime);
                    strftime(tb, sizeof(tb), "%m-%d %H:%M", tm);
                    snprintf(response + strlen(response), sizeof(response) - strlen(response),
                        "槽位 %d: ✓ 已保存 (%s, %ld 字节)\n", i, tb, st.st_size);
                } else {
                    snprintf(response + strlen(response), sizeof(response) - strlen(response),
                        "槽位 %d: ✓ 已保存\n", i);
                }
            } else {
                snprintf(response + strlen(response), sizeof(response) - strlen(response),
                    "槽位 %d: (空)\n", i);
            }
        }
        strcat(response, "\n用法：save [槽位 1-5] - 保存 | load [槽位 1-5] - 读取 | delete [槽位] - 删除");
    }
    else if (strncmp(inputText, "save ", 5) == 0) {
        int slot = atoi(inputText + 5);
        if (slot < 1 || slot > 5) {
            strcpy(response, "无效的槽位！请使用 1-5。\n用法：save [槽位]");
        } else if (savegame_save(&g_game, slot)) {
            snprintf(response, sizeof(response), "✓ 游戏已保存到槽位 %d\n路径：%s/save_slot_%d.json",
                slot, g_save_dir, slot);
        } else {
            strcpy(response, "✗ 保存失败！");
        }
    }
    else if (strncmp(inputText, "load ", 5) == 0) {
        int slot = atoi(inputText + 5);
        if (slot < 1 || slot > 5) {
            strcpy(response, "无效的槽位！请使用 1-5。\n用法：load [槽位]");
        } else if (!savegame_exists(slot)) {
            snprintf(response, sizeof(response), "槽位 %d 没有存档！", slot);
        } else if (savegame_load(&g_game, slot)) {
            snprintf(response, sizeof(response),
                "✓ 已从槽位 %d 加载\n第 %d 天 %d:00 | 生命：%d/%d | %s",
                slot, g_game.day, g_game.game_time,
                g_game.player_status.health, g_game.player_status.max_health,
                g_game.current_scene ? g_game.current_scene->name : "未知");
        } else {
            snprintf(response, sizeof(response), "✗ 加载槽位 %d 失败！", slot);
        }
    }
    else if (strncmp(inputText, "delete ", 7) == 0) {
        int slot = atoi(inputText + 7);
        if (slot < 1 || slot > 5) {
            strcpy(response, "无效的槽位！请使用 1-5。\n用法：delete [槽位]");
        } else if (!savegame_exists(slot)) {
            snprintf(response, sizeof(response), "槽位 %d 没有存档！", slot);
        } else if (savegame_delete(slot)) {
            snprintf(response, sizeof(response), "✓ 已删除槽位 %d 的存档", slot);
        } else {
            snprintf(response, sizeof(response), "✗ 删除槽位 %d 失败！", slot);
        }
    }
    
    // 未知命令
    else {
        snprintf(response, sizeof(response), 
            "未知命令：%s\n"
            "输入 'help' 查看帮助。", 
            inputText);
    }
    
    (*env)->ReleaseStringUTFChars(env, input, inputText);
    return (*env)->NewStringUTF(env, response);
}

// ============================================================================
// JNI 函数：清理游戏
// ============================================================================
JNIEXPORT void JNICALL Java_com_adventure_game_GameActivity_cleanupGame(
    JNIEnv *env, jobject thiz) {
    (void)env; (void)thiz;
    LOGI("游戏清理");
    g_initialized = 0;
    g_game.running = 0;
    memset(&g_game.player_memory, 0, sizeof(PlayerMemory));
    for (int i = 0; i < g_game.npc_count; i++) {
        memset(&g_game.npcs[i].memories, 0, sizeof(MemoryEntry) * g_game.npcs[i].memory_count);
    }
}

// ============================================================================
// JNI 函数：检查游戏是否运行
// ============================================================================
JNIEXPORT jboolean JNICALL Java_com_adventure_game_GameActivity_isGameRunning(
    JNIEnv *env, jobject thiz) {
    (void)env; (void)thiz;
    return g_initialized && g_game.running ? JNI_TRUE : JNI_FALSE;
}

// ============================================================================
// JNI 函数：获取当前场景
// ============================================================================
JNIEXPORT jstring JNICALL Java_com_adventure_game_GameActivity_getCurrentScene(
    JNIEnv *env, jobject thiz) {
    (void)env; (void)thiz;
    if (!g_initialized || !g_game.current_scene) return NULL;
    return (*env)->NewStringUTF(env, g_game.current_scene->name);
}

// ============================================================================
// JNI 函数：获取金币
// ============================================================================
JNIEXPORT jint JNICALL Java_com_adventure_game_GameActivity_getGold(
    JNIEnv *env, jobject thiz) {
    (void)env; (void)thiz;
    if (!g_initialized) return 0;
    return g_game.inventory.gold;
}

// ============================================================================
// JNI 函数：获取可用命令
// ============================================================================
JNIEXPORT jstring JNICALL Java_com_adventure_game_GameActivity_getAvailableCommands(
    JNIEnv *env, jobject thiz) {
    (void)env; (void)thiz;
    if (!g_initialized) return (*env)->NewStringUTF(env, "");
    
    char commands[2048] = "";
    strcat(commands, "look");
    strcat(commands, "|map");
    strcat(commands, "|inventory");
    strcat(commands, "|quest");
    strcat(commands, "|status");
    strcat(commands, "|appearance");
    strcat(commands, "|memory");
    strcat(commands, "|go");
    strcat(commands, "|talk");
    strcat(commands, "|gift");
    strcat(commands, "|trade");
    strcat(commands, "|interact");
    strcat(commands, "|create_npc");
    strcat(commands, "|remove_npc");
    
    return (*env)->NewStringUTF(env, commands);
}

// ============================================================================
// JNI 函数：获取命令目标
// ============================================================================
JNIEXPORT jstring JNICALL Java_com_adventure_game_GameActivity_getCommandTargets(
    JNIEnv *env, jobject thiz, jstring command) {
    (void)env; (void)thiz;
    
    if (!g_initialized) return (*env)->NewStringUTF(env, "");
    if (g_game.current_scene == NULL) return (*env)->NewStringUTF(env, "");
    
    const char *cmd = (*env)->GetStringUTFChars(env, command, NULL);
    if (cmd == NULL) return (*env)->NewStringUTF(env, "");
    
    char targets[2048] = "";
    
    if (strcmp(cmd, "go") == 0) {
        for (int i = 0; i < g_game.scene_count; i++) {
            if (i > 0) strcat(targets, "|");
            strcat(targets, g_game.scenes[i].name);
        }
    }
    else if (strcmp(cmd, "talk") == 0 || strcmp(cmd, "gift") == 0 || 
             strcmp(cmd, "trade") == 0 || strcmp(cmd, "interact") == 0) {
        // 遍历 NPC 数组，获取当前场景的 NPC
        int first = 1;
        for (int i = 0; i < g_game.npc_count; i++) {
            if (strcmp(g_game.npcs[i].location, g_game.current_scene->id) == 0) {
                if (!first) strcat(targets, "|");
                strcat(targets, g_game.npcs[i].name);
                first = 0;
            }
        }
    }
    else if (strcmp(cmd, "npc") == 0) {
        // 遍历 NPC 数组，获取当前场景的 NPC
        int first = 1;
        for (int i = 0; i < g_game.npc_count; i++) {
            if (strcmp(g_game.npcs[i].location, g_game.current_scene->id) == 0) {
                if (!first) strcat(targets, "|");
                strcat(targets, g_game.npcs[i].name);
                first = 0;
            }
        }
    }
    else if (strcmp(cmd, "remove_npc") == 0 || strcmp(cmd, "setnpc") == 0) {
        for (int i = 0; i < g_game.npc_count; i++) {
            if (g_game.npcs[i].is_custom) {
                if (strlen(targets) > 0) strcat(targets, "|");
                strcat(targets, g_game.npcs[i].name);
            }
        }
    }
    
    (*env)->ReleaseStringUTFChars(env, command, cmd);
    return (*env)->NewStringUTF(env, targets);
}

// ============================================================================
// JNI 函数：获取 NPC 上下文（用于 AI 对话）
// ============================================================================
JNIEXPORT jstring JNICALL Java_com_adventure_game_GameActivity_getNpcContext(
    JNIEnv *env, jobject thiz, jstring npcName) {
    (void)thiz;
    
    if (!g_initialized) return (*env)->NewStringUTF(env, "");
    
    const char *name = (*env)->GetStringUTFChars(env, npcName, NULL);
    if (name == NULL) return (*env)->NewStringUTF(env, "");
    
    NPC *npc = find_npc_by_name(name);
    if (npc == NULL) {
        (*env)->ReleaseStringUTFChars(env, npcName, name);
        return (*env)->NewStringUTF(env, "");
    }
    
    char context[2048] = "";
    
    // 基本信息
    snprintf(context, sizeof(context),
        "姓名：%s\n"
        "职业：%s\n"
        "描述：%s\n"
        "外貌：%s，%s，%s\n"
        "服装：%s\n"
        "特征：%s\n"
        "与玩家关系：%d/100 ",
        npc->name,
        npc->status.occupation,
        npc->description,
        npc->appearance.hair,
        npc->appearance.eyes,
        npc->appearance.body,
        npc->appearance.clothes,
        npc->appearance.features,
        npc->relation);
    
    // 关系描述
    if (npc->relation >= 80) strcat(context, "(亲密)\n");
    else if (npc->relation >= 60) strcat(context, "(友好)\n");
    else if (npc->relation >= 40) strcat(context, "(普通)\n");
    else if (npc->relation >= 20) strcat(context, "(冷淡)\n");
    else strcat(context, "(敌对)\n");
    
    // 记忆历史（最近 5 条）
    if (npc->memory_count > 0) {
        strcat(context, "\n【记忆】\n");
        int start = npc->memory_count > 5 ? npc->memory_count - 5 : 0;
        for (int i = start; i < npc->memory_count; i++) {
            MemoryEntry *mem = &npc->memories[i];
            if (strlen(mem->type) > 0 && strcmp(mem->type, "talk") == 0) {
                char mem_line[300];
                snprintf(mem_line, sizeof(mem_line),
                    "- 对话：%s 说\"%s\"\n",
                    mem->speaker, mem->content);
                strcat(context, mem_line);
            }
        }
    }
    
    (*env)->ReleaseStringUTFChars(env, npcName, name);
    return (*env)->NewStringUTF(env, context);
}

// ============================================================================
// JNI 函数：保存 NPC 对话记忆
// ============================================================================
JNIEXPORT void JNICALL Java_com_adventure_game_GameActivity_saveNpcTalk(
    JNIEnv *env, jobject thiz, jstring npcName, jstring playerSay, jstring npcReply) {
    (void)thiz;
    
    if (!g_initialized) return;
    
    const char *name = (*env)->GetStringUTFChars(env, npcName, NULL);
    const char *say = (*env)->GetStringUTFChars(env, playerSay, NULL);
    const char *reply = (*env)->GetStringUTFChars(env, npcReply, NULL);
    
    if (name == NULL || say == NULL || reply == NULL) {
        if (name) (*env)->ReleaseStringUTFChars(env, npcName, name);
        if (say) (*env)->ReleaseStringUTFChars(env, playerSay, say);
        if (reply) (*env)->ReleaseStringUTFChars(env, npcReply, reply);
        return;
    }
    
    NPC *npc = find_npc_by_name(name);
    if (npc != NULL) {
        // 使用循环缓冲区：如果记忆已满，覆盖最旧的记忆
        int index = npc->memory_count % 20;
        MemoryEntry *mem = &npc->memories[index];
        
        snprintf(mem->content, sizeof(mem->content), "%s", reply);
        strncpy(mem->speaker, "玩家", sizeof(mem->speaker) - 1);
        mem->speaker[sizeof(mem->speaker) - 1] = '\0';
        strncpy(mem->type, "talk", sizeof(mem->type) - 1);
        mem->type[sizeof(mem->type) - 1] = '\0';
        mem->timestamp = g_game.game_time + g_game.day * 24;
        mem->relation_change = 0;
        
        // 只在未满时增加计数
        if (npc->memory_count < 20) {
            npc->memory_count++;
        }
        
        // 保存主角记忆（同样使用循环缓冲区）
        char player_mem[256];
        snprintf(player_mem, sizeof(player_mem), "%s：%s", name, reply);
        add_player_memory(player_mem, 0);
        
        LOGI("保存 NPC 对话记忆：%s (count=%d/%d)", name, npc->memory_count, 20);
    } else {
        LOGW("找不到 NPC: %s", name);
    }
    
    (*env)->ReleaseStringUTFChars(env, npcName, name);
    (*env)->ReleaseStringUTFChars(env, playerSay, say);
    (*env)->ReleaseStringUTFChars(env, npcReply, reply);
}
// ============================================================================
// 存档系统实现 - JSON 格式
// 功能：保存/加载游戏进度到内部存储
// 位置：/data/data/com.adventure.game/files/saves/
// ============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <errno.h>
#include <android/log.h>

#define LOG_TAG "AdventureGame_Save"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define MAX_NPCS 20
#define MAX_MEMORIES 20
#define MAX_SAVE_SLOTS 5


// 存档目录
static inline const char* get_save_file(int slot) { static char buf[768]; snprintf(buf, sizeof(buf), "%s/save_slot_%d.json", g_save_dir, slot); return buf; }

// 转义 JSON 字符串
static void escape_json(const char *src, char *dst, size_t dst_size) {
    size_t j = 0;
    for (size_t i = 0; src[i] && j < dst_size - 2; i++) {
        switch (src[i]) {
            case '"':  if (j < dst_size - 3) { dst[j++] = '\\'; dst[j++] = '"'; } break;
            case '\\': if (j < dst_size - 3) { dst[j++] = '\\'; dst[j++] = '\\'; } break;
            case '\n': if (j < dst_size - 3) { dst[j++] = '\\'; dst[j++] = 'n'; } break;
            case '\r': if (j < dst_size - 3) { dst[j++] = '\\'; dst[j++] = 'r'; } break;
            case '\t': if (j < dst_size - 3) { dst[j++] = '\\'; dst[j++] = 't'; } break;
            default:   dst[j++] = src[i]; break;
        }
    }
    dst[j] = '\0';
}

// 创建存档目录
static bool ensure_save_dir(void) {
    struct stat st = {0};
    if (stat(g_save_dir, &st) == -1) {
        if (mkdir(g_save_dir, 0755) == -1) {
            LOGE("无法创建存档目录：%s", strerror(errno));
            return false;
        }
    }
    return true;
}

// 保存游戏
bool savegame_save(const struct GameContext *game, int slot) {
    if (slot < 1 || slot > MAX_SAVE_SLOTS) {
        LOGE("无效的存档槽位：%d", slot);
        return false;
    }
    if (!ensure_save_dir()) return false;
    
    char filepath[256];
    strncpy(filepath, get_save_file(slot), sizeof(filepath));
    
    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        LOGE("无法创建存档文件：%s", strerror(errno));
        return false;
    }
    
    LOGI("保存游戏到槽位 %d...", slot);
    
    fprintf(fp, "{\n  \"version\": \"2.4\",\n  \"timestamp\": %ld,\n  \"slot\": %d,\n", (long)time(NULL), slot);
    fprintf(fp, "  \"game\": {\n    \"day\": %d,\n    \"game_time\": %d,\n", game->day, game->game_time);
    
    char escaped[1024];
    escape_json(game->player_name, escaped, sizeof(escaped));
    fprintf(fp, "    \"player_name\": \"%s\",\n", escaped);
    escape_json(game->current_scene ? game->current_scene->id : "", escaped, sizeof(escaped));
    fprintf(fp, "    \"current_scene\": \"%s\",\n", escaped);
    
    // 玩家状态
    fprintf(fp, "    \"status\": {\"health\":%d,\"max_health\":%d,\"mana\":%d,\"max_mana\":%d,",
            game->player_status.health, game->player_status.max_health,
            game->player_status.mana, game->player_status.max_mana);
    fprintf(fp, "\"strength\":%d,\"agility\":%d,\"intelligence\":%d,",
            game->player_status.strength, game->player_status.agility, game->player_status.intelligence);
    fprintf(fp, "\"level\":%d,\"exp\":%d},\n", game->player_status.level, game->player_status.exp);
    
    // 玩家外貌
    escape_json(game->player_appearance.hair, escaped, sizeof(escaped));
    fprintf(fp, "    \"hair\": \"%s\",\n", escaped);
    escape_json(game->player_appearance.eyes, escaped, sizeof(escaped));
    fprintf(fp, "    \"eyes\": \"%s\",\n", escaped);
    escape_json(game->player_appearance.body, escaped, sizeof(escaped));
    fprintf(fp, "    \"body\": \"%s\",\n", escaped);
    escape_json(game->player_appearance.clothes, escaped, sizeof(escaped));
    fprintf(fp, "    \"clothes\": \"%s\",\n", escaped);
    escape_json(game->player_appearance.features, escaped, sizeof(escaped));
    fprintf(fp, "    \"features\": \"%s\"\n  },\n", escaped);
    
    // 背包
    fprintf(fp, "  \"inventory\": {\"gold\":%d,\"item_count\":%d", game->inventory.gold, game->inventory.item_count);
    fprintf(fp, ",\"items\":[");
    for (int i = 0; i < game->inventory.item_count; i++) {
        escape_json(game->inventory.items[i].name, escaped, sizeof(escaped));
        fprintf(fp, "%s{\"name\":\"%s\",\"type\":\"%s\",\"value\":%d,\"effect\":%d}",
                i > 0 ? "," : "", escaped, game->inventory.items[i].type,
                game->inventory.items[i].value, game->inventory.items[i].effect);
    }
    fprintf(fp, "]},\n");
    
    // 任务
    fprintf(fp, "  \"quests\": {\"count\":%d,\"quests\":[", game->quest_count);
    for (int i = 0; i < game->quest_count; i++) {
        escape_json(game->quests[i].id, escaped, sizeof(escaped));
        fprintf(fp, "%s{\"id\":\"%s\",\"name\":\"%s\",\"completed\":%d,\"active\":%d}",
                i > 0 ? "," : "", escaped, game->quests[i].name,
                game->quests[i].completed, game->quests[i].active);
    }
    fprintf(fp, "]},\n");
    
    // NPC
    fprintf(fp, "  \"npcs\": {\"count\":%d,\"npcs\":[", game->npc_count);
    for (int i = 0; i < game->npc_count; i++) {
        const NPC *n = &game->npcs[i];
        char esc[1024];
        
        fprintf(fp, "%s{\"id\":\"%s\",\"name\":\"%s\",\"relation\":%d,\"is_custom\":%d,",
                i > 0 ? "," : "", n->id, n->name, n->relation, n->is_custom);
        
        // 自定义 NPC 的完整数据
        if (n->is_custom) {
            escape_json(n->appearance.hair, esc, sizeof(esc));
            fprintf(fp, "\"hair\":\"%s\",", esc);
            escape_json(n->appearance.eyes, esc, sizeof(esc));
            fprintf(fp, "\"eyes\":\"%s\",", esc);
            escape_json(n->appearance.body, esc, sizeof(esc));
            fprintf(fp, "\"body\":\"%s\",", esc);
            escape_json(n->appearance.clothes, esc, sizeof(esc));
            fprintf(fp, "\"clothes\":\"%s\",", esc);
            escape_json(n->appearance.features, esc, sizeof(esc));
            fprintf(fp, "\"features\":\"%s\",", esc);
            
            fprintf(fp, "\"status\":{\"health\":%d,\"mood\":%d,\"energy\":%d,",
                    n->status.health, n->status.mood, n->status.energy);
            escape_json(n->status.occupation, esc, sizeof(esc));
            fprintf(fp, "\"occupation\":\"%s\",\"is_alive\":%d},", esc, n->status.is_alive);
            
            escape_json(n->description, esc, sizeof(esc));
            fprintf(fp, "\"description\":\"%s\",", esc);
            
            escape_json(n->location, esc, sizeof(esc));
            fprintf(fp, "\"location\":\"%s\",", esc);
        }
        
        // 记忆（所有 NPC）
        fprintf(fp, "\"memory_count\":%d,\"memories\":[", n->memory_count);
        for (int j = 0; j < n->memory_count; j++) {
            const MemoryEntry *m = &n->memories[j];
            char esc2[512];
            escape_json(m->content, esc2, sizeof(esc2));
            fprintf(fp, "%s{\"content\":\"%s\",\"timestamp\":%d,\"type\":\"%s\",\"speaker\":\"%s\"}",
                    j > 0 ? "," : "", esc2, m->timestamp, m->type, m->speaker);
        }
        fprintf(fp, "]}");
    }
    fprintf(fp, "]}\n}\n}\n");
    
    fclose(fp);
    LOGI("✓ 存档成功：%s", filepath);
    return true;
}

// 从 JSON 加载 NPC 数据
static void load_npcs_from_json(struct GameContext *game, const char *json) {
    const char *npcs_pos = strstr(json, "\"npcs\":");
    if (!npcs_pos) return;
    
    // 找到 npcs 数组开始
    const char *arr = strchr(npcs_pos, '[');
    if (!arr) return;
    arr++;
    
    // 重置 NPC 计数（保留初始 NPC）
    int initial_npc_count = game->npc_count;
    
    // 解析每个 NPC
    while (*arr && *arr != ']') {
        while (*arr && (*arr == ',' || *arr == ' ' || *arr == '\n')) arr++;
        if (*arr == ']') break;
        if (*arr != '{') { arr++; continue; }
        
        // 查找 id
        const char *id_start = strstr(arr, "\"id\":\"");
        if (!id_start) break;
        id_start += 6;
        const char *id_end = strchr(id_start, '"');
        if (!id_end) break;
        
        char npc_id[64];
        size_t id_len = id_end - id_start;
        if (id_len >= sizeof(npc_id)) id_len = sizeof(npc_id) - 1;
        strncpy(npc_id, id_start, id_len);
        npc_id[id_len] = '\0';
        
        // 查找现有 NPC
        NPC *npc = NULL;
        for (int i = 0; i < game->npc_count; i++) {
            if (strcmp(game->npcs[i].id, npc_id) == 0) {
                npc = &game->npcs[i];
                break;
            }
        }
        
        // 如果是新 NPC（自定义），添加
        if (!npc && game->npc_count < MAX_NPCS) {
            npc = &game->npcs[game->npc_count++];
            memset(npc, 0, sizeof(NPC));
            strncpy(npc->id, npc_id, sizeof(npc->id) - 1);
            npc->is_custom = 1;
        }
        
        if (!npc) {
            arr = strchr(arr, '}');
            if (arr) arr++;
            continue;
        }
        
        // 解析 name
        const char *name_pos = strstr(arr, "\"name\":\"");
        if (name_pos) {
            name_pos += 8;
            const char *name_end = strchr(name_pos, '"');
            if (name_end) {
                size_t len = name_end - name_pos;
                if (len >= sizeof(npc->name)) len = sizeof(npc->name) - 1;
                strncpy(npc->name, name_pos, len);
                npc->name[len] = '\0';
            }
        }
        
        // 解析 relation
        const char *rel_pos = strstr(arr, "\"relation\":");
        if (rel_pos) {
            rel_pos += 11;
            npc->relation = atoi(rel_pos);
        }
        
        // 解析 is_custom
        const char *custom_pos = strstr(arr, "\"is_custom\":");
        if (custom_pos) {
            custom_pos += 12;
            npc->is_custom = atoi(custom_pos);
        }
        
        // 如果是自定义 NPC，解析额外字段
        if (npc->is_custom) {
            // hair
            const char *hair_pos = strstr(arr, "\"hair\":\"");
            if (hair_pos) {
                hair_pos += 8;
                const char *end = strchr(hair_pos, '"');
                if (end) {
                    size_t len = end - hair_pos;
                    if (len >= sizeof(npc->appearance.hair)) len = sizeof(npc->appearance.hair) - 1;
                    strncpy(npc->appearance.hair, hair_pos, len);
                    npc->appearance.hair[len] = '\0';
                }
            }
            
            // eyes, body, clothes, features 类似处理...
            const char *eyes_pos = strstr(arr, "\"eyes\":\"");
            if (eyes_pos) {
                eyes_pos += 8;
                const char *end = strchr(eyes_pos, '"');
                if (end) {
                    size_t len = end - eyes_pos;
                    if (len >= sizeof(npc->appearance.eyes)) len = sizeof(npc->appearance.eyes) - 1;
                    strncpy(npc->appearance.eyes, eyes_pos, len);
                    npc->appearance.eyes[len] = '\0';
                }
            }
            
            const char *body_pos = strstr(arr, "\"body\":\"");
            if (body_pos) {
                body_pos += 8;
                const char *end = strchr(body_pos, '"');
                if (end) {
                    size_t len = end - body_pos;
                    if (len >= sizeof(npc->appearance.body)) len = sizeof(npc->appearance.body) - 1;
                    strncpy(npc->appearance.body, body_pos, len);
                    npc->appearance.body[len] = '\0';
                }
            }
            
            const char *clothes_pos = strstr(arr, "\"clothes\":\"");
            if (clothes_pos) {
                clothes_pos += 11;
                const char *end = strchr(clothes_pos, '"');
                if (end) {
                    size_t len = end - clothes_pos;
                    if (len >= sizeof(npc->appearance.clothes)) len = sizeof(npc->appearance.clothes) - 1;
                    strncpy(npc->appearance.clothes, clothes_pos, len);
                    npc->appearance.clothes[len] = '\0';
                }
            }
            
            const char *features_pos = strstr(arr, "\"features\":\"");
            if (features_pos) {
                features_pos += 12;
                const char *end = strchr(features_pos, '"');
                if (end) {
                    size_t len = end - features_pos;
                    if (len >= sizeof(npc->appearance.features)) len = sizeof(npc->appearance.features) - 1;
                    strncpy(npc->appearance.features, features_pos, len);
                    npc->appearance.features[len] = '\0';
                }
            }
            
            // description
            const char *desc_pos = strstr(arr, "\"description\":\"");
            if (desc_pos) {
                desc_pos += 15;
                const char *end = strchr(desc_pos, '"');
                if (end) {
                    size_t len = end - desc_pos;
                    if (len >= sizeof(npc->description)) len = sizeof(npc->description) - 1;
                    strncpy(npc->description, desc_pos, len);
                    npc->description[len] = '\0';
                }
            }
            
            // location
            const char *loc_pos = strstr(arr, "\"location\":\"");
            if (loc_pos) {
                loc_pos += 12;
                const char *end = strchr(loc_pos, '"');
                if (end) {
                    size_t len = end - loc_pos;
                    if (len >= sizeof(npc->location)) len = sizeof(npc->location) - 1;
                    strncpy(npc->location, loc_pos, len);
                    npc->location[len] = '\0';
                }
            }
            
            // status.health, mood, energy
            const char *status_pos = strstr(arr, "\"status\":{");
            if (status_pos) {
                const char *h = strstr(status_pos, "\"health\":");
                if (h) { h += 9; npc->status.health = atoi(h); }
                const char *m = strstr(status_pos, "\"mood\":");
                if (m) { m += 7; npc->status.mood = atoi(m); }
                const char *e = strstr(status_pos, "\"energy\":");
                if (e) { e += 8; npc->status.energy = atoi(e); }
            }
        }
        
        // 加载记忆
        const char *mem_pos = strstr(arr, "\"memories\":[");
        if (mem_pos) {
            mem_pos += 11;
            npc->memory_count = 0;
            while (*mem_pos && *mem_pos != ']' && npc->memory_count < MAX_MEMORIES) {
                while (*mem_pos && (*mem_pos == ',' || *mem_pos == ' ' || *mem_pos == '\n')) mem_pos++;
                if (*mem_pos == ']') break;
                if (*mem_pos != '{') { mem_pos++; continue; }
                
                MemoryEntry *mem = &npc->memories[npc->memory_count];
                
                const char *c = strstr(mem_pos, "\"content\":\"");
                if (c) {
                    c += 11;
                    const char *end = strchr(c, '"');
                    if (end) {
                        size_t len = end - c;
                        if (len >= sizeof(mem->content)) len = sizeof(mem->content) - 1;
                        strncpy(mem->content, c, len);
                        mem->content[len] = '\0';
                    }
                }
                
                const char *t = strstr(mem_pos, "\"timestamp\":");
                if (t) { t += 12; mem->timestamp = atoi(t); }
                
                const char *type = strstr(mem_pos, "\"type\":\"");
                if (type) {
                    type += 8;
                    const char *end = strchr(type, '"');
                    if (end) {
                        size_t len = end - type;
                        if (len >= sizeof(mem->type)) len = sizeof(mem->type) - 1;
                        strncpy(mem->type, type, len);
                        mem->type[len] = '\0';
                    }
                }
                
                const char *sp = strstr(mem_pos, "\"speaker\":\"");
                if (sp) {
                    sp += 11;
                    const char *end = strchr(sp, '"');
                    if (end) {
                        size_t len = end - sp;
                        if (len >= sizeof(mem->speaker)) len = sizeof(mem->speaker) - 1;
                        strncpy(mem->speaker, sp, len);
                        mem->speaker[len] = '\0';
                    }
                }
                
                npc->memory_count++;
                mem_pos = strchr(mem_pos, '}');
                if (mem_pos) mem_pos++;
            }
        }
        
        arr = strchr(arr, '}');
        if (arr) arr++;
    }
}
// 从 JSON 提取整数
static int json_get_int(const char *json, const char *key) {
    char search[128];
    snprintf(search, sizeof(search), "\"%s\":", key);
    const char *pos = strstr(json, search);
    if (!pos) return 0;
    pos += strlen(search);
    while (*pos == ' ') pos++;
    return atoi(pos);
}

// 从 JSON 提取字符串
static void json_get_str(const char *json, const char *key, char *out, size_t out_size) {
    char search[128];
    snprintf(search, sizeof(search), "\"%s\":", key);
    const char *pos = strstr(json, search);
    if (!pos) { out[0] = '\0'; return; }
    pos += strlen(search);
    while (*pos == ' ') pos++;
    if (*pos != '"') { out[0] = '\0'; return; }
    pos++;
    size_t i = 0;
    while (*pos && *pos != '"' && i < out_size - 1) {
        if (*pos == '\\' && *(pos+1)) {
            pos++;
            switch (*pos) {
                case 'n': out[i++] = '\n'; break;
                case 'r': out[i++] = '\r'; break;
                case 't': out[i++] = '\t'; break;
                default: out[i++] = *pos; break;
            }
        } else {
            out[i++] = *pos;
        }
        pos++;
    }
    out[i] = '\0';
}

// 读取文件到字符串
static char* read_file(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) return NULL;
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *buf = (char*)malloc(size + 1);
    if (!buf) { fclose(fp); return NULL; }
    size_t n = fread(buf, 1, size, fp);
    buf[n] = '\0';
    fclose(fp);
    return buf;
}

// 加载游戏
bool savegame_load(struct GameContext *game, int slot) {
    if (slot < 1 || slot > MAX_SAVE_SLOTS) {
        LOGE("无效的存档槽位：%d", slot);
        return false;
    }
    
    char filepath[256];
    strncpy(filepath, get_save_file(slot), sizeof(filepath));
    
    char *json = read_file(filepath);
    if (!json) {
        LOGE("无法读取存档：%s", filepath);
        return false;
    }
    
    LOGI("从槽位 %d 加载游戏...", slot);
    
    game->day = json_get_int(json, "day");
    game->game_time = json_get_int(json, "game_time");
    json_get_str(json, "player_name", game->player_name, sizeof(game->player_name));
    
    char scene_id[64];
    json_get_str(json, "current_scene", scene_id, sizeof(scene_id));
    
    game->player_status.health = json_get_int(json, "health");
    game->player_status.max_health = json_get_int(json, "max_health");
    game->player_status.mana = json_get_int(json, "mana");
    game->player_status.max_mana = json_get_int(json, "max_mana");
    game->player_status.strength = json_get_int(json, "strength");
    game->player_status.agility = json_get_int(json, "agility");
    game->player_status.intelligence = json_get_int(json, "intelligence");
    game->player_status.level = json_get_int(json, "level");
    game->player_status.exp = json_get_int(json, "exp");
    
    json_get_str(json, "hair", game->player_appearance.hair, sizeof(game->player_appearance.hair));
    json_get_str(json, "eyes", game->player_appearance.eyes, sizeof(game->player_appearance.eyes));
    json_get_str(json, "body", game->player_appearance.body, sizeof(game->player_appearance.body));
    json_get_str(json, "clothes", game->player_appearance.clothes, sizeof(game->player_appearance.clothes));
    json_get_str(json, "features", game->player_appearance.features, sizeof(game->player_appearance.features));
    
    game->inventory.gold = json_get_int(json, "gold");
    
    // 加载 NPC 数据（包括自定义 NPC）
    load_npcs_from_json(game, json);
    
    // 恢复场景指针
    game->current_scene = NULL;
    for (int i = 0; i < game->scene_count; i++) {
        if (strcmp(game->scenes[i].id, scene_id) == 0) {
            game->current_scene = &game->scenes[i];
            break;
        }
    }
    
    // 恢复场景的 NPC 列表（根据自定义 NPC 的 location）
    for (int i = 0; i < game->scene_count; i++) {
        game->scenes[i].npcs[0] = '\0';  // 清空
    }
    for (int i = 0; i < game->npc_count; i++) {
        NPC *n = &game->npcs[i];
        if (n->location[0] != '\0' && game->current_scene) {
            // 查找 NPC 所在的场景
            for (int j = 0; j < game->scene_count; j++) {
                if (strcmp(game->scenes[j].name, n->location) == 0 ||
                    strcmp(game->scenes[j].id, n->location) == 0) {
                    // 添加到场景 NPC 列表
                    if (strlen(game->scenes[j].npcs) == 0) {
                        strcpy(game->scenes[j].npcs, n->name);
                    } else {
                        char old_npcs[256];
                        strcpy(old_npcs, game->scenes[j].npcs);
                        snprintf(game->scenes[j].npcs, sizeof(game->scenes[j].npcs),
                                 "%s,%s", old_npcs, n->name);
                    }
                    break;
                }
            }
        }
    }
    
    free(json);
    LOGI("✓ 读档成功：%s", filepath);
    return true;
}

// 检查存档是否存在
bool savegame_exists(int slot) {
    if (slot < 1 || slot > MAX_SAVE_SLOTS) return false;
    char filepath[256];
    strncpy(filepath, get_save_file(slot), sizeof(filepath));
    struct stat st;
    return stat(filepath, &st) == 0;
}

// 删除存档
bool savegame_delete(int slot) {
    if (slot < 1 || slot > MAX_SAVE_SLOTS) return false;
    char filepath[256];
    strncpy(filepath, get_save_file(slot), sizeof(filepath));
    if (remove(filepath) == 0) {
        LOGI("✓ 已删除存档：%s", filepath);
        return true;
    }
    LOGE("删除失败：%s", strerror(errno));
    return false;
}

// JNI 函数：设置存档目录
JNIEXPORT void JNICALL Java_com_adventure_game_GameActivity_setSaveDir(JNIEnv *env, jclass clazz, jstring dir) {
    const char *saveDir = (*env)->GetStringUTFChars(env, dir, NULL);
    if (saveDir) {
        strncpy(g_save_dir, saveDir, sizeof(g_save_dir) - 1);
        g_save_dir[sizeof(g_save_dir) - 1] = '\0';
        LOGI("设置存档目录：%s", g_save_dir);
        (*env)->ReleaseStringUTFChars(env, dir, saveDir);
    }
    (void)clazz;
}
