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

#define ATTRCMP(a, b)                                                          \
  ((a).mode != (b).mode || (a).fg != (b).fg || (a).bg != (b).bg)

#define TLINE(y)                                                               \
  ((y) < term->scr                                                              \
       ? term->hist[(((y) + term->histi - term->scr + HISTSIZE +1 ) ^ HISTSIZE ) & (HISTSIZE-1) ]  : term->line[(y)-term->scr])


#define ISDELIM(u) (u && wcschr(worddelimiters, u))

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


#define IMODE_HELP 0x04

enum glyph_attribute {
  ATTR_NULL = 0,
  ATTR_BOLD = 1 << 0,
  ATTR_FAINT = 1 << 1,
  ATTR_ITALIC = 1 << 2,
  ATTR_UNDERLINE = 1 << 3,
  ATTR_BLINK = 1 << 4,
  ATTR_REVERSE = 1 << 5,
#ifndef UTF8
  ATTR_WRAP = 1 << 6, // NHIST : usable also at the first column, is not tested.
							 // attr_wrap might be not used neccessarily
  ATTR_STRUCK = 1<<7,
  //ATTR_STRUCK = 0,
 // ATTR_GREEK = 1<<7, // betacode
  ATTR_BOLD_FAINT = ATTR_BOLD | ATTR_FAINT,

  ATTR_INVISIBLE = 0,

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
	TMODE_HELP = 1 << 8,
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



#ifdef UTF8
typedef uint_least32_t Rune;
#else
typedef unsigned char Rune;
#endif

void tty_send_unicode(const Arg *arg);

#define Glyph _Glyph
typedef struct {
#ifndef UTF8
		union {
				struct { 
						unsigned char fg; /* foreground  */
						unsigned char bg; /* background  */
						unsigned char mode; /* attribute flags */ //store newline bit here NHIST
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


void redraw(void);
void draw(void);

void printscreen(const Arg *);
void printsel(const Arg *);
void sendbreak(const Arg *);
void toggleprinter(const Arg *);
void showhelp(const Arg *);
void quithelp(const Arg *);
void inverse_screen();

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

