/* See LICENSE for license details. */
#ifndef st_H
#define st_H

#include <stdint.h>
#include <sys/types.h>
#include <X11/keysym.h>
#include <X11/X.h>

#include <wchar.h>

#include "xevent.h"

/* macros */

// ascii needs the whole 256 char table, 
// therefore unsigned chars
#ifdef UTF8
#define utfchar char
#else
#define utfchar unsigned char
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
#define LEN(a) (sizeof(a) / sizeof(a)[0])
#define BETWEEN(x, a, b) ((a) <= (x) && (x) <= (b))
#define DIVCEIL(n, d) (((n) + ((d)-1)) / (d))
#define DEFAULT(a, b) (a) = (a) ? (a) : (b)
#define LIMIT(x, a, b) (x) = (x) < (a) ? (a) : (x) > (b) ? (b) : (x)
#define ATTRCMP(a, b)                                                          \
  ((a).mode != (b).mode || (a).fg != (b).fg || (a).bg != (b).bg)
#define TIMEDIFF(t1, t2)                                                       \
  ((t1.tv_sec - t2.tv_sec) * 1000 + (t1.tv_nsec - t2.tv_nsec) / 1E6)
#define MODBIT(x, set, bit) ((set) ? ((x) |= (bit)) : ((x) &= ~(bit)))

#define TLINE(y)                                                               \
  ((y) < term->scr                                                              \
       ? term->hist[term->cthist][(((y) + term->histi - term->scr + HISTSIZE +1 ) ^ HISTSIZE ) & (HISTSIZE-1) ]  : term->line[(y)-term->scr])


#define ISDELIM(u) (u && wcschr(worddelimiters, u))
#define TRUECOLOR(r, g, b) (1 << 24 | (r) << 16 | (g) << 8 | (b))
#define IS_TRUECOL(x) (1 << 24 & (x))


#define IS_SET(flag) ((term->mode & (flag)) != 0)

#ifndef HISTSIZEBITS
// Should be set in config.h.in
#define HISTSIZEBITS 11
#endif

#if (HISTSIZEBITS>20)
#error You most possibly do not want a history with a length > 1.000.000 ?
#error Either change HISTSIZEBITS accordingly, or edit the sources
#endif


#define HISTSIZE (1<<HISTSIZEBITS)

/* Arbitrary sizes */

#define UTF_SIZ 4
#define UTF_INVALID 0xFFFD
//#define utfchar char

// silence quirky cpp warnings
#ifndef UTF8
#undef UTF_INVALID
#define UTF_INVALID 0xff
#undef UTF_SIZ
#define UTF_SIZ 1
#undef utfchar
#define utfchar unsigned char
#endif

#define ESC_BUF_SIZ (128 * UTF_SIZ)
#define ESC_ARG_SIZ 16
#define STR_BUF_SIZ ESC_BUF_SIZ
#define STR_ARG_SIZ ESC_ARG_SIZ

enum glyph_attribute {
  ATTR_NULL = 0,
  ATTR_BOLD = 1 << 0,
  ATTR_FAINT = 1 << 1,
  ATTR_ITALIC = 1 << 2,
  ATTR_UNDERLINE = 1 << 3,
  ATTR_BLINK = 1 << 4,
  ATTR_REVERSE = 1 << 5,
#ifndef UTF8
  ATTR_WRAP = 1 << 6,
  ATTR_BOLD_FAINT = ATTR_BOLD | ATTR_FAINT,

  ATTR_INVISIBLE = 0,
  ATTR_STRUCK = 0,

#else
	ATTR_INVISIBLE  = 1 << 6,
  ATTR_STRUCK     = 1 << 7,
  ATTR_WRAP       = 1 << 8,
  ATTR_WIDE       = 1 << 9,
  ATTR_WDUMMY     = 1 << 10,
  ATTR_BOLD_FAINT = ATTR_BOLD | ATTR_FAINT,
#endif
};

// inputmode. switchable via lessmode_toggle
//extern int inputmode;

enum term_mode {
  MODE_WRAP = 1 << 0,
  MODE_INSERT = 1 << 1,
  MODE_ALTSCREEN = 1 << 2,
  MODE_CRLF = 1 << 3,
  MODE_ECHO = 1 << 4,
  MODE_PRINT = 1 << 5,
#ifdef UTF8
  MODE_UTF8 = 1 << 6,
#else
	MODE_UTF8 = 0,
#endif
  MODE_SIXEL = 1 << 7,
	MODE_HELP = 1 << 8,
};

enum cursor_movement { CURSOR_SAVE, CURSOR_LOAD };

enum cursor_state {
  CURSOR_DEFAULT = 0,
  CURSOR_WRAPNEXT = 1,
  CURSOR_ORIGIN = 2
};

enum charset {
  CS_GRAPHIC0,
  CS_GRAPHIC1,
  CS_UK,
  CS_USA,
  CS_MULTI,
  CS_GER,
  CS_FIN
};

enum escape_state {
  ESC_START = 1,
  ESC_CSI = 2,
  ESC_STR = 4, /* OSC, PM, APC */
  ESC_ALTCHARSET = 8,
  ESC_STR_END = 16, /* a final string was encountered */
  ESC_TEST = 32,    /* Enter in test mode */
  ESC_UTF8 = 64,
  ESC_DCS = 128,
};


enum selection_mode { SEL_IDLE = 0, SEL_EMPTY = 1, SEL_READY = 2 };

enum selection_type { SEL_REGULAR = 1, SEL_RECTANGULAR = 2 };

enum selection_snap { SNAP_WORD = 1, SNAP_LINE = 2 };

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;

#ifdef UTF8
typedef uint_least32_t Rune;
#else
typedef unsigned char Rune;
#endif

#define Glyph _Glyph
typedef struct {
#ifndef UTF8
		union {
				struct { 
						unsigned char fg; /* foreground  */
						unsigned char bg; /* background  */
						unsigned char mode; /* attribute flags */
						Rune u;             /* character code */ 
						// reorder. lowest byte is best
						// to access
				};
				uint32_t intG;
		};
#else
  Rune u;             /* character code */
  ushort mode;      /* attribute flags */
  uint32_t fg;      /* foreground  */
  uint32_t bg;      /* background  */
#endif
} Glyph;


typedef Glyph *Line;

typedef struct {
  Glyph attr; /* current char attributes */ //the rune is set to ' ' (empty)
	// Possibly there should be a difference between space ' ' and empty?
	// evtl render spaces and tabs visually? 
  int x;
  int y;
  char state;
} TCursor;

/* Internal representation of the screen */
typedef struct {
//#warning memo to me
  Line hist[1][HISTSIZE]; /* history buffer */ // the bug. Oh for god's sake.
	int guard;
  Line *line;                               /* screen */
  Line *alt;                                /* alternate screen */
  //Line *helpscr;                                /* help screen */
  TCursor c;                                /* cursor */
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
	char circledhist;
} Term;

extern Term *term; 
extern Term *p_help; 
extern Term *p_term; 

/* CSI Escape sequence structs */
/* ESC '[' [[ [<priv>] <arg> [;]] <mode> [<mode>]] */
typedef struct {
  char buf[ESC_BUF_SIZ]; /* raw string */
  size_t len;            /* raw string length */
  char priv;
  int arg[ESC_ARG_SIZ];
  int narg; /* nb of args */
  char mode[2];
} CSIEscape;

/* STR Escape sequence structs */
/* ESC type [[ [<priv>] <arg> [;]] <mode>] ESC '\' */
typedef struct {
  char type;  /* ESC type ... */
  char *buf;  /* allocated raw string */
  size_t siz; /* allocation size */
  size_t len; /* raw string length */
  char *args[STR_ARG_SIZ];
  int narg; /* nb of args */
} STREscape;


void redraw(void);
void draw(void);

void printscreen(const Arg *);
void printsel(const Arg *);
void sendbreak(const Arg *);
void toggleprinter(const Arg *);
void showhelp(const Arg *);

int tisaltscr(void);
void tnew(int, int);
void tresize(int, int);
void ttyhangup(void);
int ttynew(char *, char *, char *, char **);
size_t ttyread(void);
void set_notifmode(int type, KeySym ksym);

int tlinelen(int);

void resettitle(void);

size_t utf8encode(Rune, char *);


/* config.h globals */
extern char *utmp;
extern char *stty_args;
extern char *vtiden;
extern wchar_t *worddelimiters;
extern int allowaltscreen;
extern char *termname;
extern unsigned int tabspaces;

#ifndef UTFXXX
extern unsigned char defaultfg;
extern unsigned char defaultbg;
#else
extern unsigned int defaultfg;
extern unsigned int defaultbg;
#endif

extern int borderpx;
extern int borderperc;


void drawregion(int, int, int, int);

#endif

