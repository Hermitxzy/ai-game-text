# GitHub Releases 使用说明

## 下载 APK

由于 GitHub 仓库不直接存储 APK 文件，请使用以下方式获取：

### 方式 1: 从 Releases 页面下载（推荐）

1. 访问项目的 [Releases 页面](https://github.com/Hermitxzy/ai-game-text/releases)
2. 下载 `app-debug.apk` 文件

### 方式 2: 直接下载链接

如果已上传到 Releases，直接点击：
```
https://github.com/Hermitxzy/ai-game-text/releases/latest/download/app-debug.apk
```

### 方式 3: 本地编译 APK

如果 Releases 中没有 APK，可以自行编译：

```bash
# 设置环境变量
export ANDROID_HOME=/opt/android-sdk
export ANDROID_NDK_HOME=/opt/android-sdk/ndk/25.2.9519653

# 编译
cd adventure-game-android
./gradlew assembleDebug

# APK 输出位置
# app/build/outputs/apk/debug/app-debug.apk
```

## 安装 APK

```bash
# 通过 ADB 安装
adb install app-debug.apk

# 或者传输到手机后手动安装
```

## 版本信息

- **版本**: 1.0
- **文件大小**: 5.4 MB
- **支持架构**: ARM64 (arm64-v8a), ARMv7 (armeabi-v7a)
- **最低系统**: Android 5.0 (API 21)

## 游戏特性

- 完整的场景系统
- 背包管理和任务系统
- 预设回复模式（无需模型文件）
- 简洁的文字界面
- 支持基本游戏命令

## 命令列表

| 命令 | 功能 |
|------|------|
| help | 查看帮助 |
| look | 查看当前场景 |
| go [地点] | 移动到指定地点 |
| inventory | 查看背包 |
| quest | 查看任务 |
| exit | 退出游戏 |

---

**祝你玩得愉快！**
