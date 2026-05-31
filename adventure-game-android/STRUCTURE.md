# 项目结构说明

## 一、整体架构

```
adventure-game-android/
├── .github/                          # GitHub Actions CI/CD 配置
│   └── workflows/
│       └── build.yml                 # APK 自动构建工作流
├── .gitignore                        # Git 忽略规则
├── README.md                         # 项目说明和使用文档
├── DOWNLOAD.md                       # APK 下载与安装指南
├── STRUCTURE.md                      # 本文件：项目结构说明
├── build-apk.sh                      # 一键构建 APK 脚本
├── sync-source.sh                    # 源码同步脚本
├── build.gradle                      # 根项目 Gradle 配置
├── settings.gradle                   # Gradle 设置
├── gradle.properties                 # Gradle 属性配置
├── gradlew                           # Gradle Wrapper 脚本
├── gradle/                           # Gradle Wrapper 文件
│   └── wrapper/
│       └── gradle-wrapper.properties
├── releases/                         # 发布版本目录
│   └── README.md
└── app/                              # Android 应用主目录
    ├── build.gradle                  # 应用级 Gradle 配置
    └── src/main/
        ├── AndroidManifest.xml       # Android 应用清单
        ├── java/com/adventure/game/  # Java 源代码
        │   ├── MainActivity.java     # 主入口 Activity
        │   ├── GameActivity.java     # 游戏主界面 Activity
        │   └── ApiClient.java        # HTTP API 客户端
        ├── jni/                      # JNI 原生代码
        │   ├── CMakeLists.txt        # CMake 构建配置
        │   ├── Android.mk            # NDK 构建配置
        │   ├── Application.mk        # NDK 应用配置
        │   ├── game_android.c        # 游戏核心逻辑 (1248 行)
        │   └── include/              # 头文件目录
        │       ├── types.h           # 数据类型定义
        │       ├── game.h            # 游戏接口
        │       ├── npc.h             # NPC 系统接口
        │       ├── scene.h           # 场景系统接口
        │       ├── quest.h           # 任务系统接口
        │       ├── inventory.h       # 背包系统接口
        │       ├── ai.h              # AI 接口
        │       ├── savegame.h        # 存档接口
        │       └── adventure.h       # 游戏主头文件
        └── res/                      # Android 资源
            ├── layout/
            │   ├── activity_main.xml # 主界面布局
            │   └── activity_game.xml # 游戏界面布局
            ├── values/
            │   ├── strings.xml       # 字符串资源
            │   └── themes.xml        # 主题样式
            ├── drawable/
            │   └── ic_launcher.xml   # 应用图标
            └── xml/
                └── network_security_config.xml  # 网络安全配置
```

---

## 二、模块说明

### 1. Java 层（应用逻辑）

#### MainActivity.java
**职责**：应用启动入口，权限管理，模型文件选择

**核心方法**：
```java
onCreate()           // 初始化 UI，请求权限
checkPermissions()   // 检查存储权限
selectModelFile()    // 选择 llama 模型文件（已废弃，改用 HTTP API）
startGame()          // 启动 GameActivity
```

**流程**：
```
用户启动应用 → MainActivity → 权限检查 → GameActivity
```

---

#### GameActivity.java
**职责**：游戏主界面，UI 渲染，命令分发，JNI 调用，AI 通信

**核心组件**：

| 组件 | 类型 | 作用 |
|------|------|------|
| `gameOutput` | TextView | 游戏输出显示区 |
| `gameInput` | EditText | 命令输入框 |
| `btnLook/Map/...` | Button | 快捷命令按钮 |
| `morePanel` | LinearLayout | 更多命令面板（可展开/收起） |
| `targetContainer` | LinearLayout | 目标选择器（动态生成 NPC/地点按钮） |

**核心方法**：
```java
// UI 初始化
initViews()                    // 绑定按钮和事件监听
setupSendButton()              // 设置发送按钮和回车监听

// 命令处理
onClick()                      // 按钮点击事件分发
executeCommand()               // 执行命令（设置输入框文本并发送）
sendMessage()                  // 发送命令到 C 层或 AI API
handleChatCommand()            // 处理 chat [NPC] [消息]
handleAskCommand()             // 处理 ask [问题]
handleNpcTalk()                // NPC AI 对话（调用 ApiClient）

// 目标选择器
loadTargetsForCommand()        // 根据命令加载可用目标（NPC/地点）
showTargetButtons()            // 显示目标选择按钮
selectTarget()                 // 选中目标并填充输入框

// JNI 调用（native 方法）
initGame()                     // 初始化游戏
processInput()                 // 处理命令输入
cleanupGame()                  // 清理游戏资源
isGameRunning()                // 检查游戏状态
getCurrentScene()              // 获取当前场景
saveNpcTalk()                  // 保存 NPC 对话记忆
getNpcContext()                // 获取 NPC 上下文信息
```

**UI 布局结构**：
```
Title Bar (v2.0)
    ↓
ScrollView (游戏输出)
    ↓
快捷按钮区
├── 第一行：查看 | 地图 | 背包 | 状态
├── 第二行：任务 | 对话 | AI 对话 | 更多 ▼
    ↓
更多面板（可展开）
├── 【查看】外貌 | 记忆 | 查看 NPC
├── 【移动】前往
├── 【交互】送礼 | 交易 | 互动
├── 【自定义 NPC】创建 | 设置 | 删除
└── 【AI 功能】AI 问答
    ↓
目标选择器（动态生成 NPC/地点按钮）
    ↓
输入框 + 发送按钮
```

---

#### ApiClient.java
**职责**：HTTP API 客户端，与 Termux llama.cpp server 通信

**核心方法**：
```java
sendRequest(prompt)                // 发送普通 AI 请求
sendNpcRequest(npcContext, "", playerSay)  // 发送 NPC 角色扮演请求
parseJsonResponse(json)            // 解析 llama.cpp 返回的 JSON
```

**API 端点**：
```
POST http://127.0.0.1:8080/completion
Content-Type: application/json

{
  "prompt": "...",
  "n_predict": 300,
  "temperature": 0.7,
  "stop": ["</s>", "User:", "Question:"]
}
```

**响应格式**：
```json
{
  "content": "AI 回复内容",
  "stop": true,
  "generation_settings": {...}
}
```

---

### 2. JNI 层（游戏核心）

#### game_android.c
**职责**：游戏核心逻辑，命令解析，世界状态管理

**规模**：1248 行 C 代码

**核心模块**：

| 模块 | 函数 | 说明 |
|------|------|------|
| **初始化** | `init_game()`, `start_game()` | 创建世界、NPC、场景、任务 |
| **命令处理** | `process_input()`, `handle_command()` | 解析用户输入，分发到对应处理函数 |
| **场景系统** | `look_command()`, `go_command()` | 查看场景描述，移动到新场景 |
| **NPC 系统** | `talk_command()`, `chat_command()` | NPC 对话，AI 对话，关系计算 |
| **记忆系统** | `save_npc_talk()`, `get_npc_context()` | 保存对话记忆，生成 NPC 上下文 |
| **自定义 NPC** | `create_npc_command()`, `setnpc_command()` | 创建/编辑自定义 NPC |
| **背包系统** | `inventory_command()`, `gift_command()` | 查看背包，赠送礼物 |
| **任务系统** | `quest_command()` | 查看当前任务 |
| **JNI 接口** | `Java_com_adventure_game_GameActivity_*` | Java 调用 C 的桥接函数 |

**命令解析流程**：
```
用户输入 → process_input() → 命令匹配 → 执行对应函数 → 返回结果
    ↓
示例："chat 村长 你好"
    ↓
chat_command("村长 你好") → 解析 NPC 名和消息 → 调用 handle_npc_chat()
    ↓
调用 get_npc_context("村长") → 生成 NPC 上下文 → 返回 Java 层
    ↓
Java 层调用 ApiClient.sendNpcRequest() → HTTP 请求 llama.cpp
    ↓
返回 AI 回复 → save_npc_talk() 保存记忆 → 显示结果
```

**世界状态**：
```c
static WorldState state = {
    .current_scene = NULL,
    .player_gold = 100,
    .game_time = 480,  // 第 1 天 8:00
    .custom_npc_count = 0
};

static NPC npcs[MAX_NPCS];       // NPC 数组（内置 + 自定义）
static Scene scenes[MAX_SCENES]; // 场景数组
static Quest quest;              // 当前任务
static Inventory inventory;      // 背包
```

---

#### 头文件说明

##### types.h
**数据结构定义**：
```c
// 物品
typedef struct Item {
    char name[64];
    char description[256];
    int value;
    int type;  // 0=普通，1=任务物品，2=消耗品
} Item;

// NPC 外貌
typedef struct NPCAppearance {
    char hair[64];
    char eyes[64];
    char body[64];
    char clothes[64];
    char features[64];
} NPCAppearance;

// NPC 记忆
typedef struct MemoryEntry {
    char content[256];
    int timestamp;
    int relation_change;
    char type[32];     // "talk", "gift", "interact", "trade"
    char speaker[64];  // "玩家"
} MemoryEntry;

// NPC
typedef struct NPC {
    char id[64];
    char name[64];
    char description[256];
    NPCAppearance appearance;
    NPCStatus status;
    MemoryEntry memories[20];
    int memory_count;
    int relation;     // 0-100
    char location[64];
    int is_custom;
} NPC;

// 场景
typedef struct Scene {
    char id[64];
    char name[64];
    char description[256];
    char connections[8][64];  // 可前往的场景 ID
    int connection_count;
    char npcs[8][64];         // NPC 名称列表
    int npc_count;
} Scene;
```

---

##### game.h
**游戏主接口**：
```c
bool init_game();                    // 初始化游戏世界
void start_game();                   // 开始游戏循环
void cleanup_game();                 // 清理资源
bool is_game_running();              // 检查游戏状态
void handle_command(const char* input, char* output);  // 处理命令
const char* get_current_scene();     // 获取当前场景描述
```

---

##### npc.h
**NPC 系统接口**：
```c
NPC* find_npc_by_name(const char* name);         // 查找 NPC
void init_default_npcs();                        // 初始化内置 NPC
void create_custom_npc(const char* name, ...);   // 创建自定义 NPC
void save_npc_talk(NPC* npc, const char* talk, const char* reply);  // 保存对话
void generate_npc_context(NPC* npc, char* output);  // 生成 NPC 上下文
```

---

##### scene.h
**场景系统接口**：
```c
Scene* find_scene_by_id(const char* id);         // 查找场景
void init_default_scenes();                       // 初始化场景
const char* get_scene_description(Scene* scene); // 获取场景描述
```

---

##### quest.h
**任务系统接口**：
```c
void init_quest();                                 // 初始化任务
const char* get_quest_description();               // 获取任务描述
void update_quest_progress(int progress);          // 更新进度
```

---

##### inventory.h
**背包系统接口**：
```c
void init_inventory();                             // 初始化背包
void add_item(const char* name, int count);        // 添加物品
void remove_item(const char* name, int count);     // 移除物品
int has_item(const char* name);                    // 检查物品
int get_gold();                                    // 获取金币
void set_gold(int amount);                         // 设置金币
```

---

##### ai.h
**AI 接口**（占位，实际由 Java 层实现）：
```c
// AI 功能由 Java 层的 ApiClient 通过 HTTP 实现
// C 层只负责生成 prompt 和解析结果
void generate_ai_prompt(const char* context, char* output);
void parse_ai_response(const char* json, char* output);
```

---

### 3. 配置文件

#### build.gradle (app/)
**关键配置**：
```gradle
android {
    compileSdk 34
    defaultConfig {
        minSdk 21              // Android 5.0+
        targetSdk 34
        ndk {
            abiFilters 'arm64-v8a'  // 仅支持 64 位 ARM
        }
        externalNativeBuild {
            cmake {
                cppFlags ''
            }
        }
    }
    buildTypes {
        release {
            minifyEnabled false
        }
    }
    externalNativeBuild {
        cmake {
            path file('src/main/jni/CMakeLists.txt')
        }
    }
}
```

---

#### CMakeLists.txt
**JNI 库构建**：
```cmake
cmake_minimum_required(VERSION 3.4.1)

project(adventure-game)

add_library(adventure-game SHARED src/main/jni/game_android.c)

find_library(log-lib log)

target_link_libraries(adventure-game ${log-lib})

include_directories(src/main/jni/include)
```

---

#### AndroidManifest.xml
**应用配置**：
```xml
<manifest package="com.adventure.game">
    <uses-permission android:name="android.permission.INTERNET" />
    <uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE" />
    
    <application
        android:usesCleartextTraffic="true"  <!-- 允许 HTTP -->
        android:networkSecurityConfig="@xml/network_security_config">
        
        <activity android:name=".MainActivity">
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
        
        <activity android:name=".GameActivity" />
    </application>
</manifest>
```

---

#### network_security_config.xml
**网络安全**：
```xml
<network-security-config>
    <base-config cleartextTrafficPermitted="true">
        <trust-anchors>
            <certificates src="system" />
        </trust-anchors>
    </base-config>
    <domain-config cleartextTrafficPermitted="true">
        <domain includeSubdomains="true">127.0.0.1</domain>
        <domain includeSubdomains="true">localhost</domain>
    </domain-config>
</network-security-config>
```

---

## 三、命令系统

### 命令分类

| 类别 | 命令 | UI 按钮 |
|------|------|--------|
| **基础** | `help`, `look`, `map`, `inventory`, `quest`, `status`, `exit` | 查看、地图、背包、状态、任务 |
| **查看** | `appearance`, `memory`, `npc [名]` | 外貌、记忆、查看 NPC |
| **移动** | `go [地点]` | 前往 |
| **交互** | `talk [NPC]`, `chat [NPC] [内容]`, `ask [问题]` | 对话、AI 对话、AI 问答 |
| **社交** | `gift [NPC] [物品]`, `trade [NPC]`, `interact [NPC]` | 送礼、交易、互动 |
| **自定义** | `create_npc`, `setnpc`, `remove_npc` | 创建 NPC、设置 NPC、删除 NPC |

---

### 命令处理流程

```
用户输入/按钮点击
    ↓
GameActivity.sendMessage() 或 onClick()
    ↓
是 AI 命令? (chat/ask)
├── 是 → 调用 ApiClient.sendRequest() / sendNpcRequest()
│       ↓
│   HTTP POST → llama.cpp (127.0.0.1:8080)
│       ↓
│   解析 JSON 响应 → 显示结果
│
└── 否 → 调用 JNI processInput()
        ↓
    game_android.c:process_input()
        ↓
    命令匹配（strcmp）
        ↓
    执行对应函数（look_command, go_command, ...）
        ↓
    返回结果字符串 → Java 层显示
```

---

## 四、UI 组件

### 快捷按钮（8 个）

| 按钮 | 命令 | 说明 |
|------|------|------|
| 查看 | `look` | 查看当前场景描述 |
| 地图 | `map` | 显示完整世界地图 |
| 背包 | `inventory` | 查看背包和金币 |
| 状态 | `status` | 查看生命/法力/属性/等级 |
| 任务 | `quest` | 查看当前任务进度 |
| 对话 | `talk` | 与 NPC 固定对话 |
| AI 对话 | `chat` | 与 NPC AI 角色扮演对话 |
| 更多 ▼ | toggle panel | 展开/收起更多面板 |

---

### 更多面板按钮（11 个）

| 区域 | 按钮 | 命令 |
|------|------|------|
| **查看** | 外貌 | `appearance` |
| | 记忆 | `memory` |
| | 查看 NPC | `npc` |
| **移动** | 前往 | `go` |
| **交互** | 送礼 | `gift` |
| | 交易 | `trade` |
| | 互动 | `interact` |
| **自定义 NPC** | 创建 NPC | `create_npc` |
| | 设置 NPC | `setnpc` |
| | 删除 NPC | `remove_npc` |
| **AI 功能** | AI 问答 | `ask` |

---

### 目标选择器

**动态生成按钮**，当命令需要目标时显示：

| 命令 | 目标来源 |
|------|----------|
| `go` | 当前场景的 connections（可前往地点） |
| `talk` / `npc` | 当前场景的 npcs（NPC 列表） |
| `chat` | 当前场景的 npcs（NPC 列表） |
| `gift` / `trade` / `interact` | 当前场景的 npcs（NPC 列表） |
| `setnpc` / `remove_npc` | 自定义 NPC 列表 |

**示例**：
```
用户点击"AI 对话"按钮
    ↓
输入框显示 "chat "
    ↓
loadTargetsForCommand("talk") 获取当前场景 NPC
    ↓
targetContainer 显示按钮：[村长] [村民]
    ↓
用户点击 [村长]
    ↓
输入框变为 "chat 村长 "
    ↓
用户输入 "你好" 并发送
    ↓
执行 "chat 村长 你好"
```

---

## 五、数据流

### NPC AI 对话数据流

```
用户：chat 村长 最近的哥布林是怎么回事？
    ↓
GameActivity.handleChatCommand("村长 最近的哥布林是怎么回事？")
    ↓
解析 → npcName="村长", message="最近的哥布林是怎么回事？"
    ↓
调用 JNI: getNpcContext("村长")
    ↓
C 层：generate_npc_context()
    ↓
返回 NPC 上下文:
"姓名：村长
职业：村长
描述：村庄的领导者，睿智而仁慈。
外貌：花白长发，深邃的灰眼睛...
服装：深蓝色长袍...
特征：手持橡木手杖...
与玩家关系：50/100 (普通)
【记忆】
- 对话：玩家说"你好""
    ↓
Java 层构建 Prompt:
"你是一个角色扮演游戏中的 NPC。

【NPC 设定】
{npcContext}

玩家说：{message}

要求：
1. 保持 NPC 的人设和语气
2. 回复简洁（2-3 句话）
3. 使用中文回复
4. 不要提到你是 AI 或程序"
    ↓
ApiClient.sendNpcRequest(prompt)
    ↓
HTTP POST http://127.0.0.1:8080/completion
    ↓
llama.cpp 返回 JSON: {"content": "嗯...（捋了捋胡须）最近村子北边..."}
    ↓
解析 JSON → 提取 content
    ↓
JNI: saveNpcTalk("村长", "最近的哥布林是怎么回事？", "嗯...（捋了捋胡须）...")
    ↓
C 层保存到村长.memories[]
    ↓
显示结果到 gameOutput
```

---

### 移动命令数据流

```
用户：go 铁匠铺
    ↓
GameActivity.sendMessage()
    ↓
JNI: processInput("go 铁匠铺")
    ↓
C 层：handle_command() → go_command("铁匠铺")
    ↓
find_scene_by_id("blacksmith")
    ↓
检查是否在当前场景的 connections 中
    ↓
是 → state.current_scene = scenes[1] (铁匠铺)
    ↓
game_time += 60 (消耗 1 小时)
    ↓
返回: "你来到了铁匠铺。\n\n炉火熊熊，墙上挂满了各式武器..."
    ↓
Java 层显示结果
    ↓
loadTargetsForCommand() 更新 NPC 列表（铁匠老王）
```

---

## 六、构建流程

### 本地构建

```bash
# 1. 进入项目目录
cd adventure-game-android

# 2. 执行构建脚本
./build-apk.sh

# 3. 输出
> BUILD SUCCESSFUL
> APK: app/build/outputs/apk/debug/app-debug.apk
```

**build-apk.sh 内容**：
```bash
#!/bin/bash
export ANDROID_HOME=/opt/android-sdk
./gradlew clean assembleDebug
cp app/build/outputs/apk/debug/app-debug.apk ../app-debug.apk
```

---

### GitHub Actions CI

**.github/workflows/build.yml**：
```yaml
name: Build APK

on:
  push:
    tags: ['v*']

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - uses: actions/setup-java@v3
        with:
          java-version: '17'
          distribution: 'temurin'
      - run: ./gradlew assembleDebug
      - uses: actions/upload-artifact@v3
        with:
          name: app-debug
          path: app/build/outputs/apk/debug/app-debug.apk
```

---

## 七、版本管理

### 版本标识

| 位置 | 版本号 | 说明 |
|------|--------|------|
| `activity_game.xml` | v2.0 | UI 标题栏显示 |
| `README.md` | v2.0 | 文档说明 |
| Git Tag | v2.0-20260531 | 版本标记 |
| APK 文件 | app-debug-v2.0.apk | 文件命名 |

---

### 备份机制

| 备份文件 | 版本 | Tag | 说明 |
|---------|------|-----|------|
| `备份一.tar.gz` | v2.0 | 备份一-v2.0-stable | 稳定版备份 |

**备份内容**：
- 完整源代码
- 排除：构建缓存（.cxx, build, .gradle）

**恢复方法**：
```bash
# 方法 1: Git checkout
git checkout 备份一-v2.0-stable

# 方法 2: 解压备份
tar -xzf 备份一.tar.gz
```

---

## 八、扩展开发指南

### 添加新场景

1. 在 `game_android.c:init_game()` 中添加场景定义：
```c
scenes[3] = (Scene){
    .id = "castle",
    .name = "王国城堡",
    .description = "宏伟的石制城堡，卫兵巡逻。",
    .connections = {{"village"}},
    .connection_count = 1,
    .npcs = {{"国王"}},
    .npc_count = 1
};
```

2. 更新 `scenes[0].connections` 添加可前往城堡

3. 在 `find_npc_targets()` 中添加城堡的 NPC

4. 重新编译 APK

---

### 添加新 NPC

1. 在 `game_android.c:init_default_npcs()` 中添加：
```c
npcs[3] = (NPC){
    .id = "king",
    .name = "国王",
    .description = "王国的统治者，威严而公正。",
    .appearance = {
        .hair = "金色短发",
        .eyes = "蓝色眼睛",
        .body = "高大魁梧",
        .clothes = "紫色王袍，金色皇冠",
        .features = "手持权杖"
    },
    .relation = 30,
    .location = "castle",
    .is_custom = 0
};
```

2. 在城堡场景的 npcs 数组中添加"国王"

3. 重新编译 APK

---

### 添加新命令

1. 在 `game_android.c:process_input()` 中添加命令匹配：
```c
if (strncmp(input, "newcmd ", 7) == 0) {
    newcmd_command(input + 7, output);
    return;
}
```

2. 实现命令处理函数：
```c
void newcmd_command(const char* args, char* output) {
    sprintf(output, "执行新命令：%s", args);
}
```

3. 在 `GameActivity.java` 中添加按钮（可选）：
```java
else if (id == R.id.btnNewCmd) {
    executeCommand("newcmd ");
}
```

4. 在 `activity_game.xml` 中添加按钮定义

5. 重新编译 APK

---

## 九、技术栈

| 层级 | 技术 | 版本 |
|------|------|------|
| **开发语言** | Java, C | Java 17, C11 |
| **构建工具** | Gradle, CMake | Gradle 8.0, CMake 3.22 |
| **NDK** | Android NDK | r25c |
| **SDK** | Android SDK | API 34 |
| **最低支持** | Android | 5.0 (API 21) |
| **目标架构** | ARM | arm64-v8a |
| **AI 推理** | llama.cpp | HTTP API |
| **UI 框架** | Android View | 传统 XML 布局 |

---

## 十、文件统计

| 类型 | 数量 | 总大小 |
|------|------|--------|
| **Java 文件** | 3 | ~40 KB |
| **C 文件** | 1 | 60 KB |
| **头文件** | 8 | ~10 KB |
| **XML 布局** | 2 | ~15 KB |
| **配置文件** | 10+ | ~5 KB |
| **文档** | 3 | ~30 KB |
| **APK** | 1 | 5.4 MB |

---

## 十一、总结

本项目是一个**轻量化文字冒险游戏**，采用**混合架构**：
- **Java 层**：UI 和业务逻辑
- **C 层（JNI）**：游戏核心和世界状态
- **HTTP API**：AI 对话功能（Termux llama.cpp）

**核心特点**：
1. 轻量化（5.4MB APK，无内置模型）
2. 完整 NPC 系统（外貌/记忆/关系/状态）
3. 自定义 NPC 支持
4. AI 角色扮演对话
5. 友好的 UI（快捷按钮 + 目标选择器）

**改进方向**：
- 战斗系统
- 物品使用功能
- 存档/读档系统
- 更多场景和任务
