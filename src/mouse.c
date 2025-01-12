

// mouse handling
void mousereport(XEvent *e) {
	static int oldbutton = 3; /* button event on startup: 3 = release */

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

	ttywrite((utfchar*)buf, len, 0);
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
	int x = e->xbutton.x - twin.hborderpx;
	LIMIT(x, 0, twin.tw - 1);
	return x / twin.cw;
}

int evrow(XEvent *e) {
	int y = e->xbutton.y - twin.vborderpx;
	LIMIT(y, 0, twin.th - 1);
	return y / twin.ch;
}


