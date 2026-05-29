// jymain.c - Main entry point for iOS port
// Adapted from jinyong-legend for iOS/SDL2

#include <stdio.h>
#include <time.h>

#include "jymain.h"

// Global variables
SDL_Surface *g_Surface = NULL;
Uint32 g_MaskColor32 = 0x706020;
int g_Rotate = 0;
int g_ScreenW = 800;
int g_ScreenH = 600;
int g_ScreenBpp = 32;
int g_FullScreen = 0;
int g_EnableSound = 1;
int g_MusicVolume = 32;
int g_SoundVolume = 32;
int g_XScale = 18;
int g_YScale = 9;
int g_MMapAddX;
int g_MMapAddY;
int g_SMapAddX;
int g_SMapAddY;
int g_WMapAddX;
int g_WMapAddY;
int g_MAXCacheNum = 1000;
int g_LoadFullS = 1;
int g_LoadMMapType = 0;
int g_LoadMMapScope = 0;
int g_PreLoadPicGrp = 1;

static int IsDebug = 0;
static char JYMain_Lua[255];

// Lua library function definitions
static const struct luaL_reg jylib[] = {
    {"Debug", HAPI_Debug},
    {"GetKey", HAPI_GetKey},
    {"EnableKeyRepeat", HAPI_EnableKeyRepeat},
    {"Delay", HAPI_Delay},
    {"GetTime", HAPI_GetTime},
    {"CharSet", HAPI_CharSet},
    {"DrawStr", HAPI_DrawStr},
    {"SetClip", HAPI_SetClip},
    {"FillColor", HAPI_FillColor},
    {"Background", HAPI_Background},
    {"DrawRect", HAPI_DrawRect},
    {"ShowSurface", HAPI_ShowSurface},
    {"ShowSlow", HAPI_ShowSlow},
    {"PicInit", HAPI_PicInit},
    {"PicGetXY", HAPI_GetPicXY},
    {"PicLoadCache", HAPI_LoadPic},
    {"PicLoadFile", HAPI_PicLoadFile},
    {"FullScreen", HAPI_FullScreen},
    {"LoadPicture", HAPI_LoadPicture},
    {"PlayMIDI", HAPI_PlayMIDI},
    {"PlayWAV", HAPI_PlayWAV},
    {"PlayMPEG", HAPI_PlayMPEG},
    {"LoadMMap", HAPI_LoadMMap},
    {"DrawMMap", HAPI_DrawMMap},
    {"GetMMap", HAPI_GetMMap},
    {"UnloadMMap", HAPI_UnloadMMap},
    {"LoadSMap", HAPI_LoadSMap},
    {"SaveSMap", HAPI_SaveSMap},
    {"GetS", HAPI_GetS},
    {"SetS", HAPI_SetS},
    {"GetD", HAPI_GetD},
    {"SetD", HAPI_SetD},
    {"DrawSMap", HAPI_DrawSMap},
    {"LoadWarMap", HAPI_LoadWarMap},
    {"GetWarMap", HAPI_GetWarMap},
    {"SetWarMap", HAPI_SetWarMap},
    {"CleanWarMap", HAPI_CleanWarMap},
    {"DrawWarMap", HAPI_DrawWarMap},
    {NULL, NULL}
};

static const struct luaL_reg bytelib[] = {
    {"create", Byte_create},
    {"loadfile", Byte_loadfile},
    {"savefile", Byte_savefile},
    {"get16", Byte_get16},
    {"set16", Byte_set16},
    {"getu16", Byte_getu16},
    {"setu16", Byte_setu16},
    {"get32", Byte_get32},
    {"set32", Byte_set32},
    {"getstr", Byte_getstr},
    {"setstr", Byte_setstr},
    {NULL, NULL}
};

// Get resource path for iOS bundle
void GetResourcePath(const char *filename, char *outpath, int maxlen) {
#ifdef IOS_BUILD
    CFBundleRef bundle = CFBundleGetMainBundle();
    CFURLRef url = CFBundleCopyResourcesDirectoryURL(bundle);
    char resPath[1024];
    CFURLGetFileSystemRepresentation(url, true, (UInt8*)resPath, sizeof(resPath));
    CFRelease(url);
    snprintf(outpath, maxlen, "%s/%s", resPath, filename);
#else
    snprintf(outpath, maxlen, "%s", filename);
#endif
}

// Main function
int jy_main(int argc, char *argv[]) {
    lua_State *pL_main;

    remove(DEBUG_FILE);
    freopen(ERROR_FILE, "wt", stderr);

    // Initialize Lua
    pL_main = luaL_newstate();
    luaL_openlibs(pL_main);

    Lua_Config(pL_main, CONFIG_FILE);

    InitSDL();
    InitGame();
    LoadMB(HZMB_FILE);

    Lua_Main(pL_main);

    lua_close(pL_main);

    ExitGame();
    ExitSDL();

    return 0;
}

// Lua main function
int Lua_Main(lua_State *pL_main) {
    int result = 0;

    luaL_register(pL_main, "lib", jylib);
    luaL_register(pL_main, "Byte", bytelib);

    result = luaL_loadfile(pL_main, JYMain_Lua);
    switch (result) {
    case LUA_ERRSYNTAX:
        JY_Error("load lua file %s error: syntax error!\n", JYMain_Lua);
        break;
    case LUA_ERRMEM:
        JY_Error("load lua file %s error: memory allocation error!\n", JYMain_Lua);
        break;
    case LUA_ERRFILE:
        JY_Error("load lua file %s error: can not open file!\n", JYMain_Lua);
        break;
    }

    result = lua_pcall(pL_main, 0, LUA_MULTRET, 0);

    lua_getglobal(pL_main, "JY_Main");
    result = lua_pcall(pL_main, 0, 0, 0);

    return 0;
}

// Lua config reader
int Lua_Config(lua_State *pL, const char *filename) {
    int result = 0;

    result = luaL_loadfile(pL, filename);
    switch (result) {
    case LUA_ERRSYNTAX:
        JY_Error("load lua file %s error: syntax error!\n", filename);
        break;
    case LUA_ERRMEM:
        JY_Error("load lua file %s error: memory allocation error!\n", filename);
        break;
    case LUA_ERRFILE:
        JY_Error("load lua file %s error: can not open file!\n", filename);
        break;
    }

    result = lua_pcall(pL, 0, LUA_MULTRET, 0);

    lua_getglobal(pL, "CONFIG");
    g_Rotate = getfield(pL, "Rotate");
    g_ScreenW = getfield(pL, "Width");
    g_ScreenH = getfield(pL, "Height");
    g_ScreenBpp = getfield(pL, "bpp");
    g_FullScreen = getfield(pL, "FullScreen");
    g_XScale = getfield(pL, "XScale");
    g_YScale = getfield(pL, "YScale");
    g_EnableSound = getfield(pL, "EnableSound");
    IsDebug = getfield(pL, "Debug");
    g_MMapAddX = getfield(pL, "MMapAddX");
    g_MMapAddY = getfield(pL, "MMapAddY");
    g_SMapAddX = getfield(pL, "SMapAddX");
    g_SMapAddY = getfield(pL, "SMapAddY");
    g_WMapAddX = getfield(pL, "WMapAddX");
    g_WMapAddY = getfield(pL, "WMapAddY");
    g_SoundVolume = getfield(pL, "SoundVolume");
    g_MusicVolume = getfield(pL, "MusicVolume");
    g_MAXCacheNum = getfield(pL, "MAXCacheNum");
    g_LoadFullS = getfield(pL, "LoadFullS");
    g_LoadMMapType = getfield(pL, "LoadMMapType");
    g_LoadMMapScope = getfield(pL, "LoadMMapScope");
    g_PreLoadPicGrp = getfield(pL, "PreLoadPicGrp");
    getfieldstr(pL, "JYMain_Lua", JYMain_Lua);

    return 0;
}

int getfield(lua_State *pL, const char *key) {
    int result;
    lua_getfield(pL, -1, key);
    result = (int)lua_tonumber(pL, -1);
    lua_pop(pL, 1);
    return result;
}

int getfieldstr(lua_State *pL, const char *key, char *str) {
    const char *tmp;
    lua_getfield(pL, -1, key);
    tmp = (const char *)lua_tostring(pL, -1);
    if (tmp) strcpy(str, tmp);
    lua_pop(pL, 1);
    return 0;
}

// Debug output
int JY_Debug(const char *fmt, ...) {
    time_t t;
    FILE *fp;
    struct tm *newtime;
    va_list argptr;
    char string[1024];
    va_start(argptr, fmt);
    vsnprintf(string, sizeof(string), fmt, argptr);
    va_end(argptr);
    if (IsDebug == 0) return 0;

    fp = fopen(DEBUG_FILE, "a+t");
    time(&t);
    newtime = localtime(&t);
    fprintf(fp, "%02d:%02d:%02d %s\n", newtime->tm_hour, newtime->tm_min, newtime->tm_sec, string);
    fclose(fp);
    return 0;
}

// Error output
int JY_Error(const char *fmt, ...) {
    time_t t;
    struct tm *newtime;
    va_list argptr;
    char string[1024];
    va_start(argptr, fmt);
    vsnprintf(string, sizeof(string), fmt, argptr);
    va_end(argptr);

    time(&t);
    newtime = localtime(&t);
    fprintf(stderr, "%02d:%02d:%02d %s\n", newtime->tm_hour, newtime->tm_min, newtime->tm_sec, string);
    fflush(stderr);
    return 0;
}

int limitX(int x, int xmin, int xmax) {
    if (x > xmax) x = xmax;
    if (x < xmin) x = xmin;
    return x;
}

int FileLength(const char *filename) {
    FILE *f;
    int ll;
    if ((f = fopen(filename, "rb")) == NULL) {
        return 0;
    }
    fseek(f, 0, SEEK_END);
    ll = ftell(f);
    fclose(f);
    return ll;
}
