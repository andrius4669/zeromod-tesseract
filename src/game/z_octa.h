#ifndef Z_OCTA_H
#define Z_OCTA_H

typedef uint GLuint;
#include "octa.h"
#include "light.h"
#include "texture.h"

extern int identflags;

extern int worldscale, worldsize;
extern int mapversion;
extern char *maptitle;
extern vector<ushort> texmru;
extern int xtraverts, xtravertsva;
extern const ivec cubecoords[8];
extern const ivec facecoords[6][4];
extern const uchar fv[6][4];
extern const uchar fvmasks[64];
extern const uchar faceedgesidx[6][4];

//static inline void loaddeferredlightshaders() {}
static inline void cleardeferredlightshaders() {}
static inline void clearshadowcache() {}
static inline void initlights() {}
static inline void clearlightcache(int id = -1) {}
static inline void cleanupao() {}

static inline void clearradiancehintscache() {}
static inline void clearblendtextures() {}

static inline void renderbackground(const char *caption = NULL) {}
static inline void renderprogress(float bar, const char *text, bool background = false) {}

static inline void allchanged(bool load = false) {}

static inline void destroyva(void *va, bool reparent = true) {}

static inline void preloadglassshaders(bool force = false) {}

static inline void makeundo() {}
static inline void makeundoex(selinfo &s) {}

static inline void cleanupradiancehints() {}

static inline void resetslotshader() {}
static inline void setslotshader(Slot &s) {}

static int nompedit = 0;

enum
{
    NOT_INITING = 0,
    INIT_GAME,
    INIT_LOAD,
    INIT_RESET
};

enum
{
    CHANGE_GFX     = 1<<0,
    CHANGE_SOUND   = 1<<1,
    CHANGE_SHADERS = 1<<2
};

static inline bool initwarning(const char *desc, int level = INIT_RESET, int type = CHANGE_GFX) { return false; }

// octa
extern cube *newcubes(uint face = F_EMPTY, int mat = MAT_AIR);
extern cubeext *growcubeext(cubeext *ext, int maxverts);
extern void setcubeext(cube &c, cubeext *ext);
extern cubeext *newcubeext(cube &c, int maxverts = 0, bool init = true);
extern void getcubevector(cube &c, int d, int x, int y, int z, ivec &p);
extern void setcubevector(cube &c, int d, int x, int y, int z, const ivec &p);
extern int familysize(const cube &c);
extern void freeocta(cube *c);
extern void discardchildren(cube &c, bool fixtex = false, int depth = 0);
extern void optiface(uchar *p, cube &c);
extern void validatec(cube *c, int size = 0);
extern bool isvalidcube(const cube &c);
extern ivec lu;
extern int lusize;
extern cube &lookupcube(const ivec &to, int tsize = 0, ivec &ro = lu, int &rsize = lusize);
extern const cube *neighbourstack[32];
extern int neighbourdepth;
extern const cube &neighbourcube(const cube &c, int orient, const ivec &co, int size, ivec &ro = lu, int &rsize = lusize);
extern void resetclipplanes();
extern int getmippedtexture(const cube &p, int orient);
extern void forcemip(cube &c, bool fixtex = true);
extern bool subdividecube(cube &c, bool fullcheck=true, bool brighten=true);
extern int faceconvexity(const ivec v[4]);
extern int faceconvexity(const ivec v[4], int &vis);
extern int faceconvexity(const vertinfo *verts, int numverts, int size);
extern int faceconvexity(const cube &c, int orient);
extern void calcvert(const cube &c, const ivec &co, int size, ivec &vert, int i, bool solid = false);
extern void calcvert(const cube &c, const ivec &co, int size, vec &vert, int i, bool solid = false);
extern uint faceedges(const cube &c, int orient);
extern bool collapsedface(const cube &c, int orient);
extern bool touchingface(const cube &c, int orient);
extern bool flataxisface(const cube &c, int orient);
extern bool collideface(const cube &c, int orient);
extern int genclipplane(const cube &c, int i, vec *v, plane *clip);
extern void genclipplanes(const cube &c, const ivec &co, int size, clipplanes &p, bool collide = true);
extern bool visibleface(const cube &c, int orient, const ivec &co, int size, ushort mat = MAT_AIR, ushort nmat = MAT_AIR, ushort matmask = MATF_VOLUME);
extern int classifyface(const cube &c, int orient, const ivec &co, int size);
extern int visibletris(const cube &c, int orient, const ivec &co, int size, ushort nmat = MAT_ALPHA, ushort matmask = MAT_ALPHA);
extern int visibleorient(const cube &c, int orient);
extern void genfaceverts(const cube &c, int orient, ivec v[4]);
extern int calcmergedsize(int orient, const ivec &co, int size, const vertinfo *verts, int numverts);
extern void invalidatemerges(cube &c, const ivec &co, int size, bool msg);
extern void calcmerges();
extern int mergefaces(int orient, facebounds *m, int sz);
extern void mincubeface(const cube &cu, int orient, const ivec &o, int size, const facebounds &orig, facebounds &cf, ushort nmat = MAT_AIR, ushort matmask = MATF_VOLUME);
extern void remip();

static inline cubeext &ext(cube &c)
{
    return *(c.ext ? c.ext : newcubeext(c));
}

// octarender
extern vector<tjoint> tjoints;
extern ushort encodenormal(const vec &n);
extern vec decodenormal(ushort norm);
extern void reduceslope(ivec &n);
extern void findtjoints();

// octaedit
extern void cancelsel();
extern void commitchanges(bool force = false);
extern void changed(const ivec &bbmin, const ivec &bbmax, bool commit = true);
extern void changed(const block3 &sel, bool commit = true);

// blend
extern void optimizeblendmap();
extern int calcblendlayer(int x1, int y1, int x2, int y2);

// world
extern void freeoctaentities(cube &c);
extern bool pointinsel(const selinfo &sel, const vec &o);
extern void entcancel();
extern void entitiesinoctanodes();
extern void pasteundoents(undoblock *u);

// physics
extern bool pointincube(const clipplanes &p, const vec &v);
extern void rotatebb(vec &center, vec &radius, int yaw, int pitch, int roll = 0);

// texture
extern void compacteditvslots();
extern void compactmruvslots();
extern void compactvslots(cube *c, int n = 8);
extern void compactvslot(int &index);
extern int compactvslots(bool cull = false);

// materials
extern int findmaterial(const char *name);
extern const char *findmaterialname(int mat);
extern const char *getmaterialdesc(int mat, const char *prefix = "");
#define GETMATIDXVAR(name, var, type) \
    type get##name##var(int mat) \
    { \
        switch(mat&MATF_INDEX) \
        { \
            default: case 0: return name##var; \
            case 1: return name##2##var; \
            case 2: return name##3##var; \
            case 3: return name##4##var; \
        } \
    }

#define CHECK_CALCLIGHT_PROGRESS(exit, show_calclight_progress)

#endif // Z_OCTA_H
