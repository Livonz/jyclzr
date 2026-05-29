// sdlfun.c - SDL functions for iOS port
// Adapted from jinyong-legend for iOS/SDL2

#include "jymain.h"

static Mix_Music *currentMusic = NULL;

#define WAVNUM 5
static Mix_Chunk *WavChunk[WAVNUM];
static int currentWav = 0;

#define RECTNUM 20
static SDL_Rect ClipRect[RECTNUM];
static int currentRect = 0;

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

// Touch input state for iOS
static int g_touchX = -1;
static int g_touchY = -1;
static int g_touchActive = 0;

// iOS touch event handling
void HandleTouchInput(SDL_Event *event) {
    switch (event->type) {
    case SDL_FINGERDOWN:
        g_touchX = (int)(event->tfinger.x * g_ScreenW);
        g_touchY = (int)(event->tfinger.y * g_ScreenH);
        g_touchActive = 1;
        // Simulate ENTER key for touch
        SDL_Event keyEvent;
        keyEvent.type = SDL_KEYDOWN;
        keyEvent.key.keysym.sym = SDLK_RETURN;
        SDL_PushEvent(&keyEvent);
        break;
    case SDL_FINGERMOTION:
        g_touchX = (int)(event->tfinger.x * g_ScreenW);
        g_touchY = (int)(event->tfinger.y * g_ScreenH);
        break;
    case SDL_FINGERUP:
        g_touchActive = 0;
        break;
    }
}

// Map touch position to direction keys
void MapTouchToDirection(void) {
    if (!g_touchActive) return;

    int cx = g_ScreenW / 2;
    int cy = g_ScreenH / 2;
    int dx = g_touchX - cx;
    int dy = g_touchY - cy;
    int threshold = 50;

    if (abs(dx) < threshold && abs(dy) < threshold) return;

    SDL_Event keyEvent;
    keyEvent.type = SDL_KEYDOWN;

    if (abs(dx) > abs(dy)) {
        keyEvent.key.keysym.sym = (dx > 0) ? SDLK_RIGHT : SDLK_LEFT;
    } else {
        keyEvent.key.keysym.sym = (dy > 0) ? SDLK_DOWN : SDLK_UP;
    }
    SDL_PushEvent(&keyEvent);
}

// Key filter for iOS
static int KeyFilter(const SDL_Event *event) {
    static int Esc_KeyPress = 0;
    static int Space_KeyPress = 0;
    static int Return_KeyPress = 0;

    int r = 1;
    switch (event->type) {
    case SDL_KEYDOWN:
        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE:
            if (1 == Esc_KeyPress) r = 0;
            else Esc_KeyPress = 1;
            break;
        case SDLK_RETURN:
            if (1 == Return_KeyPress) r = 0;
            else Return_KeyPress = 1;
            break;
        case SDLK_SPACE:
            if (1 == Space_KeyPress) r = 0;
            else Space_KeyPress = 1;
            break;
        default:
            break;
        }
        break;
    case SDL_KEYUP:
        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE: Esc_KeyPress = 0; break;
        case SDLK_SPACE: Space_KeyPress = 0; break;
        case SDLK_RETURN: Return_KeyPress = 0; break;
        default: break;
        }
        break;
    case SDL_QUIT:
        ExitGame();
        ExitSDL();
        exit(0);
    default:
        break;
    }
    return r;
}

// Initialize SDL
int InitSDL(void) {
    int r;
    int i;
    char tmpstr[255];

    r = SDL_Init(SDL_INIT_VIDEO);
    if (r < 0) {
        JY_Error("Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_VideoDriverName(tmpstr, 255);
    JY_Debug("Video Driver: %s\n", tmpstr);

    InitFont();

    r = SDL_InitSubSystem(SDL_INIT_AUDIO);
    if (r < 0) g_EnableSound = 0;

    r = Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 4096);
    if (r < 0) {
        JY_Error("Couldn't initialize SDL_Mixer: %s\n", SDL_GetError());
        g_EnableSound = 0;
    }

    currentWav = 0;
    for (i = 0; i < WAVNUM; i++) WavChunk[i] = NULL;

    SDL_SetEventFilter(KeyFilter);

    return 0;
}

// Exit SDL
int ExitSDL(void) {
    int i;
    ExitFont();
    StopMIDI();
    for (i = 0; i < WAVNUM; i++) {
        if (WavChunk[i]) {
            Mix_FreeChunk(WavChunk[i]);
            WavChunk[i] = NULL;
        }
    }
    Mix_CloseAudio();
    JY_LoadPicture("", 0, 0);
    SDL_Quit();
    return 0;
}

// Convert 0RGB to current screen color
Uint32 ConvertColor(Uint32 color) {
    Uint8 *p = (Uint8 *)&color;
    return SDL_MapRGB(g_Surface->format, *(p + 2), *(p + 1), *p);
}

// Initialize game
int InitGame(void) {
    int w, h;

    if (g_Rotate == 0) {
        w = g_ScreenW;
        h = g_ScreenH;
    } else {
        w = g_ScreenH;
        h = g_ScreenW;
    }

    // Force windowed mode on iOS
    g_Surface = SDL_SetVideoMode(w, h, 0, SDL_SWSURFACE);

    if (g_Surface == NULL)
        JY_Error("Cannot set video mode");

    Init_Cache();
    JY_PicInit("");

    return 0;
}

// Exit game
int ExitGame(void) {
    JY_PicInit("");
    JY_LoadPicture("", 0, 0);
    JY_UnloadMMap();
    JY_UnloadSMap();
    JY_UnloadWarMap();
    return 0;
}

// Load picture file
int JY_LoadPicture(const char *str, int x, int y) {
    static char filename[255] = "\0";
    static SDL_Surface *pic = NULL;

    SDL_Surface *tmppic;
    SDL_Rect r;

    if (strlen(str) == 0) {
        if (pic) {
            SDL_FreeSurface(pic);
            pic = NULL;
        }
        return 0;
    }

    if (strcmp(str, filename) != 0) {
        if (pic) {
            SDL_FreeSurface(pic);
            pic = NULL;
        }
        tmppic = IMG_Load(str);
        if (tmppic) {
            pic = SDL_ConvertSurface(tmppic, g_Surface->format, 0);
            SDL_FreeSurface(tmppic);
            if (g_Rotate == 1) {
                tmppic = RotateSurface(pic);
                SDL_FreeSurface(pic);
                pic = tmppic;
            }
            strcpy(filename, str);
        }
    }

    if (pic) {
        if (g_Rotate == 0) {
            if ((x == -1) && (y == -1)) {
                x = (g_ScreenW - pic->w) / 2;
                y = (g_ScreenH - pic->h) / 2;
            }
            r.x = (Sint16)x;
            r.y = (Sint16)y;
        } else {
            if ((x == -1) && (y == -1)) {
                x = (g_ScreenW - pic->h) / 2;
                y = (g_ScreenH - pic->w) / 2;
            }
            r.x = (Sint16)y;
            r.y = (Sint16)x;
        }
        SDL_BlitSurface(pic, NULL, g_Surface, &r);
    } else {
        JY_Error("JY_LoadPicture: Load picture file %s failed!", str);
    }
    return 0;
}

// Show surface
int JY_ShowSurface(int flag) {
    if (flag == 1) {
        if (currentRect > 0) {
            SDL_UpdateRects(g_Surface, currentRect, ClipRect);
        }
    } else {
        SDL_UpdateRect(g_Surface, 0, 0, 0, 0);
    }
    return 0;
}

// Delay
int JY_Delay(int x) {
    SDL_Delay(x);
    return 0;
}

// Show slow transition
int JY_ShowSlow(int delaytime, int Flag) {
    if (Flag) {
        SDL_FillRect(g_Surface, NULL, 0);
        JY_ShowSurface(0);
        SDL_Delay(400);
    }
    return 0;
}

// Get current time in milliseconds
double JY_GetTime(void) {
    return (double)SDL_GetTicks();
}

// Play MIDI
int JY_PlayMIDI(const char *filename) {
    static char currentfile[255] = "\0";

    if (g_EnableSound == 0) return 1;
    if (strlen(filename) == 0) {
        StopMIDI();
        strcpy(currentfile, filename);
        return 0;
    }
    if (strcmp(currentfile, filename) == 0) return 0;

    StopMIDI();
    currentMusic = Mix_LoadMUS(filename);
    if (currentMusic == NULL) {
        JY_Error("Open music file %s failed!", filename);
        return 1;
    }
    Mix_VolumeMusic(g_MusicVolume);
    Mix_PlayMusic(currentMusic, -1);
    strcpy(currentfile, filename);
    return 0;
}

// Stop MIDI
int StopMIDI(void) {
    if (currentMusic != NULL) {
        Mix_HaltMusic();
        Mix_FreeMusic(currentMusic);
        currentMusic = NULL;
    }
    return 0;
}

// Play WAV
int JY_PlayWAV(const char *filename) {
    if (g_EnableSound == 0) return 1;

    if (WavChunk[currentWav]) {
        Mix_FreeChunk(WavChunk[currentWav]);
        WavChunk[currentWav] = NULL;
    }

    WavChunk[currentWav] = Mix_LoadWAV(filename);
    if (WavChunk[currentWav]) {
        Mix_VolumeChunk(WavChunk[currentWav], g_SoundVolume);
        Mix_PlayChannel(-1, WavChunk[currentWav], 0);
        currentWav++;
        if (currentWav >= WAVNUM) currentWav = 0;
    } else {
        JY_Error("Open wav file %s failed!", filename);
    }
    return 0;
}

// Get key press
int JY_GetKey(void) {
    SDL_Event event;
    int keyPress = -1;
    while (SDL_PollEvent(&event)) {
        HandleTouchInput(&event);
        switch (event.type) {
        case SDL_KEYDOWN:
            keyPress = event.key.keysym.sym;
            break;
        default:
            break;
        }
    }
    MapTouchToDirection();
    return keyPress;
}

// Set clip rect
int JY_SetClip(int x1, int y1, int x2, int y2) {
    if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0) {
        SDL_SetClipRect(g_Surface, NULL);
        currentRect = 0;
    } else {
        SDL_Rect rect;
        rect.x = (Sint16)x1;
        rect.y = (Sint16)y1;
        rect.w = (Uint16)(x2 - x1);
        rect.h = (Uint16)(y2 - y1);

        if (g_Rotate == 0) {
            ClipRect[currentRect] = rect;
        } else if (g_Rotate == 1) {
            ClipRect[currentRect] = RotateRect(&rect);
        }

        SDL_SetClipRect(g_Surface, &ClipRect[currentRect]);
        currentRect = currentRect + 1;
        if (currentRect >= RECTNUM) currentRect = 0;
    }
    return 0;
}

// Draw rect
int JY_DrawRect(int x1, int y1, int x2, int y2, int color) {
    int xmin, xmax, ymin, ymax;
    SDL_Rect rect1, rect2;
    Uint8 *p;
    int lpitch = 0;
    Uint32 c;

    if (x1 < x2) { xmin = x1; xmax = x2; }
    else { xmin = x2; xmax = x1; }
    if (y1 < y2) { ymin = y1; ymax = y2; }
    else { ymin = y2; ymax = y1; }

    rect1.x = (Sint16)xmin;
    rect1.y = (Sint16)ymin;
    rect1.w = (Uint16)(xmax - xmin + 1);
    rect1.h = (Uint16)(ymax - ymin + 1);

    SDL_LockSurface(g_Surface);
    p = (Uint8 *)g_Surface->pixels;
    lpitch = g_Surface->pitch;
    c = ConvertColor(color);

    if (g_Rotate == 0) rect2 = rect1;
    else rect2 = RotateRect(&rect1);

    x1 = rect2.x;
    y1 = rect2.y;
    x2 = rect2.x + rect2.w - 1;
    y2 = rect2.y + rect2.h - 1;

    HLine32(x1, x2, y1, c, p, lpitch);
    HLine32(x1, x2, y2, c, p, lpitch);
    VLine32(y1, y2, x1, c, p, lpitch);
    VLine32(y1, y2, x2, c, p, lpitch);

    SDL_UnlockSurface(g_Surface);
    return 0;
}

// Horizontal line
void HLine32(int x1, int x2, int y, int color, unsigned char *vbuffer, int lpitch) {
    int temp, i;
    int max_x, max_y, min_x, min_y;
    Uint8 *vbuffer2;
    int bpp;

    bpp = g_Surface->format->BytesPerPixel;
    min_x = g_Surface->clip_rect.x;
    min_y = g_Surface->clip_rect.y;
    max_x = g_Surface->clip_rect.x + g_Surface->clip_rect.w - 1;
    max_y = g_Surface->clip_rect.y + g_Surface->clip_rect.h - 1;

    if (y > max_y || y < min_y) return;
    if (x1 > x2) { temp = x1; x1 = x2; x2 = temp; }
    if (x1 > max_x || x2 < min_x) return;
    x1 = ((x1 < min_x) ? min_x : x1);
    x2 = ((x2 > max_x) ? max_x : x2);

    vbuffer2 = vbuffer + y * lpitch + x1 * bpp;
    switch (bpp) {
    case 2:
        for (i = 0; i <= x2 - x1; i++) {
            *(Uint16 *)vbuffer2 = (Uint16)color;
            vbuffer2 += 2;
        }
        break;
    case 3:
        for (i = 0; i <= x2 - x1; i++) {
            Uint8 *p = (Uint8 *)(&color);
            *vbuffer2 = *p;
            *(vbuffer2 + 1) = *(p + 1);
            *(vbuffer2 + 2) = *(p + 2);
            vbuffer2 += 3;
        }
        break;
    case 4:
        for (i = 0; i <= x2 - x1; i++) {
            *(Uint32 *)vbuffer2 = (Uint32)color;
            vbuffer2 += 4;
        }
        break;
    }
}

// Vertical line
void VLine32(int y1, int y2, int x, int color, unsigned char *vbuffer, int lpitch) {
    int temp, i;
    int max_x, max_y, min_x, min_y;
    Uint8 *vbuffer2;
    int bpp;

    bpp = g_Surface->format->BytesPerPixel;
    min_x = g_Surface->clip_rect.x;
    min_y = g_Surface->clip_rect.y;
    max_x = g_Surface->clip_rect.x + g_Surface->clip_rect.w - 1;
    max_y = g_Surface->clip_rect.y + g_Surface->clip_rect.h - 1;

    if (x > max_x || x < min_x) return;
    if (y1 > y2) { temp = y1; y1 = y2; y2 = temp; }
    if (y1 > max_y || y2 < min_y) return;
    y1 = ((y1 < min_y) ? min_y : y1);
    y2 = ((y2 > max_y) ? max_y : y2);

    vbuffer2 = vbuffer + y1 * lpitch + x * bpp;
    switch (bpp) {
    case 2:
        for (i = 0; i <= y2 - y1; i++) {
            *(Uint16 *)vbuffer2 = (Uint16)color;
            vbuffer2 += lpitch;
        }
        break;
    case 3:
        for (i = 0; i <= y2 - y1; i++) {
            Uint8 *p = (Uint8 *)(&color);
            *vbuffer2 = *p;
            *(vbuffer2 + 1) = *(p + 1);
            *(vbuffer2 + 2) = *(p + 2);
            vbuffer2 += lpitch;
        }
        break;
    case 4:
        for (i = 0; i <= y2 - y1; i++) {
            *(Uint32 *)vbuffer2 = (Uint32)color;
            vbuffer2 += lpitch;
        }
        break;
    }
}

// Fill color
int JY_FillColor(int x1, int y1, int x2, int y2, int color) {
    int c = ConvertColor(color);
    SDL_Rect rect;

    if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0) {
        SDL_FillRect(g_Surface, NULL, c);
    } else {
        rect.x = (Sint16)x1;
        rect.y = (Sint16)y1;
        rect.w = (Uint16)(x2 - x1);
        rect.h = (Uint16)(y2 - y1);
        SDL_FillRect(g_Surface, &rect, c);
    }
    return 0;
}

// Background dimming
int JY_Background(int x1, int y1, int x2, int y2, int Bright) {
    SDL_Surface *lps1;
    SDL_Rect r1, r2;

    if (x2 <= x1 || y2 <= y1) return 0;

    Bright = 256 - Bright;
    if (Bright > 255) Bright = 255;

    r1.x = (Sint16)x1;
    r1.y = (Sint16)y1;
    r1.w = (Uint16)(x2 - x1);
    r1.h = (Uint16)(y2 - y1);

    if (g_Rotate == 0) r2 = r1;
    else r2 = RotateRect(&r1);

    lps1 = SDL_CreateRGBSurface(SDL_SWSURFACE, r2.w, r2.h,
        g_Surface->format->BitsPerPixel,
        g_Surface->format->Rmask, g_Surface->format->Gmask,
        g_Surface->format->Bmask, 0);

    SDL_FillRect(lps1, NULL, 0);
    SDL_SetAlpha(lps1, SDL_SRCALPHA, (Uint8)Bright);
    SDL_BlitSurface(lps1, NULL, g_Surface, &r2);
    SDL_FreeSurface(lps1);
    return 1;
}

// Play MPEG (simplified for iOS - no MPEG support)
int JY_PlayMPEG(const char *filename, int esckey) {
    (void)filename;
    (void)esckey;
    return 0;
}

// Toggle fullscreen (not needed on iOS)
int JY_FullScreen(void) {
    return 0;
}

// Rotate surface 90 degrees
SDL_Surface *RotateSurface(SDL_Surface *src) {
    SDL_Surface *dest;
    int i, j;

    dest = SDL_CreateRGBSurface(SDL_SWSURFACE, src->h, src->w,
        src->format->BitsPerPixel,
        src->format->Rmask, src->format->Gmask,
        src->format->Bmask, src->format->Amask);

    if (src->format->BitsPerPixel == 8) {
        SDL_SetPalette(dest, SDL_LOGPAL, src->format->palette->colors, 0, src->format->palette->ncolors);
    }

    SDL_LockSurface(src);
    SDL_LockSurface(dest);

    if (src->format->BitsPerPixel == 32) {
        for (j = 0; j < src->h; j++) {
            Uint32 *psrc = (Uint32 *)((Uint8 *)src->pixels + j * src->pitch);
            Uint8 *pdest = (Uint8 *)dest->pixels + (src->h - j - 1) * 4;
            for (i = 0; i < src->w; i++) {
                *(Uint32 *)pdest = *psrc;
                psrc++;
                pdest += dest->pitch;
            }
        }
    } else if (src->format->BitsPerPixel == 16) {
        for (j = 0; j < src->h; j++) {
            Uint16 *psrc = (Uint16 *)((Uint8 *)src->pixels + j * src->pitch);
            Uint8 *pdest = (Uint8 *)dest->pixels + (src->h - j - 1) * 2;
            for (i = 0; i < src->w; i++) {
                *(Uint16 *)pdest = *psrc;
                psrc++;
                pdest += dest->pitch;
            }
        }
    } else if (src->format->BitsPerPixel == 24) {
        for (j = 0; j < src->h; j++) {
            Uint8 *psrc = ((Uint8 *)src->pixels + j * src->pitch);
            Uint8 *pdest = (Uint8 *)dest->pixels + (src->h - j - 1) * 3;
            for (i = 0; i < src->w; i++) {
                *pdest = *psrc;
                *(pdest + 1) = *(psrc + 1);
                *(pdest + 2) = *(psrc + 2);
                psrc += 3;
                pdest += dest->pitch;
            }
        }
    } else if (src->format->BitsPerPixel == 8) {
        for (j = 0; j < src->h; j++) {
            Uint8 *psrc = ((Uint8 *)src->pixels + j * src->pitch);
            Uint8 *pdest = (Uint8 *)dest->pixels + (src->h - j - 1);
            for (i = 0; i < src->w; i++) {
                *pdest = *psrc;
                psrc++;
                pdest += dest->pitch;
            }
        }
    }

    SDL_UnlockSurface(src);
    SDL_UnlockSurface(dest);
    SDL_SetColorKey(dest, SDL_SRCCOLORKEY | SDL_RLEACCEL, src->format->colorkey);

    return dest;
}

// Rotate rect
SDL_Rect RotateRect(const SDL_Rect *rect) {
    SDL_Rect r;
    r.x = (Sint16)(g_ScreenH - rect->y - rect->h);
    r.y = rect->x;
    r.w = rect->h;
    r.h = rect->w;
    return r;
}

// Rotate reverse rect
SDL_Rect RotateReverseRect(const SDL_Rect *rect) {
    SDL_Rect r;
    r.x = rect->y;
    r.y = (Sint16)(g_ScreenH - rect->x - rect->w);
    r.w = rect->h;
    r.h = rect->w;
    return r;
}

// BlitSurface helper
int BlitSurface(SDL_Surface *lps, int x, int y, int flag, int value) {
    SDL_Rect rect;
    Uint32 color = ConvertColor(g_MaskColor32);

    if (value > 255) value = 255;
    rect.x = (Sint16)x;
    rect.y = (Sint16)y;

    if ((flag & 0x2) == 0) {
        SDL_BlitSurface(lps, NULL, g_Surface, &rect);
    } else {
        SDL_SetAlpha(lps, SDL_SRCALPHA, (Uint8)value);
        SDL_BlitSurface(lps, NULL, g_Surface, &rect);
    }
    return 0;
}
