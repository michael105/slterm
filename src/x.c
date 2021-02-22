/* See LICENSE for license details. */
#include "includes.h"

static char *argv0;
#include "arg.h"
#include "st.h"
#include "x.h"
#include "input.h"
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

/* XEMBED messages */
#define XEMBED_FOCUS_IN 4
#define XEMBED_FOCUS_OUT 5

/* macros */
#ifdef IS_SET
#undef IS_SET
#endif
#define IS_SET(flag) ((win.mode & (flag)) != 0)
#define TRUERED(x) (((x)&0xff0000) >> 8)
#define TRUEGREEN(x) (((x)&0xff00))
#define TRUEBLUE(x) (((x)&0xff) << 8)

inline ushort sixd_to_16bit(int);
void xdrawglyphfontspecs(const XftGlyphFontSpec *, Glyph, int, int, int);
void xdrawglyph(Glyph, int, int);
void xclear(int, int, int, int);
int xgeommasktogravity(int);
void ximopen(Display *);
void ximinstantiate(Display *, XPointer, XPointer);
void ximdestroy(XIM, XPointer, XPointer);
void xinit(int, int);
void cresize(int, int);
void xresize(int, int);
void xhints(void);
int xloadcolor(int, const char *, Color *);
void xsetenv(void);
void xseturgency(int);
int evcol(XEvent *);
int evrow(XEvent *);

void expose(XEvent *);
void visibility(XEvent *);
void unmap(XEvent *);
void cmessage(XEvent *);
void resize(XEvent *);
void focus(XEvent *);
void propnotify(XEvent *);

void run(void);
void usage(void);


void statusbar_kpress( KeySym *ks, char *buf );
/* Globals */
DC dc;
XWindow xw;
TermWindow win;

char *opt_class = NULL;
char **opt_cmd = NULL;
char *opt_embed = NULL;
char *opt_font = NULL;
char *opt_io = NULL;
char *opt_line = NULL;
char *opt_name = NULL;
char *opt_title = NULL;
char opt_xresources;


// more/less font width spacing
//int fontspacing = -1;
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
void propnotify(XEvent *e) {
		XPropertyEvent *xpev;
		Atom clipboard = XInternAtom(xw.dpy, "CLIPBOARD", 0);

		xpev = &e->xproperty;
		if (xpev->state == PropertyNewValue &&
						(xpev->atom == XA_PRIMARY || xpev->atom == clipboard)) {
				selnotify(e);
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

ushort sixd_to_16bit(int x) { return x == 0 ? 0 : 0x3737 + 0x2828 * x; }

int xloadcolor(int i, const char *name, Color *ncolor) {
		XRenderColor color = {.alpha = 0xffff};

		if (!name) {
				if (BETWEEN(i, 16, 255)) {  /* 256 color */
						if (i < 6 * 6 * 6 + 16) { /* same colors as xterm */
								color.red = sixd_to_16bit(((i - 16) / 36) % 6);
								color.green = sixd_to_16bit(((i - 16) / 6) % 6);
								color.blue = sixd_to_16bit(((i - 16) / 1) % 6);
						} else { /* greyscale */
								color.red = 0x0808 + 0x0a0a * (i - (6 * 6 * 6 + 16));
								color.green = color.blue = color.red;
						}
						return XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &color, ncolor);
				} else
						name = colorname[i];
		}

		return XftColorAllocName(xw.dpy, xw.vis, xw.cmap, name, ncolor);
}

void xloadcols(void) {
		int i;
		static int loaded;
		Color *cp;

		if (loaded) {
				for (cp = dc.col; cp < &dc.col[dc.collen]; ++cp)
						XftColorFree(xw.dpy, xw.vis, xw.cmap, cp);
		} else {
				dc.collen = MAX(LEN(colorname), 256);
				dc.col = xmalloc(dc.collen * sizeof(Color));
		}

		for (i = 0; i < dc.collen; i++)
				if (!xloadcolor(i, NULL, &dc.col[i])) {
						if (colorname[i])
								die("could not allocate color '%s'\n", colorname[i]);
						else
								die("could not allocate color %d\n", i);
				}
		loaded = 1;
}

int xsetcolorname(int x, const char *name) {
		Color ncolor;

		if (!BETWEEN(x, 0, dc.collen))
				return 1;

		if (!xloadcolor(x, name, &ncolor))
				return 1;

		XftColorFree(xw.dpy, xw.vis, xw.cmap, &dc.col[x]);
		dc.col[x] = ncolor;

		return 0;
}

/*
 * Absolute coordinates.
 */
void xclear(int x1, int y1, int x2, int y2) {
		XftDrawRect(xw.draw, &dc.col[IS_SET(MODE_REVERSE) ? defaultfg : defaultbg],
						x1, y1, x2 - x1, y2 - y1);
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
		xloadcols();

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

void xdrawglyphfontspecs(const XftGlyphFontSpec *specs, Glyph base, int len,
				int x, int y) {
#ifdef UTF8
		int charlen = len * ((base.mode & ATTR_WIDE) ? 2 : 1);
#else
		int charlen = len;// * ((base.mode & ATTR_WIDE) ? 2 : 1);
#endif
		int winx = win.hborderpx + x * win.cw, winy = win.vborderpx + y * win.ch,
				width = charlen * win.cw;
		Color *fg, *bg, revfg, revbg, truefg, truebg;
		XRenderColor colfg, colbg;
		XRectangle r;

		/* Fallback on color display for attributes not supported by the font */
		if (base.mode & ATTR_ITALIC && base.mode & ATTR_BOLD) {
				if (dc.ibfont.badslant || dc.ibfont.badweight)
						base.fg = defaultattr;
		} else if ((base.mode & ATTR_ITALIC && dc.ifont.badslant) ||
						(base.mode & ATTR_BOLD && dc.bfont.badweight)) {
				base.fg = defaultattr;
		}

		if (IS_TRUECOL(base.fg)) {
				colfg.alpha = 0xffff;
				colfg.red = TRUERED(base.fg);
				colfg.green = TRUEGREEN(base.fg);
				colfg.blue = TRUEBLUE(base.fg);
				XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colfg, &truefg);
				fg = &truefg;
		} else {
				fg = &dc.col[base.fg];
		}

		if (IS_TRUECOL(base.bg)) {
				colbg.alpha = 0xffff;
				colbg.green = TRUEGREEN(base.bg);
				colbg.red = TRUERED(base.bg);
				colbg.blue = TRUEBLUE(base.bg);
				XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colbg, &truebg);
				bg = &truebg;
		} else {
				bg = &dc.col[base.bg];
		}

		/* Change basic system colors [0-7] to bright system colors [8-15] */
		if ((base.mode & ATTR_BOLD_FAINT) == ATTR_BOLD && BETWEEN(base.fg, 0, 7))
				fg = &dc.col[base.fg + 8];

		if (IS_SET(MODE_REVERSE)) {
				if (fg == &dc.col[defaultfg]) {
						fg = &dc.col[defaultbg];
				} else {
						colfg.red = ~fg->color.red;
						colfg.green = ~fg->color.green;
						colfg.blue = ~fg->color.blue;
						colfg.alpha = fg->color.alpha;
						XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colfg, &revfg);
						fg = &revfg;
				}

				if (bg == &dc.col[defaultbg]) {
						bg = &dc.col[defaultfg];
				} else {
						colbg.red = ~bg->color.red;
						colbg.green = ~bg->color.green;
						colbg.blue = ~bg->color.blue;
						colbg.alpha = bg->color.alpha;
						XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colbg, &revbg);
						bg = &revbg;
				}
		}

		if ((base.mode & ATTR_BOLD_FAINT) == ATTR_FAINT) {
				colfg.red = fg->color.red / 2;
				colfg.green = fg->color.green / 2;
				colfg.blue = fg->color.blue / 2;
				colfg.alpha = fg->color.alpha;
				XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colfg, &revfg);
				fg = &revfg;
		}

		if (base.mode & ATTR_REVERSE) {
				bg = &dc.col[selectionbg];
				if (!ignoreselfg)
						fg = &dc.col[selectionfg];
		}

		if (base.mode & ATTR_BLINK && win.mode & MODE_BLINK)
				fg = bg;

		if (base.mode & ATTR_INVISIBLE)
				fg = bg;

		/* Intelligent cleaning up of the borders. */
		if (x == 0) {
				xclear(0, (y == 0) ? 0 : winy, win.vborderpx,
								winy + win.ch +
								((winy + win.ch >= win.vborderpx + win.th) ? win.h : 0));
		}
		if (winx + width >= win.hborderpx + win.tw) {
				xclear(
								winx + width, (y == 0) ? 0 : winy, win.w,
								((winy + win.ch >= win.vborderpx + win.th) ? win.h : (winy + win.ch)));
		}
		if (y == 0)
				xclear(winx, 0, winx + width, win.hborderpx);
		if (winy + win.ch >= win.vborderpx + win.th)
				xclear(winx, winy + win.ch, winx + width, win.h);


		/* Clean up the region we want to draw to. */
		XftDrawRect(xw.draw, bg, winx, winy, width, win.ch);

		/* Set the clip region because Xft is sometimes dirty. */
		r.x = 0;
		r.y = 0;
		r.height = win.ch;
		r.width = width;
		XftDrawSetClipRectangles(xw.draw, winx, winy, &r, 1);

		/* Render the glyphs. */
		XftDrawGlyphFontSpec(xw.draw, fg, specs, len);

		/* Render underline and strikethrough. */
		if (base.mode & ATTR_UNDERLINE) {
				XftDrawRect(xw.draw, fg, winx, winy + dc.font.ascent + 1, width, 1);
		}

		if (base.mode & ATTR_STRUCK) {
				XftDrawRect(xw.draw, fg, winx, winy + 2 * dc.font.ascent / 3, width, 1);
		}

		/* Reset clip to none. */
		XftDrawSetClip(xw.draw, 0);
}

void xdrawglyph(Glyph g, int x, int y) {
		int numspecs;
		XftGlyphFontSpec spec;

		numspecs = xmakeglyphfontspecs(&spec, &g, 1, x, y);
		xdrawglyphfontspecs(&spec, g, numspecs, x, y);
}


void xdrawcursor(int cx, int cy, Glyph g, int ox, int oy, Glyph og) {
		Color drawcol;
		static int focusinx, focusiny;

		// hide cursor in lessmode
		if (inputmode&MODE_LESS)
				return;

		/* remove the old cursor */
		if (selected(ox, oy))
				og.mode ^= ATTR_REVERSE;
		xdrawglyph(og, ox, oy);

		/*
		 * Select the right color for the right mode.
		 */
#ifdef UTF8
		g.mode &= ATTR_BOLD | ATTR_ITALIC | ATTR_UNDERLINE | ATTR_STRUCK | ATTR_WIDE;
#else
		g.mode &= ATTR_BOLD | ATTR_ITALIC | ATTR_UNDERLINE | ATTR_STRUCK;
#endif

		if (IS_SET(MODE_REVERSE)) {
				g.mode |= ATTR_REVERSE;
				g.bg = defaultfg;
				if (selected(cx, cy)) {
						drawcol = dc.col[defaultcs];
						g.fg = defaultrcs;
				} else {
						drawcol = dc.col[defaultrcs];
						g.fg = defaultcs;
				}
		} else {
				if (selected(cx, cy)) {
						g.fg = defaultfg;
						g.bg = defaultrcs;
				} else {
						g.fg = defaultbg;
						g.bg = defaultcs;
				}
				drawcol = dc.col[g.bg];
		}

		/* draw text cursor */
		if (IS_SET(MODE_FOCUSED)) {

				switch (win.cursor) {
						case 7: /* st extension: snowman (U+2603) */
								// g.u = 0x2603;
								g.u = 'X';
						case 0: /* Blinking Block */
						case 1: /* Blinking Block (Default) */
						case 2: /* Steady Block */
								xdrawglyph(g, cx, cy);
								break;
						case 3: /* Blinking Underline */
						case 4: /* Steady Underline */
								if ( focusinx == cx && focusiny == cy ) {
										g.bg = 202;
										g.fg = 0;
										xdrawglyph(g, cx, cy);
										drawcol = dc.col[1];

										// a cross
										//XftDrawRect(xw.draw, &drawcol, win.hborderpx + cx * win.cw + win.cw/2 ,
										//				win.vborderpx + cy * win.ch+3, 1, win.ch-8);
										//XftDrawRect(xw.draw, &drawcol, (win.hborderpx + cx * win.cw) + 2,
										//				win.vborderpx + cy * win.ch + (win.ch/2)-1, win.cw - 2, 1);
										//win.vborderpx + cy * win.ch, 1, win.ch-win.ch/16*12);
#if 0
										// upper line
										XftDrawRect(xw.draw, &drawcol, win.hborderpx + cx * win.cw,
														win.vborderpx + cy * win.ch, win.cw - 1, 1);

										// lines at sides
										XftDrawRect(xw.draw, &drawcol, win.hborderpx + cx * win.cw,
														win.vborderpx + cy * win.ch, 1, win.ch);
										//win.vborderpx + cy * win.ch, 1, win.ch-win.ch/16*12);
										XftDrawRect(xw.draw, &drawcol, win.hborderpx + (cx + 1) * win.cw - 1,
														win.vborderpx + cy * win.ch, 1, win.ch);
										//win.vborderpx + cy * win.ch, 1, win.ch-win.ch/16*12);
#endif
#if 0
										// lower cursor part
										XftDrawRect(xw.draw, &drawcol, win.hborderpx + cx * win.cw,
														(win.vborderpx + cy * win.ch )+(win.ch/16)*12, 1, win.ch-win.ch/16*12);
										XftDrawRect(xw.draw, &drawcol, win.hborderpx + (cx + 1) * win.cw - 1,
														win.vborderpx + cy * win.ch + (win.ch/16)*12, 1, win.ch-win.ch/16*12);
										XftDrawRect(xw.draw, &drawcol, win.hborderpx + cx * win.cw,
														win.vborderpx + (cy + 1) * win.ch -1, win.cw, 1);
#endif

										// underline
										drawcol = dc.col[defaultcs];
										XftDrawRect(xw.draw, &drawcol, win.hborderpx + cx * win.cw,
														win.vborderpx + (cy + 1) * win.ch - cursorthickness, win.cw,
														cursorthickness);

								} else {
										focusinx=focusiny=0;
										drawcol = dc.col[defaultcs];
										XftDrawRect(xw.draw, &drawcol, win.hborderpx + cx * win.cw,
														win.vborderpx + (cy + 1) * win.ch - cursorthickness, win.cw,
														cursorthickness);
								}
								break;
						case 5: /* Blinking bar */
						case 6: /* Steady bar */
										XftDrawRect(xw.draw, &drawcol, win.hborderpx + cx * win.cw,
														win.vborderpx + (cy+1) * win.ch, cursorthickness, win.ch);
								break;
				}
		} else { // window hasn't the focus. 
				//g.fg = unfocusedrcs;
						drawcol = dc.col[unfocusedrcs];
				XftDrawRect(xw.draw, &drawcol, win.hborderpx + cx * win.cw,
								win.vborderpx + cy * win.ch, win.cw - 1, 1);
				XftDrawRect(xw.draw, &drawcol, win.hborderpx + cx * win.cw,
								win.vborderpx + cy * win.ch, 1, win.ch-win.ch/16*12);
				XftDrawRect(xw.draw, &drawcol, win.hborderpx + (cx + 1) * win.cw - 1,
								win.vborderpx + cy * win.ch, 1, win.ch-win.ch/16*12);


				XftDrawRect(xw.draw, &drawcol, win.hborderpx + cx * win.cw,
								(win.vborderpx + cy * win.ch )+(win.ch/16)*12, 1, win.ch-win.ch/16*12);
				XftDrawRect(xw.draw, &drawcol, win.hborderpx + (cx + 1) * win.cw - 1,
								win.vborderpx + cy * win.ch + (win.ch/16)*12, 1, win.ch-win.ch/16*12);
				XftDrawRect(xw.draw, &drawcol, win.hborderpx + cx * win.cw,
								win.vborderpx + (cy + 1) * win.ch -1, win.cw, 1);
				focusinx = cx;
				focusiny = cy;
		}
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

int xstartdraw(void) { return IS_SET(MODE_VISIBLE); }

void xdrawline(Line line, int x1, int y1, int x2) {
		int i, x, ox, numspecs;
		Glyph base, new;
		XftGlyphFontSpec *specs = xw.specbuf;

		numspecs = xmakeglyphfontspecs(specs, &line[x1], x2 - x1, x1, y1);
		i = ox = 0;
		for (x = x1; x < x2 && i < numspecs; x++) {
				new = line[x];
#ifdef UTF8
				if (new.mode == ATTR_WDUMMY)
						continue;
#endif
				if (selected(x, y1))
						new.mode ^= ATTR_REVERSE;
				if (i > 0 && ATTRCMP(base, new)) {
						xdrawglyphfontspecs(specs, base, i, ox, y1);
						specs += i;
						numspecs -= i;
						i = 0;
				}
				if (i == 0) {
						ox = x;
						base = new;
				}
				i++;
		}
		if (i > 0)
				xdrawglyphfontspecs(specs, base, i, ox, y1);
}

void xfinishdraw(void) {
		XCopyArea(xw.dpy, xw.buf, xw.win, dc.gc, 0, 0, win.w, win.h, 0, 0);
		XSetForeground(xw.dpy, dc.gc,
						dc.col[IS_SET(MODE_REVERSE) ? defaultfg : defaultbg].pixel);
}

void xximspot(int x, int y) {
		XPoint spot = {borderpx + x * win.cw, borderpx + (y + 1) * win.ch};
		XVaNestedList attr = XVaCreateNestedList(0, XNSpotLocation, &spot, NULL);

		XSetICValues(xw.xic, XNPreeditAttributes, attr, NULL);
		XFree(attr);
}

void expose(XEvent *ev) { redraw(); }

void visibility(XEvent *ev) {
		XVisibilityEvent *e = &ev->xvisibility;

		MODBIT(win.mode, e->state != VisibilityFullyObscured, MODE_VISIBLE);
}

void unmap(XEvent *ev) { win.mode &= ~MODE_VISIBLE; }

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

int xsetcursor(int cursor) {
		DEFAULT(cursor, 1);
		if (!BETWEEN(cursor, 0, 6))
				return 1;
		win.cursor = cursor;
		return 0;
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

void resize(XEvent *e) {
		if (e->xconfigure.width == win.w && e->xconfigure.height == win.h)
				return;

		cresize(e->xconfigure.width, e->xconfigure.height);
}

void run(void) {
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

void lessmode_toggle(const Arg *a){
		if (abs(a->i) == 2 ){ // enable
				inputmode |= MODE_LESS;
				//selscroll(0,0);
				tfulldirt();
				//set_notifmode( 2, -1 ); // show message "less"
		} else {
				if (abs(a->i) == 1 ){ // enable
						inputmode |= MODE_LESS;
						kscrollup(a);
				} else {
						if ( a->i == -3 ) //disable 
								inputmode &= ~MODE_LESS;
						else // toggle - i==0
								inputmode ^= MODE_LESS;
				}
		}

		if ( inputmode & MODE_LESS ){ // enable
				//set_notifmode( 2, -1 ); // show message "less"
				showstatus(1," -LESS- ");
				updatestatus();
		} else { // disable
				//set_notifmode( 4,-2 ); // hide message
				showstatus(0,0);
				scrolltobottom();
		}
}

void statusbar_kpress( KeySym *ks, char *buf ){

}

void set_fontwidth( const Arg *a ){
		fontspacing += a->i;
		Arg larg;
		larg.f = usedfontsize;
		zoomabs(&larg);
}

