#ifndef stX_H
#define stX_H

//typedef Glyph;
//typedef Line;
//typedef Arg;
#include "fonts.h"
#include "xdraw.h"


/* macros */
#ifdef IS_SET
#undef IS_SET
#endif
// seems ok. is there a reason?
#define IS_SET(flag) ((win.mode & (flag)) != 0)


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



void cresize(int, int);
void xhints(void);

void xinit(int cols, int rows);
void xsetenv(void);

void xbell(void);
void xclipcopy(void);
void xloadcolors(void);
int xsetcolorname(int, const char *);
void xsettitle(char *);
int xsetcursor(int);
void xsetmode(int, unsigned int);
void xsetpointermotion(int);
void xximspot(int, int);
void toggle_winmode(int);
#ifdef shared
int stmain(int argc, char *argv[]);
#endif

#endif


