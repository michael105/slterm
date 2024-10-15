#pragma once

#include "fonts.h"

// Globals, used across several source files, 
// global typedefs (type aliases), macros.
// There might be globals left within other source files,
// todo: keep them together here.


// number of saved scroll marks, set with enter.
// needs to be a power of 2.
// memory usage is RETMARKCOUNT*4 bytes,
// and the array is scanned linear when browsing to the marks.
#define RETMARKCOUNT 512


typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;


// ascii needs the whole 256 char table, 
// therefore unsigned chars
#ifdef UTF8

// silence quirky cpp warnings ("quirky". .. misc24. Hopefully fixed that.)
#define utfchar char
#define UTF_SIZ 4
#define UTF_INVALID 0xFFFD

#define IF_UTF8(...) __VA_ARGS__
#define IFNOT_UTF8(...)

#else

#define utfchar unsigned char
#define UTF_INVALID 0xff
#define UTF_SIZ 1


#define IF_UTF8(...) 
#define IFNOT_UTF8(...) __VA_ARGS__

#endif



#if (__SIZEOF_POINTER__==8)
#define POINTER unsigned long
#else
#if (__SIZEOF_POINTER__==4)
#define POINTER unsigned int
#else
#error 
#endif
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) < (b) ? (b) : (a))
// Len in Bytes
#define LEN(a) (sizeof(a) / sizeof(a)[0])
// true, if x between a and b
#define BETWEEN(x, a, b) ((a) <= (x) && (x) <= (b))
// division, round upwards.
#define DIVCEIL(n, d) (((n) + ((d)-1)) / (d))
#define DEFAULT(a, b) (a) = (a) ? (a) : (b)
// return x, if between a and b. else a or b.
#define LIMIT(x, a, b) (x) = (x) < (a) ? (a) : (x) > (b) ? (b) : (x)

#define TIMEDIFF(t1, t2)                                                       \
  ((t1.tv_sec - t2.tv_sec) * 1000 + (t1.tv_nsec - t2.tv_nsec) / 1E6)
#define MODBIT(x, set, bit) ((set) ? ((x) |= (bit)) : ((x) &= ~(bit)))

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


/* Display info of the text window */
typedef struct {
	int tw, th; /* tty width and height */
	int w, h;   /* window width and height */
	int hborderpx, vborderpx;
	int ch;     /* char height */
	int cw;     /* char width  */
	uint mode;   /* window state/mode flags */
	int cursor; /* cursor style */
	int cursor_attr[4]; // cursor attributes
} TermWindow;

extern TermWindow win;

// The xwindow data
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
	// todo: remove alt and hist[1], histsize should be smaller for help and alt screens
	Line hist[HISTSIZE]; /* history buffer */ // the bug. Oh for god's sake.
	int guard;
	Line *line;                               /* screen */
	//Line *alt;                                /* alternate screen */
	//Line *helpscr;                                /* help screen */
	TCursor cursor;                                /* cursor */
	// TODO: strip cthist
	//int cthist; // current history, need 2cond buf for resizing
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
	char trantbl[4];                          /* charset table translation */ //unused
	int charset;                              /* current charset */
	int icharset;                             /* selected charset for sequence */
	int *tabs;
	int scrollmarks[12];
	int retmarks[RETMARKCOUNT];
	int current_retmark; // current retmark. retmarks are stored circular.
	int scroll_retmark; // to which retmark was scrolled
	int retmark_scrolled; // to which retmark was scrolled

	char circledhist;
} Term;

extern Term *term; 
extern Term *p_help; 
extern Term *p_help_storedterm; 
extern Term *p_term; 
extern Term *p_alt; 




/*
Printable characters in ASCII, used to estimate the advance width
of single wide characters.

 */
#ifdef UTF8
static char ascii_printable[]
    = " !\"#$%&'()*+,-./0123456789:;<=>?"
      "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
      "`abcdefghijklmnopqrstuvwxyz{|}~";
#else
// no utf8, but using extended ascii.
// unused.
static char unused_ascii_printable[]
    = " !\"#$%&'()*+,-./0123456789:;<=>?"
      "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
      "`abcdefghijklmnopqrstuvwxyz{|}~"
			"¡¢£¤¥¦§¨©ª«¬­®¯°±²³/µ¶·¸¹º»¼½¾¿À"
			"ÁÂÃ}ÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕ{×ØÙÚÛ%ÝÞßà"
			"áâã]åæçèéêëìíîï"
			"ðñòóôõ[÷øùúû$ýþÿ";
#endif





