#!/bin/bash
# build_ios.sh - 在 Mac 上直接构建 iOS IPA
# 用法: bash build_ios.sh

set -e

echo "=========================================="
echo "  JY Engine iOS Build Script"
echo "=========================================="

# 检查环境
if [[ "$(uname)" != "Darwin" ]]; then
    echo "错误: 此脚本只能在 macOS 上运行"
    exit 1
fi

command -v cmake >/dev/null 2>&1 || { echo "错误: 请安装 cmake (brew install cmake)"; exit 1; }
command -v xcrun >/dev/null 2>&1 || { echo "错误: 请安装 Xcode"; exit 1; }

# 配置
PREFIX="$(pwd)/build-ios"
SDK=$(xcrun --sdk iphoneos --show-sdk-path)
CC=$(xcrun --sdk iphoneos -f clang)
CFLAGS="-arch arm64 -isysroot $SDK -mios-version-min=13.0 -O2 -fembed-bitcode-marker"
BUILD_DIR="$(pwd)/build-ios"

echo "SDK: $SDK"
echo "CC: $CC"
echo "PREFIX: $PREFIX"

mkdir -p "$PREFIX/lib" "$PREFIX/include" "$BUILD_DIR"

# 1. 编译 Lua
echo ""
echo "=== [1/3] 编译 Lua 5.1 ==="
if [ ! -f "$PREFIX/lib/liblua.a" ]; then
    cd third_party/lua
    $CC -O2 -Wall -c *.c -DLUA_USE_POSIX 2>/dev/null || true
    xcrun ar rcs "$PREFIX/lib/liblua.a" *.o 2>/dev/null || \
    ar rcs "$PREFIX/lib/liblua.a" *.o
    cp lua.h luaconf.h lualib.h lauxlib.h "$PREFIX/include/"
    cd ../..
    echo "Lua 编译完成"
else
    echo "Lua 已编译，跳过"
fi

# 2. 编译 SDL2
echo ""
echo "=== [2/3] 编译 SDL2 ==="
if [ ! -f "$PREFIX/lib/libSDL2.a" ]; then
    cd third_party/SDL2
    mkdir -p build-ios && cd build-ios
    cmake .. \
        -DCMAKE_SYSTEM_NAME=iOS \
        -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
        -DCMAKE_OSX_SYSROOT=iphoneos \
        -DCMAKE_C_FLAGS="-arch arm64 -mios-version-min=13.0 -fembed-bitcode-marker" \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        -DSDL_SHARED=OFF \
        -DSDL_STATIC=ON \
        -DSDL_TEST=OFF \
        -DSDL_RENDER=OFF \
        -DSDL_FILESYSTEM=OFF \
        -DSDL_SENSOR=OFF \
        -DSDL_LOCALE=OFF \
        -DSDL_HIDAPI=OFF \
        -DSDL_POWER=OFF \
        -DSDL_UNIX_CONSOLE_BUILD=ON
    cmake --build . -j$(sysctl -n hw.ncpu)
    cmake --install .
    cd ../../..
    echo "SDL2 编译完成"
else
    echo "SDL2 已编译，跳过"
fi

# 3. 编译引擎
echo ""
echo "=== [3/3] 编译 JY Engine ==="
cd "$BUILD_DIR"

COMMON_FLAGS="-arch arm64 -isysroot $SDK -mios-version-min=13.0 -O2 -fembed-bitcode-marker -I$PREFIX/include -I$(pwd)/../../src -I$(pwd)/../../third_party/lua"

# 编译所有源文件
SRCS=(
    "../../src/jymain.c"
    "../../src/sdlfun.c"
    "../../src/luafun.c"
    "../../src/mainmap.c"
    "../../src/piccache.c"
    "../../src/charset.c"
)

OBJS=""
for src in "${SRCS[@]}"; do
    obj=$(basename "$src" .c).o
    echo "  编译 $src..."
    $CC $COMMON_FLAGS -c "$src" -o "$obj"
    OBJS="$OBJS $obj"
done

# 链接
echo "  链接..."
$CC $COMMON_FLAGS $OBJS \
    -L"$PREFIX/lib" \
    -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf \
    -llua -lm \
    -framework UIKit -framework Foundation -framework CoreGraphics \
    -framework QuartzCore -framework Metal -framework MetalKit \
    -framework OpenGLES -framework AudioToolbox -framework CoreAudio \
    -framework CoreVideo -framework GameKit \
    -o jyengine

echo ""
echo "=== 编译完成 ==="
ls -la jyengine

# 4. 打包 IPA
echo ""
echo "=== 打包 IPA ==="
cd "$BUILD_DIR"
rm -rf Payload
mkdir -p Payload/jyengine.app
cp jyengine Payload/jyengine.app/
cp "$(pwd)/../../ios/Info.plist" Payload/jyengine.app/

# 复制游戏资源
for dir in data script pic sound; do
    [ -d "$(pwd)/../../$dir" ] && cp -r "$(pwd)/../../$dir" Payload/jyengine.app/
done
[ -f "$(pwd)/../../config.lua" ] && cp "$(pwd)/../../config.lua" Payload/jyengine.app/
[ -f "$(pwd)/../../hzmb.dat" ] && cp "$(pwd)/../../hzmb.dat" Payload/jyengine.app/

# 复制字体
mkdir -p Payload/jyengine.app/fonts
cp "$(pwd)/../../font/"*.ttf Payload/jyengine.app/fonts/ 2>/dev/null || true
cp "$(pwd)/../../font/"*.ttc Payload/jyengine.app/fonts/ 2>/dev/null || true

# 打包
zip -r jyengine.ipa Payload/

echo ""
echo "=========================================="
echo "  构建完成!"
echo "  IPA 文件: $BUILD_DIR/jyengine.ipa"
echo "=========================================="
echo ""
echo "安装方法:"
echo "  1. 将 jyengine.ipa 传到 iPhone"
echo "  2. 用巨魔安装"
echo "  3. 用 Filza 将游戏文件放到 Documents/game/"
