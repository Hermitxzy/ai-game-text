#!/bin/bash
# 一键构建 APK 脚本

set -e

echo "╔═══════════════════════════════════════════════╗"
echo "║     Android APK 一键构建脚本                    ║"
echo "╚═══════════════════════════════════════════════╝"
echo ""

# 颜色
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查 ANDROID_HOME
check_android_sdk() {
    if [ -z "$ANDROID_HOME" ]; then
        log_error "未设置 ANDROID_HOME"
        echo ""
        echo "请安装 Android Studio 或命令行工具："
        echo ""
        echo "方法 1: 使用 ASDF (推荐)"
        echo "  asdf install android 1.0.0"
        echo "  asdf global android 1.0.0"
        echo ""
        echo "方法 2: 手动设置"
        echo "  export ANDROID_HOME=\$HOME/Android/Sdk"
        echo ""
        echo "方法 3: 使用包管理器"
        echo "  apt install android-sdk  # Debian/Ubuntu"
        echo ""
        exit 1
    fi
    
    if [ ! -d "$ANDROID_HOME" ]; then
        log_error "ANDROID_HOME 路径不存在：$ANDROID_HOME"
        exit 1
    fi
    
    log_info "使用 Android SDK: $ANDROID_HOME"
}

# 检查 NDK
check_ndk() {
    if [ -z "$ANDROID_NDK_HOME" ]; then
        if [ -d "$ANDROID_HOME/ndk" ]; then
            export ANDROID_NDK_HOME=$(ls -d $ANDROID_HOME/ndk/* 2>/dev/null | head -1)
        fi
    fi
    
    if [ -z "$ANDROID_NDK_HOME" ] || [ ! -d "$ANDROID_NDK_HOME" ]; then
        log_error "未找到 NDK"
        echo ""
        echo "请在 Android Studio 中安装 NDK："
        echo "  Tools -> SDK Manager -> SDK Tools -> NDK"
        echo ""
        echo "或手动下载："
        echo "  https://developer.android.com/ndk/downloads"
        echo ""
        exit 1
    fi
    
    log_info "使用 NDK: $ANDROID_NDK_HOME"
}

# 接受 SDK 协议
accept_sdk_license() {
    if [ -d "$ANDROID_HOME/licenses" ]; then
        echo -e "\ny" | $ANDROID_HOME/tools/bin/sdkmanager --licenses > /dev/null 2>&1 || true
    fi
}

# 构建 APK
build_apk() {
    BUILD_TYPE="${1:-debug}"
    
    log_info "构建 $BUILD_TYPE APK..."
    
    # 设置 NDK
    export ANDROID_NDK_HOME
    
    # 赋予 gradlew 执行权限
    chmod +x gradlew
    
    # 执行 Gradle 构建
    ./gradlew assemble${BUILD_TYPE^}
    
    # 查找生成的 APK
    APK_PATH=$(find "app/build/outputs/apk/$BUILD_TYPE" -name "*.apk" 2>/dev/null | head -1)
    
    if [ -n "$APK_PATH" ]; then
        log_info "APK 生成成功！"
        echo ""
        echo "输出文件：$APK_PATH"
        echo ""
        
        # 显示文件大小
        ls -lh "$APK_PATH" | awk '{print "文件大小："$5}'
        
        # 如果有 adb，询问是否安装
        if command -v adb &> /dev/null && adb devices | grep -q "device$"; then
            echo ""
            read -p "是否安装到连接的設備？(y/n) " -n 1 -r
            echo
            if [[ $REPLY =~ ^[Yy]$ ]]; then
                adb install -r "$APK_PATH"
                log_info "安装完成！"
            fi
        fi
    else
        log_error "未找到 APK 文件"
        exit 1
    fi
}

# 清理
clean_build() {
    log_info "清理构建文件..."
    chmod +x gradlew
    ./gradlew clean
    rm -rf app/build app/.externalNativeBuild
    log_info "清理完成"
}

# 显示帮助
show_help() {
    echo "用法：$0 [命令]"
    echo ""
    echo "命令:"
    echo "  debug       - 构建 Debug APK (默认)"
    echo "  release     - 构建Release APK"
    echo "  clean       - 清理构建文件"
    echo "  help        - 显示帮助"
    echo ""
    echo "环境变量:"
    echo "  ANDROID_HOME     - Android SDK 路径"
    echo "  ANDROID_NDK_HOME - Android NDK 路径"
    echo ""
}

# 主函数
main() {
    case "${1:-debug}" in
        debug|release)
            check_android_sdk
            check_ndk
            accept_sdk_license
            build_apk "$1"
            ;;
        clean)
            clean_build
            ;;
        help|--help|-h)
            show_help
            ;;
        *)
            log_error "未知命令：$1"
            show_help
            exit 1
            ;;
    esac
}

# 运行
main "$@"
