#ifndef stX_H
#define stX_H

//typedef Glyph;
//typedef Line;
//typedef Arg;
#include "fonts.h"


/* macros */
#ifdef IS_SET
#undef IS_SET
#endif
// seems ok. is there a reason?
#define IS_SET(flag) ((win.mode & (flag)) != 0)
#define TRUERED(x) (((x)&0xff0000) >> 8)
#define TRUEGREEN(x) (((x)&0xff00))
#define TRUEBLUE(x) (((x)&0xff) << 8)

void SET(int flag);

enum win_mode {
    MODE_VISIBLE = 1 << 0,
    MODE_FOCUSED = 1 << 1,
    MODE_APPKEYPAD = 1 << 2,
    MODE_MOUSEBTN = 1 << 3,
    MODE_MOUSEMOTION = 1 << 4,
    MODE_REVERSE = 1 << 5,
    MODE_KBDLOCK = 1 << 6,
    MODE_HIDE = 1 << 7, // inactive cursor
    MODE_APPCURSOR = 1 << 8,
    MODE_MOUSESGR = 1 << 9,
    MODE_8BIT = 1 << 10,
    MODE_BLINK = 1 << 11,
    MODE_FBLINK = 1 << 12,
    MODE_FOCUS = 1 << 13,
    MODE_MOUSEX10 = 1 << 14,
    MODE_MOUSEMANY = 1 << 15,
    MODE_BRCKTPASTE = 1 << 16,
    MODE_NUMLOCK = 1 << 17,
    MODE_MOUSE = MODE_MOUSEBTN|MODE_MOUSEMOTION|MODE_MOUSEX10 \
                 |MODE_MOUSEMANY,
    MODE_KBDSELECT = 1 << 18,
		MODE_LESS = 1 << 19, // also hides cursor.
		MODE_ENTERSTRING = 1 << 20,
};


typedef XftDraw *Draw;
typedef XftColor Color;

/* Purely graphic info */
typedef struct {
		int tw, th; /* tty width and height */
		int w, h;   /* window width and height */
		int hborderpx, vborderpx;
		int ch;     /* char height */
		int cw;     /* char width  */
		int mode;   /* window state/mode flags */
		int cursor; /* cursor style */
} TermWindow;

/* Drawing Context */
typedef struct {
		Color *col;
		size_t collen;
		Font font, bfont, ifont, ibfont;
		GC gc;
} DC;

typedef struct {
		Display *dpy;
		Colormap cmap;
		Window win;
		Drawable buf;
		GlyphFontSpec *specbuf; /* font spec buffer used for rendering */
		Atom xembed, wmdeletewin, netwmname, netwmpid;
		XIM xim;
		XIC xic;
		Draw draw;
		Visual *vis;
		XSetWindowAttributes attrs;
		int scr;
		int isfixed; /* is fixed geometry? */
		int l, t;    /* left and top offset */
		int gm;      /* geometry mask */
} XWindow;



extern XWindow xw;
extern DC dc;
extern XWindow xw;
extern TermWindow win;



void cresize(int, int);
void xhints(void);

void xbell(void);
void xclipcopy(void);
void xdrawcursor(int, int, Glyph, int, int, Glyph);
void xdrawline(Line, int, int, int);
void xfinishdraw(void);
void xloadcols(void);
int xsetcolorname(int, const char *);
void xsettitle(char *);
int xsetcursor(int);
void xsetmode(int, unsigned int);
void xsetpointermotion(int);
int xstartdraw(void);
void xximspot(int, int);
void toggle_winmode(int);
#ifdef shared
int stmain(int argc, char *argv[]);
#endif

#endif


