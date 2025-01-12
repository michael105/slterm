#include "xdraw.h"

#include "xwindow.h"


DC dc;

/*
 * Absolute coordinates.
 */
void xclear(int x1, int y1, int x2, int y2) {
	XftDrawRect(xwin.draw, &dc.col[IS_SET(MODE_REVERSE) ? defaultfg : defaultbg],
			x1, y1, x2 - x1, y2 - y1);
}


void xdrawline(Line line, int x1, int y1, int x2) {
	int i, x, ox, numspecs;
	Glyph base, new;
	XftGlyphFontSpec *specs = xwin.specbuf;

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


Color* xdrawglyphfontspecs(const XftGlyphFontSpec *specs, Glyph base, int len,
		int x, int y) {
#ifdef UTF8
	int charlen = len * ((base.mode & ATTR_WIDE) ? 2 : 1);
#else
	int charlen = len;// * ((base.mode & ATTR_WIDE) ? 2 : 1);
#endif
	int winx = twin.hborderpx + x * twin.cw, winy = twin.vborderpx + y * twin.ch,
		 width = charlen * twin.cw;
	Color *fg, *bg, revfg, revbg, truefg, truebg, *cltmp;
	XRenderColor colfg, colbg;
	XRectangle r;
	int fgcache = 0, bgcache = 0;


	// Look for the right color

	/* Fallback on color display for attributes not supported by the font */
	/*
	if (base.mode & ATTR_ITALIC && base.mode & ATTR_BOLD) {
		if (dc.ibfont.badslant || dc.ibfont.badweight)
			base.fg = defaultattr;
	} else if ((base.mode & ATTR_ITALIC && dc.ifont.badslant) ||
			(base.mode & ATTR_BOLD && dc.bfont.badweight)) {
		base.fg = defaultattr;
	}
	*/

	//get colors from cache, if present
	if (  !(twin.mode & MODE_FOCUSED) || IS_SET(MODE_REVERSE) ){
		if ( ( bg =getcachecolor( 0, &base,twin.mode ) ) )
			bgcache = 1;
		if ( ( fg =getcachecolor( 1, &base,twin.mode ) ) )
			fgcache = 1;
	} else {
		if ( base.mode& (ATTR_BOLD|ATTR_FAINT) ){
			if ( base.fg>15 )
				if ( ( fg =getcachecolor( 1, &base,twin.mode ) ) )
					fgcache = 1;
		}
	}

	if ( !bgcache ){
		if ( base.bg < 16 ){ //misc use 16 color bg table
			bg = &dc.bgcolors[base.bg];
		} else {
			if (IS_TRUECOL(base.bg)) { // Always false, if compiled without UTF8
				colbg.alpha = 0xffff;
				colbg.green = TRUEGREEN(base.bg);
				colbg.red = TRUERED(base.bg);
				colbg.blue = TRUEBLUE(base.bg);
				XftColorAllocValue(xwin.dpy, xwin.vis, xwin.cmap, &colbg, &truebg);
				bg = &truebg;
			} else {
				bg = &dc.col[base.bg];
			}
		}
	}


	if ( !fgcache ){
		if ( base.fg < 8 ){
			//printf("<c %d>an",base.fg);
			int fi = 0;
			if ((base.mode & ATTR_FAINT) == ATTR_FAINT )
				fi = 2;
			if ((base.mode & ATTR_BOLD) == ATTR_BOLD )
				fi++;

			fg = &dc.colortable[base.fg+8*fi];
		} else if ( base.fg < 16 ) {
			fg = &dc.colortable[base.fg];
		} else {
			// old code
			if (IS_TRUECOL(base.fg)) {
				//printf("Truecolor\n");
				colfg.alpha = 0xffff;
				colfg.red = TRUERED(base.fg);
				colfg.green = TRUEGREEN(base.fg);
				colfg.blue = TRUEBLUE(base.fg);
				XftColorAllocValue(xwin.dpy, xwin.vis, xwin.cmap, &colfg, &truefg);
				fg = &truefg;
			} else {
				fg = &dc.col[base.fg];
			}


			//#define cbold(c) colfg.c = fg->color.c + boldf <= 0xffff ? fg->color.c+boldf : fg->color.c ;
#define cbold(c) colfg.c = ((fg->color.c + fg->color.c/2) | fg->color.c ) & 0xffff
			//#define cbold(c) colfg.c = fg->color.c > 250?  : fg->color.c+ 5;
			//#define cfaint(c) colfg.c = fg->color.c - (fg->color.c/2)
#define cfaint(c) colfg.c = fg->color.c - fg->color.c/2; // prev: - .c/4
																			//
			/* Change basic system colors [0-7] to bright system colors [8-15] */
			if ((base.mode & ATTR_BOLD) == ATTR_BOLD ){
				if ( BETWEEN(base.fg, 0, 7)){
					fg = &dc.col[base.fg + 8];
				} else { 
					//if ( ( (base.mode & ATTR_BOLD) == ATTR_BOLD ) && !BETWEEN(base.fg,0,15) )  
					cbold(red); //= fg->color.red * 2;
					cbold(green); //= fg->color.green * 2;
					cbold(blue); //= fg->color.blue +  2;
					colfg.alpha = fg->color.alpha;
					XftColorAllocValue(xwin.dpy, xwin.vis, xwin.cmap, &colfg, &revfg);
					fg = &revfg;
				}
			}

			// also BOLD|FAINT
			if ( (base.mode & ATTR_FAINT) == ATTR_FAINT ) { //&& !BETWEEN(base.fg,0,15) )  
				cfaint(red); //= fg->color.red * 2;
				cfaint(green); //= fg->color.green * 2;
				cfaint(blue); //= fg->color.blue +  2;
				colfg.alpha = fg->color.alpha;//2;
				XftColorAllocValue(xwin.dpy, xwin.vis, xwin.cmap, &colfg, &revfg);
				fg = &revfg;
			}
		}
	}


#if 1

#define CLFA 24000
	// Change colors on focusout
	if ( !(twin.mode & MODE_FOCUSED) ){
		/*colfg.red = fg->color.red / 2;
		  colfg.green = fg->color.green / 4 * 3;
		  colfg.blue = fg->color.blue / 4 * 3;*/
		if ( !fgcache && (fg->color.blue > CLFA) && (fg->color.red < CLFA) && (fg->color.green < CLFA) ){
			colfg.red = fg->color.red +13000;
			colfg.green = fg->color.green +13000;
			colfg.blue = fg->color.blue;
			colfg.alpha = fg->color.alpha;
			XftColorAllocValue(xwin.dpy, xwin.vis, xwin.cmap, &colfg, &revfg);
			fg = &revfg;
		}
		if ( !bgcache && (bg->color.blue > CLFA) && (bg->color.red < CLFA) && (bg->color.green < CLFA) ){
			colbg.red = bg->color.red +CLFA;
			colbg.green = bg->color.green +CLFA;
			colbg.blue = bg->color.blue;
			colbg.alpha = bg->color.alpha;
			XftColorAllocValue(xwin.dpy, xwin.vis, xwin.cmap, &colbg, &revbg);
			bg = &revbg;
		}
	}

#endif


	if (IS_SET(MODE_REVERSE)) { // inverse mode (Ctrl+Shift+I)
		if ( !fgcache ){
			if ( fg == &dc.col[defaultfg]) {
				fg = &dc.col[defaultbg];
			} else {
				colfg.alpha = fg->color.alpha;
#define AS(c) colfg.c = ~fg->color.c
				FOR_RGB(AS);
#undef AS


				XftColorAllocValue(xwin.dpy, xwin.vis, xwin.cmap, &colfg, &revfg);
				fg = &revfg;
			}
		}

		if ( !bgcache ){
			if ( bg == &dc.col[defaultbg]) {
				bg = &dc.col[defaultfg];
			} else {
				colbg.alpha = bg->color.alpha;
#define ASB(c) colbg.c = ~bg->color.c
				FOR_RGB(ASB);
#undef ASB
				XftColorAllocValue(xwin.dpy, xwin.vis, xwin.cmap, &colbg, &revbg);
				bg = &revbg;
			}
		}
	}


	// reverse background and foreground colors, Attribute 7
	if (base.mode & ATTR_REVERSE) {
			cltmp = bg;
			bg = fg;
			fg = cltmp;
	}


	if (base.mode & ATTR_INVISIBLE)
		fg = bg; // top secret


	if (base.mode & ATTR_BLINK && twin.mode & MODE_BLINK){
		if ( base.mode & ATTR_REVERSE ){ // blink by reversing colors
			cltmp = bg;
			bg = fg;
			fg = cltmp;
		} else fg = bg; // blink
	}

	/* Intelligent cleaning up of the borders. */
	if (x == 0) {
		xclear(0, (y == 0) ? 0 : winy, twin.vborderpx,
				winy + twin.ch +
				((winy + twin.ch >= twin.vborderpx + twin.th) ? twin.h : 0));
	}
	if (winx + width >= twin.hborderpx + twin.tw) {
		xclear(
				winx + width, (y == 0) ? 0 : winy, twin.w,
				((winy + twin.ch >= twin.vborderpx + twin.th) ? twin.h : (winy + twin.ch)));
	}
	if (y == 0)
		xclear(winx, 0, winx + width, twin.hborderpx);
	if (winy + twin.ch >= twin.vborderpx + twin.th)
		xclear(winx, winy + twin.ch, winx + width, twin.h);


	/* Clean up the region we want to draw to. */
	XftDrawRect(xwin.draw, bg, winx, winy, width, twin.ch);

	/* Set the clip region because Xft is sometimes dirty. */
	r.x = 0;
	r.y = 0;
	r.height = twin.ch;
	r.width = width;
	XftDrawSetClipRectangles(xwin.draw, winx, winy, &r, 1);

	//printf("spec %x\n",specs->glyph);
	/* Render the glyphs. */
	XftDrawGlyphFontSpec(xwin.draw, fg, specs, len);

	/* Render underline and strikethrough. */
	if (base.mode & ATTR_UNDERLINE) {
		XftDrawRect(xwin.draw, fg, winx, winy + dc.font.ascent + 1, width, 1);
		if (base.mode & ATTR_STRUCK) { // struck and underline - double underline
			XftDrawRect(xwin.draw, fg, winx, winy + dc.font.ascent + 3, width, 1);
		} 
	} else {
		if (base.mode & ATTR_STRUCK) {
			XftDrawRect(xwin.draw, fg, winx, winy + 5*dc.font.ascent /8, width, 1);
			//XftDrawRect(xwin.draw, fg, winx, winy + 2 * dc.font.ascent / 3, width, 1);
		}
	}

	/* Reset clip to none. */
	XftDrawSetClip(xwin.draw, 0);

	// free colors, if allocated and not in one of the tables
	if ( !fgcache && (fg == &revfg || fg == &revbg) && fg!=bg )
		cachecolor( base.mode & ATTR_REVERSE? 0:1, &base, twin.mode, fg );
			//XftColorFree(xwin.dpy, xwin.vis, xwin.cmap, &revfg);

	if ( !bgcache && (bg == &revbg || bg == &revfg) )
		cachecolor( base.mode & ATTR_REVERSE? 1:0, &base, twin.mode, bg );
		//	XftColorFree(xwin.dpy, xwin.vis, xwin.cmap, &revbg);


	return(bg);
}

Color* xdrawglyph(Glyph g, int x, int y) {
	int numspecs;
	XftGlyphFontSpec spec;

	numspecs = xmakeglyphfontspecs(&spec, &g, 1, x, y);
	return( xdrawglyphfontspecs(&spec, g, numspecs, x, y) );
}

int xstartdraw(void) { return IS_SET(MODE_VISIBLE); }

void xfinishdraw(void) {
	XCopyArea(xwin.dpy, xwin.buf, xwin.win, dc.gc, 0, 0, twin.w, twin.h, 0, 0);
	XSetForeground(xwin.dpy, dc.gc,
			dc.col[IS_SET(MODE_REVERSE) ? defaultfg : defaultbg].pixel);
}


