/* See LICENSE for license details. */
#include "includes.h"


#include "term.h"
#include "xwindow.h"
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

#include "config.h"

/* macros */
#ifdef IS_SET
#undef IS_SET
#endif
// (!) is redefined, with different definitions (termmode,winmode,..)
#define IS_SET(flag) ((twin.mode & (flag)) != 0)

int xgeommasktogravity(int);
void ximopen(Display *);
void ximinstantiate(Display *, XPointer, XPointer);
void ximdestroy(XIM, XPointer, XPointer);
void xinit(int, int);
void xresize(int, int);
void xsetenv(void);
void xseturgency(int);


void resettitle(void) { xsettitle(NULL); }

void cresize(int width, int height) {
	int col, row;

	if (width != 0)
		twin.w = width;
	if (height != 0)
		twin.h = height;

	col = (twin.w - 2 * borderpx) / twin.cw;
	row = (twin.h - 2 * borderpx) / twin.ch;
	col = MAX(1, col);
	row = MAX(1, row);

	twin.hborderpx = (twin.w - col * twin.cw) / 2;
	twin.vborderpx = (twin.h - row * twin.ch) / 2;

	tresize(col, row);
	xresize(col, row);
	ttyresize(twin.tw, twin.th);
}

void xresize(int col, int row) {
	twin.tw = col * twin.cw;
	twin.th = row * twin.ch;

	XFreePixmap(xwin.dpy, xwin.buf);
	xwin.buf =
		XCreatePixmap(xwin.dpy, xwin.win, twin.w, twin.h, DefaultDepth(xwin.dpy, xwin.scr));
	XftDrawChange(xwin.draw, xwin.buf);
	xclear(0, 0, twin.w, twin.h);

	/* resize to new width */
	xwin.specbuf = xrealloc(xwin.specbuf, col * sizeof(GlyphFontSpec));

	updatestatus();
}



void xhints(void) {
#ifdef XRESOURCES
	XClassHint class = {opt_name ? opt_name : "slterm", opt_class ? opt_class : "Slterm"};
#else
	XClassHint class = {opt_name ? opt_name : termname,
		opt_class ? opt_class : termname};
#endif

	XWMHints wm = {.flags = InputHint, .input = 1};
	XSizeHints *sizeh;

	sizeh = XAllocSizeHints();

	sizeh->flags = PSize | PResizeInc | PBaseSize | PMinSize;
	sizeh->height = twin.h;
	sizeh->width = twin.w;
	sizeh->height_inc = 1;
	sizeh->width_inc = 1;
	sizeh->base_height = 2 * borderpx;
	sizeh->base_width = 2 * borderpx;
	sizeh->min_height = twin.ch + 2 * borderpx;
	sizeh->min_width = twin.cw + 2 * borderpx;
	if (xwin.isfixed) {
		sizeh->flags |= PMaxSize;
		sizeh->min_width = sizeh->max_width = twin.w;
		sizeh->min_height = sizeh->max_height = twin.h;
	}
	if (xwin.gm & (XValue | YValue)) {
		sizeh->flags |= USPosition | PWinGravity;
		sizeh->x = xwin.l;
		sizeh->y = xwin.t;
		sizeh->win_gravity = xgeommasktogravity(xwin.gm);
	}

	XSetWMProperties(xwin.dpy, xwin.win, NULL, NULL, NULL, 0, sizeh, &wm, &class);
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

	if ((xwin.xim = XOpenIM(xwin.dpy, NULL, NULL, NULL)) == NULL) {
		XSetLocaleModifiers("@im=local");
		if ((xwin.xim = XOpenIM(xwin.dpy, NULL, NULL, NULL)) == NULL) {
			XSetLocaleModifiers("@im=");
			if ((xwin.xim = XOpenIM(xwin.dpy, NULL, NULL, NULL)) == NULL)
				die("XOpenIM failed. Could not open input device.\n");
		}
	}
	if (XSetIMValues(xwin.xim, XNDestroyCallback, &destroy, NULL) != NULL)
		die("XSetIMValues failed. Could not set input method value.\n");
	xwin.xic = XCreateIC(xwin.xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
			XNClientWindow, xwin.win, XNFocusWindow, xwin.win, NULL);
	if (xwin.xic == NULL)
		die("XCreateIC failed. Could not obtain input method.\n");
}

void ximinstantiate(Display *dpy, XPointer client, XPointer call) {
	ximopen(dpy);
	XUnregisterIMInstantiateCallback(xwin.dpy, NULL, NULL, NULL, ximinstantiate,
			NULL);
}

void ximdestroy(XIM xim, XPointer client, XPointer call) {
	xwin.xim = NULL;
	XRegisterIMInstantiateCallback(xwin.dpy, NULL, NULL, NULL, ximinstantiate,
			NULL);
}

void xinit(int cols, int rows) {
	XGCValues gcvalues;
	Cursor cursor;
	Window parent;
	pid_t thispid = getpid();
	XColor xmousefg, xmousebg;

#ifndef XRESOURCES
	if (!(xwin.dpy = XOpenDisplay(NULL)))
		die("can't open display\n");
#endif

	xwin.scr = XDefaultScreen(xwin.dpy);
	xwin.vis = XDefaultVisual(xwin.dpy, xwin.scr);

	/* font */
	if (!FcInit())
		die("could not init fontconfig.\n");

	//usedfont = (opt_font == NULL) ? regular_font : opt_font;
	xloadfonts(0);

	/* colors */
	xwin.cmap = XDefaultColormap(xwin.dpy, xwin.scr);
	xloadcolors();

	/* adjust fixed window geometry */
	twin.w = 2 * twin.hborderpx + cols * twin.cw;
	twin.h = 2 * twin.vborderpx + rows * twin.ch;
	if (xwin.gm & XNegative)
		xwin.l += DisplayWidth(xwin.dpy, xwin.scr) - twin.w - 2;
	if (xwin.gm & YNegative)
		xwin.t += DisplayHeight(xwin.dpy, xwin.scr) - twin.h - 2;

	/* Events */
	xwin.attrs.background_pixel = dc.color_array[defaultbg].pixel;
	xwin.attrs.border_pixel = dc.color_array[defaultbg].pixel;
	xwin.attrs.bit_gravity = NorthWestGravity;
	xwin.attrs.event_mask = FocusChangeMask | KeyPressMask | KeyReleaseMask |
		ExposureMask | VisibilityChangeMask |
		StructureNotifyMask | ButtonMotionMask |
		ButtonPressMask | ButtonReleaseMask;
	xwin.attrs.colormap = xwin.cmap;

	if (!(opt_embed && (parent = strtol(opt_embed, NULL, 0))))
		parent = XRootWindow(xwin.dpy, xwin.scr);
	xwin.win = XCreateWindow(xwin.dpy, parent, xwin.l, xwin.t, twin.w, twin.h, 0,
			XDefaultDepth(xwin.dpy, xwin.scr), InputOutput, xwin.vis,
			CWBackPixel | CWBorderPixel | CWBitGravity |
			CWEventMask | CWColormap,
			&xwin.attrs);

	memset(&gcvalues, 0, sizeof(gcvalues));
	gcvalues.graphics_exposures = False;
	dc.gc = XCreateGC(xwin.dpy, parent, GCGraphicsExposures, &gcvalues);
	xwin.buf =
		XCreatePixmap(xwin.dpy, xwin.win, twin.w, twin.h, DefaultDepth(xwin.dpy, xwin.scr));
	XSetForeground(xwin.dpy, dc.gc, dc.color_array[defaultbg].pixel);
	XFillRectangle(xwin.dpy, xwin.buf, dc.gc, 0, 0, twin.w, twin.h);

	/* font spec buffer */
	xwin.specbuf = xmalloc(cols * sizeof(GlyphFontSpec));

	/* Xft rendering context */
	xwin.draw = XftDrawCreate(xwin.dpy, xwin.buf, xwin.vis, xwin.cmap);

	/* input methods */
	ximopen(xwin.dpy);

	/* white cursor, black outline */
	cursor = XCreateFontCursor(xwin.dpy, mouseshape);
	XDefineCursor(xwin.dpy, xwin.win, cursor);

	if (XParseColor(xwin.dpy, xwin.cmap, colorname[mousefg], &xmousefg) == 0) {
		xmousefg.red = 0xffff;
		xmousefg.green = 0xffff;
		xmousefg.blue = 0xffff;
	}

	if (XParseColor(xwin.dpy, xwin.cmap, colorname[mousebg], &xmousebg) == 0) {
		xmousebg.red = 0x0000;
		xmousebg.green = 0x0000;
		xmousebg.blue = 0x0000;
	}

	XRecolorCursor(xwin.dpy, cursor, &xmousefg, &xmousebg);

	xwin.xembed = XInternAtom(xwin.dpy, "_XEMBED", False);
	xwin.wmdeletewin = XInternAtom(xwin.dpy, "WM_DELETE_WINDOW", False);
	xwin.netwmname = XInternAtom(xwin.dpy, "_NET_WM_NAME", False);
	XSetWMProtocols(xwin.dpy, xwin.win, &xwin.wmdeletewin, 1);

	xwin.netwmpid = XInternAtom(xwin.dpy, "_NET_WM_PID", False);
	XChangeProperty(xwin.dpy, xwin.win, xwin.netwmpid, XA_CARDINAL, 32, PropModeReplace,
			(uchar *)&thispid, 1);

	twin.mode = MODE_NUMLOCK;
	resettitle();
	xhints();
	XMapWindow(xwin.dpy, xwin.win);
	XSync(xwin.dpy, False);

	clock_gettime(CLOCK_MONOTONIC, &xsel.tclick1);
	clock_gettime(CLOCK_MONOTONIC, &xsel.tclick2);
	xsel.primary = NULL;
	xsel.clipboard = NULL;
	xsel.xtarget = XInternAtom(xwin.dpy, "UTF8_STRING", 0);
	if (xsel.xtarget == None)
		xsel.xtarget = XA_STRING;
}

void xsetenv(void) {
	char buf[sizeof(long) * 8 + 1];

	snprintf(buf, sizeof(buf), "%lu", xwin.win);
	setenv("WINDOWID", buf, 1);
}

void xsettitle(char *p) {
	XTextProperty prop;
	DEFAULT(p, opt_title);

	Xutf8TextListToTextProperty(xwin.dpy, &p, 1, XUTF8StringStyle, &prop);
	XSetWMName(xwin.dpy, xwin.win, &prop);
	XSetTextProperty(xwin.dpy, xwin.win, &prop, xwin.netwmname);
	XFree(prop.value);
}


void xximspot(int x, int y) {
	XPoint spot = {borderpx + x * twin.cw, borderpx + (y + 1) * twin.ch};
	XVaNestedList attr = XVaCreateNestedList(0, XNSpotLocation, &spot, NULL);

	XSetICValues(xwin.xic, XNPreeditAttributes, attr, NULL);
	XFree(attr);
}

void xsetpointermotion(int set) {
	MODBIT(xwin.attrs.event_mask, set, PointerMotionMask);
	XChangeWindowAttributes(xwin.dpy, xwin.win, CWEventMask, &xwin.attrs);
}

void xsetmode(int set, unsigned int flags) {
	int mode = twin.mode;
	MODBIT(twin.mode, set, flags);
	if ((twin.mode & MODE_REVERSE) != (mode & MODE_REVERSE))
		redraw();
}

void xseturgency(int add) {
	XWMHints *h = XGetWMHints(xwin.dpy, xwin.win);

	MODBIT(h->flags, add, XUrgencyHint);
	XSetWMHints(xwin.dpy, xwin.win, h);
	XFree(h);
}

void xbell(void) {
	if (!(IS_SET(MODE_FOCUSED)))
		xseturgency(1);
	if (bellvolume)
		XkbBell(xwin.dpy, xwin.win, bellvolume, (Atom)NULL);
}

