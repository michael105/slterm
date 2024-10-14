#include "xdraw.h"

#include "x.h"


DC dc;

/*
 * Absolute coordinates.
 */
void xclear(int x1, int y1, int x2, int y2) {
	XftDrawRect(xw.draw, &dc.col[IS_SET(MODE_REVERSE) ? defaultfg : defaultbg],
			x1, y1, x2 - x1, y2 - y1);
}


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


Color* xdrawglyphfontspecs(const XftGlyphFontSpec *specs, Glyph base, int len,
		int x, int y) {
#ifdef UTF8
	int charlen = len * ((base.mode & ATTR_WIDE) ? 2 : 1);
#else
	int charlen = len;// * ((base.mode & ATTR_WIDE) ? 2 : 1);
#endif
	int winx = win.hborderpx + x * win.cw, winy = win.vborderpx + y * win.ch,
		 width = charlen * win.cw;
	Color *fg, *bg, revfg, revbg, truefg, truebg, *cltmp;
	XRenderColor colfg, colbg;
	XRectangle r;
	int fgcache = 0, bgcache = 0;


	// Look for the right color

	/* Fallback on color display for attributes not supported by the font */
	if (base.mode & ATTR_ITALIC && base.mode & ATTR_BOLD) {
		if (dc.ibfont.badslant || dc.ibfont.badweight)
			base.fg = defaultattr;
	} else if ((base.mode & ATTR_ITALIC && dc.ifont.badslant) ||
			(base.mode & ATTR_BOLD && dc.bfont.badweight)) {
		base.fg = defaultattr;
	}

	//get colors from cache, if present
	if (  !(win.mode & MODE_FOCUSED) || IS_SET(MODE_REVERSE) ){
		if ( ( bg =getcachecolor( 0, &base,win.mode ) ) )
			bgcache = 1;
		if ( ( fg =getcachecolor( 1, &base,win.mode ) ) )
			fgcache = 1;
	} else {
		if ( base.mode& (ATTR_BOLD|ATTR_FAINT) ){
			if ( base.fg>15 )
				if ( ( fg =getcachecolor( 1, &base,win.mode ) ) )
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
				XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colbg, &truebg);
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
				XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colfg, &truefg);
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
					XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colfg, &revfg);
					fg = &revfg;
				}
			}

			// also BOLD|FAINT
			if ( (base.mode & ATTR_FAINT) == ATTR_FAINT ) { //&& !BETWEEN(base.fg,0,15) )  
				cfaint(red); //= fg->color.red * 2;
				cfaint(green); //= fg->color.green * 2;
				cfaint(blue); //= fg->color.blue +  2;
				colfg.alpha = fg->color.alpha;//2;
				XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colfg, &revfg);
				fg = &revfg;
			}
		}
	}


#if 1

#define CLFA 24000
	// Change colors on focusout
	if ( !(win.mode & MODE_FOCUSED) ){
		/*colfg.red = fg->color.red / 2;
		  colfg.green = fg->color.green / 4 * 3;
		  colfg.blue = fg->color.blue / 4 * 3;*/
		if ( !fgcache && (fg->color.blue > CLFA) && (fg->color.red < CLFA) && (fg->color.green < CLFA) ){
			colfg.red = fg->color.red +13000;
			colfg.green = fg->color.green +13000;
			colfg.blue = fg->color.blue;
			colfg.alpha = fg->color.alpha;
			XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colfg, &revfg);
			fg = &revfg;
		}
		if ( !bgcache && (bg->color.blue > CLFA) && (bg->color.red < CLFA) && (bg->color.green < CLFA) ){
			colbg.red = bg->color.red +CLFA;
			colbg.green = bg->color.green +CLFA;
			colbg.blue = bg->color.blue;
			colbg.alpha = bg->color.alpha;
			XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colbg, &revbg);
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


				XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colfg, &revfg);
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
				XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colbg, &revbg);
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


	if (base.mode & ATTR_BLINK && win.mode & MODE_BLINK){
		if ( base.mode & ATTR_REVERSE ){ // blink by reversing colors
			cltmp = bg;
			bg = fg;
			fg = cltmp;
		} else fg = bg; // blink
	}

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

	//printf("spec %x\n",specs->glyph);
	/* Render the glyphs. */
	XftDrawGlyphFontSpec(xw.draw, fg, specs, len);

	/* Render underline and strikethrough. */
	if (base.mode & ATTR_UNDERLINE) {
		XftDrawRect(xw.draw, fg, winx, winy + dc.font.ascent + 1, width, 1);
		if (base.mode & ATTR_STRUCK) { // struck and underline - double underline
			XftDrawRect(xw.draw, fg, winx, winy + dc.font.ascent + 3, width, 1);
		} 
	} else {
		if (base.mode & ATTR_STRUCK) {
			XftDrawRect(xw.draw, fg, winx, winy + 5*dc.font.ascent /8, width, 1);
			//XftDrawRect(xw.draw, fg, winx, winy + 2 * dc.font.ascent / 3, width, 1);
		}
	}

	/* Reset clip to none. */
	XftDrawSetClip(xw.draw, 0);

	// free colors, if allocated and not in one of the tables
	if ( !fgcache && (fg == &revfg || fg == &revbg) && fg!=bg )
		cachecolor( base.mode & ATTR_REVERSE? 0:1, &base, win.mode, fg );
			//XftColorFree(xw.dpy, xw.vis, xw.cmap, &revfg);

	if ( !bgcache && (bg == &revbg || bg == &revfg) )
		cachecolor( base.mode & ATTR_REVERSE? 1:0, &base, win.mode, bg );
		//	XftColorFree(xw.dpy, xw.vis, xw.cmap, &revbg);


	return(bg);
}

Color* xdrawglyph(Glyph g, int x, int y) {
	int numspecs;
	XftGlyphFontSpec spec;

	numspecs = xmakeglyphfontspecs(&spec, &g, 1, x, y);
	return( xdrawglyphfontspecs(&spec, g, numspecs, x, y) );
}

Color *getcursorcolor( Glyph g, int cx, int cy ){
	Color *col;
	Color drawcol;
	if ( g.bg == defaultbg ){
		col = &dc.col[defaultcs];
	} else {
		if ( !( col = getcachecolor( 2, &g, win.mode ) ) ){
			col = xdrawglyph(g, cx, cy); // unneccessary, but need the bg color

			// invert bgcolor
			XRenderColor csc;
			//#define ASB(c) csc.c = 0xff-col->color.c //~col->color.c
#define ASB(c) csc.c = ~col->color.c
			ASB(red);ASB(green);ASB(blue);
#undef ASB
			XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &csc, &drawcol);
			col = cachecolor(2,&g,win.mode,&drawcol);
			//col = &drawcol;
		}

	} 


	return(col);
}

void xdrawcursor(int cx, int cy, Glyph g, int ox, int oy, Glyph og) {
	Color drawcol;
	static int focusinx, focusiny;
	uchar tmp;
	Color *col = 0;

	// hide cursor in lessmode
	if (inputmode&MODE_LESS && !(win.mode & MODE_KBDSELECT))
		return;

	//printf("xdrawcursor: %d %d, %d %d, %d\n",cx,cy,ox,oy, term->scr);
	
	/* remove the old cursor */
	if (selected(ox, oy))
		og.mode ^= ATTR_REVERSE;
	
	xdrawglyph(og, ox, oy);

	/*
	 * Select the right color for the right mode.
	 */
/*
#ifdef UTF8
	g.mode &= ATTR_BOLD | ATTR_ITALIC | ATTR_UNDERLINE | ATTR_STRUCK | ATTR_WIDE;
#else
	g.mode &= ATTR_BOLD | ATTR_ITALIC | ATTR_UNDERLINE | ATTR_STRUCK;
#endif

	if (IS_SET(MODE_REVERSE)) {
		g.mode |= ATTR_REVERSE;
		g.bg = defaultfg;
		if (selected(cx, cy)) {
			drawcol = dc.col[defaultrcs];
			g.fg = defaultbg;
		} else {
			drawcol = dc.col[defaultrcs];
			g.fg = defaultbg;
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
	*/

	/* draw text cursor */
	if (IS_SET(MODE_FOCUSED)) {

		switch (win.cursor) {
			case 7: /* st extension: snowman (U+2603) */
				// g.u = 0x2603;
				g.u = 'X';
				g.bg = 9;
			case 0: /* Blinking Block */
			case 1: /* Blinking Block */ // doesnt work out. I dislike blinking anyways.
												  // Thats a feature!
				//g.mode |= ATTR_BLINK | ATTR_REVERSE;
				//term->cursor.attr.mode |= ATTR_BLINK;
				//printf("blc\n");
				//term->cursor.attr.mode |= ATTR_BLINK;
				//tsetdirtattr(ATTR_BLINK);
				//xdrawglyph(g, cx, cy);
				//break;
			case 2: /* Steady Block */
			//if ( g.bg == defaultbg ){
			//	g.bg = defaultcs;
			//} else {
				//if ( (win.cursor==2) || !( win.mode & MODE_BLINK )){
					tmp = g.fg;
					g.fg = g.bg;
					g.bg = tmp;
				//}
			//}
				xdrawglyph(g, cx, cy);
				break;
			case 3: /* Blinking Underline */
			case 4: /* Steady Underline */
			case 8: // double underline
				if ( focusinx == cx && focusiny == cy ) { // highlight cursor on focusin
					g.bg = 208;
					g.fg = 4;
					xdrawglyph(g, cx, cy);

					drawcol = dc.col[1];
					//printf("c\n");

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

				} else { // draw cursor as underline
					focusinx=focusiny=0;
					//drawcol = dc.col[defaultcs];

					col = getcursorcolor( g, cx, cy );

					if ( win.cursor == 8 ){ // double underline
						XftDrawRect(xw.draw, col, 
							win.hborderpx + cx * win.cw,
							win.vborderpx + (cy + 1) * win.ch - 1, win.cw, 1);
						XftDrawRect(xw.draw, col, 
							win.hborderpx + cx * win.cw,
							win.vborderpx + (cy + 1) * win.ch - 3, win.cw, 1);
					} else { // underline
						XftDrawRect(xw.draw, col, win.hborderpx + cx * win.cw,
							win.vborderpx + (cy + 1) * win.ch - cursorthickness, win.cw,
							cursorthickness);
					}

				}
				break;
			case 5: /* Blinking bar */
			case 6: /* Steady bar */
					if ( g.bg == defaultbg ){
						col = &dc.col[defaultcs];
					} else {
						if ( !( col = getcachecolor( 2, &g, win.mode ) ) ){
						col = xdrawglyph(g, cx, cy); // unneccessary, but need the bg color

						// invert bgcolor
						XRenderColor csc;
//#define ASB(c) csc.c = 0xff-col->color.c //~col->color.c
#define ASB(c) csc.c = ~col->color.c
						ASB(red);ASB(green);ASB(blue);
#undef ASB
						XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &csc, &drawcol);
						cachecolor(2,&g,win.mode,&drawcol);
						col = &drawcol;
						}
					}


				XftDrawRect(xw.draw, col, win.hborderpx + cx * win.cw,
						win.vborderpx + (cy) * win.ch, cursorthickness, win.ch);
				break;
			case 9: // empty block, unfilled
					if ( g.bg == defaultbg ){
						col = &dc.col[defaultcs];
					} else {
						if ( !( col = getcachecolor( 2, &g, win.mode ) ) ){
							col = xdrawglyph(g, cx, cy); // unneccessary, but need the bg color

							// invert bgcolor
							XRenderColor csc;
							//#define ASB(c) csc.c = 0xff-col->color.c //~col->color.c
#define ASB(c) csc.c = ~col->color.c
							ASB(red);ASB(green);ASB(blue);
#undef ASB
							XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &csc, &drawcol);
							cachecolor(2,&g,win.mode,&drawcol);
							col = &drawcol;
						}
					}
					// upper line
					XftDrawRect(xw.draw, col, win.hborderpx + cx * win.cw,
							win.vborderpx + cy * win.ch, win.cw - 1, 1);

					// lines at sides
					XftDrawRect(xw.draw, col, win.hborderpx + cx * win.cw,
							win.vborderpx + cy * win.ch, 1, win.ch);
					//win.vborderpx + cy * win.ch, 1, win.ch-win.ch/16*12);
					XftDrawRect(xw.draw, col, win.hborderpx + (cx + 1) * win.cw - 1,
							win.vborderpx + cy * win.ch, 1, win.ch);
					//win.vborderpx + cy * win.ch, 1, win.ch-win.ch/16*12);
					// lower cursor part
					//	XftDrawRect(xw.draw, col, win.hborderpx + cx * win.cw,
					//			(win.vborderpx + cy * win.ch )+(win.ch/16)*12, 1, win.ch-win.ch/16*12);
					//	XftDrawRect(xw.draw, col, win.hborderpx + (cx + 1) * win.cw - 1,
					//			win.vborderpx + cy * win.ch + (win.ch/16)*12, 1, win.ch-win.ch/16*12);
					XftDrawRect(xw.draw, col, win.hborderpx + cx * win.cw,
							win.vborderpx + (cy + 1) * win.ch -1, win.cw, 1);

					break;

			case 11: // 
			case 10: // bar with two lines at the sides
					col = getcursorcolor( g, cx, cy );
					// lower cursor part
					XftDrawRect(xw.draw, col, win.hborderpx + cx * win.cw,
								(win.vborderpx + cy * win.ch )+(win.ch*12)/16, 1, win.ch-win.ch*12/16 );
								//(win.vborderpx + cy * win.ch )+(win.ch/16)*12, 1, win.ch-win.ch/16*12);
					XftDrawRect(xw.draw, col, win.hborderpx + (cx + 1) * win.cw - 1,
								(win.vborderpx + cy * win.ch )+(win.ch*12)/16, 1, win.ch-win.ch*12/16 );
								//win.vborderpx + cy * win.ch + (win.ch/16)*12, 1, win.ch-win.ch/16*12);
					XftDrawRect(xw.draw, col, win.hborderpx + cx * win.cw,
							win.vborderpx + (cy + 1) * win.ch -1, win.cw, 1);

					if ( win.cursor == 10 ) 
						break;
			case 12:
					if ( !col ) col = getcursorcolor( g, cx, cy );
					// upper line
					XftDrawRect(xw.draw, col, win.hborderpx + cx * win.cw,
							win.vborderpx + cy * win.ch, win.cw - 1, 1);

					// lines at sides
					XftDrawRect(xw.draw, col, win.hborderpx + cx * win.cw,
						win.vborderpx + cy * win.ch, 1, win.ch-win.ch*12/16);
					XftDrawRect(xw.draw, col, win.hborderpx + (cx + 1) * win.cw - 1,
						win.vborderpx + cy * win.ch, 1, win.ch-win.ch*12/16);
					break;
			case 13:  // unfinished
					col = getcursorcolor( g, cx, cy );
				// a cross
					XftDrawRect(xw.draw, col, win.hborderpx + cx * win.cw + win.cw/2 ,
					win.vborderpx + cy * win.ch+3, 1, win.ch - win.ch*12/16);
					XftDrawRect(xw.draw, col, (win.hborderpx + cx * win.cw) + 2,
					win.vborderpx + cy * win.ch + (win.ch/2)-1, win.ch - win.ch*12/16, 1);
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

int xstartdraw(void) { return IS_SET(MODE_VISIBLE); }

void xfinishdraw(void) {
	XCopyArea(xw.dpy, xw.buf, xw.win, dc.gc, 0, 0, win.w, win.h, 0, 0);
	XSetForeground(xw.dpy, dc.gc,
			dc.col[IS_SET(MODE_REVERSE) ? defaultfg : defaultbg].pixel);
}


