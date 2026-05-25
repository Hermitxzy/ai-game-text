# 文字冒险游戏 - Android APK 版

C 语言文字冒险游戏的 Android APK 版本，用户可以直接安装使用，无需命令行。

## 功能特性

- ✅ 完整的游戏功能（场景、背包、NPC、任务、存档）
- ✅ 原生 C 代码通过 JNI 运行
- ✅ 简单的文本界面
- ✅ 模型文件选择器
- ✅ 自动检测 models 目录
- ✅ 支持预回复模式和 AI 对话模式

## 项目结构

```
adventure-game-android/
├── app/
│   ├── src/main/
│   │   ├── java/com/adventure/game/   # Java 代码
│   │   │   ├── MainActivity.java       # 主界面（选择模型）
│   │   │   └── GameActivity.java       # 游戏界面
│   │   ├── jni/                        # JNI 本地代码
│   │   │   ├── Android.mk
│   │   │   ├── Application.mk
│   │   │   └── gamejni.c               # JNI 桥接层
│   │   ├── res/                        # 资源文件
│   │   │   ├── layout/
│   │   │   └── values/
│   │   └── AndroidManifest.xml
│   └── build.gradle
├── build.gradle                         # 根构建配置
├── settings.gradle
├── gradle.properties
├── gradlew                              # Gradle 包装器
└── build-apk.sh                         # 一键构建脚本
```

## 快速开始

### 前提条件

1. **Android Studio** (推荐) 或 **命令行工具**
2. **NDK** (通过 Android Studio 安装)
3. **Java JDK 8+**

### 安装依赖

```bash
# 方法 1: 使用 Android Studio
# - 安装 Android Studio
# - Tools -> SDK Manager -> SDK Tools
# - 勾选 "NDK (Side by side)" 和 "CMake"

# 方法 2: 使用命令行工具
wget https://dl.google.com/android/repository/commandlinetools-linux-9477386_latest.zip
unzip commandlinetools-*.zip
mkdir -p $HOME/Android/Sdk
mv cmdline-tools $HOME/Android/Sdk/
export ANDROID_HOME=$HOME/Android/Sdk
$ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager --install "ndk;25.2.9519653"
```

### 设置环境变量

```bash
export ANDROID_HOME=$HOME/Android/Sdk
export ANDROID_NDK_HOME=$ANDROID_HOME/ndk/25.2.9519653
```

### 一键构建

```bash
cd /workspace/adventure-game-android

# 构建 Debug 版本
./build-apk.sh debug

# 构建 Release 版本
./build-apk.sh release

# 清理
./build-apk.sh clean
```

### 使用 Gradle 直接构建

```bash
# Debug
./gradlew assembleDebug

# Release
./gradlew assembleRelease

# 安装到设备
./gradlew installDebug
```

## APK 输出位置

| 类型 | 路径 |
|------|------|
| Debug | `app/build/outputs/apk/debug/app-debug.apk` |
| Release | `app/build/outputs/apk/release/app-release-unsigned.apk` |

## 安装到手机

### 方法 1: 使用 ADB

```bash
adb install app/build/outputs/apk/debug/app-debug.apk
```

### 方法 2: 手动传输

```bash
# 复制 APK 到手机
adb push app/build/outputs/apk/debug/app-debug.apk /sdcard/Download/

# 在手机上打开文件管理器，点击 APK 安装
```

## 使用方法

### 1. 首次启动

```
┌─────────────────────────────┐
│  文字冒险游戏                │
├─────────────────────────────┤
│                             │
│  模型文件                    │
│  [选择 GGUF 模型]            │
│                             │
│  [开始游戏]                 │
│                             │
│  提示：也可以将 .gguf 文件   │
│  放入 models 目录           │
└─────────────────────────────┘
```

### 2. 放置模型文件（可选）

将 GGUF 模型文件放入以下目录，启动时会自动检测：

```
内部存储/Android/data/com.adventure.game/files/models/
```

### 3. 游戏界面

```
┌─────────────────────────────┐
│        文字冒险游戏          │
├─────────────────────────────┤
│                             │
│  [游戏输出区域]             │
│  > 看                        │
│  你站在村庄广场...          │
│  可前往：铁匠铺 森林        │
│                             │
├─────────────────────────────┤
│  [输入命令...]    [发送]    │
└─────────────────────────────┘
```

## 支持的命令

| 命令 | 说明 |
|------|------|
| `help` | 查看帮助 |
| `look` | 查看当前场景 |
| `go [方向/地点]` | 移动 |
| `take [物品]` | 拾取物品 |
| `inventory` | 查看背包 |
| `talk [NPC]` | 与 NPC 对话 |
| `shop [NPC]` | 打开商店 |
| `quest` | 查看任务 |
| `save [槽位]` | 保存游戏 |
| `load [槽位]` | 加载游戏 |
| `ask [问题]` | AI 对话 (需要模型) |
| `exit` | 退出 |

## 预回复模式 vs AI 模式

### 预回复模式（无需模型）
- 使用预设的对话回复
- 适合体验基本游戏玩法
- 无需下载模型文件

### AI 模式（需要 GGUF 模型）
- 使用本地大模型生成对话
- NPC 对话更智能、更自然
- 需要下载 1-2GB 的模型文件

推荐模型：
- **Qwen2.5 1.5B** (平衡速度和质量)
- **Llama 3.2 1B** (最小体积)
- **Qwen2.5 0.5B** (最快速度)

## 故障排除

### 问题 1: 找不到 NDK

**解决**:
```bash
# Android Studio 中安装
Tools -> SDK Manager -> SDK Tools -> NDK
```

### 问题 2: Gradle 构建失败

**解决**:
```bash
# 清理缓存
./gradlew clean
rm -rf ~/.gradle/caches

# 重新构建
./gradlew assembleDebug
```

### 问题 3: APK 安装失败

**解决**:
```bash
# 检查签名 (Debug 版本不需要单独签名)
# Android 11+ 需要允许未知来源安装
```

### 问题 4: 游戏闪退

**解决**:
```bash
# 查看日志
adb logcat | grep AdventureGame

# 检查是否选择了模型文件
# 或使用预回复模式（不选择模型直接开始）
```

## 文件说明

### JNI 桥接层 (gamejni.c)

```c
// Java 调用初始化游戏
JNIEXPORT jboolean JNICALL Java_com_adventure_game_GameActivity_initGame(
    JNIEnv *env, jobject thiz, jstring modelPath);

// Java 调用处理输入
JNIEXPORT jstring JNICALL Java_com_adventure_game_GameActivity_processInput(
    JNIEnv *env, jobject thiz, jstring input);

// 游戏输出回调到 Java
void send_output_to_java(const char *text);
```

### 权限说明

| 权限 | 用途 |
|------|------|
| `READ_EXTERNAL_STORAGE` | 读取模型文件 (Android 12 及以下) |
| `WRITE_EXTERNAL_STORAGE` | 写入存档文件 (Android 9 及以下) |
| `MANAGE_EXTERNAL_STORAGE` | 访问 models 目录 (Android 11+) |

## 编译优化

### 减小 APK 体积

```bash
# 只编译需要的架构
# app/build.gradle 中修改：
ndk {
    abiFilters 'arm64-v8a'  // 只保留 64 位
}
```

### 提高性能

```bash
# 修改 C 代码编译选项
# app/src/main/jni/Android.mk
LOCAL_CFLAGS := -O3 -march=armv8-a
```

## 与命令行版对比

| 特性 | 命令行版 | APK 版 |
|------|---------|-------|
| 安装方式 | 手动编译 | 直接安装 APK |
| 运行环境 | Termux/命令行 | 任意 Android 设备 |
| 模型选择 | 命令行参数 | 图形界面 |
| 用户体验 | 开发者友好 | 普通用户友好 |
| 文件大小 | ~100KB | ~10-20MB |

---

**祝你玩得愉快！**

## 下载 APK

### 方式 1: GitHub Release（推荐）

访问 Releases 页面下载编译好的 APK：
**https://github.com/Hermitxzy/ai-game-text/releases**

### 方式 2: GitHub Actions

1. 访问 Actions 页面：**https://github.com/Hermitxzy/ai-game-text/actions**
2. 点击最近一次的编译工作流
3. 向下滚动找到 Artifacts
4. 下载 `app-debug.apk`

### 方式 3: 本地编译

```shell
export ANDROID_HOME=/opt/android-sdk
export ANDROID_NDK_HOME=/opt/android-sdk/ndk/25.2.9519653
cd adventure-game-android
./gradlew assembleDebug
```

APK 输出位置：`app/build/outputs/apk/debug/app-debug.apk`
