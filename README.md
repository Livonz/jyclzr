# JY Engine iOS Port

金庸群侠传 (苍龙天赋完全版) iOS 移植项目

## 项目说明

将经典的金庸群侠传 Lua+SDL 引擎移植到 iOS 平台，通过 TrollStore (巨魔系统) 安装。

### 技术架构

- **引擎核心**: C + Lua 5.1 + SDL 2.x
- **目标平台**: iOS 13.0+ (arm64)
- **构建系统**: CMake + GitHub Actions CI/CD
- **安装方式**: TrollStore (.tipa / .ipa)

## 快速开始

### 方式一：GitHub Actions 自动构建 (推荐)

1. Fork 本仓库
2. 将你的游戏文件放到对应目录：
   - `game/data/` - 游戏数据
   - `game/script/` - Lua 脚本
   - `game/pic/` - 贴图资源
   - `game/sound/` - 音频文件
   - `config.lua` - 配置文件
   - `hzmb.dat` - 字符转换表
3. 推送代码到 GitHub
4. 在 Actions 页面触发构建
5. 下载生成的 IPA 文件
6. 通过 TrollStore 安装

### 方式二：本地构建 (需要 Mac)

```bash
# 安装依赖
brew install cmake sdl2 sdl2_image sdl2_mixer sdl2_ttf lua

# 下载 Lua 源码
mkdir -p third_party/lua
cd third_party/lua
curl -L https://www.lua.org/ftp/lua-5.1.5.tar.gz | tar xz --strip-components=1
cd ../..

# 配置构建
cmake -B build \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
  -DCMAKE_OSX_SYSROOT=iphoneos \
  -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY="-"

# 编译
cmake --build build --config Release

# 打包 IPA
mkdir -p Payload
cp -r build/ios/jyengine.app Payload/
# 复制游戏资源到 Payload/jyengine.app/
zip -r jyengine.ipa Payload/
```

## 安装到设备

### 前提条件
- iOS 设备已安装 TrollStore (巨魔系统)
- 设备架构: arm64

### 安装步骤

1. 下载构建好的 `jyengine.ipa`
2. 通过以下方式之一安装：
   - 直接在设备上用 TrollStore 打开 IPA
   - 通过 AltServer / Sideloadly 安装
   - 使用 `tailinstall` 命令行工具

3. 首次启动时，需要将游戏文件放到 App 的 Documents 目录：
   - 使用 Filza 文件管理器
   - 路径: `var/mobile/Containers/Data/Application/[APP-ID]/Documents/game/`

### 文件结构

```
Documents/
└── game/
    ├── data/          # 游戏数据文件
    │   ├── *.grp      # 贴图组文件
    │   ├── *.idx      # 索引文件
    │   ├── *.002      # 地图数据
    │   └── *.wav      # 音效文件
    ├── script/        # Lua 脚本
    │   ├── jymain.lua # 主入口
    │   ├── *.lua      # 其他脚本
    │   └── ...
    ├── pic/           # 贴图资源
    ├── sound/         # 音乐文件
    ├── config.lua     # 配置文件
    └── hzmb.dat       # 字符转换表
```

## 触摸操作

- **方向控制**: 点击屏幕四角或滑动
- **确认/交互**: 点击屏幕中央
- **菜单/返回**: 点击左上角

## 配置说明

编辑 `config.lua` 可调整：

```lua
CONFIG.Width = 1280;    -- 屏幕宽度
CONFIG.Height = 720;    -- 屏幕高度
CONFIG.EnableSound = 1; -- 音效开关
CONFIG.MusicVolume = 50; -- 音乐音量
CONFIG.SoundVolume = 40; -- 音效音量
```

## 已知限制

1. **MPEG 播放**: iOS 不支持 MPEG1 视频播放
2. **字体**: 需要包含中文字体文件
3. **存档**: 存档文件保存在 App 沙盒内
4. **内存**: iOS 内存限制可能导致大地图加载较慢

## 开发说明

### 项目结构

```
jy-ios-port/
├── src/                    # 引擎 C 源码
│   ├── jymain.c           # 主入口
│   ├── jymain.h           # 主头文件
│   ├── sdlfun.c           # SDL 函数
│   ├── luafun.c           # Lua 绑定
│   ├── mainmap.c          # 地图渲染
│   ├── piccache.c         # 贴图缓存
│   └── charset.c          # 字符集转换
├── ios/                    # iOS 特定代码
│   ├── jyengine/          # 应用源码
│   ├── Info.plist         # 应用配置
│   └── LaunchScreen.storyboard
├── .github/workflows/      # CI/CD 配置
├── CMakeLists.txt          # 构建配置
└── config.lua              # 游戏配置
```

### 移植说明

原始引擎使用 SDL 1.x，本项目通过兼容层适配到 SDL 2.x：
- `SDL_SetVideoMode` → `SDL_CreateWindow` + `SDL_CreateRenderer`
- `SDL_Surface` 渲染 → `SDL_Texture` 纹理渲染
- 键盘输入 → 触摸输入映射

## 致谢

- [jinyong-legend](https://github.com/ZhanruiLiang/jinyong-legend) - 原始开源引擎
- SDL2 - 跨平台多媒体库
- Lua - 脚本语言
- TrollStore - iOS 侧载工具

## 许可证

本项目基于原始引擎的许可协议（无版权限制）。
