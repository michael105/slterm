// selection.c
//
// selection mode, and mouse selection

#include "xwindow.h"

#include "term.h"


#include <X11/X.h>
#include <X11/keysym.h>


#include "utf8.h"
#include "selection.h"
#include "termdraw.h"
#include "mem.h"

#include "debug.h"


Selection sel;

static int sel_savedcursor;


void keyboard_select(const Arg *dummy) {
	twin.mode ^= trt_kbdselect(-1, NULL, 0);
	sel_savedcursor = xgetcursor();
	xsetcursor(11); // empty block
}


void selstart(int col, int row, int snap) {
	selclear();
	sel.mode = SEL_EMPTY;
	sel.type = SEL_REGULAR;
	sel.alt = IS_SET(MODE_ALTSCREEN);
	sel.snap = snap;
	sel.oe.x = sel.ob.x = col;
	sel.oe.y = sel.ob.y = row;
	selnormalize();

	if (sel.snap != 0) {
		sel.mode = SEL_READY;
	}
	xsetcursor(2); // block
	tsetdirt(sel.nb.y, sel.ne.y);
}


void mousesel(XEvent *e, int done) {
	int type, seltype = SEL_REGULAR;
	uint state = e->xbutton.state & ~(Button1Mask | forcemousemod);

	for (type = 1; type < LEN(selmasks); ++type) {
		if (match(selmasks[type], state)) {
			seltype = type;
			break;
		}
	}
	selextend(evcol(e), evrow(e), seltype, done);
	if (done)
		setsel(getsel(), e->xbutton.time);
}


void selextend(int col, int row, int type, int done) {
	int oldey, oldex, oldsby, oldsey, oldtype;

	if (sel.mode == SEL_IDLE) {
		return;
	}
	if (done && sel.mode == SEL_EMPTY) {
		selclear();
		return;
	}

	oldey = sel.oe.y;
	oldex = sel.oe.x;
	oldsby = sel.nb.y;
	oldsey = sel.ne.y;
	oldtype = sel.type;

	sel.oe.x = col;
	sel.oe.y = row;
	selnormalize();
	sel.type = type;

	if (oldey != sel.oe.y || oldex != sel.oe.x || oldtype != sel.type ||
			sel.mode == SEL_EMPTY) {
		tsetdirt(MIN(sel.nb.y, oldsby), MAX(sel.ne.y, oldsey));
	}

	sel.mode = done ? SEL_IDLE : SEL_READY;
}


void selclear(void) {
	if (sel.ob.x == -1) {
		return;
	}
	sel.mode = SEL_IDLE;
	sel.ob.x = -1;
	tsetdirt(sel.nb.y, sel.ne.y);
}


void select_or_drawcursor(int selectsearch_mode, int type) {
	int done = 0;

	if (selectsearch_mode & 1) {
		selextend(term->cursor.x, term->cursor.y, type, done);
		xsetsel(getsel());
	} else { //  
		xdrawcursor(term->cursor.x, term->cursor.y, TLINE(term->cursor.y)[term->cursor.x], term->ocx,
				term->ocy, TLINE(term->ocy)[term->ocx]);
	}
}

void selinit(void) {
	sel.mode = SEL_IDLE;
	sel.snap = 0;
	sel.ob.x = -1;
}


void selnormalize(void) {
	int i;

	if (sel.type == SEL_REGULAR && sel.ob.y != sel.oe.y) {
		sel.nb.x = sel.ob.y < sel.oe.y ? sel.ob.x : sel.oe.x;
		sel.ne.x = sel.ob.y < sel.oe.y ? sel.oe.x : sel.ob.x;
	} else {
		sel.nb.x = MIN(sel.ob.x, sel.oe.x);
		sel.ne.x = MAX(sel.ob.x, sel.oe.x);
	}
	sel.nb.y = MIN(sel.ob.y, sel.oe.y);
	sel.ne.y = MAX(sel.ob.y, sel.oe.y);

	selsnap(&sel.nb.x, &sel.nb.y, -1);
	selsnap(&sel.ne.x, &sel.ne.y, +1);

	/* expand selection over line breaks */
	if (sel.type == SEL_RECTANGULAR) {
		return;
	}
	i = tlinelen(sel.nb.y);
	if (i < sel.nb.x) {
		sel.nb.x = i;
	}
	if (tlinelen(sel.ne.y) <= sel.ne.x) {
		sel.ne.x = term->col - 1;
	}
}

int selected(int x, int y) {
	if (sel.mode == SEL_EMPTY || sel.ob.x == -1 ||
			sel.alt != IS_SET(MODE_ALTSCREEN)) {
		return 0;
	}

	if (sel.type == SEL_RECTANGULAR) {
		return BETWEEN(y, sel.nb.y, sel.ne.y) && BETWEEN(x, sel.nb.x, sel.ne.x);
	}

	return BETWEEN(y, sel.nb.y, sel.ne.y) && (y != sel.nb.y || x >= sel.nb.x) &&
		(y != sel.ne.y || x <= sel.ne.x);
}

void search(int selectsearch_mode, Rune *target, int ptarget, int incr,
		int type, TCursor *cu) {
	Rune *r;
	int i, bound = (term->col * cu->y + cu->x) * (incr > 0) + incr;

	for (i = term->col * term->cursor.y + term->cursor.x + incr; i != bound; i += incr) {
		for (r = target; r - target < ptarget; r++) {
			if (*r ==
					term->line[(i + r - target) / term->col][(i + r - target) % term->col]
					.u) {
				if (r - target == ptarget - 1) {
					break;
				}
			} else {
				r = NULL;
				break;
			}
		}
		if (r != NULL) {
			break;
		}
	}

	if (i != bound) {
		term->cursor.y = i / term->col, term->cursor.x = i % term->col;
		select_or_drawcursor(selectsearch_mode, type);
	}
}

void selsnap(int *x, int *y, int direction) {
	int newx, newy, xt, yt;
	int delim, prevdelim;
	Glyph *gp, *prevgp;

	switch (sel.snap) {
		case SNAP_WORD:
			/*
			 * Snap around if the word wraps around at the end or
			 * beginning of a line.
			 */
			prevgp = &TLINE(*y)[*x];
			prevdelim = ISDELIM(prevgp->u);
			for (;;) {
				newx = *x + direction;
				newy = *y;
				if (!BETWEEN(newx, 0, term->col - 1)) {
					newy += direction;
					newx = (newx + term->col) % term->col;
					if (!BETWEEN(newy, 0, term->row - 1)) {
						break;
					}

					if (direction > 0) {
						yt = *y, xt = *x;
					} else {
						yt = newy, xt = newx;
					}
					if (!(TLINE(yt)[xt].mode & ATTR_WRAP)) {
						break;
					}
				}

				if (newx >= tlinelen(newy)) {
					break;
				}

				gp = &TLINE(newy)[newx];
				delim = ISDELIM(gp->u);
#ifdef UTF8
				if (!(gp->mode & ATTR_WDUMMY) &&
						(delim != prevdelim || (delim && gp->u != prevgp->u))) {
					break;
				}
#else
				if (delim != prevdelim || (delim && gp->u != prevgp->u)) {
					break;
				}
#endif



				*x = newx;
				*y = newy;
				prevgp = gp;
				prevdelim = delim;
			}
			break;
		case SNAP_LINE:
			/*
			 * Snap around if the the previous line or the current one
			 * has set ATTR_WRAP at its end. Then the whole next or
			 * previous line will be selected.
			 */
			*x = (direction < 0) ? 0 : term->col - 1;
			if (direction < 0) {
				for (; *y > 0; *y += direction) {
					if (!(TLINE(*y - 1)[term->col - 1].mode & ATTR_WRAP)) {
						break;
					}
				}
			} else if (direction > 0) {
				for (; *y < term->row - 1; *y += direction) {
					if (!(TLINE(*y)[term->col - 1].mode & ATTR_WRAP)) {
						break;
					}
				}
			}
			break;
	}
}

char *getsel(void) {
	char *str, *ptr;
	int y, bufsize, lastx, linelen;
	Glyph *gp, *last;

	if (sel.ob.x == -1) {
		return NULL;
	}

	bufsize = (term->col + 1) * (sel.ne.y - sel.nb.y + 1) * UTF_SIZ;
	ptr = str = xmalloc(bufsize);

	/* append every set & selected glyph to the selection */
	for (y = sel.nb.y; y <= sel.ne.y; y++) {
		if ((linelen = tlinelen(y)) == 0) {
			*ptr++ = '\n';
			continue;
		}

		if (sel.type == SEL_RECTANGULAR) {
			gp = &TLINE(y)[sel.nb.x];
			lastx = sel.ne.x;
		} else {
			gp = &TLINE(y)[sel.nb.y == y ? sel.nb.x : 0];
			lastx = (sel.ne.y == y) ? sel.ne.x : term->col - 1;
		}
		last = &TLINE(y)[MIN(lastx, linelen - 1)];
		while (last >= gp && last->u == ' ') {
			--last;
		}

		for (; gp <= last; ++gp) {
#ifdef UTF8
			if (gp->mode & ATTR_WDUMMY) {
				continue;
			}
			ptr += utf8encode(gp->u, ptr);
#else
			*ptr = gp->u;
			ptr++;
#endif
		}


		/*
		 * Copy and pasting of line endings is inconsistent
		 * in the inconsistent terminal and GUI world.
		 * The best solution seems like to produce '\n' when
		 * something is copied from st and convert '\n' to
		 * '\r', when something to be pasted is received by
		 * st.
		 * FIXME: Fix the computer world.
		 */
		if ((y < sel.ne.y || lastx >= linelen) && !(last->mode & ATTR_WRAP)) {
			*ptr++ = '\n';
		}
	}
	*ptr = 0;
	return str;
}


void selscroll(int orig, int n) {
	if (sel.ob.x == -1) {
		return;
	}

	if (BETWEEN(sel.ob.y, orig, term->bot) || BETWEEN(sel.oe.y, orig, term->bot)) {
		if ((sel.ob.y += n) > term->bot || (sel.oe.y += n) < term->top) {
			selclear();
			return;
		}
		if (sel.type == SEL_RECTANGULAR) {
			if (sel.ob.y < term->top) {
				sel.ob.y = term->top;
			}
			if (sel.oe.y > term->bot) {
				sel.oe.y = term->bot;
			}
		} else {
			if (sel.ob.y < term->top) {
				sel.ob.y = term->top;
				sel.ob.x = 0;
			}
			if (sel.oe.y > term->bot) {
				sel.oe.y = term->bot;
				sel.oe.x = term->col;
			}
		}
		selnormalize();
	}
}


int trt_kbdselect(KeySym ksym, char *buf, int len) {
	static TCursor cu;
	static Rune target[64];
	static int type = 1, ptarget, in_use;
	static int sens, quant;
	static char selectsearch_mode;
	int i, bound, *xy;

	if (selectsearch_mode & 2) { // never ?? misc
		dbg("selectsearch_mode & 2\n");
		if (ksym == XK_Return) {
			selectsearch_mode ^= 2;
			set_notifmode(selectsearch_mode, -2);
			if (ksym == XK_Escape) {
				ptarget = 0;
			}
			return 0;
		} else if (ksym == XK_BackSpace) {
			if (!ptarget) {
				return 0;
			}
			term->line[term->bot][ptarget--].u = ' ';
		} else if (len < 1) {
			return 0;
		} else if (ptarget == term->col || ksym == XK_Escape) {
			return 0;
		} else {
			utf8decode(buf, &target[ptarget++], len);
			term->line[term->bot][ptarget].u = target[ptarget - 1];
		}

		if (ksym != XK_BackSpace) {
			search(selectsearch_mode, &target[0], ptarget, sens, type, &cu);
		}

		term->dirty[term->bot] = 1;
		drawregion(0, term->bot, term->col, term->bot + 1);
		return 0;
	}

	switch (ksym) {
		case -1:
			in_use = 1;
			cu.x = term->cursor.x, cu.y = term->cursor.y;
			set_notifmode(0, ksym);
			return MODE_KBDSELECT;
		case XK_s:
		case XK_v:
		case XK_V:
			if (selectsearch_mode & 1) {
				selclear();
				xsetcursor(11); // empty block
			} else {
				selstart(term->cursor.x, term->cursor.y, 0);
			}
			set_notifmode(selectsearch_mode ^= 1, ksym);
#if 1
			if ( ksym == XK_V ){
				if ( type ^ 3 ){
					type ^= 3;
					selextend(term->cursor.x, term->cursor.y, type, i = 0); /* 2 fois */
					selextend(term->cursor.x, term->cursor.y, type, i = 0); /* 2 fois */
				}
			} 
			if ( ksym == XK_v ){ 
				if ( !(type ^ 3) ){
					type ^= 3;
					selextend(term->cursor.x, term->cursor.y, type, i = 0); /* 2 fois */
					selextend(term->cursor.x, term->cursor.y, type, i = 0); /* 2 fois */
				}
			}   // well. patched. works.
				 // type is static. and this enum doesn't seem to , whatever.
#endif
			break;
		case XK_t:
			selextend(term->cursor.x, term->cursor.y, type ^= 3, i = 0); /* 2 fois */
			selextend(term->cursor.x, term->cursor.y, type, i = 0);
			break;
		case XK_slash:
		case XK_KP_Divide:
		case XK_question:
			ksym &= XK_question; /* Divide to slash */
			sens = (ksym == XK_slash) ? -1 : 1;
			ptarget = 0;
			set_notifmode(15, ksym);
			selectsearch_mode ^= 2;
			break;
		case XK_y:
			if ( selectsearch_mode == 0 ){
				selstart(0,term->cursor.y,0);
				set_notifmode(selectsearch_mode = 1, ksym);
				selextend(term->col-1, term->cursor.y, type, 0);
				xsetsel(getsel());
				break;
			}
			goto L_XK_Return;
		case XK_q:
		case XK_Escape:
			if (!in_use) {
				break;
			}
			selclear();
		case XK_Return:
			L_XK_Return:
			set_notifmode(4, ksym);
			term->cursor.x = cu.x, term->cursor.y = cu.y;
			select_or_drawcursor(selectsearch_mode = 0, type);
			in_use = quant = 0;
			xsetcursor( sel_savedcursor );
			return MODE_KBDSELECT;

		case XK_p:
			set_notifmode(4, ksym);
			char *tmp = getsel(); 
			term->cursor.x = cu.x, term->cursor.y = cu.y;
			select_or_drawcursor(selectsearch_mode = 0, type);
			in_use = quant = 0;
			xsetcursor( sel_savedcursor );
			ttywrite( (utfchar*)tmp, strlen(tmp), 1 );
			return MODE_KBDSELECT;

		case XK_n:
		case XK_N:
			if (ptarget) {
				search(selectsearch_mode, &target[0], ptarget, (ksym == XK_n) ? -1 : 1,
						type, &cu);
			}
			break;
		case XK_BackSpace:
			term->cursor.x = 0;
			select_or_drawcursor(selectsearch_mode, type);
			break;
		case XK_dollar:
			term->cursor.x = term->col - 1;
			select_or_drawcursor(selectsearch_mode, type);
			break;
		case XK_g:
			term->cursor.x = 0, term->cursor.y = 0;
			select_or_drawcursor(selectsearch_mode, type);
			break;
		case XK_Home:
			term->cursor.x = 0;
			select_or_drawcursor(selectsearch_mode, type);
			break;
		case XK_G:
			term->cursor.x = cu.x, term->cursor.y = cu.y;
			select_or_drawcursor(selectsearch_mode, type);
			break;
		case XK_End:
			term->cursor.x = term->col-1;
			select_or_drawcursor(selectsearch_mode, type);
			break;

		case XK_Page_Up:
		case XK_Page_Down:
			term->cursor.y = (ksym == XK_Prior) ? 0 : cu.y;
			select_or_drawcursor(selectsearch_mode, type);
			break;
		case XK_exclam:
			term->cursor.x = term->col >> 1;
			select_or_drawcursor(selectsearch_mode, type);
			break;
		case XK_asterisk:
		case XK_KP_Multiply:
			term->cursor.x = term->col >> 1;
		case XK_underscore:
			term->cursor.y = cu.y >> 1;
			select_or_drawcursor(selectsearch_mode, type);
			break;
		default:
			if (ksym >= XK_0 && ksym <= XK_9) { /* 0-9 keyboard */
				quant = (quant * 10) + (ksym ^ XK_0);
				return 0;
			} else if (ksym >= XK_KP_0 && ksym <= XK_KP_9) { /* 0-9 numpad */
				quant = (quant * 10) + (ksym ^ XK_KP_0);
				return 0;
			} else if (ksym == XK_k || ksym == XK_h) {
				i = ksym & 1;
			} else if (ksym == XK_l || ksym == XK_j) {
				i = ((ksym & 6) | 4) >> 1;
			} else if ((XK_Home & ksym) != XK_Home || (i = (ksym ^ XK_Home) - 1) > 3) {
				break;
			}

			xy = (i & 1) ? &term->cursor.y : &term->cursor.x;
			sens = (i & 2) ? 1 : -1;
			bound = (i >> 1 ^ 1) ? 0 : (i ^ 3) ? term->col - 1 : term->bot;

			if (quant == 0) {
				quant++;
			}

			if (*xy == bound && ((sens < 0 && bound == 0) || (sens > 0 && bound > 0))) {
				break;
			}

			*xy += quant * sens;
			if (*xy < 0 || (bound > 0 && *xy > bound)) {
				*xy = bound;
			}

			select_or_drawcursor(selectsearch_mode, type);
	}
	quant = 0;
	return 0;
}



