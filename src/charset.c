// charset.c - Complete character set conversion from jinyong-legend
// Adapted for iOS/SDL2

#include <stdlib.h>
#include "jymain.h"

static UseFont Font[FONTNUM];
static int currentFont = 0;

static Uint16 gbk_unicode[128][256];
static Uint16 gbk_big5[128][256];
static Uint16 big5_gbk[128][256];
static Uint16 big5_unicode[128][256];

extern SDL_Surface *g_Surface;
extern int g_Rotate;

int InitFont(void) {
    int i;
    TTF_Init();
    for (i = 0; i < FONTNUM; i++) {
        Font[i].size = 0;
        Font[i].name = NULL;
        Font[i].font = NULL;
    }
    return 0;
}

int ExitFont(void) {
    int i;
    for (i = 0; i < FONTNUM; i++) {
        if (Font[i].font) {
            TTF_CloseFont(Font[i].font);
        }
        SafeFree(Font[i].name);
    }
    TTF_Quit();
    return 0;
}

static TTF_Font *GetFont(const char *filename, int size) {
    int i;
    TTF_Font *myfont = NULL;

    for (i = 0; i < FONTNUM; i++) {
        if ((Font[i].size == size) && (Font[i].name) && (strcmp(filename, Font[i].name) == 0)) {
            myfont = Font[i].font;
            break;
        }
    }

    if (myfont == NULL) {
        myfont = TTF_OpenFont(filename, size);
        if (myfont == NULL) {
            JY_Error("GetFont error: can not open font file %s\n", filename);
            return NULL;
        }
        Font[currentFont].size = size;
        if (Font[currentFont].font) TTF_CloseFont(Font[currentFont].font);
        Font[currentFont].font = myfont;
        SafeFree(Font[currentFont].name);
        Font[currentFont].name = (char *)malloc(strlen(filename) + 1);
        strcpy(Font[currentFont].name, filename);
        currentFont++;
        if (currentFont == FONTNUM) currentFont = 0;
    }

    return myfont;
}

int JY_DrawStr(int x, int y, const char *str, int color, int size,
               const char *fontname, int charset, int OScharset) {
    SDL_Color c, c2;
    SDL_Surface *fontSurface = NULL;
    int w, h;
    SDL_Rect rect1, rect2, rect_dest;
    SDL_Rect rect;
    char tmp1[256], tmp2[256];
    TTF_Font *myfont;
    SDL_Surface *tempSurface;

    if (strlen(str) > 127) {
        JY_Error("JY_DrawStr: string length more than 127: %s", str);
        return 0;
    }

    myfont = GetFont(fontname, size);
    if (myfont == NULL) return 1;

    c.r = (Uint8)((color & 0xff0000) >> 16);
    c.g = (Uint8)((color & 0xff00) >> 8);
    c.b = (Uint8)((color & 0xff));

    c2.r = c.r >> 1;
    c2.b = c.b >> 1;
    c2.g = c.g >> 1;

    if (charset == 0 && OScharset == 0) {
        JY_CharSet(str, tmp2, 3);
    } else if (charset == 0 && OScharset == 1) {
        JY_CharSet(str, tmp1, 1);
        JY_CharSet(tmp1, tmp2, 2);
    } else if (charset == 1 && OScharset == 0) {
        JY_CharSet(str, tmp1, 0);
        JY_CharSet(tmp1, tmp2, 3);
    } else if (charset == 1 && OScharset == 1) {
        JY_CharSet(str, tmp2, 2);
    } else {
        strcpy(tmp2, str);
    }

    if (g_Rotate == 0) {
        rect = g_Surface->clip_rect;
    } else {
        rect = RotateReverseRect(&g_Surface->clip_rect);
    }

    TTF_SizeUNICODE(myfont, (Uint16 *)tmp2, &w, &h);

    if ((x >= rect.x + rect.w) || (x + w + 1) <= rect.x ||
        (y >= rect.y + rect.h) || (y + h + 1) <= rect.y) {
        return 1;
    }

    fontSurface = TTF_RenderUNICODE_Solid(myfont, (Uint16 *)tmp2, c);
    if (fontSurface == NULL) return 1;

    rect1.x = (Sint16)x;
    rect1.y = (Sint16)y;
    rect1.w = (Uint16)fontSurface->w;
    rect1.h = (Uint16)fontSurface->h;

    if (g_Rotate == 0) {
        rect2 = rect1;
        rect_dest.x = rect2.x + 1;
        rect_dest.y = rect2.y + 1;
        SDL_SetColors(fontSurface, &c2, 1, 1);
        SDL_BlitSurface(fontSurface, NULL, g_Surface, &rect_dest);

        rect_dest.x = rect2.x;
        rect_dest.y = rect2.y;
        SDL_SetColors(fontSurface, &c, 1, 1);
        SDL_BlitSurface(fontSurface, NULL, g_Surface, &rect_dest);
    } else if (g_Rotate == 1) {
        tempSurface = RotateSurface(fontSurface);
        SDL_FreeSurface(fontSurface);
        fontSurface = tempSurface;
        rect2 = RotateRect(&rect1);

        rect_dest.x = rect2.x - 1;
        rect_dest.y = rect2.y + 1;
        SDL_SetColors(fontSurface, &c2, 1, 1);
        SDL_BlitSurface(fontSurface, NULL, g_Surface, &rect_dest);

        rect_dest.x = rect2.x;
        rect_dest.y = rect2.y;
        SDL_SetColors(fontSurface, &c, 1, 1);
        SDL_BlitSurface(fontSurface, NULL, g_Surface, &rect_dest);
    }

    SDL_FreeSurface(fontSurface);
    return 0;
}

int JY_CharSet(const char *src, char *dest, int flag) {
    Uint8 *psrc, *pdest;
    Uint8 b0, b1;
    int d0;
    Uint16 tmpchar;

    psrc = (Uint8 *)src;
    pdest = (Uint8 *)dest;

    for (;;) {
        b0 = *psrc;
        if (b0 == 0) {
            if ((flag == 0) || (flag == 1)) {
                *pdest = 0;
                break;
            } else {
                *pdest = 0;
                *(pdest + 1) = 0;
                break;
            }
        }
        if (b0 < 128) {
            if ((flag == 0) || (flag == 1)) {
                *pdest = b0;
                pdest++;
                psrc++;
            } else {
                *pdest = b0;
                pdest++;
                *pdest = 0;
                pdest++;
                psrc++;
            }
        } else {
            b1 = *(psrc + 1);
            if (b1 == 0) {
                *pdest = '?';
                *(pdest + 1) = 0;
                break;
            } else {
                d0 = b0 + b1 * 256;
                switch (flag) {
                case 0: tmpchar = big5_gbk[b0 - 128][b1]; break;
                case 1: tmpchar = gbk_big5[b0 - 128][b1]; break;
                case 2: tmpchar = big5_unicode[b0 - 128][b1]; break;
                case 3: tmpchar = gbk_unicode[b0 - 128][b1]; break;
                default: tmpchar = 0;
                }
                if (tmpchar != 0) {
                    *(Uint16 *)pdest = tmpchar;
                } else {
                    *pdest = '?';
                    *(pdest + 1) = '?';
                }
                pdest = pdest + 2;
                psrc = psrc + 2;
            }
        }
    }

    return 0;
}

int LoadMB(const char *mbfile) {
    FILE *fp;
    int i, j;
    Uint16 gbk, big5, unicode;

    fp = fopen(mbfile, "rb");
    if (fp == NULL) {
        JY_Error("cannot open mbfile");
        return 1;
    }

    for (i = 0; i < 128; i++) {
        for (j = 0; j < 256; j++) {
            gbk_unicode[i][j] = 0;
            gbk_big5[i][j] = 0;
            big5_gbk[i][j] = 0;
            big5_unicode[i][j] = 0;
        }
    }

    for (i = 0x81; i <= 0xfe; i++) {
        for (j = 0x40; j <= 0xfe; j++) {
            if (j != 0x7f) {
                fread(&unicode, 2, 1, fp);
                fread(&big5, 2, 1, fp);
                gbk_unicode[i - 128][j] = unicode;
                gbk_big5[i - 128][j] = big5;
            }
        }
    }

    for (i = 0xa0; i <= 0xfe; i++) {
        for (j = 0x40; j <= 0xfe; j++) {
            if (j <= 0x7e || j >= 0xa1) {
                fread(&unicode, 2, 1, fp);
                fread(&gbk, 2, 1, fp);
                big5_unicode[i - 128][j] = unicode;
                big5_gbk[i - 128][j] = gbk;
            }
        }
    }

    fclose(fp);
    return 0;
}
