# 保留 JNI 方法
-keepclasseswithmembernames class * {
    native <methods>;
}

# 保留 Activity
-keep class com.adventure.game.** { *; }
