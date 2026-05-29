// jymain.h - Main header for iOS port
// Complete implementation from jinyong-legend

#ifndef JYMAIN_H
#define JYMAIN_H

#include "config.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// Safe free macro
#define SafeFree(p) do { if(p) { free(p); p=NULL; } } while(0)

// Platform-specific paths for iOS
#ifdef __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE
        #define IOS_BUILD 1
    #endif
#endif

// File paths
#define CONFIG_FILE "config.lua"
#define DEBUG_FILE "debug.txt"
#define ERROR_FILE "error.txt"
#define HZMB_FILE "hzmb.dat"

// Font structure
typedef struct UseFont_Type {
    int size;
    char *name;
    TTF_Font *font;
} UseFont;

#define FONTNUM 10

// Building type for map rendering
typedef struct Building_Type {
    int x;
    int y;
    int num;
} BuildingType;

// Cache node for picture cache
struct CacheNode {
    SDL_Surface *s;
    int xoff;
    int yoff;
    int id;
    int fileid;
    struct list_head list;
};

// Picture file cache structure
struct PicFileCache {
    int num;
    int *idx;
    int filelength;
    FILE *fp;
    unsigned char *grp;
    struct CacheNode **pcache;
};

#define PIC_FILE_NUM 40

// Global variables (extern)
extern SDL_Surface *g_Surface;
extern Uint32 g_MaskColor32;
extern int g_Rotate;
extern int g_ScreenW;
extern int g_ScreenH;
extern int g_ScreenBpp;
extern int g_FullScreen;
extern int g_EnableSound;
extern int g_MusicVolume;
extern int g_SoundVolume;
extern int g_XScale;
extern int g_YScale;
extern int g_MMapAddX;
extern int g_MMapAddY;
extern int g_SMapAddX;
extern int g_SMapAddY;
extern int g_WMapAddX;
extern int g_WMapAddY;
extern int g_MAXCacheNum;
extern int g_LoadFullS;
extern int g_LoadMMapType;
extern int g_LoadMMapScope;
extern int g_PreLoadPicGrp;

// Function declarations

// jymain.c
int jy_main(int argc, char *argv[]);
int Lua_Main(lua_State *pL_main);
int Lua_Config(lua_State *pL, const char *filename);
int getfield(lua_State *pL, const char *key);
int getfieldstr(lua_State *pL, const char *key, char *str);
int JY_Debug(const char *fmt, ...);
int JY_Error(const char *fmt, ...);
int limitX(int x, int xmin, int xmax);
int FileLength(const char *filename);

// sdlfun.c
int InitSDL(void);
int ExitSDL(void);
Uint32 ConvertColor(Uint32 color);
int InitGame(void);
int ExitGame(void);
int JY_LoadPicture(const char *str, int x, int y);
int JY_ShowSurface(int flag);
int JY_Delay(int x);
int JY_ShowSlow(int delaytime, int Flag);
double JY_GetTime(void);
int JY_PlayMIDI(const char *filename);
int StopMIDI(void);
int JY_PlayWAV(const char *filename);
int JY_GetKey(void);
int JY_SetClip(int x1, int y1, int x2, int y2);
int JY_DrawRect(int x1, int y1, int x2, int y2, int color);
void HLine32(int x1, int x2, int y, int color, unsigned char *vbuffer, int lpitch);
void VLine32(int y1, int y2, int x, int color, unsigned char *vbuffer, int lpitch);
int JY_FillColor(int x1, int y1, int x2, int y2, int color);
int BlitSurface(SDL_Surface *lps, int x, int y, int flag, int value);
int JY_Background(int x1, int y1, int x2, int y2, int Bright);
int JY_PlayMPEG(const char *filename, int esckey);
int JY_FullScreen(void);
SDL_Surface *RotateSurface(SDL_Surface *src);
SDL_Rect RotateRect(const SDL_Rect *rect);
SDL_Rect RotateReverseRect(const SDL_Rect *rect);

// piccache.c
int Init_Cache(void);
int JY_PicInit(char *PalletteFilename);
int JY_PicLoadFile(const char *idxfilename, const char *grpfilename, int id);
int JY_LoadPic(int fileid, int picid, int x, int y, int flag, int value);
int JY_GetPicXY(int fileid, int picid, int *w, int *h, int *xoff, int *yoff);

// mainmap.c
int JY_LoadMMap(const char *earthname, const char *surfacename, const char *buildingname,
                const char *buildxname, const char *buildyname,
                int x_max, int y_max, int x, int y);
int JY_UnloadMMap(void);
int JY_GetMMap(int x, int y, int flag);
int JY_SetMMap(short x, short y, int flag, short v);
int JY_DrawMMap(int x, int y, int Mypic);
int JY_LoadSMap(const char *Sfilename, const char *tmpfilename, int num,
                int x_max, int y_max, const char *Dfilename, int d_num1, int d_num2);
int JY_SaveSMap(const char *Sfilename, const char *Dfilename);
int JY_UnloadSMap(void);
int JY_GetS(int id, int x, int y, int level);
int JY_SetS(int id, int x, int y, int level, int v);
int JY_GetD(int Sceneid, int id, int i);
int JY_SetD(int Sceneid, int id, int i, int v);
int JY_DrawSMap(int sceneid, int x, int y, int xoff, int yoff, int Mypic);
int JY_LoadWarMap(const char *WarIDXfilename, const char *WarGRPfilename,
                  int mapid, int num, int x_max, int y_max);
int JY_UnloadWarMap(void);
int JY_GetWarMap(int x, int y, int level);
int JY_SetWarMap(int x, int y, int level, int v);
int JY_CleanWarMap(int level, int v);
int JY_DrawWarMap(int flag, int x, int y, int v1, int v2, int v3);

// charset.c
int InitFont(void);
int ExitFont(void);
int JY_DrawStr(int x, int y, const char *str, int color, int size,
               const char *fontname, int charset, int OScharset);
int JY_CharSet(const char *src, char *dest, int flag);
int LoadMB(const char *mbfile);

// Lua API wrappers (from luafun.c)
int HAPI_Debug(lua_State *pL);
int HAPI_GetKey(lua_State *pL);
int HAPI_EnableKeyRepeat(lua_State *pL);
int HAPI_Delay(lua_State *pL);
int HAPI_GetTime(lua_State *pL);
int HAPI_CharSet(lua_State *pL);
int HAPI_DrawStr(lua_State *pL);
int HAPI_SetClip(lua_State *pL);
int HAPI_FillColor(lua_State *pL);
int HAPI_Background(lua_State *pL);
int HAPI_DrawRect(lua_State *pL);
int HAPI_ShowSurface(lua_State *pL);
int HAPI_ShowSlow(lua_State *pL);
int HAPI_PicInit(lua_State *pL);
int HAPI_GetPicXY(lua_State *pL);
int HAPI_LoadPic(lua_State *pL);
int HAPI_PicLoadFile(lua_State *pL);
int HAPI_FullScreen(lua_State *pL);
int HAPI_LoadPicture(lua_State *pL);
int HAPI_PlayMIDI(lua_State *pL);
int HAPI_PlayWAV(lua_State *pL);
int HAPI_PlayMPEG(lua_State *pL);
int HAPI_LoadMMap(lua_State *pL);
int HAPI_DrawMMap(lua_State *pL);
int HAPI_GetMMap(lua_State *pL);
int HAPI_UnloadMMap(lua_State *pL);
int HAPI_LoadSMap(lua_State *pL);
int HAPI_SaveSMap(lua_State *pL);
int HAPI_GetS(lua_State *pL);
int HAPI_SetS(lua_State *pL);
int HAPI_GetD(lua_State *pL);
int HAPI_SetD(lua_State *pL);
int HAPI_DrawSMap(lua_State *pL);
int HAPI_LoadWarMap(lua_State *pL);
int HAPI_GetWarMap(lua_State *pL);
int HAPI_SetWarMap(lua_State *pL);
int HAPI_CleanWarMap(lua_State *pL);
int HAPI_DrawWarMap(lua_State *pL);

// Byte operations for Lua
int Byte_create(lua_State *pL);
int Byte_loadfile(lua_State *pL);
int Byte_savefile(lua_State *pL);
int Byte_get16(lua_State *pL);
int Byte_set16(lua_State *pL);
int Byte_getu16(lua_State *pL);
int Byte_setu16(lua_State *pL);
int Byte_get32(lua_State *pL);
int Byte_set32(lua_State *pL);
int Byte_getstr(lua_State *pL);
int Byte_setstr(lua_State *pL);

// Path helper
void GetResourcePath(const char *filename, char *outpath, int maxlen);

#ifdef __cplusplus
}
#endif

#endif // JYMAIN_H
