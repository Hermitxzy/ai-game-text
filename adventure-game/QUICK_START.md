# 快速编译与运行指南

## 快速开始

```bash
cd /workspace/adventure-game

# 编译
make

# 运行（预设回复模式）
./bin/adventure-game
```

## 编译命令

| 平台 | 命令 |
|------|------|
| Linux | `make` |
| macOS | `make` |
| Android arm64 | `make android-arm64` |
| Android armv7 | `make android-arm` |

## 运行模式

### 模式 1: 预设回复（无需模型）

```bash
./bin/adventure-game
```

### 模式 2: AI 对话（需要 GGUF 模型）

```bash
# 下载模型
./download-model.sh

# 启动游戏
./bin/adventure-game models/qwen2.5-1.5b.gguf
```

## 游戏示例

```
> help

╔═══════════════════════════════════════════╗
║            游戏命令帮助                   ║
╠═══════════════════════════════════════════╣
║ 移动：go [方向] / n,s,e,w,u,d           ║
║ 查看：look / l                           ║
║ 物品：take [物品] / drop [物品]          ║
║ 背包：inventory / i                      ║
║ NPC:   talk [NPC] / shop [NPC]            ║
║ 任务：quest / quests                     ║
║ 系统：save [槽位], load [槽位], help      ║
║ AI:    ask [问题] (需要加载模型)          ║
║ 退出：quit / exit                         ║
╚═══════════════════════════════════════════╝

> look

┌──────────────────────────────────────────────┐
│ 新手村广场                              │
├──────────────────────────────────────────────┤
你站在一个宁静的小村庄广场中央。四周是古朴的木屋，村民们忙碌地走动。
北方是村庄的铁匠铺，东方有通往森林的小路。

可前往：[铁匠铺] [迷雾森林入口] 
NPC: [村长] [铁匠老王] 
└──────────────────────────────────────────────┘

> inventory

========== 背包 ==========
金币：50
物品 (1/100):
--------------------------
  [1] 面包 x2
==========================

> talk 铁匠老王

========== 与 铁匠老王 对话 ==========
铁匠老王：欢迎来到这里，冒险者！有什么我可以帮你的吗？

> quest

========== 任务列表 ==========

[进行中] 清剿哥布林
  村庄附近的哥布林越来越多，需要教训它们。

[进行中] 收集水晶矿
  铁匠需要水晶矿来锻造武器，帮他收集 3 块。
  需要：crystal_ore 0/3
================================

> save 1
游戏已保存到槽位 1

> exit
感谢您的游玩，再见！
```

## 安装 llama.cpp（AI 模式必需）

### Linux

```bash
git clone https://github.com/ggerganov/llama.cpp
cd llama.cpp && make shared -j
sudo cp build/libllama.so /usr/local/lib/
sudo ldconfig
```

### macOS

```bash
git clone https://github.com/ggerganov/llama.cpp
cd llama.cpp && make shared -j
sudo cp build/libllama.dylib /usr/local/lib/
```

### Android

详见 `ANDROID_BUILD.md`

## 下载模型

```bash
./download-model.sh
```

或手动下载：

```bash
wget -O models/qwen2.5-1.5b.gguf \
  https://huggingface.co/Qwen/Qwen2.5-1.5B-Instruct-GGUF/resolve/main/qwen2.5-1.5b-instruct-q4_k_m.gguf
```

## 故障排除

### 问题：libllama.so 无法加载

```bash
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

### 问题：模型文件不存在

```bash
ls models/
# 如果没有模型文件，游戏会自动使用预设回复模式
```

## 项目文件

```
adventure-game/
├── include/           # 头文件
├── src/               # 源代码
├── bin/               # 编译输出
├── models/            # GGUF 模型
├── data/saves/        # 存档文件
├── Makefile           # 编译配置
├── README.md          # 完整文档
├── ANDROID_BUILD.md   # Android 编译指南
├── start.sh           # 快速启动脚本
├── download-model.sh  # 模型下载脚本
└── test.sh            # 测试脚本
```

## 系统需求

| 项目 | 要求 |
|------|------|
| 编译器 | GCC/Clang |
| 运行库 | libdl |
| AI 模式 | llama.cpp + GGUF 模型 |
| 内存（AI 模式） | 模型大小的 1.5 倍 |

---

**祝你冒险愉快！**
