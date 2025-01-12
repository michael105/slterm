// cursor drawing and
// cursor color functions
//

#include "xcursor.h"


int _xsetcursor(int cursor, int attr) {
	DEFAULT(cursor, 1);
	if (!BETWEEN(cursor, 0, 12))
		return 1;
	twin.cursor = cursor;
	twin.cursor_attr[0] = attr;
	return 0;
}

int xgetcursor(){
	return(twin.cursor);
}


Color *getcursorcolor( Glyph g, int cx, int cy ){
	Color *col;
	Color drawcol;
	if ( g.bg == defaultbg ){
		col = &dc.col[defaultcs];
	} else {
		if ( !( col = getcachecolor( 2, &g, twin.mode ) ) ){
			col = xdrawglyph(g, cx, cy); // unneccessary, but need the bg color

			// invert bgcolor
			XRenderColor csc;
			//#define ASB(c) csc.c = 0xff-col->color.c //~col->color.c
#define ASB(c) csc.c = ~col->color.c
			ASB(red);ASB(green);ASB(blue);
#undef ASB
			XftColorAllocValue(xwin.dpy, xwin.vis, xwin.cmap, &csc, &drawcol);
			col = cachecolor(2,&g,twin.mode,&drawcol);
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
	if (inputmode&MODE_LESS && !(twin.mode & MODE_KBDSELECT))
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

		switch (twin.cursor) {
			case 7: /* st extension: snowman (U+2603) */
				// g.u = 0x2603;
				if ( twin.cursor_attr[0] )
					g.u = twin.cursor_attr[0];
				else
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
				//if ( (twin.cursor==2) || !( twin.mode & MODE_BLINK )){
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

					// underline
					drawcol = dc.col[defaultcs];
					XftDrawRect(xwin.draw, &drawcol, twin.hborderpx + cx * twin.cw,
							twin.vborderpx + (cy + 1) * twin.ch - cursorthickness, twin.cw,
							cursorthickness);

				} else { // draw cursor as underline
					focusinx=focusiny=0;
					//drawcol = dc.col[defaultcs];

					col = getcursorcolor( g, cx, cy );

					if ( twin.cursor == 8 ){ // double underline
						XftDrawRect(xwin.draw, col, 
							twin.hborderpx + cx * twin.cw,
							twin.vborderpx + (cy + 1) * twin.ch - 1, twin.cw, 1);
						XftDrawRect(xwin.draw, col, 
							twin.hborderpx + cx * twin.cw,
							twin.vborderpx + (cy + 1) * twin.ch - 3, twin.cw, 1);
					} else { // underline
						XftDrawRect(xwin.draw, col, twin.hborderpx + cx * twin.cw,
							twin.vborderpx + (cy + 1) * twin.ch - cursorthickness, twin.cw,
							cursorthickness);
					}

				}
				break;

			case 5: /* Blinking bar */
			case 6: /* Steady bar */
				col = getcursorcolor( g, cx, cy );
				
				XftDrawRect(xwin.draw, col, twin.hborderpx + cx * twin.cw,
						twin.vborderpx + (cy) * twin.ch, cursorthickness, twin.ch);
				break;

			case 9: // empty block, unfilled
					col = getcursorcolor( g, cx, cy );
					
					// upper line
					XftDrawRect(xwin.draw, col, twin.hborderpx + cx * twin.cw,
							twin.vborderpx + cy * twin.ch, twin.cw - 1, 1);

					// lines at sides
					XftDrawRect(xwin.draw, col, twin.hborderpx + cx * twin.cw,
							twin.vborderpx + cy * twin.ch, 1, twin.ch);

					XftDrawRect(xwin.draw, col, twin.hborderpx + (cx + 1) * twin.cw - 1,
							twin.vborderpx + cy * twin.ch, 1, twin.ch);
					
					// lower line
					XftDrawRect(xwin.draw, col, twin.hborderpx + cx * twin.cw,
							twin.vborderpx + (cy + 1) * twin.ch -1, twin.cw, 1);

					break;

			case 11: // 
			case 10: // bar with two lines at the sides
					col = getcursorcolor( g, cx, cy );
					// lower cursor part
					XftDrawRect(xwin.draw, col, twin.hborderpx + cx * twin.cw,
								(twin.vborderpx + cy * twin.ch )+(twin.ch*12)/16, 1, twin.ch-twin.ch*12/16 );

					XftDrawRect(xwin.draw, col, twin.hborderpx + (cx + 1) * twin.cw - 1,
								(twin.vborderpx + cy * twin.ch )+(twin.ch*12)/16, 1, twin.ch-twin.ch*12/16 );

					XftDrawRect(xwin.draw, col, twin.hborderpx + cx * twin.cw,
							twin.vborderpx + (cy + 1) * twin.ch -1, twin.cw, 1);

					if ( twin.cursor == 10 ) 
						break;

			case 12:
					if ( !col ) col = getcursorcolor( g, cx, cy );
					// upper line
					XftDrawRect(xwin.draw, col, twin.hborderpx + cx * twin.cw,
							twin.vborderpx + cy * twin.ch, twin.cw - 1, 1);

					// lines at sides
					XftDrawRect(xwin.draw, col, twin.hborderpx + cx * twin.cw,
						twin.vborderpx + cy * twin.ch, 1, twin.ch-twin.ch*12/16);
					XftDrawRect(xwin.draw, col, twin.hborderpx + (cx + 1) * twin.cw - 1,
						twin.vborderpx + cy * twin.ch, 1, twin.ch-twin.ch*12/16);
					break;

			case 13:  // unfinished
					col = getcursorcolor( g, cx, cy );
				// a cross
					XftDrawRect(xwin.draw, col, twin.hborderpx + cx * twin.cw + twin.cw/2 ,
					twin.vborderpx + cy * twin.ch+3, 1, twin.ch - twin.ch*12/16);
					XftDrawRect(xwin.draw, col, (twin.hborderpx + cx * twin.cw) + 2,
					twin.vborderpx + cy * twin.ch + (twin.ch/2)-1, twin.ch - twin.ch*12/16, 1);
				break;


	
		}
	} else { // window hasn't the focus. 
				//g.fg = unfocusedrcs;
		drawcol = dc.col[unfocusedrcs];
		XftDrawRect(xwin.draw, &drawcol, twin.hborderpx + cx * twin.cw,
				twin.vborderpx + cy * twin.ch, twin.cw - 1, 1);
		XftDrawRect(xwin.draw, &drawcol, twin.hborderpx + cx * twin.cw,
				twin.vborderpx + cy * twin.ch, 1, twin.ch-twin.ch/16*12);
		XftDrawRect(xwin.draw, &drawcol, twin.hborderpx + (cx + 1) * twin.cw - 1,
				twin.vborderpx + cy * twin.ch, 1, twin.ch-twin.ch/16*12);


		XftDrawRect(xwin.draw, &drawcol, twin.hborderpx + cx * twin.cw,
				(twin.vborderpx + cy * twin.ch )+(twin.ch/16)*12, 1, twin.ch-twin.ch/16*12);
		XftDrawRect(xwin.draw, &drawcol, twin.hborderpx + (cx + 1) * twin.cw - 1,
				twin.vborderpx + cy * twin.ch + (twin.ch/16)*12, 1, twin.ch-twin.ch/16*12);
		XftDrawRect(xwin.draw, &drawcol, twin.hborderpx + cx * twin.cw,
				twin.vborderpx + (cy + 1) * twin.ch -1, twin.cw, 1);
		focusinx = cx;
		focusiny = cy;
	}
}


