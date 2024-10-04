// Color related functions
// Default colors are defined in config.h

#include "colors.h"


ushort sixd_to_16bit(int x) { return x == 0 ? 0 : 0x3737 + 0x2828 * x; }


static XftColor cc_rc[COLORCACHESIZE];
static uint cc_mode[COLORCACHESIZE];
static uint cc_p;

// get a cached color, if present
Color* getcachecolor( uint fg, Glyph*g, uint winmode ){
	
	for ( int a = 0; a<cc_p; a++ ){
		if ( cc_mode[a] == (( (fg<<16) | ( (winmode<<24)&0xff000000) | ( fg==1?g->fg:g->bg) ) |(g->mode<<8))){
		//printf("cachehit: %d  %d  %x\n",fg,a,cc_mode[a]);
		if ( a>0 ){
			//printf("swap: %d\n",a);
			Color tmp;
			memcpy( &tmp, &cc_rc[a], sizeof(Color) );
			memcpy( &cc_rc[a], &cc_rc[a-1], sizeof(Color) );
			memcpy( &cc_rc[a-1], &tmp, sizeof(Color) );
			uint t = cc_mode[a];
			cc_mode[a] = cc_mode[a-1];
			cc_mode[a-1] = t;
			a--;
		}
			return( &cc_rc[a] );
		}
	}
	return(0);
}



// Cache a color, fg: 0=bg, 1=fg, 2=cursor(fg)
void cachecolor( uint fg, Glyph*g, uint winmode, Color *color ){ 
	if ( getcachecolor(fg,g,winmode) ){
		//printf("double store\n");
		return;
	}
	if ( cc_p == COLORCACHESIZE-1 ){
		//printf("Free cachecolor\n");
		XftColorFree(xw.dpy, xw.vis, xw.cmap, &cc_rc[cc_p]);
		//cc_rc[cc_p] = cc_rc[0];
	} else cc_p++;

	memmove( &cc_rc[1], &cc_rc[0], sizeof(Color)*(COLORCACHESIZE-1) );
	memmove( &cc_mode[1], &cc_mode[0], sizeof(uint)*(COLORCACHESIZE-1) );
	memcpy( cc_rc, color, sizeof(Color) );
	cc_mode[0] = ( fg<<16 ) | ((winmode<<24)&0xff000000 ) | g->mode<<8 | ( fg==1?g->fg:g->bg);
	//printf("cache: %d  %x\n",fg,cc_mode[cc_p]);
}



int xloadcolor(int i, const char *name, Color *p_color) {
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
			return XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &color, p_color);
		} else
			name = colorname[i]; // color 0..16
	}

	return XftColorAllocName(xw.dpy, xw.vis, xw.cmap, name, p_color);
}


void xloadcolors(void) {
	int i;
	static int loaded;
	Color *cp;

	if (loaded) {
		printf("Loaded!! - colors.c\n");
		return; // dunno. this code drives me crazy
				  // there might be memory leaks
				  // misc24
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

	// Load gradient tables
	dc.colortable = xmalloc( 4*8*sizeof(Color) );
	Color *p_color = dc.colortable;
	for ( int a = 0; a<4; a++ ){
		for ( int b = 0; b<8; b++ ){
			 if ( !XftColorAllocName(xw.dpy, xw.vis, xw.cmap, colortablenames[b][a], p_color) ){
				 printf( "Cannot load color: %s\n",colortablenames[b][a] );
			 	if ( !XftColorAllocName(xw.dpy, xw.vis, xw.cmap, "gray", p_color) )
				 	die( "Cannot load fallback \"gray\"\n" );
			 }
			 p_color++;
		}
	}

	dc.bgcolors = xmalloc( 16*sizeof(Color) );
	p_color = dc.bgcolors;
	for ( int a = 0; a<16; a++ ){
		if ( !XftColorAllocName(xw.dpy, xw.vis, xw.cmap, bgcolornames[a],p_color) ){
			printf( "Cannot load color: %s\n",bgcolornames[a]);
			if ( !XftColorAllocName(xw.dpy, xw.vis, xw.cmap, "gray", p_color) )
				die( "Cannot load fallback \"gray\"\n" );
		}
		p_color++;
	}

		loaded = 1;
}

int xsetcolorname(int x, const char *name) {
	Color p_color;

	if (!BETWEEN(x, 0, dc.collen))
		return 1;

	if (!xloadcolor(x, name, &p_color))
		return 1;

	XftColorFree(xw.dpy, xw.vis, xw.cmap, &dc.col[x]);
	dc.col[x] = p_color;

	return 0;
}


#if 0
// is yet in xdraw.c. somepointer trouble, as to expect.
void getGlyphColor( Glyph *base, Color **pfg, Color **pbg ){
	XRenderColor colfg, colbg;
	Color revfg, revbg, truefg, truebg, *cltmp;
	Color *fg,*bg;

	/* Fallback on color display for attributes not supported by the font */
	if (base->mode & ATTR_ITALIC && base->mode & ATTR_BOLD) {
		if (dc.ibfont.badslant || dc.ibfont.badweight)
			base->fg = defaultattr;
	} else if ((base->mode & ATTR_ITALIC && dc.ifont.badslant) ||
			(base->mode & ATTR_BOLD && dc.bfont.badweight)) {
		base->fg = defaultattr;
	}

	if (IS_TRUECOL(base->fg)) {
		//printf("Truecolor\n");
		colfg.alpha = 0xffff;
		colfg.red = TRUERED(base->fg);
		colfg.green = TRUEGREEN(base->fg);
		colfg.blue = TRUEBLUE(base->fg);
		XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colfg, &truefg);
		fg = &truefg;
	} else {
		fg = &dc.col[base->fg];
	}

	if (IS_TRUECOL(base->bg)) {
		colbg.alpha = 0xffff;
		colbg.green = TRUEGREEN(base->bg);
		colbg.red = TRUERED(base->bg);
		colbg.blue = TRUEBLUE(base->bg);
		XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colbg, &truebg);
		bg = &truebg;
	} else {
		bg = &dc.col[base->bg];
	}
#define AS(c) colfg.c = fg->color.c
#define ASB(c) colbg.c = bg->color.c
	//AS(red);AS(green);AS(blue);AS(alpha);
	//ASB(red);ASB(green);ASB(blue);ASB(alpha);
#undef AS
#undef ASB



#define boldf 0xfff
	//#define cbold(c) colfg.c = fg->color.c + boldf <= 0xffff ? fg->color.c+boldf : fg->color.c ;
#define cbold(c) colfg.c = ((fg->color.c + fg->color.c/2) | fg->color.c ) & 0xffff
	//#define cbold(c) colfg.c = fg->color.c > 250?  : fg->color.c+ 5;
	//#define cfaint(c) colfg.c = fg->color.c - (fg->color.c/2)
#define cfaint(c) colfg.c = fg->color.c - fg->color.c/2; // prev: - .c/4
																			//
	/* Change basic system colors [0-7] to bright system colors [8-15] */
	if ((base->mode & ATTR_BOLD) == ATTR_BOLD ){
		if ( BETWEEN(base->fg, 0, 7)){
			fg = &dc.col[base->fg + 8];
	} else { 
	//if ( ( (base->mode & ATTR_BOLD) == ATTR_BOLD ) && !BETWEEN(base->fg,0,15) )  {
		cbold(red); //= fg->color.red * 2;
		cbold(green); //= fg->color.green * 2;
		cbold(blue); //= fg->color.blue +  2;
		colfg.alpha = fg->color.alpha;
		XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colfg, &revfg);
		fg = &revfg;
		}
	}

		// also BOLD|FAINT
	if ( (base->mode & ATTR_FAINT) == ATTR_FAINT ) { //&& !BETWEEN(base->fg,0,15) )  {
		cfaint(red); //= fg->color.red * 2;
		cfaint(green); //= fg->color.green * 2;
		cfaint(blue); //= fg->color.blue +  2;
		colfg.alpha = fg->color.alpha;//2;
		XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colfg, &revfg);
		fg = &revfg;
	}



	if (IS_SET(MODE_REVERSE)) {
		if (fg == &dc.col[defaultfg]) {
			fg = &dc.col[defaultbg];
		} else {
			//						colfg.blue = ~fg->color.blue;
			//						colfg.alpha = fg->color.alpha;
#define AS(c) colfg.c = ~colfg.c
			AS(red);AS(green);AS(blue);
#undef AS


			XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colfg, &revfg);
			fg = &revfg;
		}

		if ( bg == &dc.col[defaultbg]) {
			bg = &dc.col[defaultfg];
		} else {
			//					fprintf(stderr,"inv\n");//D
			//colbg.red = ~bg->color.red;
			//colbg.green = ~bg->color.green;
			//colbg.blue = ~bg->color.blue;
			//colbg.alpha = bg->color.alpha;
#define ASB(c) colbg.c = ~bg->color.c
			ASB(red);ASB(green);ASB(blue);
#undef ASB
			XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colbg, &revbg);
			bg = &revbg;
		}
	}


#define CLFA 24000
	// Change colors on focusout
	if ( !(win.mode & MODE_FOCUSED) ){
		/*colfg.red = fg->color.red / 2;
		  colfg.green = fg->color.green / 4 * 3;
		  colfg.blue = fg->color.blue / 4 * 3;*/
		if ( (fg->color.blue > CLFA) && (fg->color.red < CLFA) && (fg->color.green < CLFA) ){
			colfg.red = fg->color.red +13000;
			colfg.green = fg->color.green +13000;
			colfg.blue = fg->color.blue;
			colfg.alpha = fg->color.alpha;
			XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colfg, &revfg);
			fg = &revfg;
		}
		if ( (bg->color.blue > CLFA) && (bg->color.red < CLFA) && (bg->color.green < CLFA) ){
			colbg.red = bg->color.red +CLFA;
			colbg.green = bg->color.green +CLFA;
			colbg.blue = bg->color.blue;
			colbg.alpha = bg->color.alpha;
			XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colbg, &revbg);
			bg = &revbg;
		}
#if 0
		if ( (fg->color.blue > CLFA) && (fg->color.red < CLFA) ){
			colfg.red = fg->color.red +CLFA;
			colfg.green = fg->color.green;// +CLFA;
			colfg.blue = fg->color.blue;
			colfg.alpha = fg->color.alpha;
			XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colfg, &revfg);
			fg = &revfg;
		}
		if ( (fg->color.blue > CLFA) && (fg->color.green < CLFA) ){
			colfg.red = fg->color.red;// +CLFA;
			colfg.green = fg->color.green +CLFA;
			colfg.blue = fg->color.blue;
			colfg.alpha = fg->color.alpha;
			XftColorAllocValue(xw.dpy, xw.vis, xw.cmap, &colfg, &revfg);
			fg = &revfg;
		}
#endif
	}


	if (base->mode & ATTR_REVERSE) {
		//	fprintf(stderr,"attrinv\n");//D
#if 0
		bg = &dc.col[selectionbg];
		if (!ignoreselfg)
			fg = &dc.col[selectionfg];
#else
			cltmp = bg;
			bg = fg;
			fg = cltmp;
//#define AS(c) {int tc = colfg.c; colfg.c=colbg.c;colbg.c = tc;}
//					AS(red);AS(green);AS(blue);
//#undef AS
#endif
		//				fg = &revfg;
		//				bg = &revbg;
	}

	if (base->mode & ATTR_BLINK && win.mode & MODE_BLINK)
		fg = bg;

	if (base->mode & ATTR_INVISIBLE)
		fg = bg;

	*pfg = fg;
	*pbg = bg;
}
#endif


