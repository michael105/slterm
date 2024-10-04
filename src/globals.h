#pragma once

#include "fonts.h"

// Globals, used across several source files, 
// and some global typedefs (type aliases)
// (It's been not my idea.)

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;


typedef XftColor Color;
typedef XftDraw *Draw;


/* Global drawing context */
typedef struct {
		Color *col; // Pointer to an array of 256 colors
		size_t collen;
		Color *colortable; // Colors 0..7 in normal, bold, faint, bold|faint 
								 // Pointer to an array
		Color *bgcolors; // Background colors 0..15
		Font font, bfont, ifont, ibfont;
		GC gc;
} DC;

extern DC dc;


/* Purely graphic info */
typedef struct {
		int tw, th; /* tty width and height */
		int w, h;   /* window width and height */
		int hborderpx, vborderpx;
		int ch;     /* char height */
		int cw;     /* char width  */
		uint mode;   /* window state/mode flags */
		int cursor; /* cursor style */
} TermWindow;

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
extern TermWindow win;
extern char* argv0;




