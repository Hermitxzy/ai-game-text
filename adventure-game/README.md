# C 语言文字冒险游戏

一个完整可编译的 C 语言文字冒险游戏，集成 llama.cpp 实现本地 AI 对话。

## 功能特性

- ✅ **场景系统**: 多场景连接、场景描述、出口导航
- ✅ **背包系统**: 物品管理、金币系统、任务物品
- ✅ **NPC 系统**: NPC 对话、预设对话、AI 辅助对话
- ✅ **任务系统**: 任务创建、进度追踪、任务完成奖励
- ✅ **商店系统**: 物品买卖、价格计算、NPC 商店
- ✅ **存档系统**: 多存档槽位、存档加载、状态保存
- ✅ **AI 对话**: llama.cpp 集成、GGUF 模型支持、本地推理
- ✅ **模块化设计**: 无全局变量、清晰的模块划分
- ✅ **跨平台**: 支持 Linux、macOS、Android arm64/armv7

## 项目结构

```
adventure-game/
├── include/              # 头文件
│   ├── types.h          # 类型定义
│   ├── game.h           # 游戏核心
│   ├── scene.h          # 场景管理
│   ├── inventory.h      # 背包系统
│   ├── npc.h            # NPC 系统
│   ├── quest.h          # 任务系统
│   ├── savegame.h       # 存档系统
│   └── ai.h             # AI 模块
├── src/                  # 源代码
│   ├── main.c           # 程序入口
│   ├── game.c           # 游戏逻辑
│   ├── scene.c          # 场景实现
│   ├── inventory.c      # 背包实现
│   ├── npc.c            # NPC 实现
│   ├── quest.c          # 任务实现
│   ├── savegame.c       # 存档实现
│   └── ai.c             # AI 集成
├── models/               # GGUF 模型目录
├── data/saves/          # 存档目录
├── Makefile             # 编译配置
└── README.md            # 本文档
```

## 快速开始

### 1. 编译游戏

```bash
# Linux / macOS
cd /workspace/adventure-game
make

# Android arm64 (需要 NDK)
make android-arm64

# Android armv7
make android-arm
```

### 2. 运行游戏

```bash
# 预设回复模式（不需要模型）
./bin/adventure-game

# 使用 GGUF 模型（启用 AI 对话）
./bin/adventure-game models/your-model.gguf
```

## llama.cpp 集成

### 方式一：预编译库（推荐）

1. **下载 llama.cpp**:
```bash
git clone https://github.com/ggerganov/llama.cpp
cd llama.cpp
```

2. **编译动态库**:
```bash
# Linux
make shared -j
sudo cp build/libllama.so /usr/local/lib/

# macOS
make shared -j
sudo cp build/libllama.dylib /usr/local/lib/

# Android (需要 NDK)
export NDK=/path/to/ndk
./scripts/build-android.sh
```

3. **下载 GGUF 模型**:
```bash
# Qwen2.5 1.5B (推荐用于低配置设备)
wget https://huggingface.co/Qwen/Qwen2.5-1.5B-Instruct-GGUF/resolve/main/qwen2.5-1.5b-instruct-q4_k_m.gguf \
  -O models/qwen2.5-1.5b.gguf

# Llama 3.2 1B
wget https://huggingface.co/bartowski/Llama-3.2-1B-Instruct-GGUF/resolve/main/Llama-3.2-1B-Instruct-Q4_K_M.gguf \
  -O models/llama-3.2-1b.gguf
```

4. **运行游戏**:
```bash
./bin/adventure-game models/qwen2.5-1.5b.gguf
```

### 方式二：源码集成

修改 `src/ai.c`，添加完整的 llama.cpp API 调用：

```c
#include <llama.h>

int ai_init(AIContext *ai, const char *model_path) {
    struct llama_model_params model_params = llama_model_default_params();
    ai->model = llama_load_model_from_file(model_path, model_params);
    
    if (!ai->model) {
        set_error("无法加载模型文件");
        return -1;
    }
    
    struct llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = ai->n_ctx;
    ai->context = llama_init_from_model(ai->model, ctx_params);
    
    ai->loaded = true;
    return 0;
}

char* ai_generate_response(AIContext *ai, const char *prompt, int max_tokens) {
    // 1. Tokenize 输入
    llama_token tokens[512];
    int n_tokens = llama_tokenize(ai->model, prompt, strlen(prompt), 
                                   tokens, 512, true, true);
    
    // 2. 创建 batch
    struct llama_batch batch = llama_batch_get_one(tokens, n_tokens);
    
    // 3. 解码 (prefill)
    llama_decode(ai->context, batch);
    
    // 4. 生成回复
    // ... (实现文本生成逻辑)
    
    return response;
}
```

## 游戏命令

### 基本命令

| 命令 | 说明 |
|------|------|
| `help` / `h` | 显示帮助信息 |
| `look` / `l` | 查看当前场景 |
| `go [方向]` | 移动到相邻场景 |
| `n/s/e/w` | 快速移动（北/南/东/西） |
| `take [物品]` | 拾取物品 |
| `drop [物品]` | 丢弃物品 |

### 交互命令

| 命令 | 说明 |
|------|------|
| `inventory` / `i` | 查看背包 |
| `talk [NPC]` | 与 NPC 对话 |
| `shop [NPC]` | 打开 NPC 商店 |
| `quest` / `quests` | 查看任务列表 |

### 系统命令

| 命令 | 说明 |
|------|------|
| `save [槽位]` | 保存游戏 (1-5) |
| `load [槽位]` | 加载游戏 (1-5) |
| `saves` | 查看存档列表 |
| `ask [问题]` | 向 AI 提问 |
| `quit` / `exit` | 退出游戏 |

## 游戏示例

### 场景探索

```
> look

┌──────────────────────────────────────────────┐
│ 新手村广场                                    │
├──────────────────────────────────────────────┤
你站在一个宁静的小村庄广场中央。四周是古朴的木屋，村民们
忙碌地走动。北方是村庄的铁匠铺，东方有通往森林的小路。

可前往：[铁匠铺] [迷雾森林入口] 
NPC: [铁匠老王] [村长] 
└──────────────────────────────────────────────┘
```

### NPC 对话

```
> talk 铁匠老王

========== 与 铁匠老王 对话 ==========
预设对话:
  [1] 你好，冒险者！需要武器或护甲吗？
  [2] 最近森林里的哥布林越来越猖狂了...
  [3] 听说洞穴里有珍贵的魔法矿石。

输入对话内容，或输入 'bye' 结束对话
==================================

铁匠老王：欢迎来到这里，冒险者！有什么我可以帮你的吗？

> 最近有什么任务吗？
你：最近有什么任务吗？
铁匠老王：嗯，森林里的哥布林越来越猖狂，如果你能教训它们，
          村民会非常感激你的！

> bye
结束了对话。
```

### AI 问答

```
> ask 这个世界的背景故事是什么？

思考中...

AI: 这个世界曾经是一个和平的大陆，各村庄和睦相处。
然而最近，迷雾森林中出现了一股黑暗力量，哥布林和
其他魔物开始袭击村庄。铁匠老王正在打造武器准备抵抗，
村长也在寻求勇敢的冒险者帮助。传说神秘洞穴深处藏着
能够驱散黑暗的水晶矿，但很少有人敢深入其中...
```

## 代码架构

### 模块依赖

```
main.c
  └── game.c
      ├── scene.c (场景管理)
      ├── inventory.c (背包系统)
      ├── npc.c (NPC 和商店)
      ├── quest.c (任务系统)
      ├── savegame.c (存档系统)
      └── ai.c (llama.cpp 集成)
```

### 核心数据结构

```c
// 游戏上下文 (无全局变量)
struct GameContext {
    Scene *current_scene;
    Inventory inventory;
    Quest quests[MAX_QUESTS];
    NPC npcs[MAX_NPCS];
    Scene scenes[MAX_SCENES];
    bool running;
    bool in_dialogue;
    bool in_shop;
};

// AI 上下文
struct AIContext {
    void *model;
    void *context;
    char model_path[512];
    int64_t n_ctx;
    bool loaded;
};
```

## Android 编译指南

### 前置条件

1. **安装 Android NDK**:
```bash
# 下载 NDK
wget https://dl.google.com/android/repository/android-ndk-r25b-linux.zip
unzip android-ndk-r25b-linux.zip
export NDK_HOME=$PWD/android-ndk-r25b
```

2. **设置环境变量**:
```bash
export PATH=$NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH
```

3. **编译 llama.cpp for Android**:
```bash
cd llama.cpp
./scripts/build-android.sh
```

4. **编译游戏**:
```bash
cd adventure-game
make android-arm64
```

### 在 Android 设备上运行

```bash
# 推送文件到设备
adb push bin/adventure-game /data/local/tmp/
adb push models/*.gguf /data/local/tmp/models/
adb push libs/android/arm64/libllama.so /data/local/tmp/

# 设置权限和库路径
adb shell
cd /data/local/tmp
export LD_LIBRARY_PATH=/data/local/tmp
chmod +x adventure-game

# 运行
./adventure-game models/qwen2.5-1.5b.gguf
```

## 存档格式

存档文件格式为二进制，位于 `data/saves/save_N.dat`:

```
[Inventory 结构体]
[Quest 数量]
[Quest 数组]
[Scene 数量]
[当前场景 ID (64 字节)]
[对话状态]
[商店状态]
```

## 自定义扩展

### 添加新场景

```c
// 在 init_game_world() 中
create_scene(game, "castle", "古老城堡", "一座废弃的城堡...");
add_scene_connection(&game->scenes[i], "forest");
```

### 添加新 NPC

```c
NPC *merchant = create_npc(game, "merchant", "商人", "精明的旅行商人...");
npc_add_dialogue(merchant, "看看我的商品吧！");
npc_enable_shop(merchant);
npc_add_shop_item(merchant, &item);
```

### 添加新任务

```c
quest_create(game, "explore_cave", "探索洞穴",
             "探索神秘洞穴的深处。", NULL, 0);
```

## 故障排除

### 问题：libllama.so 无法加载

**解决**:
```bash
# 检查库文件位置
ls -la /usr/local/lib/libllama.so

# 设置库路径
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# 或复制到当前目录
cp /usr/local/lib/libllama.so ./libs/linux/
```

### 问题：模型加载失败

**解决**:
- 确认 GGUF 模型文件完整
- 检查模型路径是否正确
- 确保内存足够（1.5B 模型约需 2GB RAM）

### 问题：Android 上崩溃

**解决**:
- 使用较新的 Android 版本（API 21+）
- 减少模型大小（使用量化版本 Q4_K_M）
- 检查 SELinux 权限

## 性能优化建议

1. **模型选择**: 使用 Q4_K_M 量化版本，平衡速度和质量
2. **上下文长度**: 设置 `n_ctx = 512` 减少内存占用
3. **线程数**: 移动设备设置 `n_threads = 2`
4. **批次大小**: `n_batch = 256` 提高推理速度

## 许可证

本项目代码采用 MIT 许可证。

llama.cpp 遵循其原有许可证 (MIT)。

## 贡献

欢迎提交 Issue 和 Pull Request！

---

**祝你冒险愉快！** 🎮
