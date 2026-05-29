-- config.lua - iOS configuration for JY Engine
-- 苍龙天赋完全版 iOS 移植配置

CONFIG = {};

CONFIG.Debug = 1;

-- iOS screen configuration
-- Landscape mode, optimized for modern iPhones/iPads
CONFIG.Type = 1;
CONFIG.Rotate = 0;

-- Screen resolution
-- Use 1280x720 for good balance of quality and performance
CONFIG.Width = 1280;
CONFIG.Height = 720;

CONFIG.bpp = 32;

CONFIG.FullScreen = 0;  -- Always fullscreen on iOS
CONFIG.EnableSound = 1;

CONFIG.KeyRepeat = 0;
CONFIG.KeyRepeatDelay = 300;
CONFIG.KeyRePeatInterval = 30;

CONFIG.Zoom = 1;
if CONFIG.Zoom == 1 then
    CONFIG.XScale = 36;
    CONFIG.YScale = 18;
else
    CONFIG.XScale = 18;
    CONFIG.YScale = 9;
end

-- Resource paths (relative to app bundle)
-- On iOS, files are in the app's Documents/game directory
CONFIG.CurrentPath = "./";
CONFIG.DataPath = CONFIG.CurrentPath .. "data/";
CONFIG.PicturePath = CONFIG.CurrentPath .. "pic/";
CONFIG.SoundPath = CONFIG.CurrentPath .. "sound/";
CONFIG.ScriptPath = CONFIG.CurrentPath .. "script/";
CONFIG.OldEventPath = CONFIG.ScriptPath .. "oldevent/";
CONFIG.NewEventPath = CONFIG.ScriptPath .. "newevent/";

CONFIG.JYMain_Lua = CONFIG.ScriptPath .. "jymain.lua";

-- Font configuration for iOS
-- Use system font or bundle font
CONFIG.FontName = "./fonts/simsun.ttc";

CONFIG.OSCharSet = 1;

-- Map display offsets
CONFIG.MMapAddX = 2;
CONFIG.MMapAddY = 2;
CONFIG.SMapAddX = 2;
CONFIG.SMapAddY = 16;
CONFIG.WMapAddX = 2;
CONFIG.WMapAddY = 18;

-- Audio settings
CONFIG.MusicVolume = 50;
CONFIG.SoundVolume = 40;

-- Memory settings optimized for iOS
CONFIG.MAXCacheNum = 1000;
CONFIG.CleanMemory = 0;
CONFIG.LoadFullS = 1;
CONFIG.LoadMMapType = 0;
CONFIG.PreLoadPicGrp = 1;
CONFIG.LoadMMapScope = 0;

-- Display refresh mode
CONFIG.FastShowScreen = 1;
