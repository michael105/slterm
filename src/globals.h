#pragma once

#include "fonts.h"

// Globals, used across several source files, 
// and some global typedefs (type aliases)
// There might be globals left within other source files,
// todo: keep them together here.

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;


extern char* argv0;

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

extern TermWindow win;

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


/* Internal representation of the screen */
typedef struct {
	// todo: remove alt and hist[1]
	Line hist[1][HISTSIZE]; /* history buffer */ // the bug. Oh for god's sake.
	int guard;
	Line *line;                               /* screen */
	Line *alt;                                /* alternate screen */
	//Line *helpscr;                                /* help screen */
	TCursor cursor;                                /* cursor */
	// TODO: strip cthist
	int cthist; // current history, need 2cond buf for resizing
	int row;                                  /* nb row */
	int col;                                  /* nb col */
	int colalloc; // allocated col. won't shrink, only enlarge. 
	int histi;                                /* history index */ // points to the bottom of the terminal
	int scr;                                  /* scroll back */
	int *dirty;                               /* dirtyness of lines */
	int ocx;                                  /* old cursor col */
	int ocy;                                  /* old cursor row */
	int top;                                  /* top    scroll limit */
	int bot;                                  /* bottom scroll limit */
	int mode;                                 /* terminal mode flags */
	int esc;                                  /* escape state flags */
	char trantbl[4];                          /* charset table translation */
	int charset;                              /* current charset */
	int icharset;                             /* selected charset for sequence */
	int *tabs;
	int scrollmarks[12];
	int retmarks[10];

	char circledhist;
} Term;

extern Term *term; 
extern Term *p_help; 
extern Term *p_term; 
extern Term *p_alt; 



