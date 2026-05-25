# APK 下载与安装指南

## 方法 1: 通过 GitHub Actions 自动编译（推荐）

本项目的 GitHub Actions 工作流会在每次 push 时自动编译 APK 并作为 artifact 保存。

**步骤：**

1. 访问仓库的 **Actions** 标签页
2. 点击最近的编译工作流
3. 在底部的 **"Artifacts"** 部分下载 `app-debug.apk`

![下载示意](https://docs.github.com/assets/cb-58869/images/help/repository/actions-artifact.png)

## 方法 2: 直接下载（如已上传）

如果已手动上传 APK：
- 链接：[下载 APK](https://github.com/Hermitxzy/ai-game-text/releases/latest/download/app-debug.apk)

## 方法 3: 本地编译

### Windows

```bash
# 需要 Java、Android SDK、Android NDK

# 设置环境变量
set ANDROID_HOME=C:\Users\YourName\AppData\Local\Android\Sdk
set ANDROID_NDK_HOME=%ANDROID_HOME%\ndk\25.2.9519653

# 编译
cd adventure-game-android
gradlew assembleDebug

# APK 位置：app\build\outputs\apk\debug\app-debug.apk
```

### macOS / Linux

```bash
export ANDROID_HOME=$HOME/Android/Sdk
export ANDROID_NDK_HOME=$ANDROID_HOME/ndk/25.2.9519653

cd adventure-game-android
./gradlew assembleDebug

# APK 位置：app/build/outputs/apk/debug/app-debug.apk
```

## 安装到手机

### 通过 ADB（推荐）

```bash
adb install app-debug.apk
```

### 传输到手机

```bash
adb push app-debug.apk /sdcard/Download/

# 然后在手机文件管理器中找到并安装
```

## 系统要求

| 项目 | 要求 |
|------|------|
| Android 版本 | 5.0 (API 21) 或更高 |
| CPU 架构 | ARM64 (优先) 或 ARMv7 |
| 存储空间 | 至少 20MB |
| RAM | 建议 2GB 以上 |

## 故障排除

### 安装失败

```bash
# 允许未知来源
设置 -> 安全 -> 未知来源应用 -> 允许

# 通过 ADB 安装
adb install -r app-debug.apk
```

### 闪退

```bash
adb logcat | grep AdventureGame

# 查看错误日志
```

---

**仓库**: https://github.com/Hermitxzy/ai-game-text

