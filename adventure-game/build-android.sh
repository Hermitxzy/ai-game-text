#!/bin/bash
# Android 一键编译脚本
# 自动检测环境、下载 NDK、编译项目

set -e

echo "╔═══════════════════════════════════════════════╗"
echo "║     C 语言文字冒险游戏 - Android 编译脚本      ║"
echo "╚═══════════════════════════════════════════════╝"
echo ""

# 配置
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
NDK_VERSION="25.2.9519653"
BUILD_DIR="$SCRIPT_DIR/build-android"
OUTPUT_DIR="$SCRIPT_DIR/bin/android"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检测系统
detect_os() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        OS="mac"
    elif [[ "$OSTYPE" == "linux"* ]]; then
        OS="linux"
    else
        log_error "不支持的操作系统：$OSTYPE"
        exit 1
    fi
    log_info "检测到操作系统：$OS"
}

# 检测 Android NDK
check_ndk() {
    if command -v ndk-build &> /dev/null; then
        NDK_HOME="$(dirname "$(dirname "$(which ndk-build)")")"
        log_info "检测到 NDK: $NDK_HOME"
        return 0
    fi
    
    if [ -n "$ANDROID_NDK_HOME" ] && [ -d "$ANDROID_NDK_HOME" ]; then
        NDK_HOME="$ANDROID_NDK_HOME"
        log_info "检测到 NDK: $NDK_HOME"
        return 0
    fi
    
    if [ -n "$ANDROID_HOME" ] && [ -d "$ANDROID_HOME/ndk" ]; then
        NDK_HOME="$ANDROID_HOME/ndk/$NDK_VERSION"
        if [ -d "$NDK_HOME" ]; then
            log_info "检测到 NDK: $NDK_HOME"
            return 0
        fi
    fi
    
    # 检查脚本目录
    if [ -d "$SCRIPT_DIR/ndk" ]; then
        NDK_HOME="$SCRIPT_DIR/ndk"
        log_info "检测到本地 NDK: $NDK_HOME"
        return 0
    fi
    
    log_warn "未检测到 Android NDK"
    return 1
}

# 下载 Android NDK
download_ndk() {
    log_info "正在下载 Android NDK..."
    
    # 下载链接
    if [[ "$OS" == "linux" ]]; then
        NDK_URL="https://dl.google.com/android/repository/android-ndk-r25b-linux.zip"
        NDK_ZIP="android-ndk-r25b-linux.zip"
    elif [[ "$OS" == "mac" ]]; then
        NDK_URL="https://dl.google.com/android/repository/android-ndk-r25b-darwin.zip"
        NDK_ZIP="android-ndk-r25b-darwin.zip"
    fi
    
    # 使用 aria2c 或 wget 或 curl 下载
    if command -v aria2c &> /dev/null; then
        aria2c -x 4 -s 4 -k 1M "$NDK_URL" -o "$NDK_ZIP"
    elif command -v wget &> /dev/null; then
        wget --show-progress -c "$NDK_URL" -O "$NDK_ZIP"
    elif command -v curl &> /dev/null; then
        curl -L -o "$NDK_ZIP" "$NDK_URL"
    else
        log_error "请安装 wget 或 curl"
        exit 1
    fi
    
    # 解压
    log_info "正在解压 NDK..."
    unzip -q "$NDK_ZIP"
    mv "android-ndk-r25b" "$SCRIPT_DIR/ndk"
    rm "$NDK_ZIP"
    
    NDK_HOME="$SCRIPT_DIR/ndk"
    log_info "NDK 下载完成：$NDK_HOME"
}

# 编译 ARM64
build_arm64() {
    log_info "开始编译 ARM64 版本..."
    
    export PATH="$NDK_HOME/toolchains/llvm/prebuilt/$OS-x86_64/bin:$PATH"
    export TARGET=aarch64-linux-android21
    export CC=$TARGET-clang
    
    mkdir -p "$BUILD_DIR/arm64"
    mkdir -p "$OUTPUT_DIR/arm64"
    
    # 编译
    make clean ANDROID=1 ARCH=arm64 || make clean
    make ANDROID=1 ARCH=arm64
    
    # 复制文件
    cp "$SCRIPT_DIR/bin/adventure-game" "$OUTPUT_DIR/arm64/"
    log_info "ARM64 编译完成：$OUTPUT_DIR/arm64/adventure-game"
}

# 编译 ARMv7
build_armv7() {
    log_info "开始编译 ARMv7 版本..."
    
    export PATH="$NDK_HOME/toolchains/llvm/prebuilt/$OS-x86_64/bin:$PATH"
    export TARGET=armv7a-linux-androideabi21
    export CC=$TARGET-clang
    
    mkdir -p "$BUILD_DIR/armv7"
    mkdir -p "$OUTPUT_DIR/armv7"
    
    # 编译
    make clean ANDROID=1 ARCH=arm
    make ANDROID=1 ARCH=arm
    
    # 复制文件
    cp "$SCRIPT_DIR/bin/adventure-game" "$OUTPUT_DIR/armv7/"
    log_info "ARMv7 编译完成：$OUTPUT_DIR/armv7/adventure-game"
}

# 部署到设备
deploy_to_device() {
    if ! command -v adb &> /dev/null; then
        log_warn "未检测到 adb，跳过度署到设备"
        return
    fi
    
    if ! adb devices | grep -q "device$"; then
        log_warn "未检测到 Android 设备，跳过度署"
        return
    fi
    
    log_info "部署到 Android 设备..."
    
    # 选择架构
    ARCH="arm64"
    if [ "$1" == "armv7" ]; then
        ARCH="armv7"
    fi
    
    # 推送文件
    adb push "$OUTPUT_DIR/$ARCH/adventure-game" /sdcard/Download/
    adb push "$SCRIPT_DIR/download-model.sh" /sdcard/Download/
    adb shell "chmod +x /sdcard/Download/adventure-game"
    
    log_info "部署完成！"
    echo ""
    echo "在设备上运行："
    echo "  cd /sdcard/Download"
    echo "  export LD_LIBRARY_PATH=/sdcard/Download"
    echo "  ./adventure-game"
}

# 显示帮助
show_help() {
    echo "用法：$0 [选项]"
    echo ""
    echo "选项:"
    echo "  all       编译 ARM64 和 ARMv7 (默认)"
    echo "  arm64     只编译 ARM64"
    echo "  armv7     只编译 ARMv7"
    echo "  deploy    编译并部署到设备"
    echo "  clean     清理编译文件"
    echo "  help      显示帮助"
    echo ""
}

# 主函数
main() {
    detect_os
    
    # 检查或下载 NDK
    if ! check_ndk; then
        echo ""
        echo "需要下载 Android NDK (约 1GB)"
        echo "选项:"
        echo "  1) 自动下载 NDK (推荐)"
        echo "  2) 使用已有 NDK (请输入路径)"
        echo "  3) 退出"
        echo ""
        
        read -p "请选择 [1-3]: " choice
        
        case $choice in
            1)
                download_ndk
                ;;
            2)
                read -p "请输入 NDK 路径：" user_path
                if [ -d "$user_path" ]; then
                    NDK_HOME="$user_path"
                else
                    log_error "路径不存在：$user_path"
                    exit 1
                fi
                ;;
            *)
                exit 0
                ;;
        esac
    fi
    
    # 编译
    ACTION="${1:-all}"
    
    case $ACTION in
        all)
            build_arm64
            build_armv7
            ;;
        arm64)
            build_arm64
            ;;
        armv7)
            build_armv7
            ;;
        deploy)
            build_arm64
            deploy_to_device
            ;;
        clean)
            rm -rf "$BUILD_DIR"
            rm -rf "$OUTPUT_DIR"
            make clean
            log_info "清理完成"
            ;;
        help|--help|-h)
            show_help
            ;;
        *)
            show_help
            exit 1
            ;;
    esac
    
    echo ""
    log_info "完成！输出目录：$OUTPUT_DIR"
}

# 运行
main "$@"
