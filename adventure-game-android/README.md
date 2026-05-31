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

### 4. 游戏操作

**快捷按钮（第一行）**
- 查看 - 查看当前场景
- 地图 - 查看完整地图
- 背包 - 查看背包物品
- 状态 - 查看主角属性

**快捷按钮（第二行）**
- 任务 - 查看当前任务
- 对话 - 选择 NPC 进行对话
- 更多 ▼ - 展开更多命令

**更多命令（点击"更多 ▼"展开）**

查看类：
- 外貌 - 查看主角外貌
- 记忆 - 查看主角记忆

移动：
- 前往 - 选择地点移动

交互类：
- 送礼 - 赠送礼物给 NPC
- 交易 - 与 NPC 交易
- 互动 - 与 NPC 友好互动

自定义：
- 创建 NPC - 创建自定义 NPC（点击查看详情）
- 设置 NPC - 修改自定义 NPC 属性
- 删除 NPC - 删除自定义 NPC

**输入框**
- 可手动输入命令（如 AI 对话）
- 选择目标后自动填充命令

### 5. 游戏命令详解
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
create_npc              # 查看创建向导和详细说明
create_npc [名] [职业]  # 快速创建 NPC
例：create_npc 艾莉 法师
    create_npc 剑圣 战士

setnpc [名] [属性] [值] # 修改 NPC 属性
属性：name, hair, eyes, body, clothes, features, occupation, desc
例：setnpc 艾莉 hair 金色长发
    setnpc 艾莉 occupation 大法师

remove_npc [NPC 名]     # 删除自定义 NPC
例：remove_npc 艾莉
```

**AI 对话**
```
ask [问题]              # 普通 AI 对话
例：ask 请用中文介绍一下你自己
    ask 什么是人工智能？
    ask Write a short story about a knight

chat [NPC] [内容]       # NPC 角色扮演对话（AI）
例：chat 村长 最近的哥布林是怎么回事？
    chat 艾莉 教我魔法吧
    chat 铁匠老王 有什么好武器推荐？
```

**AI 对话示例**

NPC 对话（v2.0 新增）：

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
- **快捷操作** - 常用命令一键执行，无需频繁切换

## 版本历史

### v2.3 (当前版本)
- ✅ 修复多次 NPC AI 对话闪退问题：使用循环缓冲区防止数组越界
- ✅ 修复 NPC 记忆溢出 problem：当记忆满 20 条时自动覆盖最旧记忆
- ✅ 修复主角记忆溢出 problem：当记忆满 30 条时自动覆盖最旧记忆
- ✅ 添加 LOGW 日志宏定义
- ✅ 增强内存安全性：所有 strncpy 添加 null 终止符

### v2.2
- ✅ 修复"AI 对话"按钮无响应问题：添加按钮变量声明和点击事件绑定
- ✅ 修复"AI 问答"按钮无响应问题：添加 findViewById 和 onClick 绑定
- ✅ 代码优化：清理重复的 findViewById 调用

### v2.1
- ✅ 新增"AI 对话"快捷按钮：一键启动 NPC AI 对话，自动加载 NPC 列表
- ✅ 新增"AI 问答"按钮到更多面板：支持普通 AI 问答功能
- ✅ 完善项目结构文档：新增 STRUCTURE.md 详细说明架构和代码
- ✅ UI 版本号更新：标题栏更新为 v2.0 → v2.1
- ✅ 优化目标选择器：chat 命令支持 NPC 快速选择
- ✅ 代码清理：移除重复代码，优化按钮事件处理

### v2.0
- ✅ 实现 NPC AI 对话功能：使用 `chat [NPC] [内容]` 命令进行 AI 驱动的对话
- ✅ NPC 上下文系统：AI 对话时自动加载 NPC 设定（外貌/职业/关系/记忆）
- ✅ 对话记忆功能：NPC 会记住与玩家的对话历史
- ✅ 角色扮演模式：AI 根据 NPC 人设生成个性化回复
- ✅ 新增 ApiClient.sendNpcRequest() 方法：支持带上下文的 AI 请求
- ✅ JNI 接口：getNpcContext() / saveNpcTalk() 用于获取 NPC 信息和保存对话
- ✅ 记忆类型系统：区分 talk/gift/interact/trade 等不同互动类型

### v1.9
- ✅ 完善自定义 NPC 创建功能，添加三种创建方式：
  - 快速创建：`create_npc [名称] [职业]`
  - 详细创建：`create_npc [名称] [职业] [发型] [眼睛] [身材] [服装] [特征]`
  - AI 创建：`ask 创建一个 [描述]`
- ✅ 新增 `setnpc` 命令：修改自定义 NPC 属性（name/hair/eyes/body/clothes/features/occupation/desc）
- ✅ 新增 `btnSetNpc` 按钮：在更多面板中可快速设置 NPC 属性
- ✅ 创建向导：输入 `create_npc` 查看完整使用说明
- ✅ 创建反馈：成功创建后显示 NPC 完整信息卡，包含操作提示
- ✅ 删除 NPC：优化为只显示自定义 NPC 列表
- ✅ 统计显示：创建向导显示当前已创建的自定义 NPC 数量

### v1.8
- ✅ 修复 NPC 位置检查 bug：交互指令（对话/送礼/交易/互动）现在正确识别 NPC 位置
- ✅ 统一使用 `NPC.location` 字段判断 NPC 是否在场，而非过时的场景字符串
- ✅ 修复后村长、村民等 NPC 可以正常交互

### v1.7
- ✅ 修复 NPC 目标列表解析：使用 NPC 数组而非场景字符串，正确区分独立 NPC
- ✅ 交互命令（对话/送礼/交易/互动）现在正确显示单个 NPC 按钮
- ✅ 查看 NPC 按钮只显示当前场景的 NPC

### v1.6
- ✅ 新增"查看 NPC"按钮：在更多面板中可快速查看 NPC 外貌和状态
- ✅ 查看 NPC 时自动显示当前场景 NPC 选择按钮

### v1.5
- ✅ 新增快捷按钮面板：look, map, inventory, status, quest, talk 一键执行
- ✅ 新增"更多"命令面板：展开查看完整命令（外貌/记忆/移动/交互/自定义）
- ✅ 优化操作流程：需要目标的命令自动显示目标选择按钮
- ✅ 移除←→切换命令：不再需要频繁点击切换，操作更直观

### v1.4
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
