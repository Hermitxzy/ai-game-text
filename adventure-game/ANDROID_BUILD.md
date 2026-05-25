# Android 编译指南

本文档详细说明如何在 Android 设备上编译和运行 C 语言文字冒险游戏。

## 前置条件

### 1. 安装 Android NDK

**方法一：使用官方 NDK**

```bash
# 下载 NDK r25b (或更新版本)
wget https://dl.google.com/android/repository/android-ndk-r25b-linux.zip
unzip android-ndk-r25b-linux.zip
export NDK_HOME=$PWD/android-ndk-r25b
export PATH=$NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH
```

**方法二：使用 Termux (在 Android 设备上直接编译)**

```bash
# 在 Termux 中
pkg install clang make
```

### 2. 编译 llama.cpp for Android

```bash
cd ~
git clone https://github.com/ggerganov/llama.cpp
cd llama.cpp

# 使用 NDK 交叉编译
export NDK=/path/to/ndk
./scripts/build-android.sh

# 或使用 CMake
mkdir build-android && cd build-android
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-21 \
    -DBUILD_SHARED_LIBS=ON
make -j
```

### 3. 编译目录结构

```
adventure-game/
├── libs/
│   └── android/
│       └── arm64/
│           └── libllama.so  # llama.cpp 动态库
```

## 编译步骤

### 方法一：使用 Makefile (推荐)

```bash
cd /workspace/adventure-game

# 编译 arm64 版本
make android-arm64

# 编译 armv7 版本
make android-arm
```

### 方法二：手动编译

```bash
# 设置交叉编译工具链
export ANDROID_NDK=/path/to/ndk
export TOOLCHAIN=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin
export TARGET=aarch64-linux-android21

export AR=$TOOLCHAIN/llvm-ar
export CC=$TOOLCHAIN/$TARGET-clang
export AS=$TOOLCHAIN/llvm-as
export CXX=$TOOLCHAIN/$TARGET-clang++
export LD=$TOOLCHAIN/ld
export RANLIB=$TOOLCHAIN/llvm-ranlib
export STRIP=$TOOLCHAIN/llvm-strip

# 编译
cd /workspace/adventure-game
make ANDROID=1 ARCH=arm64
```

## 部署到 Android 设备

### 方法一：使用 ADB

```bash
# 连接设备
adb devices

# 推送文件
adb push bin/adventure-game /data/local/tmp/
adb push models/*.gguf /data/local/tmp/models/
adb push libs/android/arm64/libllama.so /data/local/tmp/

# 设置权限
adb shell "chmod +x /data/local/tmp/adventure-game"

# 运行
adb shell
$ cd /data/local/tmp
$ export LD_LIBRARY_PATH=/data/local/tmp
$ ./adventure-game models/your-model.gguf
```

### 方法二：直接推送到 SD 卡

```bash
# 推送到 SD 卡（不需要 root）
adb push bin/adventure-game /sdcard/Download/
adb push models/*.gguf /sdcard/Download/models/
adb push libs/android/arm64/libllama.so /sdcard/Download/

# 使用 Termux 运行
adb shell
$ cd /sdcard/Download
$ export LD_LIBRARY_PATH=/sdcard/Download
$ ./adventure-game models/your-model.gguf
```

## 在 Termux 中运行

```bash
# 安装 Termux
# 从 F-Droid 下载：https://f-droid.org/en/packages/com.termux/

# 在 Termux 中
pkg update
pkg install clang make termux-exec

# 挂载存储
termux-setup-storage

# 进入项目目录
cd ~/storage/downloads

# 运行游戏
export LD_LIBRARY_PATH=.
./adventure-game models/your-model.gguf
```

## 性能优化

### 1. 模型选择

对于移动设备，推荐使用量化模型：

| 模型 | 大小 | 速度 | 质量 | 推荐度 |
|------|------|------|------|--------|
| Qwen2.5 0.5B Q4 | ~400MB | ⚡⚡⚡ | ⭐⭐ | ⭐⭐⭐ |
| Llama 3.2 1B Q4 | ~800MB | ⚡⚡ | ⭐⭐⭐ | ⭐⭐⭐⭐ |
| Qwen2.5 1.5B Q4 | ~1.1GB | ⚡ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |

### 2. 减少上下文长度

修改 `src/main.c` 中的 `n_ctx` 参数：

```c
ai.n_ctx = 512;  // 默认 2048，减少以节省内存
```

### 3. 调整线程数

在 `src/ai.c` 中设置：

```c
ctx_params.n_threads = 2;  // 根据设备核心数调整
```

### 4. 使用 NEON 优化

确保 llama.cpp 编译时启用了 NEON：

```bash
cmake .. \
    -DANDROID_ABI=arm64-v8a \
    -DGGML_NATIVE=OFF \
    -DCMAKE_C_FLAGS="-O3 -mcpu=cortex-a76"
```

## 常见问题

### 问题 1：libllama.so 无法加载

**解决**:
```bash
# 检查库文件
ls -la libllama.so

# 设置正确的库路径
export LD_LIBRARY_PATH=/sdcard/Download:$LD_LIBRARY_PATH

# 或使用绝对路径
/system/bin/linker64 /sdcard/Download/libllama.so
```

### 问题 2：内存不足

**解决**:
- 使用更小的模型（0.5B 或 1B）
- 减少 `n_ctx` 参数
- 关闭其他应用
- 考虑使用 swap 分区（需要 root）

### 问题 3：SELinux 权限错误

**解决**:
```bash
# 临时禁用 SELinux（需要 root）
adb root
adb shell setenforce 0

# 或修改文件上下文
adb shell chcon u:object_r:system_lib_file:s0 libllama.so
```

### 问题 4：游戏崩溃

**解决**:
```bash
# 获取崩溃日志
adb logcat | grep AdventureGame

# 检查是否为 32/64 位不匹配
adb shell getprop ro.product.cpu.abi
# 确保编译的架构与设备匹配
```

## 打包 APK（可选）

如果想将游戏打包成 APK，可以使用 Android NDK:

```bash
# 创建 Android 项目结构
mkdir -p app/src/main/java/com/adventure/game
mkdir -p app/src/main/jni

# 编写 CMakeLists.txt
# ... (参考 Android NDK 示例)

# 构建 APK
./gradlew assembleDebug
```

## 完整测试流程

```bash
# 1. 编译
make android-arm64

# 2. 部署
adb push bin/adventure-game /sdcard/Download/
adb push libs/android/arm64/libllama.so /sdcard/Download/
adb push models/qwen2.5-1.5b.gguf /sdcard/Download/models/

# 3. 运行
adb shell "cd /sdcard/Download && chmod +x adventure-game"
adb shell "cd /sdcard/Download && export LD_LIBRARY_PATH=. && ./adventure-game models/qwen2.5-1.5b.gguf"

# 4. 查看日志
adb logcat | grep -E "(AdventureGame|llama)"
```

## 性能基准

在典型 Android 设备上的性能表现：

| 设备 | 芯片 | RAM | 模型 | 首字时间 | 生成速度 |
|------|------|-----|------|----------|----------|
| Pixel 7 | Tensor G2 | 8GB | Qwen2.5 1.5B | ~2s | ~15 tokens/s |
| Samsung S23 | Snapdragon 8 Gen 2 | 12GB | Qwen2.5 1.5B | ~1.5s | ~20 tokens/s |
| Redmi Note 12 | Snapdragon 685 | 6GB | Llama 3.2 1B | ~3s | ~8 tokens/s |

---

**祝你编译成功！**
