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

static void tcursor(int);
static void tmoveto(int, int);
static void tmoveato(int, int);
static void treset(void);
static void tswapscreen(void);

static void tdectest(utfchar);
static int32_t tdefcolor(int *, int *, int);


void tty_send_unicode(const Arg *arg){ 
	utfchar c = (utfchar)arg->i;
	ttywrite(&c, 1, 1); 
}

void check_canary(){
	asm( "" : "+m"(term->guard));
	if ( term->guard != 0xf0f0f0f0 )
		die("term->guard modified");
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
	asm( "" : "+m"(term->guard));
	tresize(col,row);
	treset();
}


int tlinelen(int y) {
	int i = term->cols;

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
	term->bot = term->rows - 1;
	term->mode = MODE_WRAP | MODE_UTF8; // not UTF8-> MODE_UTF8 eq 0
	memset(term->trantbl, CS_USA, sizeof(term->trantbl));
	term->charset = 0;

	//for (i = 0; i < 2; i++) {
		tmoveto(0, 0);
		tcursor(CURSOR_SAVE);
		tclearregion(0, 0, term->colalloc - 1, term->rows - 1);
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
			tnew(term->cols, term->rows);
			p_alt = term;
		} else { // p_alt != term
			term = p_alt;
			if ( ( p_term->rows != term->rows ) || ( p_term->cols != term->cols )){
				tresize( p_term->cols, p_term->rows );
			}
		}
		term->mode |= MODE_ALTSCREEN;
	} else {
		term = p_term;
		if ( ( p_alt->rows != term->rows ) || ( p_alt->cols != term->cols ))
			tresize( p_alt->cols, p_alt->rows );
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
			tnew(term->cols, term->rows);
			p_help = term;
			twrite( (utfchar*)helpcontents, strlen(helpcontents), 0 );
		} else {
			p_help_storedterm = term;
			term = p_help;
			if ( ( p_term->rows != term->rows ) || ( p_term->cols != term->cols )){
				tresize( p_term->cols, p_term->rows );
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
				//tnew(term->cols, term->rows);
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
		if ( ( p_help->rows != term->rows ) || ( p_help->cols != term->cols ))
			tresize( p_help->cols, p_help->rows );
	}

	tfulldirt();
}



/* for absolute user moves, when decom is set */
void tmoveato(int x, int y) {
	//printf("tmoveato: %d\n",y);
	

	// delete retmarks, within the region.
	// needed amongst others for screen based programs
	if ( y==0 ){
		int tend = term->current_retmark;
		for ( int t = (term->current_retmark -1 ) & (RETMARKCOUNT-1); 
				(t!=tend) &&
			 	(term->histi < term->retmarks[t] ) && 
				(term->histi + term->rows+1 > term->retmarks[t]);
				t = (t-1) & ( RETMARKCOUNT-1 ) ){
			term->current_retmark = t;
			term->retmarks[ term->current_retmark ] = 0;
		}
		term->scroll_retmark = term->current_retmark;
		
	}

	tmoveto(x, y + ((term->cursor.state & CURSOR_ORIGIN) ? term->top : 0));
}

void tmoveto(int x, int y) {
	int miny, maxy;

	if (term->cursor.state & CURSOR_ORIGIN) {
		miny = term->top;
		maxy = term->bot;
	} else {
		miny = 0;
		maxy = term->rows - 1;
	}
	term->cursor.state &= ~CURSOR_WRAPNEXT;
	term->cursor.x = LIMIT(x, 0, term->cols - 1);
	term->cursor.y = LIMIT(y, miny, maxy);
}
void tdectest(utfchar c) {
	int x, y;

	if (c == '8') { /* DEC screen alignment test. */
		for (x = 0; x < term->cols; ++x) {
			for (y = 0; y < term->rows; ++y) {
				tsetchar('E', &term->cursor.attr, x, y);
			}
		}
	}
}


void tresize(int col, int row) {
	int i;
	int minrow = MIN(row, term->rows);
	int mincol = MIN(col, term->cols);
	int *bp;
	int enlarge = 0;
	TCursor c;
	int oldwidth = term->colalloc;
	if ( col > term->colalloc ){
		term->colalloc = col;
		enlarge = 1;
	}


	if (row < term->rows || col < term->cols) {
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
	for (i += row; i < term->rows; i++) {
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

	/* int oldline = 0;
	int newline = 0;
	int oldcol = 0;
	int newcol = 0;
//	int newhist = !(term->cthist);
//	int oldhist = term->cthist;
	// delay here. Collect resize events
	if ( term->circledhist  ){
		oldline = (term->histi+1 > HISTSIZE ) ? 0 : (term->histi+1);
	}
	dbg2(AC_YELLOW "oldline: %d  term->histi: %d  term->col: %d col: %d" AC_NORM,oldline,term->histi, term->cols, col);
	*/
	term->cursor.attr.u = ' '; 
#if 0

	if ( oldline != term->histi ){
		term->hist[newline] = xmalloc( col * sizeof(Glyph));
		memset32( &term->hist[newline][mincol].intG, term->cursor.attr.intG, col-mincol );
	}

	while (oldline!=term->histi) { // Didn't reach the end of the old history yet
		dbg3( "oldhist: %d term->cols %d newhist %d oldline: %d oldcol: %d newline: %d newcol: %d", oldhist, term->cols,newhist, oldline, oldcol, newline, newcol );
		while( (oldline!=term->histi) && (oldcol < term->cols ) ){ // && !( ( oldcol>0 ) && (term->hist[oldline][oldcol-1].mode & ATTR_WRAP )) ){
			dbg3( "term->col: %d L2: oldline: %d oldcol: %d newline: %d newcol: %d",term->cols, oldline, oldcol, newline, newcol );
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
			if ( oldcol == term->cols ){// && !( ( oldcol>0 ) && (term->hist[oldline][oldcol-1].mode & ATTR_WRAP )) ){
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
			if (col > term->cols) {
				bp = term->tabs + term->cols;

				memset(bp, 0, sizeof(*term->tabs) * (term->colalloc - term->cols));
				while (--bp > term->tabs && !*bp) {
					/* nothing */
				}
				for (bp += tabspaces; bp < term->tabs + term->colalloc; bp += tabspaces) {
					*bp = 1;
				}
			}
		/* update terminal size */
		term->cols = col;
		term->rows = row;
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



