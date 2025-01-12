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
	int w = twin.w, h = twin.h;
	fd_set rfd;
	int xfd = XConnectionNumber(xwin.dpy), xev, blinkset = 0, dodraw = 0;
	int ttyfd;
	struct timespec drawtimeout, *tv = NULL, now, last, lastblink;
	long deltatime;

	/* Waiting for window mapping */
	do {
		XNextEvent(xwin.dpy, &ev);
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

	// also start shell in ttynew. args are global. somewhere.
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
					MODBIT(twin.mode, 0, MODE_BLINK);
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
			twin.mode ^= MODE_BLINK; // read in xdraw.
			lastblink = now;
			dodraw = 1;
		}
		deltatime = TIMEDIFF(now, last);
		if (deltatime > (1000 >> (xev ? xfps_shift : actionfps_shift) ) ) {
			dodraw = 1;
			last = now;
		}

		if (dodraw) {
			while (XPending(xwin.dpy)) {
				XNextEvent(xwin.dpy, &ev);
				if (XFilterEvent(&ev, None))
					continue;
				if (handler[ev.type]) // process x events 
					(handler[ev.type])(&ev);
			}

			draw();
			XFlush(xwin.dpy);

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
void toggle_winmode(int flag) { twin.mode ^= flag; }


void ttysend(const Arg *arg) { ttywrite((utfchar*)arg->s, strlen(arg->s), 1); }



void resize(XEvent *e) {
	if (e->xconfigure.width == twin.w && e->xconfigure.height == twin.h)
		return;

	cresize(e->xconfigure.width, e->xconfigure.height);
}

void cmessage(XEvent *e) {
	/*
	 * See xembed specs
	 *  http://standards.freedesktop.org/xembed-spec/xembed-spec-latest.html
	 */
	if (e->xclient.message_type == xwin.xembed && e->xclient.format == 32) {
		if (e->xclient.data.l[1] == XEMBED_FOCUS_IN) {
			twin.mode |= MODE_FOCUSED;
			xseturgency(0);
		} else if (e->xclient.data.l[1] == XEMBED_FOCUS_OUT) {
			twin.mode &= ~MODE_FOCUSED;
		}
	} else if (e->xclient.data.l[0] == xwin.wmdeletewin) {
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
		if ( !( twin.mode & MODE_FOCUSED) ){
			twin.mode |= MODE_FOCUSED;
			XSetICFocus(xwin.xic);
			xseturgency(0);
			if (IS_SET(MODE_FOCUS))
				ttywrite((utfchar*)"\033[I", 3, 0);
			statusbar_focusin();
			dc.col[FCB] = tmp;
			redraw();
		}

	} else { // focus out
		if ( ( twin.mode & MODE_FOCUSED) ){
			twin.mode &= ~MODE_FOCUSED;
			XUnsetICFocus(xwin.xic);
			if (IS_SET(MODE_FOCUS))
				ttywrite((utfchar*)"\033[O", 3, 0);
			statusbar_focusout();
			dc.col[FCB] = dc.col[7];
			redraw();
		}

	}
}


void expose(XEvent *ev) { redraw(); }

void visibility(XEvent *ev) {
	XVisibilityEvent *e = &ev->xvisibility;

	MODBIT(twin.mode, e->state != VisibilityFullyObscured, MODE_VISIBLE);
}

void unmap(XEvent *ev) { twin.mode &= ~MODE_VISIBLE; }

void propnotify(XEvent *e) {
	XPropertyEvent *xpev;
	Atom clipboard = XInternAtom(xwin.dpy, "CLIPBOARD", 0);

	xpev = &e->xproperty;
	if (xpev->state == PropertyNewValue &&
			(xpev->atom == XA_PRIMARY || xpev->atom == clipboard)) {
		selnotify(e);
	}
}



