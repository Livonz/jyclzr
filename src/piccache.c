// piccache.c - Complete picture cache implementation from jinyong-legend
// Adapted for iOS/SDL2

#include <stdlib.h>
#include "jymain.h"

static struct PicFileCache pic_file[PIC_FILE_NUM];

LIST_HEAD(cache_head);

static int currentCacheNum = 0;
static Uint32 m_color32[256];

extern int g_MAXCacheNum;
extern Uint32 g_MaskColor32;
extern SDL_Surface *g_Surface;
extern int g_Rotate;
extern int g_ScreenW;
extern int g_ScreenH;
extern int g_PreLoadPicGrp;

int g_EnableRLE = 0;
int CacheFailNum = 0;

// Helper functions
static int LoadPic(int fileid, int picid, struct CacheNode *cache);
static SDL_Surface *CreatePicSurface32(unsigned char *data, int w, int h, int datalong);
static int LoadPallette(char *filename);

int Init_Cache(void) {
    int i;
    for (i = 0; i < PIC_FILE_NUM; i++) {
        pic_file[i].num = 0;
        pic_file[i].idx = NULL;
        pic_file[i].grp = NULL;
        pic_file[i].fp = NULL;
        pic_file[i].pcache = NULL;
    }
    return 0;
}

int JY_PicInit(char *PalletteFilename) {
    struct list_head *pos, *p;
    int i;

    LoadPallette(PalletteFilename);

    list_for_each_safe(pos, p, &cache_head) {
        struct CacheNode *tmp = list_entry(pos, struct CacheNode, list);
        if (tmp->s != NULL) SDL_FreeSurface(tmp->s);
        list_del(pos);
        SafeFree(tmp);
    }

    for (i = 0; i < PIC_FILE_NUM; i++) {
        pic_file[i].num = 0;
        SafeFree(pic_file[i].idx);
        SafeFree(pic_file[i].grp);
        SafeFree(pic_file[i].pcache);
        if (pic_file[i].fp) {
            fclose(pic_file[i].fp);
            pic_file[i].fp = NULL;
        }
    }

    currentCacheNum = 0;
    CacheFailNum = 0;
    return 0;
}

int JY_PicLoadFile(const char *idxfilename, const char *grpfilename, int id) {
    int i;
    struct CacheNode *tmpcache;
    FILE *fp;

    if (id < 0 || id >= PIC_FILE_NUM) return 1;

    if (pic_file[id].pcache) {
        for (i = 0; i < pic_file[id].num; i++) {
            tmpcache = pic_file[id].pcache[i];
            if (tmpcache) {
                if (tmpcache->s != NULL) SDL_FreeSurface(tmpcache->s);
                list_del(&tmpcache->list);
                SafeFree(tmpcache);
                currentCacheNum--;
            }
        }
        SafeFree(pic_file[id].pcache);
    }
    SafeFree(pic_file[id].idx);
    SafeFree(pic_file[id].grp);
    if (pic_file[id].fp) {
        fclose(pic_file[id].fp);
        pic_file[id].fp = NULL;
    }

    pic_file[id].num = FileLength(idxfilename) / 4;
    pic_file[id].idx = (int *)malloc((pic_file[id].num + 1) * 4);
    if (pic_file[id].idx == NULL) {
        JY_Error("JY_PicLoadFile: cannot malloc idx memory!\n");
        return 1;
    }

    if ((fp = fopen(idxfilename, "rb")) == NULL) {
        JY_Error("JY_PicLoadFile: idx file not open ---%s", idxfilename);
        return 1;
    }
    fread(&pic_file[id].idx[1], 4, pic_file[id].num, fp);
    fclose(fp);
    pic_file[id].idx[0] = 0;

    pic_file[id].filelength = FileLength(grpfilename);
    if ((fp = fopen(grpfilename, "rb")) == NULL) {
        JY_Error("JY_PicLoadFile: grp file not open ---%s", grpfilename);
        return 1;
    }
    if (g_PreLoadPicGrp == 1) {
        pic_file[id].grp = (unsigned char *)malloc(pic_file[id].filelength);
        if (pic_file[id].grp == NULL) {
            JY_Error("JY_PicLoadFile: cannot malloc grp memory!\n");
            return 1;
        }
        fread(pic_file[id].grp, 1, pic_file[id].filelength, fp);
        fclose(fp);
    } else {
        pic_file[id].fp = fp;
    }

    pic_file[id].pcache = (struct CacheNode **)malloc(pic_file[id].num * sizeof(struct CacheNode *));
    if (pic_file[id].pcache == NULL) {
        JY_Error("JY_PicLoadFile: cannot malloc pcache memory!\n");
        return 1;
    }

    for (i = 0; i < pic_file[id].num; i++)
        pic_file[id].pcache[i] = NULL;

    return 0;
}

int JY_LoadPic(int fileid, int picid, int x, int y, int flag, int value) {
    struct CacheNode *newcache, *tmpcache;
    int xnew, ynew;

    picid = picid / 2;

    if (fileid < 0 || fileid >= PIC_FILE_NUM || picid < 0 || picid >= pic_file[fileid].num)
        return 1;

    if (pic_file[fileid].pcache[picid] == NULL) {
        newcache = (struct CacheNode *)malloc(sizeof(struct CacheNode));
        if (newcache == NULL) {
            JY_Error("JY_LoadPic: cannot malloc newcache memory!\n");
            return 1;
        }

        newcache->id = picid;
        newcache->fileid = fileid;
        LoadPic(fileid, picid, newcache);

        pic_file[fileid].pcache[picid] = newcache;

        if (currentCacheNum < g_MAXCacheNum) {
            list_add(&newcache->list, &cache_head);
            currentCacheNum = currentCacheNum + 1;
        } else {
            tmpcache = list_entry(cache_head.prev, struct CacheNode, list);
            pic_file[tmpcache->fileid].pcache[tmpcache->id] = NULL;
            if (tmpcache->s != NULL) SDL_FreeSurface(tmpcache->s);
            list_del(&tmpcache->list);
            SafeFree(tmpcache);

            list_add(&newcache->list, &cache_head);
            CacheFailNum++;
            if (CacheFailNum % 100 == 1)
                JY_Debug("Pic Cache is full!");
        }
    } else {
        newcache = pic_file[fileid].pcache[picid];
        list_del(&newcache->list);
        list_add(&newcache->list, &cache_head);
    }

    if (newcache->s == NULL) return 1;

    if (flag & 0x00000001) {
        xnew = x;
        ynew = y;
    } else {
        xnew = x - newcache->xoff;
        ynew = y - newcache->yoff;
    }

    if (g_Rotate == 0) {
        BlitSurface(newcache->s, xnew, ynew, flag, value);
    } else {
        BlitSurface(newcache->s, g_ScreenH - ynew - newcache->s->w, xnew, flag, value);
    }

    return 0;
}

static int LoadPic(int fileid, int picid, struct CacheNode *cache) {
    SDL_RWops *fp_SDL;
    int id1, id2;
    int datalong;
    unsigned char *p, *data;
    SDL_Surface *tmpsurf = NULL;

    if (pic_file[fileid].idx == NULL) {
        JY_Error("LoadPic: fileid %d can not load?\n", fileid);
        return 1;
    }

    id1 = pic_file[fileid].idx[picid];
    id2 = pic_file[fileid].idx[picid + 1];

    if (id1 < 0) datalong = 0;
    if (id2 > pic_file[fileid].filelength) id2 = pic_file[fileid].filelength;
    datalong = id2 - id1;

    if (datalong > 0) {
        if (g_PreLoadPicGrp == 1) {
            data = pic_file[fileid].grp + id1;
            p = NULL;
        } else {
            fseek(pic_file[fileid].fp, id1, SEEK_SET);
            data = (unsigned char *)malloc(datalong);
            p = data;
            fread(data, 1, datalong, pic_file[fileid].fp);
        }

        fp_SDL = SDL_RWFromMem(data, datalong);
        if (IMG_isPNG(fp_SDL) == 0) {
            int w, h;
            w = *(short *)data;
            h = *(short *)(data + 2);
            cache->xoff = *(short *)(data + 4);
            cache->yoff = *(short *)(data + 6);
            cache->s = CreatePicSurface32(data + 8, w, h, datalong - 8);
        } else {
            tmpsurf = IMG_LoadPNG_RW(fp_SDL);
            if (tmpsurf == NULL) {
                JY_Error("LoadPic: cannot create SDL_Surface tmpsurf!\n");
            }
            cache->xoff = tmpsurf->w / 2;
            cache->yoff = tmpsurf->h / 2;
            if (g_Rotate == 0) {
                cache->s = tmpsurf;
            } else {
                cache->s = RotateSurface(tmpsurf);
                SDL_FreeSurface(tmpsurf);
            }
        }
        SDL_FreeRW(fp_SDL);
        SafeFree(p);
    } else {
        cache->s = NULL;
        cache->xoff = 0;
        cache->yoff = 0;
    }

    return 0;
}

int JY_GetPicXY(int fileid, int picid, int *w, int *h, int *xoff, int *yoff) {
    struct CacheNode *newcache;
    int r = JY_LoadPic(fileid, picid, g_ScreenW + 1, g_ScreenH + 1, 1, 0);

    *w = 0;
    *h = 0;
    *xoff = 0;
    *yoff = 0;

    if (r != 0) return 1;

    newcache = pic_file[fileid].pcache[picid / 2];

    if (newcache->s) {
        if (g_Rotate == 0) {
            *w = newcache->s->w;
            *h = newcache->s->h;
        } else {
            *w = newcache->s->h;
            *h = newcache->s->w;
        }
        *xoff = newcache->xoff;
        *yoff = newcache->yoff;
    }

    return 0;
}

static SDL_Surface *CreatePicSurface32(unsigned char *data, int w, int h, int datalong) {
    int p = 0;
    int i, j;
    int yoffset;
    int row;
    int start;
    int x;
    int solidnum;
    SDL_Surface *ps1, *ps2;
    Uint32 *data32 = NULL;

    data32 = (Uint32 *)malloc(w * h * 4);
    if (data32 == NULL) {
        JY_Error("CreatePicSurface32: cannot malloc data32 memory!\n");
        return NULL;
    }

    for (i = 0; i < w * h; i++)
        data32[i] = g_MaskColor32;

    for (i = 0; i < h; i++) {
        yoffset = i * w;
        row = data[p];
        start = p;
        p++;
        if (row > 0) {
            x = 0;
            for (;;) {
                x = x + data[p];
                if (x >= w) break;
                p++;
                solidnum = data[p];
                p++;
                for (j = 0; j < solidnum; j++) {
                    if (g_Rotate == 0) {
                        data32[yoffset + x] = m_color32[data[p]];
                    } else {
                        data32[h - i - 1 + x * h] = m_color32[data[p]];
                    }
                    p++;
                    x++;
                }
                if (x >= w) break;
                if (p - start >= row) break;
            }
            if (p + 1 >= datalong) break;
        }
    }

    if (g_Rotate == 0) {
        ps1 = SDL_CreateRGBSurfaceFrom(data32, w, h, 32, w * 4, 0xff0000, 0xff00, 0xff, 0);
    } else {
        ps1 = SDL_CreateRGBSurfaceFrom(data32, h, w, 32, h * 4, 0xff0000, 0xff00, 0xff, 0);
    }
    if (ps1 == NULL) {
        JY_Error("CreatePicSurface32: cannot create SDL_Surface ps1!\n");
    }
    ps2 = SDL_DisplayFormat(ps1);
    if (ps2 == NULL) {
        JY_Error("CreatePicSurface32: cannot create SDL_Surface ps2!\n");
    }

    SDL_FreeSurface(ps1);
    SafeFree(data32);

    SDL_SetColorKey(ps2, SDL_SRCCOLORKEY | SDL_RLEACCEL, ConvertColor(g_MaskColor32));

    return ps2;
}

static int LoadPallette(char *filename) {
    FILE *fp;
    char color[3];
    int i;
    if (strlen(filename) == 0) return 1;
    if ((fp = fopen(filename, "rb")) == NULL) {
        JY_Error("Pallette File not open ---%s", filename);
        return 1;
    }
    for (i = 0; i < 256; i++) {
        fread(color, 1, 3, fp);
        m_color32[i] = color[0] * 4 * 65536 + color[1] * 4 * 256 + color[2] * 4;
    }
    fclose(fp);
    return 0;
}
