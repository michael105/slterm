// input event handling
//
//

#include "xevent.h"

/* the configuration is in config.h (generated from config.h.in) */
#include "config.h"



// inputmode. switchable via lessmode_toggle
int inputmode = 1;

int oldbutton = 3; /* button event on startup: 3 = release */

XSelection xsel;


void (*handler[LASTEvent])(XEvent *) = {
		[KeyPress] = kpress,
		[ClientMessage] = cmessage,
		[ConfigureNotify] = resize,
		[VisibilityNotify] = visibility,
		[UnmapNotify] = unmap,
		[Expose] = expose,
		[FocusIn] = focus,
		[FocusOut] = focus,
		[MotionNotify] = bmotion,
		[ButtonPress] = bpress,
		[ButtonRelease] = brelease,
		/*
		 * Uncomment if you want the selection to disappear when you select
		 * something different in another window.
		 */
		/*	[SelectionClear] = selclear_, */
		[SelectionNotify] = selnotify,
		/*
		 * PropertyNotify is only turned on when there is some INCR transfer
		 * happening for the selection retrieval.
		 */
		[PropertyNotify] = propnotify,
		[SelectionRequest] = selrequest,
};


void zoom(const Arg *arg) {
		Arg larg;

		larg.f = usedfontsize + arg->f;
		zoomabs(&larg);
}

void zoomabs(const Arg *arg) {
		xunloadfonts();
		xloadfonts(usedfont, arg->f);
		cresize(0, 0);
		redraw();
		xhints();
}

void zoomreset(const Arg *arg) {
		Arg larg;

		if (defaultfontsize > 0) {
				larg.f = defaultfontsize;
				zoomabs(&larg);
		}
}

void ttysend(const Arg *arg) { ttywrite(arg->s, strlen(arg->s), 1); }




int match(uint mask, uint state) {
		return mask == XK_ANY_MOD || mask == (state & ~ignoremod);
}

char *kmap(KeySym k, uint state) {
		Key *kp;
		int i;

		// printf("Key: %d %c\n", k,k);
		/* Check for mapped keys out of X11 function keys. */
		for (i = 0; i < LEN(mappedkeys); i++) { //? misc.
				// Better compare first with a bitfield,
				// by the or'ed mapped keys
				if (mappedkeys[i] == k)
						break;
		}
		if (i == LEN(mappedkeys)) {
				if ((k & 0xFFFF) < 0xFD00) { // No control/function/mod key
						// printf ("Here\n");//
						return NULL;
				}
		}

		for (kp = key; kp < key + LEN(key); kp++) {
				if (kp->k != k)
						continue;

				if (!match(kp->mask, state))
						continue;

				if (IS_SET(MODE_APPKEYPAD) ? kp->appkey < 0 : kp->appkey > 0)
						continue;
				if (IS_SET(MODE_NUMLOCK) && kp->appkey == 2)
						continue;

				if (IS_SET(MODE_APPCURSOR) ? kp->appcursor < 0 : kp->appcursor > 0)
						continue;

				return kp->s;
		}

		return NULL;
}


void keyboard_select(const Arg *dummy) {
		win.mode ^= trt_kbdselect(-1, NULL, 0);
}


void kpress(XEvent *ev) {
		XKeyEvent *e = &ev->xkey;
		KeySym ksym;
		unsigned char buf[32], *customkey;
		int len;
		Rune c;
		Status status;
		Shortcut *bp;

		if (IS_SET(MODE_KBDLOCK))
				return;

		len = XmbLookupString(xw.xic, e, buf, sizeof buf, &ksym, &status);

		if (IS_SET(MODE_KBDSELECT)) {
				if (match(XK_NO_MOD, e->state) || (XK_Shift_L | XK_Shift_R) & e->state)
						win.mode ^= trt_kbdselect(ksym, buf, len);
				return;
		}


		if ( IS_SET( MODE_ENTERSTRING ) ){
				statusbar_kpress( &ksym, buf );
				return;
		}

		/* 1. shortcuts */
		for (bp = shortcuts; bp < shortcuts + LEN(shortcuts); bp++) {
				if (ksym == bp->keysym && match(bp->mod, e->state) && (bp->inputmode & inputmode)) {
						bp->func(&(bp->arg));
						return;
				}
		}

		/* 2. custom keys from config.h */
		if ((customkey = kmap(ksym, e->state))) {
				ttywrite(customkey, strlen(customkey), 1);
				return;
		}

		dbg("Key2: %d %c, state:%x, mod1: %x, len %d\n", ksym, ksym, e->state,
						Mod1Mask, len);
		/* 3. composed string from input method */
		if (len == 0)
				return;
		if (len == 1 && e->state & Mod1Mask) {
				dbg("K\n");
				if (IS_SET(MODE_8BIT)) {
						if (*buf < 0177) {
								c = *buf | 0x80;
								len = utf8encode(c, buf);
						}
				} else {
						buf[1] = buf[0];
						buf[0] = '\033';
						len = 2;
				}
		}
		ttywrite(buf, len, 1);
}


void numlock(const Arg *dummy) { win.mode ^= MODE_NUMLOCK; }



// clipboard handling
//


void clipcopy(const Arg *dummy) {
		Atom clipboard;

		free(xsel.clipboard);
		xsel.clipboard = NULL;

		if (xsel.primary != NULL) {
				xsel.clipboard = xstrdup(xsel.primary);
				clipboard = XInternAtom(xw.dpy, "CLIPBOARD", 0);
				XSetSelectionOwner(xw.dpy, clipboard, xw.win, CurrentTime);
		}
}

void clippaste(const Arg *dummy) {
		Atom clipboard;

		clipboard = XInternAtom(xw.dpy, "CLIPBOARD", 0);
		XConvertSelection(xw.dpy, clipboard, xsel.xtarget, clipboard, xw.win,
						CurrentTime);
}

void selpaste(const Arg *dummy) {
		XConvertSelection(xw.dpy, XA_PRIMARY, xsel.xtarget, XA_PRIMARY, xw.win,
						CurrentTime);
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


void xclipcopy(void) { clipcopy(NULL); }

void selclear_(XEvent *e) { selclear(); }

void selrequest(XEvent *e) {
		XSelectionRequestEvent *xsre;
		XSelectionEvent xev;
		Atom xa_targets, string, clipboard;
		char *seltext;

		xsre = (XSelectionRequestEvent *)e;
		xev.type = SelectionNotify;
		xev.requestor = xsre->requestor;
		xev.selection = xsre->selection;
		xev.target = xsre->target;
		xev.time = xsre->time;
		if (xsre->property == None)
				xsre->property = xsre->target;

		/* reject */
		xev.property = None;

		xa_targets = XInternAtom(xw.dpy, "TARGETS", 0);
		if (xsre->target == xa_targets) {
				/* respond with the supported type */
				string = xsel.xtarget;
				XChangeProperty(xsre->display, xsre->requestor, xsre->property, XA_ATOM, 32,
								PropModeReplace, (uchar *)&string, 1);
				xev.property = xsre->property;
		} else if (xsre->target == xsel.xtarget || xsre->target == XA_STRING) {
				/*
				 * xith XA_STRING non ascii characters may be incorrect in the
				 * requestor. It is not our problem, use utf8.
				 */
				clipboard = XInternAtom(xw.dpy, "CLIPBOARD", 0);
				if (xsre->selection == XA_PRIMARY) {
						seltext = xsel.primary;
				} else if (xsre->selection == clipboard) {
						seltext = xsel.clipboard;
				} else {
						fprintf(stderr, "Unhandled clipboard selection 0x%lx\n", xsre->selection);
						return;
				}
				if (seltext != NULL) {
						XChangeProperty(xsre->display, xsre->requestor, xsre->property,
										xsre->target, 8, PropModeReplace, (uchar *)seltext,
										strlen(seltext));
						xev.property = xsre->property;
				}
		}

		/* all done, send a notification to the listener */
		if (!XSendEvent(xsre->display, xsre->requestor, 1, 0, (XEvent *)&xev))
				fprintf(stderr, "Error sending SelectionNotify event\n");
}

void setsel(char *str, Time t) {
		if (!str)
				return;

		free(xsel.primary);
		xsel.primary = str;

		XSetSelectionOwner(xw.dpy, XA_PRIMARY, xw.win, t);
		if (XGetSelectionOwner(xw.dpy, XA_PRIMARY) != xw.win)
				selclear();

		clipcopy(NULL);
}

void xsetsel(char *str) { setsel(str, CurrentTime); }


// mouse handling
void mousereport(XEvent *e) {
		int len, x = evcol(e), y = evrow(e), button = e->xbutton.button,
				state = e->xbutton.state;
		char buf[40];
		static int ox, oy;

		/* from urxvt */
		if (e->xbutton.type == MotionNotify) {
				if (x == ox && y == oy)
						return;
				if (!IS_SET(MODE_MOUSEMOTION) && !IS_SET(MODE_MOUSEMANY))
						return;
				/* MOUSE_MOTION: no reporting if no button is pressed */
				if (IS_SET(MODE_MOUSEMOTION) && oldbutton == 3)
						return;

				button = oldbutton + 32;
				ox = x;
				oy = y;
		} else {
				if (!IS_SET(MODE_MOUSESGR) && e->xbutton.type == ButtonRelease) {
						button = 3;
				} else {
						button -= Button1;
						if (button >= 3)
								button += 64 - 3;
				}
				if (e->xbutton.type == ButtonPress) {
						oldbutton = button;
						ox = x;
						oy = y;
				} else if (e->xbutton.type == ButtonRelease) {
						oldbutton = 3;
						/* MODE_MOUSEX10: no button release reporting */
						if (IS_SET(MODE_MOUSEX10))
								return;
						if (button == 64 || button == 65)
								return;
				}
		}

		if (!IS_SET(MODE_MOUSEX10)) {
				button += ((state & ShiftMask) ? 4 : 0) + ((state & Mod4Mask) ? 8 : 0) +
						((state & ControlMask) ? 16 : 0);
		}

		if (IS_SET(MODE_MOUSESGR)) {
				len = snprintf(buf, sizeof(buf), "\033[<%d;%d;%d%c", button, x + 1, y + 1,
								e->xbutton.type == ButtonRelease ? 'm' : 'M');
		} else if (x < 223 && y < 223) {
				len = snprintf(buf, sizeof(buf), "\033[M%c%c%c", 32 + button, 32 + x + 1,
								32 + y + 1);
		} else {
				return;
		}

		ttywrite(buf, len, 0);
}

int mouseaction(XEvent *e, uint release) {
		MouseShortcut *ms;

		for (ms = mshortcuts; ms < mshortcuts + LEN(mshortcuts); ms++) {
				if (ms->release == release && ms->button == e->xbutton.button &&
								(!ms->altscrn || (ms->altscrn == (tisaltscr() ? 1 : -1))) &&
								(match(ms->mod, e->xbutton.state) || /* exact or forced */
								 match(ms->mod, e->xbutton.state & ~forcemousemod))) {
						ms->func(&(ms->arg));
						return 1;
				}
		}

		return 0;
}

void bpress(XEvent *e) {
		struct timespec now;
		int snap;

		if (IS_SET(MODE_MOUSE) && !(e->xbutton.state & forcemousemod)) {
				mousereport(e);
				return;
		}

		if (mouseaction(e, 0))
				return;

		if (e->xbutton.button == Button1) {
				/*
				 * If the user clicks below predefined timeouts specific
				 * snapping behaviour is exposed.
				 */
				clock_gettime(CLOCK_MONOTONIC, &now);
				if (TIMEDIFF(now, xsel.tclick2) <= tripleclicktimeout) {
						snap = SNAP_LINE;
				} else if (TIMEDIFF(now, xsel.tclick1) <= doubleclicktimeout) {
						snap = SNAP_WORD;
				} else {
						snap = 0;
				}
				xsel.tclick2 = xsel.tclick1;
				xsel.tclick1 = now;

				selstart(evcol(e), evrow(e), snap);
		}
}



