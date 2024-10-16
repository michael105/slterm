// input event handling
//
//

#include "xevent.h"

/* the configuration is in config.h (generated from config.h.in) */
#include "config.h"




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

	xev = (1<<actionfps_shift);
	while (1) { // main loop
	//for (xev = (1<<actionfps_shift);;) { // main loop
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
			win.mode ^= MODE_BLINK; // read in xdraw.
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

#define FCB 4
void focus(XEvent *ev) {
	XFocusChangeEvent *e = &ev->xfocus;

	if (e->mode == NotifyGrab)
		return;
	static int colorsaved = 0;
	static Color tmp;
	if ( !colorsaved ){
		tmp = dc.col[FCB];
		colorsaved = 1;
	}

	if (ev->type == FocusIn) {
		if ( !( win.mode & MODE_FOCUSED) ){
			win.mode |= MODE_FOCUSED;
			XSetICFocus(xw.xic);
			xseturgency(0);
			if (IS_SET(MODE_FOCUS))
				ttywrite("\033[I", 3, 0);
			statusbar_focusin();
			dc.col[FCB] = tmp;
			redraw();
		}

	} else { // focus out
		if ( ( win.mode & MODE_FOCUSED) ){
			win.mode &= ~MODE_FOCUSED;
			XUnsetICFocus(xw.xic);
			if (IS_SET(MODE_FOCUS))
				ttywrite("\033[O", 3, 0);
			statusbar_focusout();
			dc.col[FCB] = dc.col[7];
			redraw();
		}

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



