/* See LICENSE for license details. */

#include "term.h"
#include "xwindow.h"
#include "selection.h"
#include "scroll.h"
#include "mem.h"
#include "base64.h"
#include "utf8.h"
#include "statusbar.h"
#include "termdraw.h"

#include "help.h"

#if defined(__linux)
#include <pty.h>
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#include <util.h>
#elif defined(__FreeBSD__) || defined(__DragonFly__)
#include <libutil.h>
#endif

#undef IS_SET
#define IS_SET(flag) ((term->mode & (flag)) != 0)

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
static void strparse(void);
static void strreset(void);

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


void tty_send_unicode(const Arg *arg){ 
	char c = (char)arg->i;
	ttywrite(&c, 1, 1); 
}


// initiate new terminal window and buffers
void tnew(int col, int row) {
	dbg2("tnew *******************************************************\n");
	dbg2("col: %d, row: %d\n",col,row);
	//term = (Term){.c = {.attr = {.fg = defaultfg, .bg = defaultbg}}};
	term = xmalloc(sizeof(Term));
	memset ( term, 0, sizeof(Term) );
	if ( !p_term )
		p_term = term;
	term->cursor = (TCursor){.attr = {.fg = defaultfg, .bg = defaultbg}};

	term->hist[0] = xmalloc( col * sizeof(Glyph));
	memset(term->hist[0],0, col * sizeof(Glyph));

	term->colalloc = 0;

	term->scroll_retmark = 1;
	term->current_retmark = 0;

	term->guard = 0xf0f0f0f0;
	tresize(col,row);
	treset();
}


int tlinelen(int y) {
	int i = term->col;

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
		c[alt] = term->cursor;
	} else if (mode == CURSOR_LOAD) {
		term->cursor = c[alt];
		tmoveto(c[alt].x, c[alt].y);
	}
}

void treset(void) {
	uint i;

	term->cursor = (TCursor){{.mode = ATTR_NULL, .fg = defaultfg, .bg = defaultbg,.u=' '},
		.x = 0,
		.y = 0,
		.state = CURSOR_DEFAULT};

	memset(term->tabs, 0, term->colalloc * sizeof(*term->tabs));
	for (i = tabspaces; i < term->colalloc; i += tabspaces) {
		term->tabs[i] = 1;
	}
	term->top = 0;
	term->bot = term->row - 1;
	term->mode = MODE_WRAP | MODE_UTF8; // not UTF8-> MODE_UTF8 eq 0
	memset(term->trantbl, CS_USA, sizeof(term->trantbl));
	term->charset = 0;

	//for (i = 0; i < 2; i++) {
		tmoveto(0, 0);
		tcursor(CURSOR_SAVE);
		tclearregion(0, 0, term->colalloc - 1, term->row - 1);
		//tswapscreen(); //xxx
	//}
}


int tisaltscr(void) { 
	
	return IS_SET(MODE_ALTSCREEN); 
}

// show alt screen (swap)
void tswapscreen(void) {
#if 0
	SWAPp( term->line, term->alt ); //xxx
	term->mode ^= MODE_ALTSCREEN; //xxx
	//printf("swapscreen, mode: %x\n",term->mode);
	tfulldirt(); 
#else
	// swap the whole term struct
	///printf("swapscreen, mode: %x\n",term->mode);
	if ( p_alt != term ){ // altscr is not visible now
		lessmode_toggle( &(Arg){.i=LESSMODE_OFF} ); 
		if ( !p_alt ){ // displayed first time
			tnew(term->col, term->row);
			p_alt = term;
		} else { // p_alt != term
			term = p_alt;
			if ( ( p_term->row != term->row ) || ( p_term->col != term->col )){
				tresize( p_term->col, p_term->row );
			}
		}
		term->mode |= MODE_ALTSCREEN;
	} else {
		term = p_term;
		if ( ( p_alt->row != term->row ) || ( p_alt->col != term->col ))
			tresize( p_alt->col, p_alt->row );
	}

	tfulldirt();
#endif
}

void inverse_screen(){
	twin.mode ^= MODE_REVERSE;
	redraw();
}

void quithelp( const Arg *a ){
	showhelp( a );
}

void showhelp(const Arg *a) {
	//printf("showhelp\n");

	if ( p_help != term ){ // help is not visible now
								  //p_term = term;
		if ( !p_help ){ // displayed first time
			p_help_storedterm = term;
			tnew(term->col, term->row);
			p_help = term;
			twrite( helpcontents, strlen(helpcontents), 0 );
		} else {
			p_help_storedterm = term;
			term = p_help;
			if ( ( p_term->row != term->row ) || ( p_term->col != term->col )){
				tresize( p_term->col, p_term->row );
				//free(term);	// buggy. There might be some memory leaks.
				// to fix it, I guess it would be best rewriting all this unlucky
				// screen buffer stuff. Separating screen buffer and history clearly 
				// isn't a good idea. It works now. Should have rewritten it from the beginning.
				// Would have been less work, than fiddling around with really hard to get lines of code
				// and bugs.
				// Oh. And just now I realize - could be uclibc (I'm working with) or gcc related
				// as well. I did have already other bugs, obviously compiler or standard libc related.
				// But, well. im going outside, 
				//  sun is shining.
				//
				//tnew(term->col, term->row);
				//p_help = term;
				//twrite( helpcontents, strlen(helpcontents), 0 );
			}
		}
		lessmode_toggle( ARGPi( LESSMODE_ON) );

		help_storedinputmode = inputmode;
		inputmode = inputmode | IMODE_HELP;
		inputmode = inputmode & ~(MODE_LESS);
		//inputmode = inputmode | MODE_LESS | IMODE_HELP;
		term->mode = term->mode | TMODE_HELP;
		//enterscroll(&a);
		showstatus(1,"-HELP- ( q to exit )");

		scrolltotop();

	} else { //help is visible. toggle back to term.
		//inputmode = inputmode & ~(IMODE_HELP);
		inputmode = help_storedinputmode;
		//inputmode = inputmode & ~(MODE_LESS | IMODE_HELP);
		showstatus(0,0);
		term = p_help_storedterm;
		lessmode_toggle( ARGPi( LESSMODE_OFF ) ); // bugs else.
		if ( ( p_help->row != term->row ) || ( p_help->col != term->col ))
			tresize( p_help->col, p_help->row );
	}

	tfulldirt();
}



/* for absolute user moves, when decom is set */
void tmoveato(int x, int y) {
	tmoveto(x, y + ((term->cursor.state & CURSOR_ORIGIN) ? term->top : 0));
}

void tmoveto(int x, int y) {
	int miny, maxy;

	if (term->cursor.state & CURSOR_ORIGIN) {
		miny = term->top;
		maxy = term->bot;
	} else {
		miny = 0;
		maxy = term->row - 1;
	}
	term->cursor.state &= ~CURSOR_WRAPNEXT;
	term->cursor.x = LIMIT(x, 0, term->col - 1);
	term->cursor.y = LIMIT(y, miny, maxy);
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
					MODBIT(term->cursor.state, set, CURSOR_ORIGIN);
					tmoveato(0, 0);
					break;
				case 7: /* DECAWM -- Auto wrap */
					MODBIT(term->mode, set, MODE_WRAP);
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
						tclearregion(0, 0, term->col - 1, term->row - 1);
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
					MODBIT(term->mode, set, MODE_INSERT);
					break;
				case 12: /* SRM -- Send/Receive */
					MODBIT(term->mode, !set, MODE_ECHO);
					break;
				case 20: /* LNM -- Linefeed/new line */
					MODBIT(term->mode, set, MODE_CRLF);
					break;
				default:
					fprintf(stderr, "erresc: unknown set/reset mode %d\n", *args);
					break;
			}
		}
	}
}

void sendbreak(const Arg *arg) {
	if (tcsendbreak(cmdfd, 0)) {
		perror("Error sending break");
	}
}


void tdefutf8(utfchar ascii) {
#ifdef UTF8
	if (ascii == 'G')
		term->mode |= MODE_UTF8;
	else
		if (ascii == '@') {
			term->mode &= ~MODE_UTF8;
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
		term->trantbl[term->icharset] = vcs[p - cs];
	}
}

void tdectest(utfchar c) {
	int x, y;

	if (c == '8') { /* DEC screen alignment test. */
		for (x = 0; x < term->col; ++x) {
			for (y = 0; y < term->row; ++y) {
				tsetchar('E', &term->cursor.attr, x, y);
			}
		}
	}
}


void tresize(int col, int row) {
	int i, j;
	int minrow = MIN(row, term->row);
	int mincol = MIN(col, term->col);
	int *bp;
	int enlarge = 0;
	TCursor c;
	int oldwidth = term->colalloc;
	if ( col > term->colalloc ){
		term->colalloc = col;
		enlarge = 1;
	}


	if (row < term->row || col < term->col) {
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
	for (i = 0; i <= term->cursor.y - row; i++) {
		free(term->line[i]);
		//free(term->alt[i]);
		//free(term->helpscr[i]);
	}
	/* ensure that both src and dst are not NULL */
	if (i > 0) {
		memmove(term->line, term->line + i, row * sizeof(Line));
		//memmove(term->alt, term->alt + i, row * sizeof(Line));
		//memmove(term->helpscr,term->helpscr + i, row * sizeof(Line));
	}
	for (i += row; i < term->row; i++) {
		free(term->line[i]);
		//free(term->alt[i]);
		//free(term->helpscr[i]);
	}

	/* resize to new height */
	term->line = xrealloc(term->line, row * sizeof(Line));
	//term->alt = xrealloc(term->alt, row * sizeof(Line));
	//term->helpscr = xrealloc(term->helpscr, row * sizeof(Line));
	term->dirty = xrealloc(term->dirty, row * sizeof(*term->dirty));
	term->tabs = xrealloc(term->tabs, term->colalloc * sizeof(*term->tabs));

	int oldline = 0;
	int newline = 0;
	int oldcol = 0;
	int newcol = 0;
//	int newhist = !(term->cthist);
//	int oldhist = term->cthist;
	// delay here. Collect resize events
	if ( term->circledhist  ){
		oldline = (term->histi+1 > HISTSIZE ) ? 0 : (term->histi+1);
	}
	term->cursor.attr.u = ' '; 
	dbg2(AC_YELLOW "oldline: %d  term->histi: %d  term->col: %d col: %d" AC_NORM,oldline,term->histi, term->col, col);
#if 0

	if ( oldline != term->histi ){
		term->hist[newline] = xmalloc( col * sizeof(Glyph));
		memset32( &term->hist[newline][mincol].intG, term->cursor.attr.intG, col-mincol );
	}

	while (oldline!=term->histi) { // Didn't reach the end of the old history yet
		dbg3( "oldhist: %d term->col %d newhist %d oldline: %d oldcol: %d newline: %d newcol: %d", oldhist, term->col,newhist, oldline, oldcol, newline, newcol );
		while( (oldline!=term->histi) && (oldcol < term->col) ){ // && !( ( oldcol>0 ) && (term->hist[oldline][oldcol-1].mode & ATTR_WRAP )) ){
			dbg3( "term->col: %d L2: oldline: %d oldcol: %d newline: %d newcol: %d",term->col, oldline, oldcol, newline, newcol );
			//dbg3( "intG oldhist: %d - %d\n", term->hist[oldline][oldcol].intG, term->hist[oldline][oldcol].u );
			if ( term->hist[oldline][oldcol].mode & ATTR_WRAP ){
				dbg2("WRAP");
			}
			term->hist[newline][newcol].intG = term->hist[oldline][oldcol].intG;
			//term->hist[newline][newcol].mode
			oldcol++;
			newcol++;
			if ( ( newcol == col) || ( (oldcol>0) && (term->hist[oldline][oldcol-1].mode & ATTR_WRAP )) ){ // end of line
				dbg3("Eol. newcol: %d  oldcol:%d",newcol,oldcol);
				//term->hist[newline][newcol-1].mode |= ATTR_WRAP;
				newline++;
				newline &= ((1<<HISTSIZEBITS)-1);
				newcol=0;
				if ( !term->hist[newline] ){ 
					term->hist[newline] = xmalloc( col * sizeof(Glyph));
					dbg3(AC_BLUE"malloc: hist %d, line %d, cols: %d"AC_NORM, newhist, newline, col );
				} else {
					dbg3(AC_GREEN"realloc: hist %d, line %d, cols: %d"AC_NORM, newhist, newline, col );
					term->hist[newline] = xrealloc( 	term->hist[newline], col * sizeof(Glyph));
				}
				//dbg3("newline: %d",newline);
				memset32( &term->hist[newline][mincol].intG, term->cursor.attr.intG, col-mincol );
			}
			if ( oldcol == term->col ){// && !( ( oldcol>0 ) && (term->hist[oldline][oldcol-1].mode & ATTR_WRAP )) ){
				dbg3( "YY: newline: %d newcol: %d", newline, newcol );
				//free( term->hist[oldline] );
				oldcol = 0;
				//term->hist[oldline] = 0;
				oldline++;
				oldline &= ((1<<HISTSIZEBITS)-1); // modulo

			}

			}
		}
		term->cthist = newhist;
		dbg2("copied hist. oldhist: %d  term->cthist: %d", oldhist, term->cthist );
#else
		int t = term->histi;
		if ( term->circledhist  ){
			t = HISTSIZE;
		}

		if ( enlarge )
			for (i = 0; i < t; i++) { // 
				term->hist[i] = xrealloc(term->hist[i], term->colalloc * sizeof(Glyph));
#ifndef UTF8
				memset32( &term->hist[i][oldwidth].intG, term->cursor.attr.intG, term->colalloc-oldwidth );
				//memset32( &term->hist[i][mincol].intG, term->cursor.attr.intG, term->colalloc-mincol );
				//for (j = mincol; j < col; j++) {
				//		term->hist[i][j].intG = term->cursor.attr.intG;
				//}
#else
				for (j = mincol; j < term->colalloc; j++) {
					term->hist[i][j] = term->cursor.attr;
					term->hist[i][j].u = ' '; 
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
				term->line[i] = xrealloc(term->line[i], term->colalloc * sizeof(Glyph));
				//dbg3("i2\n");
				//term->alt[i] = xrealloc(term->alt[i], term->colalloc * sizeof(Glyph));
				//term->helpscr[i] = xrealloc(term->helpscr[i], term->colalloc * sizeof(Glyph));
			}
		} else { // don't shrink the width.
			for (i = 0; i < minrow; i++) {

				//term->alt[i] = xrealloc(term->alt[i], term->colalloc * sizeof(Glyph));
				//dbg3("i: %d, %d", i, minrow );
				//term->line[i] = xrealloc(term->line[i], col * sizeof(Glyph));
				//dbg3("i2\n");
				//term->alt[i] = xrealloc(term->alt[i], col * sizeof(Glyph));
				//memset(term->alt[i], 0, col);
			}
		}



		dbg3("i4\n");
		/* allocate any new rows */
		for ( i = minrow ; i < row; i++) {
			term->line[i] = xmalloc(term->colalloc * sizeof(Glyph));
			memset(term->line[i], 0, sizeof(Glyph) * term->colalloc);
		}
		/*if ( term->colalloc > col ){
		  if ( minrow < row )
		  term->colalloc = col;
		  } else {
		  if ( col > term->colalloc )
		  term->colalloc = col;
		  }*/

		if ( enlarge )
			if (col > term->col) {
				bp = term->tabs + term->col;

				memset(bp, 0, sizeof(*term->tabs) * (term->colalloc - term->col));
				while (--bp > term->tabs && !*bp) {
					/* nothing */
				}
				for (bp += tabspaces; bp < term->tabs + term->colalloc; bp += tabspaces) {
					*bp = 1;
				}
			}
		/* update terminal size */
		term->col = col;
		term->row = row;
		/* reset scrolling region */
		tsetscroll(0, row - 1);
		/* make use of the LIMIT in tmoveto */
		tmoveto(term->cursor.x, term->cursor.y);
		/* Clearing both screens (it makes dirty all lines) */
		c = term->cursor;
		//for (i = 0; i < 2; i++) {
			//if ( mincol < term->colalloc && !enlarge )
			if (mincol < term->colalloc && 0 < minrow && enlarge) {
				tclearregion(mincol, 0, term->colalloc - 1, minrow - 1);
			}
			if (0 < col && minrow < row) {
				tclearregion(0, minrow, term->colalloc - 1, row - 1);
			}
			//tswapscreen(); //xxx
			//tcursor(CURSOR_LOAD);
		//}
		term->cursor = c;

		if ( !enlarge )
			tfulldirt();

}



