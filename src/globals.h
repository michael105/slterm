#pragma once


// Globals, used across several source files, 
// global typedefs (type aliases), macros.
// There might be globals left within other source files,
// todo: keep them together here.


// parsed commandline options
char *argv0;
char *opt_class = NULL;
char **opt_cmd = NULL;
char *opt_embed = NULL;
char *opt_font = NULL;
char *opt_io = NULL;
char *opt_line = NULL;
char *opt_name = NULL;
char *opt_title = NULL;
char opt_xresources;

char opt_regular_font = 0;
char opt_italic_font = 0;
char opt_bold_font = 0;
char opt_bolditalic_font = 0;


int ispagebased = 0; // counter, tries to keep track, whether the running program 
							// has it's own screen buffer. (e.g. vim)
							// doesnt work well.

// number of saved scroll marks, set with enter.
// needs to be a power of 2.
// memory usage is RETMARKCOUNT*4 bytes,
// and the array is scanned linear when browsing to the marks.
#define RETMARKCOUNT 512


typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;

// callback argument
typedef union {
  int i;
  unsigned int ui;
  float f;
  const void *v;
  const char *s;
} Arg;

#define ARGP(_assign) &(Arg){ ._assign }
#define ARGPi(_value) &(Arg){ .i= _value }

#include "fonts.h"

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

//#define utfchar char
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

// those macros are partially terrorism. 
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


//#define SWAPp(a,b) {a = (void*)((POINTER)a ^ (POINTER)b);\
	b = (void*)((POINTER)a ^ (POINTER)b);\
	a = (void*)((POINTER)a ^ (POINTER)b);}

#define SWAPp(_a,_b) { void* _tmp = _a; _a=_b; _b=_tmp; }

#define SWAPint(a,b) {a^=b;b^=a;a^=b;}



extern char* argv0;

typedef XftColor Color;
typedef XftDraw *Draw;


/* Global drawing context */
typedef struct {
	Color *color_array; // Pointer to an array of 256 colors
	size_t color_arraylen;
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

extern TermWindow twin;


typedef struct {
  Glyph attr; /* current char attributes */ //the rune is set to ' ' (empty)
	// Possibly there should be a difference between space ' ' and empty?
	// evtl render spaces and tabs visually? 
  int x;
  int y;
  char state;
} TCursor;


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


extern XWindow xwin;


/*  Glyph based representation of the screen and history buffer */
typedef struct {
	// todo: histsize should be smaller for help and alt screens
	//Line hist[HISTSIZE]; /* history buffer */ // 
	Line *hist; /* history buffer */ // 
	int guard; // canary, debugging hist
	Line *line;                               // visible lines on screen
	int rows;                                  // number of rows visible
	int cols;                                  // number of cols visible
	int colalloc; // allocated cols in the hist buf, can be > cols. won't shrink, only enlarge. 
	int histindex;    /* history index */ // points to the top of the terminal, last line (bottom) in hist
	uint histsize; // history size-1, size of the buffer. (the count of written lines is in histindex,
						// albite histindex only indicates the number of written lines in the hist buffer,
						// until the buffer is circled.)
	int scr;   // scroll back. scr is counted for scrolled lines. scr=histsize: scroll to the top
				  // scr=0 : scroll to the bottom

	// needed for defining scroll areas
	// would be possible to abandon the separation of "line"(s) buffer of the visible screen,
	// and the hist buffer. however, if only parts of the screen are scrolled, 
	// some pointer copying would be needed. Since it works now,
	// I leave it for now. I'm further abstracting anyways, eventually I'm going
	// to change the line buffer to point at the hist buffer.
	// Maybe. I'm always getting dizzy with the buffer stuff here.
	// Other things are more important, imho. If someone likes to
	// change it, the Macro TLINE and the functions in scroll.c are the locations to begin with.
	int top;                                  /* top    scroll limit */
	int scroll_bottom;                                  /* bottom scroll limit */
	// top / bot: when only parts of the screen are scrolled (scroll areas set)
			 
	int *dirty;  /* dirtyness of lines */ // points to an array

	TCursor cursor;                                /* cursor */
	int oldcursor_x;                                  /* old cursor col */
	int oldcursor_y;                                  /* old cursor row */
	int mode;                                 /* terminal mode flags */
	int esc;                                  /* escape state flags */
	char trantbl[4];                          /* charset table translation */ 
	int charset;                              /* current charset */
	int icharset;                             /* selected charset for sequence */
	int *tabs;

	int scrollmarks[12];
	int retmarks[RETMARKCOUNT];
	int current_retmark; // current retmark. retmarks are stored circular.
	int scrolled_retmark; // to which retmark was scrolled
	//int scroll_retmark; // to which retmark was scrolled
	char circledhist;
} Term;

// would be possible to have several terminals as tabbed or split screens
// I suggest to use i3 or "tabbed", so I'm not going to add that
// the visible terminal
extern Term *term; 
// help terminal. 
extern Term *p_help; 
// alt screen
extern Term *p_alt; 

// temporary storages
extern Term *p_help_storedterm; 
extern Term *p_term; 


// pid of executed shell 
extern pid_t shellpid;

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
// unused. todo: remove that.
// This table was used to calculate the medium advance of fonts without
// fixed width. However, fonts are used as fixed width fonts in each case.
//
/*
static char unused_ascii_printable[]
    = " !\"#$%&'()*+,-./0123456789:;<=>?"
      "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
      "`abcdefghijklmnopqrstuvwxyz{|}~"
			"¡¢£¤¥¦§¨©ª«¬­®¯°±²³/µ¶·¸¹º»¼½¾¿À"
			"ÁÂÃ}ÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕ{×ØÙÚÛ%ÝÞßà"
			"áâã]åæçèéêëìíîï"
			"ðñòóôõ[÷øùúû$ýþÿ";
			*/
#endif





