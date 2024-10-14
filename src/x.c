/* See LICENSE for license details. */
#include "includes.h"


#include "arg.h"
#include "term.h"
#include "x.h"
#include "fonts.h"
#include "mem.h"
#include "scroll.h"
#include "selection.h"
#include "xevent.h"
#include "tty.h"
#include "system.h"
#include "compile.h"

//#define ENABLE_DEBUG 5
#include "debug.h"

#ifdef XRESOURCES
/* Xresources preferences */
enum resource_type { STRING = 0, INTEGER = 1, FLOAT = 2 };

typedef struct {
	char *name;
	enum resource_type type;
	void *dst;
} ResourcePref;
#endif

/* the configuration is in config.h (generated from config.h.in) */
#include "config.h"


/* macros */
#ifdef IS_SET
#undef IS_SET
#endif
#define IS_SET(flag) ((win.mode & (flag)) != 0)
#define TRUERED(x) (((x)&0xff0000) >> 8)
#define TRUEGREEN(x) (((x)&0xff00))
#define TRUEBLUE(x) (((x)&0xff) << 8)

int xgeommasktogravity(int);
void ximopen(Display *);
void ximinstantiate(Display *, XPointer, XPointer);
void ximdestroy(XIM, XPointer, XPointer);
void xinit(int, int);
void xresize(int, int);
void xsetenv(void);
void xseturgency(int);

void usage(void);


/* Globals */
XWindow xw;
TermWindow win;



void resettitle(void) { xsettitle(NULL); }

void cresize(int width, int height) {
	int col, row;

	if (width != 0)
		win.w = width;
	if (height != 0)
		win.h = height;

	col = (win.w - 2 * borderpx) / win.cw;
	row = (win.h - 2 * borderpx) / win.ch;
	col = MAX(1, col);
	row = MAX(1, row);

	win.hborderpx = (win.w - col * win.cw) / 2;
	win.vborderpx = (win.h - row * win.ch) / 2;

	tresize(col, row);
	xresize(col, row);
	ttyresize(win.tw, win.th);
}

void xresize(int col, int row) {
	win.tw = col * win.cw;
	win.th = row * win.ch;

	XFreePixmap(xw.dpy, xw.buf);
	xw.buf =
		XCreatePixmap(xw.dpy, xw.win, win.w, win.h, DefaultDepth(xw.dpy, xw.scr));
	XftDrawChange(xw.draw, xw.buf);
	xclear(0, 0, win.w, win.h);

	/* resize to new width */
	xw.specbuf = xrealloc(xw.specbuf, col * sizeof(GlyphFontSpec));
}



void xhints(void) {
#ifdef XRESOURCES
	XClassHint class = {opt_name ? opt_name : "st", opt_class ? opt_class : "St"};
#else
	XClassHint class = {opt_name ? opt_name : termname,
		opt_class ? opt_class : termname};
#endif

	XWMHints wm = {.flags = InputHint, .input = 1};
	XSizeHints *sizeh;

	sizeh = XAllocSizeHints();

	sizeh->flags = PSize | PResizeInc | PBaseSize | PMinSize;
	sizeh->height = win.h;
	sizeh->width = win.w;
	sizeh->height_inc = 1;
	sizeh->width_inc = 1;
	sizeh->base_height = 2 * borderpx;
	sizeh->base_width = 2 * borderpx;
	sizeh->min_height = win.ch + 2 * borderpx;
	sizeh->min_width = win.cw + 2 * borderpx;
	if (xw.isfixed) {
		sizeh->flags |= PMaxSize;
		sizeh->min_width = sizeh->max_width = win.w;
		sizeh->min_height = sizeh->max_height = win.h;
	}
	if (xw.gm & (XValue | YValue)) {
		sizeh->flags |= USPosition | PWinGravity;
		sizeh->x = xw.l;
		sizeh->y = xw.t;
		sizeh->win_gravity = xgeommasktogravity(xw.gm);
	}

	XSetWMProperties(xw.dpy, xw.win, NULL, NULL, NULL, 0, sizeh, &wm, &class);
	XFree(sizeh);
}

int xgeommasktogravity(int mask) {
	switch (mask & (XNegative | YNegative)) {
		case 0:
			return NorthWestGravity;
		case XNegative:
			return NorthEastGravity;
		case YNegative:
			return SouthWestGravity;
	}

	return SouthEastGravity;
}
void ximopen(Display *dpy) {
	XIMCallback destroy = {.client_data = NULL, .callback = ximdestroy};

	if ((xw.xim = XOpenIM(xw.dpy, NULL, NULL, NULL)) == NULL) {
		XSetLocaleModifiers("@im=local");
		if ((xw.xim = XOpenIM(xw.dpy, NULL, NULL, NULL)) == NULL) {
			XSetLocaleModifiers("@im=");
			if ((xw.xim = XOpenIM(xw.dpy, NULL, NULL, NULL)) == NULL)
				die("XOpenIM failed. Could not open input device.\n");
		}
	}
	if (XSetIMValues(xw.xim, XNDestroyCallback, &destroy, NULL) != NULL)
		die("XSetIMValues failed. Could not set input method value.\n");
	xw.xic = XCreateIC(xw.xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
			XNClientWindow, xw.win, XNFocusWindow, xw.win, NULL);
	if (xw.xic == NULL)
		die("XCreateIC failed. Could not obtain input method.\n");
}

void ximinstantiate(Display *dpy, XPointer client, XPointer call) {
	ximopen(dpy);
	XUnregisterIMInstantiateCallback(xw.dpy, NULL, NULL, NULL, ximinstantiate,
			NULL);
}

void ximdestroy(XIM xim, XPointer client, XPointer call) {
	xw.xim = NULL;
	XRegisterIMInstantiateCallback(xw.dpy, NULL, NULL, NULL, ximinstantiate,
			NULL);
}

void xinit(int cols, int rows) {
	XGCValues gcvalues;
	Cursor cursor;
	Window parent;
	pid_t thispid = getpid();
	XColor xmousefg, xmousebg;

#ifndef XRESOURCES
	if (!(xw.dpy = XOpenDisplay(NULL)))
		die("can't open display\n");
#endif

	xw.scr = XDefaultScreen(xw.dpy);
	xw.vis = XDefaultVisual(xw.dpy, xw.scr);

	/* font */
	if (!FcInit())
		die("could not init fontconfig.\n");

	usedfont = (opt_font == NULL) ? font : opt_font;
	xloadfonts(usedfont, 0);

	/* colors */
	xw.cmap = XDefaultColormap(xw.dpy, xw.scr);
	xloadcolors();

	/* adjust fixed window geometry */
	win.w = 2 * win.hborderpx + cols * win.cw;
	win.h = 2 * win.vborderpx + rows * win.ch;
	if (xw.gm & XNegative)
		xw.l += DisplayWidth(xw.dpy, xw.scr) - win.w - 2;
	if (xw.gm & YNegative)
		xw.t += DisplayHeight(xw.dpy, xw.scr) - win.h - 2;

	/* Events */
	xw.attrs.background_pixel = dc.col[defaultbg].pixel;
	xw.attrs.border_pixel = dc.col[defaultbg].pixel;
	xw.attrs.bit_gravity = NorthWestGravity;
	xw.attrs.event_mask = FocusChangeMask | KeyPressMask | KeyReleaseMask |
		ExposureMask | VisibilityChangeMask |
		StructureNotifyMask | ButtonMotionMask |
		ButtonPressMask | ButtonReleaseMask;
	xw.attrs.colormap = xw.cmap;

	if (!(opt_embed && (parent = strtol(opt_embed, NULL, 0))))
		parent = XRootWindow(xw.dpy, xw.scr);
	xw.win = XCreateWindow(xw.dpy, parent, xw.l, xw.t, win.w, win.h, 0,
			XDefaultDepth(xw.dpy, xw.scr), InputOutput, xw.vis,
			CWBackPixel | CWBorderPixel | CWBitGravity |
			CWEventMask | CWColormap,
			&xw.attrs);

	memset(&gcvalues, 0, sizeof(gcvalues));
	gcvalues.graphics_exposures = False;
	dc.gc = XCreateGC(xw.dpy, parent, GCGraphicsExposures, &gcvalues);
	xw.buf =
		XCreatePixmap(xw.dpy, xw.win, win.w, win.h, DefaultDepth(xw.dpy, xw.scr));
	XSetForeground(xw.dpy, dc.gc, dc.col[defaultbg].pixel);
	XFillRectangle(xw.dpy, xw.buf, dc.gc, 0, 0, win.w, win.h);

	/* font spec buffer */
	xw.specbuf = xmalloc(cols * sizeof(GlyphFontSpec));

	/* Xft rendering context */
	xw.draw = XftDrawCreate(xw.dpy, xw.buf, xw.vis, xw.cmap);

	/* input methods */
	ximopen(xw.dpy);

	/* white cursor, black outline */
	cursor = XCreateFontCursor(xw.dpy, mouseshape);
	XDefineCursor(xw.dpy, xw.win, cursor);

	if (XParseColor(xw.dpy, xw.cmap, colorname[mousefg], &xmousefg) == 0) {
		xmousefg.red = 0xffff;
		xmousefg.green = 0xffff;
		xmousefg.blue = 0xffff;
	}

	if (XParseColor(xw.dpy, xw.cmap, colorname[mousebg], &xmousebg) == 0) {
		xmousebg.red = 0x0000;
		xmousebg.green = 0x0000;
		xmousebg.blue = 0x0000;
	}

	XRecolorCursor(xw.dpy, cursor, &xmousefg, &xmousebg);

	xw.xembed = XInternAtom(xw.dpy, "_XEMBED", False);
	xw.wmdeletewin = XInternAtom(xw.dpy, "WM_DELETE_WINDOW", False);
	xw.netwmname = XInternAtom(xw.dpy, "_NET_WM_NAME", False);
	XSetWMProtocols(xw.dpy, xw.win, &xw.wmdeletewin, 1);

	xw.netwmpid = XInternAtom(xw.dpy, "_NET_WM_PID", False);
	XChangeProperty(xw.dpy, xw.win, xw.netwmpid, XA_CARDINAL, 32, PropModeReplace,
			(uchar *)&thispid, 1);

	win.mode = MODE_NUMLOCK;
	resettitle();
	xhints();
	XMapWindow(xw.dpy, xw.win);
	XSync(xw.dpy, False);

	clock_gettime(CLOCK_MONOTONIC, &xsel.tclick1);
	clock_gettime(CLOCK_MONOTONIC, &xsel.tclick2);
	xsel.primary = NULL;
	xsel.clipboard = NULL;
	xsel.xtarget = XInternAtom(xw.dpy, "UTF8_STRING", 0);
	if (xsel.xtarget == None)
		xsel.xtarget = XA_STRING;
}

void xsetenv(void) {
	char buf[sizeof(long) * 8 + 1];

	snprintf(buf, sizeof(buf), "%lu", xw.win);
	setenv("WINDOWID", buf, 1);
}

void xsettitle(char *p) {
	XTextProperty prop;
	DEFAULT(p, opt_title);

	Xutf8TextListToTextProperty(xw.dpy, &p, 1, XUTF8StringStyle, &prop);
	XSetWMName(xw.dpy, xw.win, &prop);
	XSetTextProperty(xw.dpy, xw.win, &prop, xw.netwmname);
	XFree(prop.value);
}


void xximspot(int x, int y) {
	XPoint spot = {borderpx + x * win.cw, borderpx + (y + 1) * win.ch};
	XVaNestedList attr = XVaCreateNestedList(0, XNSpotLocation, &spot, NULL);

	XSetICValues(xw.xic, XNPreeditAttributes, attr, NULL);
	XFree(attr);
}

void xsetpointermotion(int set) {
	MODBIT(xw.attrs.event_mask, set, PointerMotionMask);
	XChangeWindowAttributes(xw.dpy, xw.win, CWEventMask, &xw.attrs);
}

void xsetmode(int set, unsigned int flags) {
	int mode = win.mode;
	MODBIT(win.mode, set, flags);
	if ((win.mode & MODE_REVERSE) != (mode & MODE_REVERSE))
		redraw();
}

void xseturgency(int add) {
	XWMHints *h = XGetWMHints(xw.dpy, xw.win);

	MODBIT(h->flags, add, XUrgencyHint);
	XSetWMHints(xw.dpy, xw.win, h);
	XFree(h);
}

void xbell(void) {
	if (!(IS_SET(MODE_FOCUSED)))
		xseturgency(1);
	if (bellvolume)
		XkbBell(xw.dpy, xw.win, bellvolume, (Atom)NULL);
}

