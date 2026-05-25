# Android 一键编译指南

## 快速开始（三选一）

### 方案 A：使用一键脚本（最简单 ⭐）

```bash
cd /workspace/adventure-game

# 一键编译（自动下载 NDK + 编译）
./build-android.sh

# 选择架构编译
./build-android.sh arm64    # 只编译 ARM64
./build-android.sh armv7    # 只编译 ARMv7
./build-android.sh all      # 编译所有架构

# 编译并部署到设备
./build-android.sh deploy
```

### 方案 B：使用 Makefile（需要 NDK）

```bash
# 设置 NDK 路径
export ANDROID_NDK_HOME=/path/to/ndk

# 编译 ARM64
make android-arm64

# 编译 ARMv7
make android-armv7
```

### 方案 C：Docker 编译（无需安装 NDK）

```bash
# 使用 Docker 容器编译
docker run --rm -v $(pwd):/app -w /app android-ndk \
    /bin/bash -c "./build-android.sh all"
```

## 输出文件

编译完成后，文件位于：

```
bin/android/
├── arm64/
│   └── adventure-game    # ARM64 版本
└── armv7/
    └── adventure-game    # ARMv7 版本
```

## 部署到手机

### 方法 1：使用脚本自动部署

```bash
./build-android.sh deploy
```

### 方法 2：手动部署

```bash
# 连接手机并确认 adb 可用
adb devices

# 推送文件
adb push bin/android/arm64/adventure-game /sdcard/Download/
adb push download-model.sh /sdcard/Download/

# 设置权限
adb shell "chmod +x /sdcard/Download/adventure-game"

# 在手机上运行
adb shell
$ cd /sdcard/Download
$ ./adventure-game
```

### 方法 3：使用 Termux（无需电脑）

```bash
# 在 Termux 中
pkg install clang make
cd /sdcard/Download
make
./adventure-game
```

## 常见问题

### 1. 缺少 NDK

**解决**：运行 `./build-android.sh` 会自动下载

或手动下载：
```bash
wget https://dl.google.com/android/repository/android-ndk-r25b-linux.zip
unzip android-ndk-r25b-linux.zip
export ANDROID_NDK_HOME=$PWD/android-ndk-r25b
```

### 2. 缺少 adb

**Linux**:
```bash
sudo apt install adb
```

**macOS**:
```bash
brew install android-platform-tools
```

### 3. 部署后无法运行

```bash
# 检查权限
adb shell "chmod +x /sdcard/Download/adventure-game"

# 检查架构是否匹配
adb shell "getprop ro.product.cpu.abi"
# arm64-v8a -> 使用 arm64 版本
# armeabi-v7a -> 使用 armv7 版本
```

### 4. 缺少 libllama.so

游戏可以在没有 llama.cpp 的情况下运行（预设回复模式）。

如需 AI 对话功能：
```bash
# 编译 llama.cpp for Android
git clone https://github.com/ggerganov/llama.cpp
cd llama.cpp
mkdir build && cd build
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$NDK_HOME/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-21 \
    -DBUILD_SHARED_LIBS=ON
make -j

# 复制库文件
cp libllama.so /sdcard/Download/
```

## 性能优化

### 编译优化

```bash
# 使用最高优化级别
make android-arm64 CFLAGS="-O3 -march=armv8-a+crypto"
```

### 运行时优化

在手机上创建启动脚本：
```bash
cat > /sdcard/Download/run.sh << 'EOF'
#!/system/bin/sh
cd /sdcard/Download
export LD_LIBRARY_PATH=/sdcard/Download

# 设置性能模式
echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

# 运行游戏
./adventure-game models/qwen2.5-1.5b.gguf
EOF
chmod +x /sdcard/Download/run.sh
```

## 完整流程示例

```bash
# 1. 编译
./build-android.sh arm64

# 2. 下载模型（可选）
./download-model.sh

# 3. 部署
./build-android.sh deploy

# 4. 在手机上运行
# （在 adb shell 或 Termux 中）
cd /sdcard/Download
export LD_LIBRARY_PATH=/sdcard/Download
./adventure-game models/qwen2.5-1.5b.gguf
```

---

**编译时间参考**：
- ARM64: ~30 秒
- ARMv7: ~25 秒
- 一键脚本（含 NDK 下载）: ~5 分钟（取决于网速）
