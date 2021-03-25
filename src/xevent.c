// input event handling
//
//

#include "xevent.h"

/* the configuration is in config.h (generated from config.h.in) */
#include "config.h"



// inputmode. switchable via lessmode_toggle
int inputmode = 1;

int oldbutton = 3; /* button event on startup: 3 = release */

// The callbacks for the different Events are egistered here.
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


// the main event loop
void run() {
		XEvent ev;
		int w = win.w, h = win.h;
		fd_set rfd;
		int xfd = XConnectionNumber(xw.dpy), xev, blinkset = 0, dodraw = 0;
		int ttyfd;
		struct timespec drawtimeout, *tv = NULL, now, last, lastblink;
		long deltatime;

		/* Waiting for window mapping */
		do {
				XNextEvent(xw.dpy, &ev);
				/*
				 * This XFilterEvent call is required because of XOpenIM. It
				 * does filter out the key event and some client message for
				 * the input method too.
				 */
				if (XFilterEvent(&ev, None))
						continue;
				if (ev.type == ConfigureNotify) {
						w = ev.xconfigure.width;
						h = ev.xconfigure.height;
				}
		} while (ev.type != MapNotify);

		ttyfd = ttynew(opt_line, shell, opt_io, opt_cmd);
		cresize(w, h);

		clock_gettime(CLOCK_MONOTONIC, &last);
		lastblink = last;

		for (xev = (1<<actionfps_shift);;) { // main loop
				FD_ZERO(&rfd);
				FD_SET(ttyfd, &rfd);
				FD_SET(xfd, &rfd);

				if (pselect(MAX(xfd, ttyfd) + 1, &rfd, NULL, NULL, tv, NULL) < 0) {
						if (errno == EINTR) //? xevent ?
								continue;
						die("select failed: %s\n", strerror(errno));
				}
				if (FD_ISSET(ttyfd, &rfd)) {
						ttyread();
						if (blinktimeout) {
								blinkset = tattrset(ATTR_BLINK);
								if (!blinkset)
										MODBIT(win.mode, 0, MODE_BLINK);
						}
				}

				if (FD_ISSET(xfd, &rfd))
						xev = (1<<actionfps_shift);

				clock_gettime(CLOCK_MONOTONIC, &now);
				drawtimeout.tv_sec = 0;
				drawtimeout.tv_nsec = 1000000000 >> xfps_shift;
				//drawtimeout.tv_nsec = (1000 * 1E6) / xfps;
				tv = &drawtimeout;

				dodraw = 0;
				if (blinktimeout && TIMEDIFF(now, lastblink) > blinktimeout) {
						tsetdirtattr(ATTR_BLINK);
						win.mode ^= MODE_BLINK;
						lastblink = now;
						dodraw = 1;
				}
				deltatime = TIMEDIFF(now, last);
				if (deltatime > (1000 >> (xev ? xfps_shift : actionfps_shift) ) ) {
						dodraw = 1;
						last = now;
				}

				if (dodraw) {
						while (XPending(xw.dpy)) {
								XNextEvent(xw.dpy, &ev);
								if (XFilterEvent(&ev, None))
										continue;
								if (handler[ev.type]) // process x events 
										(handler[ev.type])(&ev);
						}

						draw();
						XFlush(xw.dpy);

						if (xev && !FD_ISSET(xfd, &rfd))
								xev--;
						if (!FD_ISSET(ttyfd, &rfd) && !FD_ISSET(xfd, &rfd)) {
								if (blinkset) {
										if (TIMEDIFF(now, lastblink) > blinktimeout) {
												drawtimeout.tv_nsec = 1000;
										} else {
												drawtimeout.tv_nsec =
														((long)(blinktimeout - TIMEDIFF(now, lastblink)) << 20);
												//(1E6 * (blinktimeout - TIMEDIFF(now, lastblink)));
										}
										// drawtimeout.tv_nsec = ( drawtimeout.tv_nsec - ( drawtimeout.tv_sec = (drawtimeout.tv_nsec >> 9 )) >>9) ; //
										//drawtimeout.tv_sec = drawtimeout.tv_nsec / 1E9; // Thats bad misc
										//drawtimeout.tv_nsec %= (long)1E9; // Better don't divide
										//
										// better shift ? division is the most expensive operation.
										// Anyways. doesn't seem to be a big difference. I'm however wondering.
										// More strange. E.g. dividing by 512 prevents blinking??? 
										// Ok. I confused exponential and hexadecimal. It's exponential.
										// 1E9 equals 1.000.000.000
										// 2^30 might be close enough

										drawtimeout.tv_sec = (drawtimeout.tv_nsec >> 30); // .. 
										drawtimeout.tv_nsec -= (long)(drawtimeout.tv_sec << 30); 
								} else {
										tv = NULL;
								}
						}
				}
		}
}
void toggle_winmode(int flag) { win.mode ^= flag; }


void ttysend(const Arg *arg) { ttywrite(arg->s, strlen(arg->s), 1); }


void resize(XEvent *e) {
		if (e->xconfigure.width == win.w && e->xconfigure.height == win.h)
				return;

		cresize(e->xconfigure.width, e->xconfigure.height);
}

void cmessage(XEvent *e) {
		/*
		 * See xembed specs
		 *  http://standards.freedesktop.org/xembed-spec/xembed-spec-latest.html
		 */
		if (e->xclient.message_type == xw.xembed && e->xclient.format == 32) {
				if (e->xclient.data.l[1] == XEMBED_FOCUS_IN) {
						win.mode |= MODE_FOCUSED;
						xseturgency(0);
				} else if (e->xclient.data.l[1] == XEMBED_FOCUS_OUT) {
						win.mode &= ~MODE_FOCUSED;
				}
		} else if (e->xclient.data.l[0] == xw.wmdeletewin) {
				ttyhangup();
				exit(0);
		}
}

void focus(XEvent *ev) {
		XFocusChangeEvent *e = &ev->xfocus;

		if (e->mode == NotifyGrab)
				return;

		if (ev->type == FocusIn) {
				XSetICFocus(xw.xic);
				win.mode |= MODE_FOCUSED;
				xseturgency(0);
				if (IS_SET(MODE_FOCUS))
						ttywrite("\033[I", 3, 0);
		} else {
				XUnsetICFocus(xw.xic);
				win.mode &= ~MODE_FOCUSED;
				if (IS_SET(MODE_FOCUS))
						ttywrite("\033[O", 3, 0);
		}
}


void expose(XEvent *ev) { redraw(); }

void visibility(XEvent *ev) {
		XVisibilityEvent *e = &ev->xvisibility;

		MODBIT(win.mode, e->state != VisibilityFullyObscured, MODE_VISIBLE);
}

void unmap(XEvent *ev) { win.mode &= ~MODE_VISIBLE; }



void propnotify(XEvent *e) {
		XPropertyEvent *xpev;
		Atom clipboard = XInternAtom(xw.dpy, "CLIPBOARD", 0);

		xpev = &e->xproperty;
		if (xpev->state == PropertyNewValue &&
						(xpev->atom == XA_PRIMARY || xpev->atom == clipboard)) {
				selnotify(e);
		}
}



// keyboard handling

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

// Keystrokes are handled here.
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

		dbg("key: %x, keycode: %x, state: %x\n",ksym, e->keycode, e->state );

		// handle return, set scrollmark 0
		if ( ( ksym == XK_Return ) ){
//if ( (!IS_SET(MODE_ALTSCREEN)) && ( ksym == XK_Return ) ){
				set_retmark();
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

void brelease(XEvent *e) {
		if (IS_SET(MODE_MOUSE) && !(e->xbutton.state & forcemousemod)) {
				mousereport(e);
				return;
		}

		//	if (mouseaction(e, 1))
		//		return;

		if (e->xbutton.button == Button2)
				clippaste(NULL);
		else if (e->xbutton.button == Button1)
				mousesel(e, 1);
}

void bmotion(XEvent *e) {
		if (IS_SET(MODE_MOUSE) && !(e->xbutton.state & forcemousemod)) {
				mousereport(e);
				return;
		}

		mousesel(e, 0);
}


int evcol(XEvent *e) {
		int x = e->xbutton.x - win.hborderpx;
		LIMIT(x, 0, win.tw - 1);
		return x / win.cw;
}

int evrow(XEvent *e) {
		int y = e->xbutton.y - win.vborderpx;
		LIMIT(y, 0, win.th - 1);
		return y / win.ch;
}
