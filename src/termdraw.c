#include "includes.h"

#include "termdraw.h"
#include "utf8.h"
#include "st.h"

/* macros */
#define ISCONTROLC0(c) (BETWEEN(c, 0, 0x1f) || (c) == 0x7f) // 0x7f = delete who tf did write '\177'??
//#define ISCONTROLC1(c) (BETWEEN(c, 0x80, 0x9f))
//#define ISCONTROL(c) ((c <= 0x1f) || BETWEEN(c, 0x7f, 0x9f))


#define ISCONTROL(c) ((c <= 0x1f) || (c==0x7f))

#define ISCONTROLC1(c) (0)
//#define ISCONTROL(c) (0)



// exclusively in drawregion the real drawing( line per line, with xdrawline ) 
// of the terminal (buffer) contents (without cursor) is done.
void drawregion(int x1, int y1, int x2, int y2) {
		int y;
		for (y = y1; y < y2; y++) {
				if (!term->dirty[y]) {
						continue;
				}
				term->dirty[y] = 0;
				xdrawline(TLINE(y), x1, y, x2);
		}
				// statusline
		if ( y==term->row && statusvisible ){
				xdrawline(statusbar, x1, y-1, x2);
		}
}


void draw(void) {
		int cx = term->c.x;

		if (!xstartdraw()) {
				return;
		}

		/* adjust cursor position */
		LIMIT(term->ocx, 0, term->col - 1);
		LIMIT(term->ocy, 0, term->row - 1);

#ifdef UTF8
		if (term->line[term->ocy][term->ocx].mode & ATTR_WDUMMY) {
				term->ocx--;
		}
		if (term->line[term->c.y][cx].mode & ATTR_WDUMMY) {
				cx--;
		}
#endif 

		drawregion(0, 0, term->col, term->row);
		if (term->scr == 0) {
				xdrawcursor(cx, term->c.y, term->line[term->c.y][cx], term->ocx, term->ocy,
								term->line[term->ocy][term->ocx]);
		}
		term->ocx = cx, term->ocy = term->c.y;
		xfinishdraw();
		xximspot(term->ocx, term->ocy);
}

void redraw(void) {
		tfulldirt();
		draw();
}



int twrite(const utfchar *buf, int buflen, int show_ctrl) {
		int charsize;
		Rune u;
		int n;

		dbg("twrite0 buflen: %d buf[0]: %c  show_ctrl: %d", buflen, buf[0], show_ctrl);

#ifdef UTF8
		for (n = 0; n < buflen; n += charsize) { // misc dfq
				if (IS_SET(MODE_UTF8) && !IS_SET(MODE_SIXEL)) {
						// process a complete utf8 char
						charsize = utf8decode(buf + n, &u, buflen - n);
						if (charsize == 0)
								break;
				} else  {
						u = buf[n] & 0xFF;
						charsize = 1;
				}
#else
		for (n = 0; n < buflen; n++ ) { // misc dfq
					u = buf[n];
#endif
				dbg("twrite1 %d %c", u, u);
				if (show_ctrl && ISCONTROL(u)) {
						dbg("twrite ISCONTROL %d %c", u, u);
						if (u & 0x80) {
								u &= 0x7f;
								tputc('^');
								tputc('[');
						} else if (u != '\n' && u != '\r' && u != '\t') {
								u ^= 0x40;
								tputc('^');
						}
				}
				tputc(u);
		}
		dbg("ok\n");
		return n;
}


void tputtab(int n) {
		uint x = term->c.x;

		if (n > 0) {
				while (x < term->col && n--) {
						for (++x; x < term->col && !term->tabs[x]; ++x) {
								/* nothing */
						}
				}
		} else if (n < 0) {
				while (x > 0 && n++) {
						for (--x; x > 0 && !term->tabs[x]; --x) {
								/* nothing */
						}
				}
		}
		term->c.x = LIMIT(x, 0, term->col - 1);
}

void tputc(Rune u) {
		char c[UTF_SIZ];
		int control;
		int width, len;
		Glyph *gp;
//	if ( u>=0x80 )
//		printf("r: %x\n",u);


		control = ISCONTROL(u);
#ifndef UTF8
		c[0] = u;
		width = len = 1;
#else 
		if (!IS_SET(MODE_UTF8) && !IS_SET(MODE_SIXEL)) {
				c[0] = u;
				width = len = 1;
		} else {
				len = utf8encode(u, c);
				if (!control && (width = wcwidth(u)) == -1) {
						memcpy(c, "\357\277\275", 4); // UTF_INVALID
						width = 1;
				}
		}
#endif

		if (IS_SET(MODE_PRINT)) {
				tprinter(c, len);
		}

		/*
		 * STR sequence must be checked before anything else
		 * because it uses all following characters until it
		 * receives a ESC, a SUB, a ST or any other C1 control
		 * character.
		 */
		if (term->esc & ESC_STR) {
				if (u == '\a' || u == 030 || u == 032 || u == 033 || ISCONTROLC1(u)) {
						term->esc &= ~(ESC_START | ESC_STR | ESC_DCS);
						if (IS_SET(MODE_SIXEL)) {
								/* TODO: render sixel */
								term->mode &= ~MODE_SIXEL;
								return;
						}
						term->esc |= ESC_STR_END;
						goto check_control_code;
				}

				if (IS_SET(MODE_SIXEL)) {
						/* TODO: implement sixel mode */
						return;
				}
				if (term->esc & ESC_DCS && strescseq.len == 0 && u == 'q') {
						term->mode |= MODE_SIXEL;
				}

				if (strescseq.len + len >= strescseq.siz) {
						/*
						 * Here is a bug in terminals. If the user never sends
						 * some code to stop the str or esc command, then st
						 * will stop responding. But this is better than
						 * silently failing with unknown characters. At least
						 * then users will report back.
						 *
						 * In the case users ever get fixed, here is the code:
						 */
						/*
						 * term->esc = 0;
						 * strhandle();
						 */
						if (strescseq.siz > (SIZE_MAX - UTF_SIZ) / 2) {
								return;
						}
						strescseq.siz *= 2;
						strescseq.buf = xrealloc(strescseq.buf, strescseq.siz);
				}

				memmove(&strescseq.buf[strescseq.len], c, len);
				strescseq.len += len;
				return;
		}

check_control_code:
		/*
		 * Actions of control codes must be performed as soon they arrive
		 * because they can be embedded inside a control sequence, and
		 * they must not cause conflicts with sequences.
		 */
		if (control) {
//	if ( u>=0x80 )
//		printf("cont r: %x\n",u);

				tcontrolcode(u);
				/*
				 * control codes are not shown ever
				 */
				return;
		} else if (term->esc & ESC_START) {
				if (term->esc & ESC_CSI) {
						csiescseq.buf[csiescseq.len++] = u;
						if (BETWEEN(u, 0x40, 0x7E) ||
										csiescseq.len >= sizeof(csiescseq.buf) - 1) {
								term->esc = 0;
								csiparse();
								csihandle();
						}
						return;
				} else if (term->esc & ESC_UTF8) {
						tdefutf8(u);
				} else if (term->esc & ESC_ALTCHARSET) {
						tdeftran(u);
				} else if (term->esc & ESC_TEST) {
						tdectest(u);
				} else {
						if (!eschandle(u)) {
								return;
						}
						/* sequence already finished */
				}
				term->esc = 0;
				/*
				 * All characters which form part of a sequence are not
				 * printed
				 */
				return;
		}
		if (sel.ob.x != -1 && BETWEEN(term->c.y, sel.ob.y, sel.oe.y)) {
				selclear();
		}

		gp = &term->line[term->c.y][term->c.x];
		if (IS_SET(MODE_WRAP) && (term->c.state & CURSOR_WRAPNEXT)) {
				gp->mode |= ATTR_WRAP; //misc wrapping here NHIST
				tnewline(1);
				gp = &term->line[term->c.y][term->c.x];
		}

		if (IS_SET(MODE_INSERT) && term->c.x + width < term->col) {
				memmove(gp + width, gp, (term->col - term->c.x - width) * sizeof(Glyph));
		}

		if (term->c.x + width > term->col) {
				tnewline(1);  // NHIST
				gp = &term->line[term->c.y][term->c.x];
		}

//	if ( u>=0x80 )
//		printf("set r: %x\n",u);

		// NHIST
		tsetchar(u, &term->c.attr, term->c.x, term->c.y);

#ifdef UTF8
		if (width == 2) {
				dbg2("tputchar width2: %x %c", gp[0].u, gp[0].u );
				gp->mode |= ATTR_WIDE;
				if (term->c.x + 1 < term->col) {
						gp[1].u = '\0';
						gp[1].mode = ATTR_WDUMMY;
				}
		}
#endif

		if (term->c.x + width < term->col) {
				tmoveto(term->c.x + width, term->c.y);
		} else {
				term->c.state |= CURSOR_WRAPNEXT;
		}
}
#if 0
void histputc(utfchar c){
		gp = &term->line[term->c.y][term->c.x];
		if (IS_SET(MODE_WRAP) && (term->c.state & CURSOR_WRAPNEXT)) {
				gp->mode |= ATTR_WRAP;
				tnewline(1);
				gp = &term->line[term->c.y][term->c.x];
		}

		if (IS_SET(MODE_INSERT) && term->c.x + width < term->col) {
				memmove(gp + width, gp, (term->col - term->c.x - width) * sizeof(Glyph));
		}

		if (term->c.x + width > term->col) {
				tnewline(1);
				gp = &term->line[term->c.y][term->c.x];
		}

		tsetchar(u, &term->c.attr, term->c.x, term->c.y);


#ifdef UTF8
		if (width == 2) {
				dbg2("tputchar width2: %x %c", gp[0].u, gp[0].u );
				gp->mode |= ATTR_WIDE;
				if (term->c.x + 1 < term->col) {
						gp[1].u = '\0';
						gp[1].mode = ATTR_WDUMMY;
				}
		}
#endif

		if (term->c.x + width < term->col) {
				tmoveto(term->c.x + width, term->c.y);
		} else {
				term->c.state |= CURSOR_WRAPNEXT;
		}
}
#endif



// put a char onto screen
void tsetchar(Rune u, Glyph *attr, int x, int y) {
#ifndef UTF8
	//if ( u>=0x80 )
	//	printf("r: %x\n",u);
		term->dirty[y] = 1;
		term->line[y][x] = *attr; 
		term->line[y][x].u = u;
		return;
#else
		static char *vt100_0[62] = {
				/* 0x41 - 0x7e */
				"↑", "↓", "→", "←", "█", "▚", "☃",      /* A - G */
				0,   0,   0,   0,   0,   0,   0,   0,   /* H - O */
				0,   0,   0,   0,   0,   0,   0,   0,   /* P - W */
				0,   0,   0,   0,   0,   0,   0,   " ", /* X - _ */
				"◆", "▒", "␉", "␌", "␍", "␊", "°", "±", /* ` - g */
				"␤", "␋", "┘", "┐", "┌", "└", "┼", "⎺", /* h - o */
				"⎻", "─", "⎼", "⎽", "├", "┤", "┴", "┬", /* p - w */
				"│", "≤", "≥", "π", "≠", "£", "·",      /* x - ~ */
		};

		/*
		 * The table is proudly stolen from rxvt.
		 */
		if (term->trantbl[term->charset] == CS_GRAPHIC0 &&
						BETWEEN(u, 0x41, 0x7e) && vt100_0[u - 0x41])
				utf8decode(vt100_0[u - 0x41], &u, UTF_SIZ);

		if (term->line[y][x].mode & ATTR_WIDE) {
				dbg2("Attr_wide: %c\n",term->line[y][x].u);
				if (x + 1 < term->col) {
						term->line[y][x + 1].u = ' ';
						term->line[y][x + 1].mode &= ~ATTR_WDUMMY;
				}
		} else if (term->line[y][x].mode & ATTR_WDUMMY) {
				dbg2("Attr_dummy: %c\n",term->line[y][x].u);
				term->line[y][x - 1].u = ' ';
				term->line[y][x - 1].mode &= ~ATTR_WIDE;
		}
		term->dirty[y] = 1;
		term->line[y][x] = *attr; //misc optimize here
		term->line[y][x].u = u;
#endif
}

void tclearregion(int x1, int y1, int x2, int y2) {
		int x, y;
		Glyph *gp;

		if (x1 > x2) {
				SWAPint(x1,x2);
		}
		if (y1 > y2) {
				SWAPint(y1,y2);
		}

		LIMIT(x1, 0, term->col - 1);
		LIMIT(x2, 0, term->col - 1);
		LIMIT(y1, 0, term->row - 1);
		LIMIT(y2, 0, term->row - 1);

		selclear(); // only call once.
		//term->c.attr.u=' ';

		for (y = y1; y <= y2; y++) { 
				term->dirty[y] = 1;
#ifndef UTF8
				dbg("y: %d, x1: %d, x2: %d\n", y, x1, x2);
				memset32( &term->line[y][x1].intG, term->c.attr.intG, (x2-x1)+1 ); // memset64 or comp
				dbg("ok\n");
#else
				for (x = x1; x <= x2; x++) {//misc copy longs (64bit)or,better: memset. mmx/sse?
						//if (selected(x, y)) { // room for optimization. only ask once, when no selection
						//		selclear();
						//}
						gp = &term->line[y][x];
						gp->fg = term->c.attr.fg;
						gp->bg = term->c.attr.bg;
						gp->mode = 0;
						gp->u = ' ';
				}
#endif
		}
}

void tdeletechar(int n) {
		int dst, src, size;
		Glyph *line;

		LIMIT(n, 0, term->col - term->c.x);

		dst = term->c.x;
		src = term->c.x + n;
		size = term->col - src;
		line = term->line[term->c.y];

		memmove(&line[dst], &line[src], size * sizeof(Glyph));
		tclearregion(term->col - n, term->c.y, term->col - 1, term->c.y);
}

void tinsertblank(int n) {
		int dst, src, size;
		Glyph *line;

		LIMIT(n, 0, term->col - term->c.x);

		dst = term->c.x + n;
		src = term->c.x;
		size = term->col - dst;
		line = term->line[term->c.y];

		memmove(&line[dst], &line[src], size * sizeof(Glyph));
		tclearregion(src, term->c.y, dst - 1, term->c.y);
}

void tinsertblankline(int n) {
		if (BETWEEN(term->c.y, term->top, term->bot)) {
				tscrolldown(term->c.y, n, 0);
		}
}

void tdeleteline(int n) {
		if (BETWEEN(term->c.y, term->top, term->bot)) {
				tscrollup(term->c.y, n, 0);
		}
}


void tfulldirt(void) { tsetdirt(0, term->row - 1); }


void tsetdirtattr(int attr) {
		int i, j;

		for (i = 0; i < term->row - 1; i++) {
				for (j = 0; j < term->col - 1; j++) {
						if (term->line[i][j].mode & attr) {
								tsetdirt(i, i);
								break;
						}
				}
		}
}


int tattrset(int attr) {
		int i, j;

		for (i = 0; i < term->row - 1; i++) {
				for (j = 0; j < term->col - 1; j++) {
						if (term->line[i][j].mode & attr) {
								return 1;
						}
				}
		}

		return 0;
}

void tsetdirt(int top, int bot) {
		int i;

		LIMIT(top, 0, term->row - 1);
		LIMIT(bot, 0, term->row - 1);

		for (i = top; i <= bot; i++) {
				term->dirty[i] = 1;
		}
}



int32_t tdefcolor(int *attr, int *npar, int l) {
		int32_t idx = -1;
		uint r, g, b;

		switch (attr[*npar + 1]) {
				case 2: /* direct color in RGB space */
						if (*npar + 4 >= l) {
								fprintf(stderr, "erresc(38): Incorrect number of parameters (%d)\n",
												*npar);
								break;
						}
						r = attr[*npar + 2];
						g = attr[*npar + 3];
						b = attr[*npar + 4];
						*npar += 4;
						if (!BETWEEN(r, 0, 255) || !BETWEEN(g, 0, 255) || !BETWEEN(b, 0, 255)) {
								fprintf(stderr, "erresc: bad rgb color (%u,%u,%u)\n", r, g, b);
						} else {
								idx = TRUECOLOR(r, g, b);
						}
						break;
				case 5: /* indexed color */
						if (*npar + 2 >= l) {
								fprintf(stderr, "erresc(38): Incorrect number of parameters (%d)\n",
												*npar);
								break;
						}
						*npar += 2;
						if (!BETWEEN(attr[*npar], 0, 255)) {
								fprintf(stderr, "erresc: bad fgcolor %d\n", attr[*npar]);
						} else {
						//	fprintf(stderr,"color: %d\n",attr[*npar]); //D
								idx = attr[*npar];
						//		printf("idx: %x\n",idx); //D
						}
						break;
				case 0: /* implemented defined (only foreground) */
				case 1: /* transparent */
				case 3: /* direct color in CMY space */
				case 4: /* direct color in CMYK space */
				default:
						fprintf(stderr, "erresc(38): gfx attr %d unknown\n", attr[*npar]);
						break;
		}

		return idx;
}

void tsetattr(int *attr, int l) {
		int i;
		int32_t idx;

		for (i = 0; i < l; i++) {
				switch (attr[i]) {
						case 0:
								term->c.attr.mode &=
										~(ATTR_BOLD | ATTR_FAINT | ATTR_ITALIC | ATTR_UNDERLINE | ATTR_BLINK |
														ATTR_REVERSE | ATTR_INVISIBLE | ATTR_STRUCK);
								term->c.attr.fg = defaultfg;
								term->c.attr.bg = defaultbg;
								break;
						case 1:
								term->c.attr.mode |= ATTR_BOLD;
								break;
						case 2:
								term->c.attr.mode |= ATTR_FAINT;
								break;
						case 3:
								term->c.attr.mode |= ATTR_ITALIC;
								break;
						case 4:
								term->c.attr.mode |= ATTR_UNDERLINE;
								break;
						case 5: /* slow blink */
								/* FALLTHROUGH */
						case 6: /* rapid blink */
								term->c.attr.mode |= ATTR_BLINK;
								break;
						case 7:
								term->c.attr.mode |= ATTR_REVERSE;
								break;
						case 8:
								term->c.attr.mode |= ATTR_INVISIBLE;
								break;
						case 9:
								term->c.attr.mode |= ATTR_STRUCK;
								break;
						case 22:
								term->c.attr.mode &= ~(ATTR_BOLD | ATTR_FAINT);
								break;
						case 23:
								term->c.attr.mode &= ~ATTR_ITALIC;
								break;
						case 24:
								term->c.attr.mode &= ~ATTR_UNDERLINE;
								break;
						case 25:
								term->c.attr.mode &= ~ATTR_BLINK;
								break;
						case 27:
								term->c.attr.mode &= ~ATTR_REVERSE;
								break;
						case 28:
								term->c.attr.mode &= ~ATTR_INVISIBLE;
								break;
						case 29:
								term->c.attr.mode &= ~ATTR_STRUCK;
								break;
						case 38:
								if ((idx = tdefcolor(attr, &i, l)) >= 0) {
										term->c.attr.fg = idx;
								}
								break;
						case 39:
								term->c.attr.fg = defaultfg;
								break;
						case 48:
								if ((idx = tdefcolor(attr, &i, l)) >= 0) {
										term->c.attr.bg = idx;
								}
								break;
						case 49:
								term->c.attr.bg = defaultbg;
								break;
						default:
								if (BETWEEN(attr[i], 30, 37)) {
										term->c.attr.fg = attr[i] - 30;
								} else if (BETWEEN(attr[i], 40, 47)) {
										term->c.attr.bg = attr[i] - 40;
								} else if (BETWEEN(attr[i], 90, 97)) {
										term->c.attr.fg = attr[i] - 90 + 8;
								} else if (BETWEEN(attr[i], 100, 107)) {
										term->c.attr.bg = attr[i] - 100 + 8;
								} else {
										fprintf(stderr, "erresc(default): gfx attr %d unknown\n", attr[i]);
										csidump();
								}
								break;
				}
		}
}


void printscreen(const Arg *arg) { tdump(); }

void tprinter(char *s, size_t len) {
		if (iofd != -1 && xwrite(iofd, s, len) < 0) {
				perror("Error writing to output file");
				close(iofd);
				iofd = -1;
		}
}

void toggleprinter(const Arg *arg) { term->mode ^= MODE_PRINT; }


void tdumpline(int n) {
		char buf[UTF_SIZ];
		Glyph *bp, *end;

		bp = &term->line[n][0];
		end = &bp[MIN(tlinelen(n), term->col) - 1];
		if (bp != end || bp->u != ' ') {
				for (; bp <= end; ++bp) {
						tprinter(buf, utf8encode(bp->u, buf));
				}
		}
		tprinter("\n", 1);
}

void tdump(void) {
		int i;

		for (i = 0; i < term->row; ++i) {
				tdumpline(i);
		}
}

