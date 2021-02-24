/* See LICENSE for license details. */

#include "st.h"
#include "x.h"
#include "selection.h"
#include "scroll.h"
#include "mem.h"
#include "base64.h"
#include "utf8.h"
#include "statusbar.h"
#include "termdraw.h"

#if defined(__linux)
#include <pty.h>
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#include <util.h>
#elif defined(__FreeBSD__) || defined(__DragonFly__)
#include <libutil.h>
#endif

/* macros */
#define ISCONTROLC0(c) (BETWEEN(c, 0, 0x1f) || (c) == '\177')
#define ISCONTROLC1(c) (BETWEEN(c, 0x80, 0x9f))
#define ISCONTROL(c) ((c <= 0x1f) || BETWEEN(c, 0x7f, 0x9f))

#undef IS_SET
#define IS_SET(flag) ((term.mode & (flag)) != 0)

#define SWAPp(a,b) {a = (void*)((POINTER)a ^ (POINTER)b);\
		b = (void*)((POINTER)a ^ (POINTER)b);\
		a = (void*)((POINTER)a ^ (POINTER)b);}
#define SWAPint(a,b) {a^=b;b^=a;a^=b;}


static void csidump(void);
static void csihandle(void);
static void csiparse(void);
static void csireset(void);
static int eschandle(uchar);
static void strdump(void);
static void strhandle(void);
static void strparse(void);
static void strreset(void);

static void tprinter(char *, size_t);
static void tcursor(int);
static void tmoveto(int, int);
static void tmoveato(int, int);
static void treset(void);
static void tswapscreen(void);
static void tsetmode(int, int, int *, int);
static void tcontrolcode(uchar);
static void tdectest(utfchar);
static void tdefutf8(utfchar);
static int32_t tdefcolor(int *, int *, int);
static void tdeftran(utfchar);
static void tstrsequence(uchar);


/* Globals */
Term term; // misc make local?
static CSIEscape csiescseq;
static STREscape strescseq;
int borderpx;
int enterlessmode;

// initiate new terminal window and buffers
void tnew(int col, int row) {
		dbg2("tnew *******************************************************\n");
		dbg2("col: %d, row: %d\n",col,row);
		term = (Term){.c = {.attr = {.fg = defaultfg, .bg = defaultbg}}};
		
		// might be buggy. fix this!
		// works flawless at arch.
		// crashes at gentoo. quite seldom.
		// guessing the problem is here.
		term.hist[0][0] = xmalloc( col * sizeof(Glyph));
		memset(term.hist[0][0],0, col * sizeof(Glyph));

		term.colalloc=0;
		//term.hist[1][0] = xmalloc( col * sizeof(Glyph));

		term.guard=0xf0f0f0f0;
		tresize(col, row);
		treset();
}


int tlinelen(int y) {
		int i = term.col;

		if (TLINE(y)[i - 1].mode & ATTR_WRAP) {
				return i;
		}

		while (i > 0 && TLINE(y)[i - 1].u == ' ') {
				--i;
		}

		return i;
}


void tcursor(int mode) {
		static TCursor c[2];
		int alt = IS_SET(MODE_ALTSCREEN);

		if (mode == CURSOR_SAVE) {
				c[alt] = term.c;
		} else if (mode == CURSOR_LOAD) {
				term.c = c[alt];
				tmoveto(c[alt].x, c[alt].y);
		}
}

void treset(void) {
		uint i;

		term.c = (TCursor){{.mode = ATTR_NULL, .fg = defaultfg, .bg = defaultbg,.u=' '},
				.x = 0,
				.y = 0,
				.state = CURSOR_DEFAULT};

		memset(term.tabs, 0, term.colalloc * sizeof(*term.tabs));
		for (i = tabspaces; i < term.colalloc; i += tabspaces) {
				term.tabs[i] = 1;
		}
		term.top = 0;
		term.bot = term.row - 1;
		term.mode = MODE_WRAP | MODE_UTF8; // not UTF8-> MODE_UTF8 eq 0
		memset(term.trantbl, CS_USA, sizeof(term.trantbl));
		term.charset = 0;

		for (i = 0; i < 2; i++) {
				tmoveto(0, 0);
				tcursor(CURSOR_SAVE);
				tclearregion(0, 0, term.colalloc - 1, term.row - 1);
				tswapscreen();
		}
}


int tisaltscr(void) { return IS_SET(MODE_ALTSCREEN); }

void tswapscreen(void) {
		SWAPp( term.line, term.alt );
		term.mode ^= MODE_ALTSCREEN;
		tfulldirt();
}

// TODO: show help screen. Too many keys. misc.
void showhelp(const Arg *a) {
		printf("showhelp\n");
		SWAPp( term.line, term.helpscr );
		term.mode ^= MODE_HELP;
		char *c = strdup("The help.: x\n");
		if ( term.mode & MODE_HELP ){
				for ( int a=0; a<40; a++ ){
						twrite(c,13,1);
				}

		}


		tfulldirt();
}

void csiparse(void) {
		char *p = csiescseq.buf, *np;
		long int v;

		csiescseq.narg = 0;
		if (*p == '?') {
				csiescseq.priv = 1;
				p++;
		}

		csiescseq.buf[csiescseq.len] = '\0';
		while (p < csiescseq.buf + csiescseq.len) {
				np = NULL;
				v = strtol(p, &np, 10);
				if (np == p) {
						v = 0;
				}
				if (v == LONG_MAX || v == LONG_MIN) {
						v = -1;
				}
				csiescseq.arg[csiescseq.narg++] = v;
				p = np;
				if (*p != ';' || csiescseq.narg == ESC_ARG_SIZ) {
						break;
				}
				p++;
		}
		csiescseq.mode[0] = *p++;
		csiescseq.mode[1] = (p < csiescseq.buf + csiescseq.len) ? *p : '\0';
}

/* for absolute user moves, when decom is set */
void tmoveato(int x, int y) {
		tmoveto(x, y + ((term.c.state & CURSOR_ORIGIN) ? term.top : 0));
}

void tmoveto(int x, int y) {
		int miny, maxy;

		if (term.c.state & CURSOR_ORIGIN) {
				miny = term.top;
				maxy = term.bot;
		} else {
				miny = 0;
				maxy = term.row - 1;
		}
		term.c.state &= ~CURSOR_WRAPNEXT;
		term.c.x = LIMIT(x, 0, term.col - 1);
		term.c.y = LIMIT(y, miny, maxy);
}

void tsetmode(int priv, int set, int *args, int narg) {
		int alt, *lim;

		for (lim = args + narg; args < lim; ++args) {
				if (priv) {
						switch (*args) {
								case 1: /* DECCKM -- Cursor key */
										xsetmode(set, MODE_APPCURSOR);
										break;
								case 5: /* DECSCNM -- Reverse video */
										xsetmode(set, MODE_REVERSE);
										break;
								case 6: /* DECOM -- Origin */
										MODBIT(term.c.state, set, CURSOR_ORIGIN);
										tmoveato(0, 0);
										break;
								case 7: /* DECAWM -- Auto wrap */
										MODBIT(term.mode, set, MODE_WRAP);
										break;
								case 0:  /* Error (IGNORED) */
								case 2:  /* DECANM -- ANSI/VT52 (IGNORED) */
								case 3:  /* DECCOLM -- Column  (IGNORED) */
								case 4:  /* DECSCLM -- Scroll (IGNORED) */
								case 8:  /* DECARM -- Auto repeat (IGNORED) */
								case 18: /* DECPFF -- Printer feed (IGNORED) */
								case 19: /* DECPEX -- Printer extent (IGNORED) */
								case 42: /* DECNRCM -- National characters (IGNORED) */
								case 12: /* att610 -- Start blinking cursor (IGNORED) */
										break;
								case 25: /* DECTCEM -- Text Cursor Enable Mode */
										xsetmode(!set, MODE_HIDE);
										break;
								case 9: /* X10 mouse compatibility mode */
										xsetpointermotion(0);
										xsetmode(0, MODE_MOUSE);
										xsetmode(set, MODE_MOUSEX10);
										break;
								case 1000: /* 1000: report button press */
										xsetpointermotion(0);
										xsetmode(0, MODE_MOUSE);
										xsetmode(set, MODE_MOUSEBTN);
										break;
								case 1002: /* 1002: report motion on button press */
										xsetpointermotion(0);
										xsetmode(0, MODE_MOUSE);
										xsetmode(set, MODE_MOUSEMOTION);
										break;
								case 1003: /* 1003: enable all mouse motions */
										xsetpointermotion(set);
										xsetmode(0, MODE_MOUSE);
										xsetmode(set, MODE_MOUSEMANY);
										break;
								case 1004: /* 1004: send focus events to tty */
										xsetmode(set, MODE_FOCUS);
										break;
								case 1006: /* 1006: extended reporting mode */
										xsetmode(set, MODE_MOUSESGR);
										break;
								case 1034:
										xsetmode(set, MODE_8BIT);
										break;
								case 1049: /* swap screen & set/restore cursor as xterm */
										if (!allowaltscreen) {
												break;
										}
										tcursor((set) ? CURSOR_SAVE : CURSOR_LOAD);
										/* FALLTHROUGH */
								case 47: /* swap screen */
								case 1047:
										if (!allowaltscreen) {
												break;
										}
										alt = IS_SET(MODE_ALTSCREEN);
										if (alt) {
												tclearregion(0, 0, term.col - 1, term.row - 1);
										}
										if (set ^ alt) { /* set is always 1 or 0 */
												tswapscreen();
										}
										if (*args != 1049) {
												break;
										}
										/* FALLTHROUGH */
								case 1048:
										tcursor((set) ? CURSOR_SAVE : CURSOR_LOAD);
										break;
								case 2004: /* 2004: bracketed paste mode */
										xsetmode(set, MODE_BRCKTPASTE);
										break;
										/* Not implemented mouse modes. See comments there. */
								case 1001: /* mouse highlight mode; can hang the
															terminal by design when implemented. */
								case 1005: /* UTF-8 mouse mode; will confuse
															applications not supporting UTF-8
															and luit. */
								case 1015: /* urxvt mangled mouse mode; incompatible
															and can be mistaken for other control
															codes. */
										break;
								default:
										fprintf(stderr, "erresc: unknown private set/reset mode %d\n", *args);
										break;
						}
				} else {
						switch (*args) {
								case 0: /* Error (IGNORED) */
										break;
								case 2:
										xsetmode(set, MODE_KBDLOCK);
										break;
								case 4: /* IRM -- Insertion-replacement */
										MODBIT(term.mode, set, MODE_INSERT);
										break;
								case 12: /* SRM -- Send/Receive */
										MODBIT(term.mode, !set, MODE_ECHO);
										break;
								case 20: /* LNM -- Linefeed/new line */
										MODBIT(term.mode, set, MODE_CRLF);
										break;
								default:
										fprintf(stderr, "erresc: unknown set/reset mode %d\n", *args);
										break;
						}
				}
		}
}

void csihandle(void) {
		char buf[40];
		int len;

		//printf("scseq.mode: %c\n", csiescseq.mode[0]);//DBG
		switch (csiescseq.mode[0]) {
				default:
unknown:
						fprintf(stderr, "erresc: unknown csi ");
						csidump();
						/* die(""); */
						break;
				case '@': /* ICH -- Insert <n> blank char */
						DEFAULT(csiescseq.arg[0], 1);
						tinsertblank(csiescseq.arg[0]);
						break;
				case 'A': /* CUU -- Cursor <n> Up */
						DEFAULT(csiescseq.arg[0], 1);
						tmoveto(term.c.x, term.c.y - csiescseq.arg[0]);
						break;
				case 'B': /* CUD -- Cursor <n> Down */
				case 'e': /* VPR --Cursor <n> Down */
						DEFAULT(csiescseq.arg[0], 1);
						tmoveto(term.c.x, term.c.y + csiescseq.arg[0]);
						break;
				case 'i': /* MC -- Media Copy */
						switch (csiescseq.arg[0]) {
								case 0:
										tdump();
										break;
								case 1:
										tdumpline(term.c.y);
										break;
								case 2:
										tdumpsel();
										break;
								case 4:
										term.mode &= ~MODE_PRINT;
										break;
								case 5:
										term.mode |= MODE_PRINT;
										break;
						}
						break;
				case 'c': /* DA -- Device Attributes */
						if (csiescseq.arg[0] == 0) {
								ttywrite(vtiden, strlen(vtiden), 0);
						}
						break;
				case 'C': /* CUF -- Cursor <n> Forward */
				case 'a': /* HPR -- Cursor <n> Forward */
						DEFAULT(csiescseq.arg[0], 1);
						tmoveto(term.c.x + csiescseq.arg[0], term.c.y);
						break;
				case 'D': /* CUB -- Cursor <n> Backward */
						DEFAULT(csiescseq.arg[0], 1);
						tmoveto(term.c.x - csiescseq.arg[0], term.c.y);
						break;
				case 'E': /* CNL -- Cursor <n> Down and first col */
						DEFAULT(csiescseq.arg[0], 1);
						tmoveto(0, term.c.y + csiescseq.arg[0]);
						break;
				case 'F': /* CPL -- Cursor <n> Up and first col */
						DEFAULT(csiescseq.arg[0], 1);
						tmoveto(0, term.c.y - csiescseq.arg[0]);
						break;
				case 'g': /* TBC -- Tabulation clear */
						switch (csiescseq.arg[0]) {
								case 0: /* clear current tab stop */
										term.tabs[term.c.x] = 0;
										break;
								case 3: /* clear all the tabs */
										memset(term.tabs, 0, term.colalloc * sizeof(*term.tabs));
										break;
								default:
										goto unknown;
						}
						break;
				case 'G': /* CHA -- Move to <col> */
				case '`': /* HPA */
						DEFAULT(csiescseq.arg[0], 1);
						tmoveto(csiescseq.arg[0] - 1, term.c.y);
						break;
				case 'H': /* CUP -- Move to <row> <col> */
				case 'f': /* HVP */
						DEFAULT(csiescseq.arg[0], 1);
						DEFAULT(csiescseq.arg[1], 1);
						tmoveato(csiescseq.arg[1] - 1, csiescseq.arg[0] - 1);
						break;
				case 'I': /* CHT -- Cursor Forward Tabulation <n> tab stops */
						DEFAULT(csiescseq.arg[0], 1);
						tputtab(csiescseq.arg[0]);
						break;
				case 'J': /* ED -- Clear screen */
						switch (csiescseq.arg[0]) {
								case 0: /* below */
										tclearregion(term.c.x, term.c.y, term.colalloc - 1, term.c.y);
										if (term.c.y < term.row - 1) {
												tclearregion(0, term.c.y + 1, term.colalloc - 1, term.row - 1);
										}
										break;
								case 1: /* above */
										if (term.c.y > 1) {
												tclearregion(0, 0, term.col - 1, term.c.y - 1);
										}
										tclearregion(0, term.c.y, term.c.x, term.c.y);
										break;
								case 2: /* all */
										tclearregion(0, 0, term.col - 1, term.row - 1);
										break;
								default:
										goto unknown;
						}
						break;
				case 'K': /* EL -- Clear line */
						switch (csiescseq.arg[0]) {
								case 0: /* right */
										tclearregion(term.c.x, term.c.y, term.colalloc - 1, term.c.y);
										break;
								case 1: /* left */
										tclearregion(0, term.c.y, term.c.x, term.c.y);
										break;
								case 2: /* all */
										tclearregion(0, term.c.y, term.colalloc - 1, term.c.y);
										break;
						}
						break;
				case 'S': /* SU -- Scroll <n> line up */
						DEFAULT(csiescseq.arg[0], 1);
						tscrollup(term.top, csiescseq.arg[0], 0);
						break;
				case 'T': /* SD -- Scroll <n> line down */
						DEFAULT(csiescseq.arg[0], 1);
						tscrolldown(term.top, csiescseq.arg[0], 0);
						break;
				case 'L': /* IL -- Insert <n> blank lines */
						DEFAULT(csiescseq.arg[0], 1);
						tinsertblankline(csiescseq.arg[0]);
						break;
				case 'l': /* RM -- Reset Mode */
						tsetmode(csiescseq.priv, 0, csiescseq.arg, csiescseq.narg);
						break;
				case 'M': /* DL -- Delete <n> lines */
						DEFAULT(csiescseq.arg[0], 1);
						tdeleteline(csiescseq.arg[0]);
						break;
				case 'X': /* ECH -- Erase <n> char */
						DEFAULT(csiescseq.arg[0], 1);
						tclearregion(term.c.x, term.c.y, term.c.x + csiescseq.arg[0] - 1, term.c.y);
						break;
				case 'P': /* DCH -- Delete <n> char */
						DEFAULT(csiescseq.arg[0], 1);
						tdeletechar(csiescseq.arg[0]);
						break;
				case 'Z': /* CBT -- Cursor Backward Tabulation <n> tab stops */
						DEFAULT(csiescseq.arg[0], 1);
						tputtab(-csiescseq.arg[0]);
						break;
				case 'd': /* VPA -- Move to <row> */
						DEFAULT(csiescseq.arg[0], 1);
						tmoveato(term.c.x, csiescseq.arg[0] - 1);
						break;
				case 'h': /* SM -- Set terminal mode */
						tsetmode(csiescseq.priv, 1, csiescseq.arg, csiescseq.narg);
						break;
				case 'm': /* SGR -- Terminal attribute (color) */
						tsetattr(csiescseq.arg, csiescseq.narg);
						break;
				case 'n': /* DSR – Device Status Report (cursor position) */
						if (csiescseq.arg[0] == 6) {
								len =
										snprintf(buf, sizeof(buf), "\033[%i;%iR", term.c.y + 1, term.c.x + 1);
								ttywrite(buf, len, 0);
						}
						break;
				case 'r': /* DECSTBM -- Set Scrolling Region */
						if (csiescseq.priv) {
								goto unknown;
						} else {
								DEFAULT(csiescseq.arg[0], 1);
								DEFAULT(csiescseq.arg[1], term.row);
								tsetscroll(csiescseq.arg[0] - 1, csiescseq.arg[1] - 1);
								tmoveato(0, 0);
						}
						break;
				case 's': /* DECSC -- Save cursor position (ANSI.SYS) */
						tcursor(CURSOR_SAVE);
						break;
				case 'u': /* DECRC -- Restore cursor position (ANSI.SYS) */
						tcursor(CURSOR_LOAD);
						break;
				case ' ':
						switch (csiescseq.mode[1]) {
								case 'q': /* DECSCUSR -- Set Cursor Style */
										if (xsetcursor(csiescseq.arg[0])) {
												goto unknown;
										}
										break;
								default:
										goto unknown;
						}
						break;
		}
}

void csidump(void) {
		size_t i;
		uint c;

		fprintf(stderr, "ESC[");
		for (i = 0; i < csiescseq.len; i++) {
				c = csiescseq.buf[i] & 0xff;
				if (isprint(c)) {
						putc(c, stderr);
				} else if (c == '\n') {
						fprintf(stderr, "(\\n)");
				} else if (c == '\r') {
						fprintf(stderr, "(\\r)");
				} else if (c == 0x1b) {
						fprintf(stderr, "(\\e)");
				} else {
						fprintf(stderr, "(%02x)", c);
				}
		}
		putc('\n', stderr);
}

void csireset(void) { memset(&csiescseq, 0, sizeof(csiescseq)); }

void strhandle(void) {
		char *p = NULL, *dec;
		int j, narg, par;

		term.esc &= ~(ESC_STR_END | ESC_STR);
		strparse();
		par = (narg = strescseq.narg) ? atoi(strescseq.args[0]) : 0;

		switch (strescseq.type) {
				case ']': /* OSC -- Operating System Command */
						switch (par) {
								case 0:
								case 1:
								case 2:
										if (narg > 1) {
												xsettitle(strescseq.args[1]);
										}
										return;
								case 52:
										if (narg > 2) {
												dec = base64dec(strescseq.args[2]);
												if (dec) {
														xsetsel(dec);
														xclipcopy();
												} else {
														fprintf(stderr, "erresc: invalid base64\n");
												}
										}
										return;
								case 4: /* color set */
										if (narg < 3) {
												break;
										}
										p = strescseq.args[2];
										/* FALLTHROUGH */
								case 104: /* color reset, here p = NULL */
										j = (narg > 1) ? atoi(strescseq.args[1]) : -1;
										if (xsetcolorname(j, p)) {
												if (par == 104 && narg <= 1) {
														return; /* color reset without parameter */
												}
												fprintf(stderr, "erresc: invalid color j=%d, p=%s\n", j,
																p ? p : "(null)");
										} else {
												/*
												 * TODO if defaultbg color is changed, borders
												 * are dirty
												 */
												redraw();
										}
										return;
						}
						break;
				case 'k': /* old title set compatibility */
						xsettitle(strescseq.args[0]);
						return;
				case 'P': /* DCS -- Device Control String */
						term.mode |= ESC_DCS;
				case '_': /* APC -- Application Program Command */
				case '^': /* PM -- Privacy Message */
						return;
		}

		fprintf(stderr, "erresc: unknown str ");
		strdump();
}

void strparse(void) {
		int c;
		char *p = strescseq.buf;

		strescseq.narg = 0;
		strescseq.buf[strescseq.len] = '\0';

		if (*p == '\0') {
				return;
		}

		while (strescseq.narg < STR_ARG_SIZ) {
				strescseq.args[strescseq.narg++] = p;
				while ((c = *p) != ';' && c != '\0') {
						++p;
				}
				if (c == '\0') {
						return;
				}
				*p++ = '\0';
		}
}

void strdump(void) {
		size_t i;
		uint c;

		fprintf(stderr, "ESC%c", strescseq.type);
		for (i = 0; i < strescseq.len; i++) {
				c = strescseq.buf[i] & 0xff;
				if (c == '\0') {
						putc('\n', stderr);
						return;
				} else if (isprint(c)) {
						putc(c, stderr);
				} else if (c == '\n') {
						fprintf(stderr, "(\\n)");
				} else if (c == '\r') {
						fprintf(stderr, "(\\r)");
				} else if (c == 0x1b) {
						fprintf(stderr, "(\\e)");
				} else {
						fprintf(stderr, "(%02x)", c);
				}
		}
		fprintf(stderr, "ESC\\\n");
}

void strreset(void) {
		strescseq = (STREscape){
				.buf = xrealloc(strescseq.buf, STR_BUF_SIZ),
						.siz = STR_BUF_SIZ,
		};
}

void sendbreak(const Arg *arg) {
		if (tcsendbreak(cmdfd, 0)) {
				perror("Error sending break");
		}
}


void tdefutf8(utfchar ascii) {
#ifdef UTF8
		if (ascii == 'G')
				term.mode |= MODE_UTF8;
		else
				if (ascii == '@') {
						term.mode &= ~MODE_UTF8;
				}
#endif
}

void tdeftran(utfchar ascii) {
		static char cs[] = "0B";
		static int vcs[] = {CS_GRAPHIC0, CS_USA};
		char *p;

		if ((p = strchr(cs, ascii)) == NULL) {
				fprintf(stderr, "esc unhandled charset: ESC ( %c\n", ascii);
		} else {
				term.trantbl[term.icharset] = vcs[p - cs];
		}
}

void tdectest(utfchar c) {
		int x, y;

		if (c == '8') { /* DEC screen alignment test. */
				for (x = 0; x < term.col; ++x) {
						for (y = 0; y < term.row; ++y) {
								tsetchar('E', &term.c.attr, x, y);
						}
				}
		}
}

void tstrsequence(uchar c) {
		strreset();

		switch (c) {
				case 0x90: /* DCS -- Device Control String */
						c = 'P';
						term.esc |= ESC_DCS;
						break;
				case 0x9f: /* APC -- Application Program Command */
						c = '_';
						break;
				case 0x9e: /* PM -- Privacy Message */
						c = '^';
						break;
				case 0x9d: /* OSC -- Operating System Command */
						c = ']';
						break;
		}
		strescseq.type = c;
		term.esc |= ESC_STR;
}

void tcontrolcode(uchar ascii) {
		//printf("tcontrolcode: %c\n", ascii); //DBG
		switch (ascii) {
				case '\t': /* HT */
						tputtab(1);
						return;
				case '\b': /* BS */
						tmoveto(term.c.x - 1, term.c.y);
						return;
				case '\r': /* CR */
						tmoveto(0, term.c.y);
						return;
				case '\f': /* LF */
				case '\v': /* VT */
				case '\n': /* LF */
						/* go to first col if the mode is set */
						tnewline(IS_SET(MODE_CRLF));
						return;
				case '\a': /* BEL */
						if (term.esc & ESC_STR_END) {
								/* backwards compatibility to xterm */
								strhandle();
						} else {
								xbell();
						}
						break;
				case '\033': /* ESC */
						csireset();
						term.esc &= ~(ESC_CSI | ESC_ALTCHARSET | ESC_TEST);
						term.esc |= ESC_START;
						return;
				case '\016': /* SO (LS1 -- Locking shift 1) */
				case '\017': /* SI (LS0 -- Locking shift 0) */
						term.charset = 1 - (ascii - '\016');
						return;
				case '\032': /* SUB */
						tsetchar('?', &term.c.attr, term.c.x, term.c.y);
				case '\030': /* CAN */
						csireset();
						break;
				case '\005': /* ENQ (IGNORED) */
				case '\000': /* NUL (IGNORED) */
				case '\021': /* XON (IGNORED) */
				case '\023': /* XOFF (IGNORED) */
				case 0177:   /* DEL (IGNORED) */
						return;
				case 0x80: /* TODO: PAD */
				case 0x81: /* TODO: HOP */
				case 0x82: /* TODO: BPH */
				case 0x83: /* TODO: NBH */
				case 0x84: /* TODO: IND */
						break;
				case 0x85:     /* NEL -- Next line */
						tnewline(1); /* always go to first col */
						break;
				case 0x86: /* TODO: SSA */
				case 0x87: /* TODO: ESA */
						break;
				case 0x88: /* HTS -- Horizontal tab stop */
						term.tabs[term.c.x] = 1;
						break;
				case 0x89: /* TODO: HTJ */
				case 0x8a: /* TODO: VTS */
				case 0x8b: /* TODO: PLD */
				case 0x8c: /* TODO: PLU */
				case 0x8d: /* TODO: RI */
				case 0x8e: /* TODO: SS2 */
				case 0x8f: /* TODO: SS3 */
				case 0x91: /* TODO: PU1 */
				case 0x92: /* TODO: PU2 */
				case 0x93: /* TODO: STS */
				case 0x94: /* TODO: CCH */
				case 0x95: /* TODO: MW */
				case 0x96: /* TODO: SPA */
				case 0x97: /* TODO: EPA */
				case 0x98: /* TODO: SOS */
				case 0x99: /* TODO: SGCI */
						break;
				case 0x9a: /* DECID -- Identify Terminal */
						ttywrite(vtiden, strlen(vtiden), 0);
						break;
				case 0x9b: /* TODO: CSI */
				case 0x9c: /* TODO: ST */
						break;
				case 0x90: /* DCS -- Device Control String */
				case 0x9d: /* OSC -- Operating System Command */
				case 0x9e: /* PM -- Privacy Message */
				case 0x9f: /* APC -- Application Program Command */
						tstrsequence(ascii);
						return;
		}
		/* only CAN, SUB, \a and C1 chars interrupt a sequence */
		term.esc &= ~(ESC_STR_END | ESC_STR);
}

/*
 * returns 1 when the sequence is finished and it hasn't to read
 * more characters for this sequence, otherwise 0
 */
int eschandle(uchar ascii) {
		//printf("eschandle: %c\n", ascii); //DBG
		switch (ascii) {
				case '[':
						term.esc |= ESC_CSI;
						return 0;
				case '#':
						term.esc |= ESC_TEST;
						return 0;
				case '%':
						term.esc |= ESC_UTF8;
						return 0;
				case 'P': /* DCS -- Device Control String */
				case '_': /* APC -- Application Program Command */
				case '^': /* PM -- Privacy Message */
				case ']': /* OSC -- Operating System Command */
				case 'k': /* old title set compatibility */
						tstrsequence(ascii);
						return 0;
				case 'n': /* LS2 -- Locking shift 2 */
				case 'o': /* LS3 -- Locking shift 3 */
						term.charset = 2 + (ascii - 'n');
						break;
				case '(': /* GZD4 -- set primary charset G0 */
				case ')': /* G1D4 -- set secondary charset G1 */
				case '*': /* G2D4 -- set tertiary charset G2 */
				case '+': /* G3D4 -- set quaternary charset G3 */
						term.icharset = ascii - '(';
						term.esc |= ESC_ALTCHARSET;
						return 0;
				case 'D': /* IND -- Linefeed */
						if (term.c.y == term.bot) {
								tscrollup(term.top, 1, 1);
						} else {
								tmoveto(term.c.x, term.c.y + 1);
						}
						break;
				case 'E':      /* NEL -- Next line */
						tnewline(1); /* always go to first col */
						break;
				case 'H': /* HTS -- Horizontal tab stop */
						term.tabs[term.c.x] = 1;
						break;
				case 'M': /* RI -- Reverse index */
						if (term.c.y == term.top) {
								tscrolldown(term.top, 1, 1);
						} else {
								tmoveto(term.c.x, term.c.y - 1);
						}
						break;
				case 'Z': /* DECID -- Identify Terminal */
						ttywrite(vtiden, strlen(vtiden), 0);
						break;
				case 'c': /* RIS -- Reset to initial state */
						treset();
						resettitle();
						xloadcols();
						break;
				case '=': /* DECPAM -- Application keypad */
						xsetmode(1, MODE_APPKEYPAD);
						break;
				case '>': /* DECPNM -- Normal keypad */
						xsetmode(0, MODE_APPKEYPAD);
						break;
				case '7': /* DECSC -- Save Cursor */
						tcursor(CURSOR_SAVE);
						break;
				case '8': /* DECRC -- Restore Cursor */
						tcursor(CURSOR_LOAD);
						break;
				case '\\': /* ST -- String Terminator */
						if (term.esc & ESC_STR_END) {
								strhandle();
						}
						break;
				default:
						fprintf(stderr, "erresc: unknown sequence ESC 0x%02X '%c'\n", (uchar)ascii,
										isprint(ascii) ? ascii : '.');
						break;
		}
		return 1;
}

void tresize(int col, int row) {
		int i, j;
		int minrow = MIN(row, term.row);
		int mincol = MIN(col, term.col);
		int *bp;
		int enlarge = 0;
		TCursor c;
		if ( col > term.colalloc ){
				term.colalloc = col;
				enlarge = 1;
		}


		if (row < term.row || col < term.col) {
				toggle_winmode(trt_kbdselect(XK_Escape, NULL, 0));
		}

		if (col < 1 || row < 1) {
				fprintf(stderr, "tresize: error resizing to %dx%d\n", col, row);
				return;
		}

		/*
		 * slide screen to keep cursor where we expect it -
		 * tscrollup would work here, but we can optimize to
		 * memmove because we're freeing the earlier lines
		 */
		for (i = 0; i <= term.c.y - row; i++) {
				free(term.line[i]);
				free(term.alt[i]);
				free(term.helpscr[i]);
		}
		/* ensure that both src and dst are not NULL */
		if (i > 0) {
				memmove(term.line, term.line + i, row * sizeof(Line));
				memmove(term.alt, term.alt + i, row * sizeof(Line));
				memmove(term.helpscr,term.helpscr + i, row * sizeof(Line));
		}
		for (i += row; i < term.row; i++) {
				free(term.line[i]);
				free(term.alt[i]);
				free(term.helpscr[i]);
		}

		/* resize to new height */
		term.line = xrealloc(term.line, row * sizeof(Line));
		term.alt = xrealloc(term.alt, row * sizeof(Line));
		term.helpscr = xrealloc(term.helpscr, row * sizeof(Line));
		term.dirty = xrealloc(term.dirty, row * sizeof(*term.dirty));
		term.tabs = xrealloc(term.tabs, term.colalloc * sizeof(*term.tabs));

		int oldline = 0;
		int newline = 0;
		int oldcol = 0;
		int newcol = 0;
		int newhist = !(term.cthist);
		int oldhist = term.cthist;
		// delay here. Collect resize events
		if ( term.circledhist  ){
				oldline = (term.histi+1 > HISTSIZE ) ? 0 : (term.histi+1);
		}
		term.c.attr.u = ' '; 
		dbg2(AC_YELLOW "oldline: %d  term.histi: %d  term.col: %d col: %d" AC_NORM,oldline,term.histi, term.col, col);
#if 0

		if ( oldline != term.histi ){
				term.hist[newhist][newline] = xmalloc( col * sizeof(Glyph));
				memset32( &term.hist[newhist][newline][mincol].intG, term.c.attr.intG, col-mincol );
		}

		while (oldline!=term.histi) { // Didn't reach the end of the old history yet
				dbg3( "oldhist: %d term.col %d newhist %d oldline: %d oldcol: %d newline: %d newcol: %d", oldhist, term.col,newhist, oldline, oldcol, newline, newcol );
				while( (oldline!=term.histi) && (oldcol < term.col) ){ // && !( ( oldcol>0 ) && (term.hist[oldhist][oldline][oldcol-1].mode & ATTR_WRAP )) ){
						dbg3( "term.col: %d L2: oldline: %d oldcol: %d newline: %d newcol: %d",term.col, oldline, oldcol, newline, newcol );
						//dbg3( "intG oldhist: %d - %d\n", term.hist[oldhist][oldline][oldcol].intG, term.hist[oldhist][oldline][oldcol].u );
						if ( term.hist[oldhist][oldline][oldcol].mode & ATTR_WRAP ){
								dbg2("WRAP");
						}
						term.hist[newhist][newline][newcol].intG = term.hist[oldhist][oldline][oldcol].intG;
						//term.hist[newhist][newline][newcol].mode
						oldcol++;
						newcol++;
						if ( ( newcol == col) || ( (oldcol>0) && (term.hist[oldhist][oldline][oldcol-1].mode & ATTR_WRAP )) ){ // end of line
								dbg3("Eol. newcol: %d  oldcol:%d",newcol,oldcol);
								//term.hist[newhist][newline][newcol-1].mode |= ATTR_WRAP;
								newline++;
								newline &= ((1<<HISTSIZEBITS)-1);
								newcol=0;
								if ( !term.hist[newhist][newline] ){ 
										term.hist[newhist][newline] = xmalloc( col * sizeof(Glyph));
										dbg3(AC_BLUE"malloc: hist %d, line %d, cols: %d"AC_NORM, newhist, newline, col );
								} else {
										dbg3(AC_GREEN"realloc: hist %d, line %d, cols: %d"AC_NORM, newhist, newline, col );
										term.hist[newhist][newline] = xrealloc( 	term.hist[newhist][newline], col * sizeof(Glyph));
								}
								//dbg3("newline: %d",newline);
								memset32( &term.hist[newhist][newline][mincol].intG, term.c.attr.intG, col-mincol );
						}
						if ( oldcol == term.col ){// && !( ( oldcol>0 ) && (term.hist[oldhist][oldline][oldcol-1].mode & ATTR_WRAP )) ){
								dbg3( "YY: newline: %d newcol: %d", newline, newcol );
								//free( term.hist[oldhist][oldline] );
								oldcol = 0;
								//term.hist[oldhist][oldline] = 0;
								oldline++;
								oldline &= ((1<<HISTSIZEBITS)-1); // modulo

						}

				}
		}
		term.cthist = newhist;
		dbg2("copied hist. oldhist: %d  term.cthist: %d", oldhist, term.cthist );
#else
		int t = term.histi;
		if ( term.circledhist  ){
				t = HISTSIZE;
		}

		if ( enlarge )
		for (i = 0; i < t; i++) { // 
				term.hist[(term.cthist)][i] = xrealloc(term.hist[term.cthist][i], term.colalloc * sizeof(Glyph));
#ifndef UTF8
				memset32( &term.hist[term.cthist][i][0].intG, term.c.attr.intG, term.colalloc );
				//memset32( &term.hist[term.cthist][i][mincol].intG, term.c.attr.intG, term.colalloc-mincol );
				//for (j = mincol; j < col; j++) {
				//		term.hist[term.cthist][i][j].intG = term.c.attr.intG;
				//}
#else
				for (j = mincol; j < term.colalloc; j++) {
						term.hist[term.cthist][i][j] = term.c.attr;
						term.hist[term.cthist][i][j].u = ' '; 
						//append empty chars, if more cols than before
				}
#endif
		}

#endif
		dbg("cp\n");
		/* resize each row to new width, zero-pad if needed */
		if ( enlarge ){
				for (i = 0; i < minrow; i++) {
						//dbg3("i: %d, %d", i, minrow );
						term.line[i] = xrealloc(term.line[i], term.colalloc * sizeof(Glyph));
						//dbg3("i2\n");
						term.alt[i] = xrealloc(term.alt[i], term.colalloc * sizeof(Glyph));
						term.helpscr[i] = xrealloc(term.helpscr[i], term.colalloc * sizeof(Glyph));
				}
		} else {
				for (i = 0; i < minrow; i++) {

						//term.alt[i] = xrealloc(term.alt[i], term.colalloc * sizeof(Glyph));
						//dbg3("i: %d, %d", i, minrow );
						//term.line[i] = xrealloc(term.line[i], col * sizeof(Glyph));
						//dbg3("i2\n");
						//term.alt[i] = xrealloc(term.alt[i], col * sizeof(Glyph));
						//memset(term.alt[i], 0, col);
				}
		}



		dbg3("i4\n");
		/* allocate any new rows */
		for ( i = minrow ; i < row; i++) {
				term.line[i] = xmalloc(term.colalloc * sizeof(Glyph));
				memset(term.line[i], 0, sizeof(Glyph) * term.colalloc);
				term.alt[i] = xmalloc(term.colalloc * sizeof(Glyph));
				term.helpscr[i] = xmalloc(term.colalloc * sizeof(Glyph));
				memset(term.alt[i], 0, sizeof(Glyph) * term.colalloc);
				memset(term.helpscr[i], 0, sizeof(Glyph) * term.colalloc);
		}
				/*if ( term.colalloc > col ){
						if ( minrow < row )
								term.colalloc = col;
				} else {
						if ( col > term.colalloc )
								term.colalloc = col;
				}*/

		if ( enlarge )
		if (col > term.col) {
				bp = term.tabs + term.col;

				memset(bp, 0, sizeof(*term.tabs) * (term.colalloc - term.col));
				while (--bp > term.tabs && !*bp) {
						/* nothing */
				}
				for (bp += tabspaces; bp < term.tabs + term.colalloc; bp += tabspaces) {
						*bp = 1;
				}
		}
		/* update terminal size */
		term.col = col;
		term.row = row;
		/* reset scrolling region */
		tsetscroll(0, row - 1);
		/* make use of the LIMIT in tmoveto */
		tmoveto(term.c.x, term.c.y);
		/* Clearing both screens (it makes dirty all lines) */
		c = term.c;
		for (i = 0; i < 2; i++) {
				//if ( mincol < term.colalloc && !enlarge )
				if (mincol < term.colalloc && 0 < minrow && enlarge) {
						tclearregion(mincol, 0, term.colalloc - 1, minrow - 1);
				}
				if (0 < col && minrow < row) {
						tclearregion(0, minrow, term.colalloc - 1, row - 1);
				}
				tswapscreen();
				tcursor(CURSOR_LOAD);
		}
		term.c = c;
}



