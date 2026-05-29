#!/bin/bash
# build_local.sh - Local build script for iOS (requires Mac)
set -e

echo "=== JY Engine iOS Build ==="

# Check if running on macOS
if [[ "$(uname)" != "Darwin" ]]; then
    echo "Error: This script must be run on macOS"
    exit 1
fi

# Check for required tools
command -v cmake >/dev/null 2>&1 || { echo "Error: cmake not found. Install with: brew install cmake"; exit 1; }
command -v xcodebuild >/dev/null 2>&1 || { echo "Error: Xcode not found"; exit 1; }

# Install dependencies if needed
echo "Checking dependencies..."
brew list sdl2 &>/dev/null || brew install sdl2
brew list sdl2_image &>/dev/null || brew install sdl2_image
brew list sdl2_mixer &>/dev/null || brew install sdl2_mixer
brew list sdl2_ttf &>/dev/null || brew install sdl2_ttf

# Download Lua source
echo "Setting up Lua..."
mkdir -p third_party/lua
cd third_party/lua
if [ ! -f "lua.h" ]; then
    echo "Downloading Lua 5.1.5..."
    curl -L https://www.lua.org/ftp/lua-5.1.5.tar.gz | tar xz --strip-components=1
fi
cd ../..

# Configure
echo "Configuring CMake..."
cmake -B build \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
    -DCMAKE_OSX_SYSROOT=iphoneos \
    -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY="-" \
    -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED=NO \
    -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO

# Build
echo "Building..."
cmake --build build --config Release

# Create IPA
echo "Creating IPA..."
mkdir -p Payload
cp -r build/ios/jyengine.app Payload/

# Copy game resources if they exist
if [ -d "game" ]; then
    echo "Including game resources..."
    cp -r game/data Payload/jyengine.app/ 2>/dev/null || true
    cp -r game/script Payload/jyengine.app/ 2>/dev/null || true
    cp -r game/pic Payload/jyengine.app/ 2>/dev/null || true
    cp -r game/sound Payload/jyengine.app/ 2>/dev/null || true
    cp game/config.lua Payload/jyengine.app/ 2>/dev/null || true
    cp game/hzmb.dat Payload/jyengine.app/ 2>/dev/null || true
fi

# Package
zip -r jyengine.ipa Payload/
rm -rf Payload

echo ""
echo "=== Build Complete ==="
echo "IPA file: jyengine.ipa"
echo ""
echo "Install with TrollStore:"
echo "1. Transfer jyengine.ipa to your device"
echo "2. Open with TrollStore"
echo "3. Place game files in Documents/game/"
