# 文字冒险游戏 - HTTP API 版

Android 文字冒险游戏，通过 HTTP API 调用 Termux 上运行的 llama.cpp 模型。

## 项目结构

```
adventure-game-android/
├── app/
│   └── src/main/
│       ├── java/com/adventure/game/
│       │   ├── MainActivity.java      # 主入口（可选，可直接启动 GameActivity）
│       │   ├── GameActivity.java      # 游戏主界面
│       │   └── ApiClient.java         # HTTP API 客户端
│       ├── jni/
│       │   ├── game_android.c         # 游戏核心逻辑（JNI）
│       │   └── CMakeLists.txt         # CMake 配置
│       └── res/layout/
│           ├── activity_main.xml      # 主界面布局
│           └── activity_game.xml      # 游戏界面布局
└── build.gradle
```

## APK 大小

**5.4 MB**（相比之前的 472MB 大幅减小）

## 使用说明

### 1. 安装 APK

```bash
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

### 2. 在 Termux 中启动 llama.cpp server

```bash
cd ~/llama.cpp
./server -m /path/to/your-model.gguf --host 127.0.0.1 --port 8080
```

### 3. 启动游戏

打开应用，等待 API 连接检查：
- ✓ 绿色 - API 服务器已连接
- ✗ 红色 - 需要启动 Termux 中的 server

### 4. 游戏命令

**基础命令**
```
help                    # 查看帮助
look / l                # 查看当前场景
map / m                 # 查看完整地图
inventory / i           # 查看背包
go [地点]               # 移动
  go 铁匠铺              # 前往铁匠铺
  go 森林                # 前往迷雾森林
  go 广场                # 返回新手村广场
quest                   # 查看任务
talk [NPC]              # 与 NPC 对话
exit / quit             # 退出游戏
```

**查看命令**
```
status                  # 查看主角状态（生命/法力/属性/等级）
appearance              # 查看主角外貌
memory                  # 查看主角记忆
npc [NPC 名]            # 查看 NPC 详情（外貌/状态/关系）
```

**交互命令**
```
gift [NPC] [物品]       # 赠送礼物提升关系
trade [NPC]             # 打开交易界面
interact [NPC]          # 与 NPC 互动（聊天等）
```

**自定义 NPC**
```
create_npc              # 创建自定义 NPC
remove_npc [NPC 名]     # 删除自定义 NPC
```

**AI 对话**
```
ask [问题]              # AI 对话（使用 Termux 模型）
例：ask 请用中文介绍一下你自己
    ask 什么是人工智能？
    ask Write a short story about a knight
```

### 5. AI 对话示例

```
ask 请用中文介绍一下你自己
ask 什么是人工智能？
ask Write a short story about a knight
```

## API 配置

默认配置：
- **地址**: `http://127.0.0.1:8080`
- **端点**: `/completion`
- **超时**: 60 秒

如需修改，编辑 `ApiClient.java` 中的 `BASE_URL` 常量。

## 技术特性

- **纯本地运行** - 无需互联网连接
- **轻量化** - 5.4MB APK，无内置模型
- **灵活** - 支持 Termux 上任意 GGUF 模型
- **低延迟** - localhost HTTP 通信
- **完整 NPC 系统** - 外貌/记忆/状态/关系值
- **主角成长** - 属性/等级/经验/记忆系统
- **交互动作** - 送礼/交易/互动影响关系
- **自定义 NPC** - 创建和管理自定义 NPC

## 版本历史

### v1.4 (当前版本)
- ✅ 重构 NPC 系统：添加外貌、记忆、状态属性
- ✅ 重构主角系统：添加外貌、状态（生命/法力/属性）、记忆
- ✅ 新增查看命令：status, appearance, memory, npc [NPC 名]
- ✅ 新增交互命令：gift, trade, interact
- ✅ 实现关系系统：NPC 关系值影响对话和行为
- ✅ 实现记忆系统：记录与 NPC 的互动历史
- ✅ 实现自定义 NPC：create_npc, remove_npc
- ✅ 时间系统：移动会消耗时间，显示第 X 天 X:00
- ✅ 命令选择器升级：支持更多命令和目标选择

### v1.3
- ✅ 无目标命令支持点击直接执行（map/look/inventory/quest）
- ✅ 有目标命令按钮选择（go/talk）
- ✅ 可点击的命令显示高亮效果

### v1.2
- ✅ 添加详细代码注释
- ✅ 统一 NPC 解析逻辑
- ✅ 修复 talk 命令 bug

### v1.1
- ✅ 修复闪退问题
- ✅ 优化命令解析

### v1.0
- ✅ 基础 HTTP API 版本
- ✅ 场景/任务/NPC 系统

## 故障排查

### API 服务器未响应

1. 确认 Termux 中 server 已启动
2. 检查端口是否正确（默认 8080）
3. 确保手机和 Termux 网络相通

### AI 响应慢

1. 使用更小的模型（如 Q2_K 量化）
2. 减少 `n_predict` 参数（在 ApiClient.java 中）
3. 降低 temperature 值

## 编译

```bash
cd adventure-game-android
export ANDROID_HOME=/opt/android-sdk
./gradlew assembleDebug
```

## 许可证

MIT License
