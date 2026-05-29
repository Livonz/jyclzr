// SDL 1.x to SDL 2.x compatibility layer for iOS port
// Translates SDL 1.x API calls to SDL 2.x equivalents

#ifndef SDL1_COMPAT_H
#define SDL1_COMPAT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

// SDL 1.x color key constant
#ifndef SDL_SRCCOLORKEY
#define SDL_SRCCOLORKEY 0x00001000
#endif

#ifndef SDL_RLEACCEL
#define SDL_RLEACCEL 0x00004000
#endif

// SDL 1.x video flags
#ifndef SDL_SWSURFACE
#define SDL_SWSURFACE 0
#endif

#ifndef SDL_HWSURFACE
#define SDL_HWSURFACE 0
#endif

#ifndef SDL_FULLSCREEN
#define SDL_FULLSCREEN 0
#endif

// SDL 1.x key repeat
static inline int SDL_EnableKeyRepeat(int delay, int interval) {
    // SDL 2.x handles key repeat via SDL_SetKeyRepeat which is not available
    // We'll use event state instead
    (void)delay;
    (void)interval;
    return 0;
}

// SDL 1.x video mode functions replaced with SDL 2.x window/renderer
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Surface *surface;
    int width;
    int height;
    int bpp;
    Uint32 flags;
    SDL_Rect clip_rect;
} SDLCompat_Surface;

static SDLCompat_Surface g_compat_surface = {0};

// Convert SDL 1.x SetVideoMode to SDL 2.x
static inline SDL_Surface* SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags) {
    (void)flags;
    
    if (g_compat_surface.window) {
        SDL_DestroyWindow(g_compat_surface.window);
    }
    if (g_compat_surface.renderer) {
        SDL_DestroyRenderer(g_compat_surface.renderer);
    }
    if (g_compat_surface.texture) {
        SDL_DestroyTexture(g_compat_surface.texture);
    }
    if (g_compat_surface.surface) {
        SDL_FreeSurface(g_compat_surface.surface);
    }
    
    g_compat_surface.window = SDL_CreateWindow("JY Engine",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height, SDL_WINDOW_SHOWN);
    
    if (!g_compat_surface.window) {
        return NULL;
    }
    
    g_compat_surface.renderer = SDL_CreateRenderer(g_compat_surface.window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    if (!g_compat_surface.renderer) {
        g_compat_surface.renderer = SDL_CreateRenderer(g_compat_surface.window, -1,
            SDL_RENDERER_SOFTWARE);
    }
    
    // Create software surface for rendering
    Uint32 rmask = 0x00ff0000;
    Uint32 gmask = 0x0000ff00;
    Uint32 bmask = 0x000000ff;
    Uint32 amask = 0xff000000;
    
    if (bpp == 16) {
        rmask = 0xf800;
        gmask = 0x07e0;
        bmask = 0x001f;
        amask = 0;
    }
    
    g_compat_surface.surface = SDL_CreateRGBSurface(0, width, height, bpp,
        rmask, gmask, bmask, amask);
    
    if (!g_compat_surface.surface) {
        return NULL;
    }
    
    g_compat_surface.texture = SDL_CreateTexture(g_compat_surface.renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        width, height);
    
    g_compat_surface.width = width;
    g_compat_surface.height = height;
    g_compat_surface.bpp = bpp;
    g_compat_surface.clip_rect.x = 0;
    g_compat_surface.clip_rect.y = 0;
    g_compat_surface.clip_rect.w = width;
    g_compat_surface.clip_rect.h = height;
    
    return g_compat_surface.surface;
}

// SDL_GetVideoInfo compatibility
typedef struct {
    int hw_available;
    int wm_available;
    int blit_hw;
    int blit_hw_CC;
    int blit_hw_A;
    int blit_sw;
    int blit_sw_CC;
    int blit_sw_A;
    int blit_fill;
    int video_mem;
    SDL_PixelFormat *vfmt;
} SDLCompat_VideoInfo;

static SDL_PixelFormat g_compat_fmt = {0};
static SDLCompat_VideoInfo g_compat_videoinfo = {0};

static inline const SDLCompat_VideoInfo* SDL_GetVideoInfo(void) {
    g_compat_fmt.BitsPerPixel = g_compat_surface.bpp;
    g_compat_fmt.BytesPerPixel = g_compat_surface.bpp / 8;
    
    g_compat_videoinfo.hw_available = 0;
    g_compat_videoinfo.wm_available = 1;
    g_compat_videoinfo.blit_hw = 0;
    g_compat_videoinfo.blit_hw_CC = 0;
    g_compat_videoinfo.blit_hw_A = 0;
    g_compat_videoinfo.blit_sw = 1;
    g_compat_videoinfo.blit_sw_CC = 1;
    g_compat_videoinfo.blit_sw_A = 1;
    g_compat_videoinfo.blit_fill = 1;
    g_compat_videoinfo.video_mem = g_compat_surface.width * g_compat_surface.height * 4;
    g_compat_videoinfo.vfmt = &g_compat_fmt;
    
    return &g_compat_videoinfo;
}

// SDL_VideoDriverName compatibility
static inline int SDL_VideoDriverName(char *name, int maxlen) {
    const char *drv = SDL_GetCurrentVideoDriver();
    if (drv) {
        strncpy(name, drv, maxlen - 1);
        name[maxlen - 1] = '\0';
        return 0;
    }
    return -1;
}

// SDL_UpdateRect compatibility
static inline void SDL_UpdateRect(SDL_Surface *screen, int x, int y, int w, int h) {
    (void)screen;
    (void)x;
    (void)y;
    (void)w;
    (void)h;
    
    if (g_compat_surface.texture && g_compat_surface.surface) {
        SDL_UpdateTexture(g_compat_surface.texture, NULL,
            g_compat_surface.surface->pixels,
            g_compat_surface.surface->pitch);
        SDL_RenderCopy(g_compat_surface.renderer, g_compat_surface.texture, NULL, NULL);
        SDL_RenderPresent(g_compat_surface.renderer);
    }
}

// SDL_UpdateRects compatibility
static inline void SDL_UpdateRects(SDL_Surface *screen, int numrects, SDL_Rect *rects) {
    (void)screen;
    (void)numrects;
    (void)rects;
    SDL_UpdateRect(screen, 0, 0, 0, 0);
}

// SDL_SetClipRect compatibility (works on SDL 2.x surfaces)
// The actual SDL_SetClipRect works on SDL 2.x surfaces, so no wrapper needed

// SDL_FillRect compatibility (works on SDL 2.x surfaces)
// The actual SDL_FillRect works on SDL 2.x surfaces, so no wrapper needed

// SDL_BlitSurface compatibility (works on SDL 2.x surfaces)
// The actual SDL_BlitSurface works on SDL 2.x surfaces, so no wrapper needed

// SDL_SetColorKey compatibility
static inline int SDL_SetColorKey2(SDL_Surface *surface, int flag, Uint32 key) {
    Uint32 flags = (flag & SDL_SRCCOLORKEY) ? SDL_TRUE : SDL_FALSE;
    return SDL_SetColorKey(surface, flags, key);
}

// SDL_SetAlpha compatibility
static inline int SDL_SetAlpha2(SDL_Surface *surface, Uint32 flag, Uint8 alpha) {
    (void)flag;
    return SDL_SetSurfaceAlphaMod(surface, alpha);
}

// SDL_DisplayFormat compatibility
static inline SDL_Surface* SDL_DisplayFormat2(SDL_Surface *surface) {
    if (!g_compat_surface.surface) return surface;
    return SDL_ConvertSurface(surface, g_compat_surface.surface->format, 0);
}

// SDL_LockSurface / SDL_UnlockSurface compatibility
// These work on SDL 2.x surfaces

// SDL_CreateRGBSurface compatibility
// The actual SDL_CreateRGBSurface works on SDL 2.x, so no wrapper needed

// SDL_FreeSurface compatibility
// The actual SDL_FreeSurface works on SDL 2.x

// SDL_Quit compatibility
static inline void SDL_Quit2(void) {
    if (g_compat_surface.texture) {
        SDL_DestroyTexture(g_compat_surface.texture);
        g_compat_surface.texture = NULL;
    }
    if (g_compat_surface.renderer) {
        SDL_DestroyRenderer(g_compat_surface.renderer);
        g_compat_surface.renderer = NULL;
    }
    if (g_compat_surface.window) {
        SDL_DestroyWindow(g_compat_surface.window);
        g_compat_surface.window = NULL;
    }
    if (g_compat_surface.surface) {
        SDL_FreeSurface(g_compat_surface.surface);
        g_compat_surface.surface = NULL;
    }
    SDL_Quit();
}

// Map original SDL 1.x names to compatibility wrappers
#define SDL_SetVideoMode SDL_SetVideoMode
#define SDL_GetVideoInfo SDL_GetVideoInfo
#define SDL_VideoDriverName SDL_VideoDriverName
#define SDL_UpdateRect SDL_UpdateRect
#define SDL_UpdateRects SDL_UpdateRects
#define SDL_SetColorKey SDL_SetColorKey2
#define SDL_SetAlpha SDL_SetAlpha2
#define SDL_DisplayFormat SDL_DisplayFormat2
#define SDL_Quit SDL_Quit2

// Event filter type compatibility
typedef int (*SDLCompat_EventFilter)(const SDL_Event*);
static SDLCompat_EventFilter g_event_filter = NULL;

static inline void SDL_SetEventFilter(SDLCompat_EventFilter filter) {
    g_event_filter = filter;
}

#endif // SDL1_COMPAT_H
