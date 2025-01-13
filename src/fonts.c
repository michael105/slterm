// font loading and handling
//
//


#include "includes.h"
#include "fonts.h"
#include "xwindow.h"
#include "config.h"

#if EMBEDFONT == 1
	 #define SINFL_IMPLEMENTATION
  	 #include "sinfl.h"
#endif
 
#if 0
#define INCLUDED_FONT
#include "font_ttf.h"
#endif

//char *usedfont = NULL;
double usedfontsize = 0;
double defaultfontsize = 0;



// callbacks
void set_fontwidth( const Arg *a ){
	fontspacing += a->i;
	Arg larg;
	larg.f = usedfontsize;
	zoomabs(&larg);
}


void zoom(const Arg *arg) {
	Arg larg;

	larg.f = usedfontsize + arg->f;
	zoomabs(&larg);
}

void zoomabs(const Arg *arg) {
	xunloadfonts();
	xloadfonts(arg->f);
	cresize(0, 0);
	redraw();
	xhints();
}

void zoomreset(const Arg *arg) {
	Arg larg;

	if (defaultfontsize > 0) {
		larg.f = defaultfontsize;
		zoomabs(&larg);
	}
}



int xloadfont(Font *f, FcPattern *pattern, int pixelsize,  const char* fontfile){
	//#define xloadfont(_font,_pattern,...) __xloadfont(_font,_pattern,__VA_OPT__(__VA_ARGS__,) 0 )
	//#define __xloadfont(_font,_pattern,_file,...) _xloadfont(_font,_pattern,_file)
	FcPattern *configured;
	FcPattern *match;
	FcResult result;
	XGlyphInfo extents;
	int wantattr, haveattr;

	if (pixelsize) {
		FcPatternDel(pattern, FC_PIXEL_SIZE);
		FcPatternDel(pattern, FC_SIZE);
		FcPatternAddDouble(pattern, FC_PIXEL_SIZE, (double)pixelsize);
	}

	/*
	 * Manually configure instead of calling XftMatchFont
	 * so that we can use the configured pattern for
	 * "missing glyph" lookups.
	 */
	configured = FcPatternDuplicate(pattern);
	if (!configured)
		return 1;

	FcConfigSubstitute(NULL, configured, FcMatchPattern);
	XftDefaultSubstitute(xwin.dpy, xwin.scr, configured);

	match = FcFontMatch(NULL, configured, &result);
	if (!match && !fontfile ) {
		FcPatternDestroy(configured);
		return 1;
	}

	if ( *fontfile ){
		FcPatternDel( match, FC_FILE );
		FcPatternAddString(match, FC_FILE, (const FcChar8 *) fontfile);
	}

//#ifdef DEBUG
#if 0
	FcPatternPrint( match );
	char *s = FcNameUnparse( match );
	printf( "match name: %s\n", s );
	free(s);
#endif


	f->match = XftFontOpenPattern(xwin.dpy, match);
	if (!f->match) {
		FcPatternDestroy(configured);
		FcPatternDestroy(match);
		return 1;
	}


	if ((XftPatternGetInteger(pattern, "slant", 0, &wantattr) ==
				XftResultMatch)) {
		/*
		 * Check if xft was unable to find a font with the appropriate
		 * slant but gave us one anyway. Try to mitigate.
		 */
		if ((XftPatternGetInteger(f->match->pattern, "slant", 0, &haveattr) !=
					XftResultMatch) ||
				haveattr < wantattr) {
			f->badslant = 1;
			fputs("***** font slant does not match\n", stderr);
		}
	}

	if ((XftPatternGetInteger(pattern, "weight", 0, &wantattr) ==
				XftResultMatch)) {
		if ((XftPatternGetInteger(f->match->pattern, "weight", 0, &haveattr) !=
					XftResultMatch) ||
				haveattr != wantattr) {
			f->badweight = 1;
			fputs("***** font weight does not match\n", stderr);
		}
	}

	if ( fontwidth ){
		f->width = fontwidth;
	} else {


#ifdef UTF8
			XftTextExtentsUtf8(xwin.dpy, f->match, (const FcChar8 *)ascii_printable,
							strlen(ascii_printable), &extents);
	f->width = DIVCEIL(extents.xOff, strlen(ascii_printable));
	//f->width = DIVCEIL(extents.xOff,190 );

#else
			// might be unneeded, fonts are rendered as monospace
			// don't know. something is happening. It works.
			//
	char printable[255];
	int p = 0;
	for ( int a = 32; a<127; a++) 
		printable[p++] = a; 
	for ( int a = 128; a<256; a++) 
		printable[p++] = charmap_convert(a,0); // 
	printable[p] = 0;


	XftTextExtents8(xwin.dpy, f->match, (const FcChar8 *)printable,
	//XftTextExtentsUtf8(xwin.dpy, f->match, (const FcChar8 *)printable,
			(127-32)+127, &extents);
			//sizeof(printable), &extents);
	//f->width=8;
	f->width = DIVCEIL(extents.xOff, (127-32)+127 );
	//printf("w:%d, height: %d\n",f->width, f->height);
	// pixelsize 13: w 8, h 15
	//f->width = DIVCEIL(extents.xOff, 96);

#endif
	}

	f->set = NULL;
	f->pattern = configured;

	f->ascent = f->match->ascent;
	f->descent = f->match->descent;
	f->lbearing = 0;
	f->rbearing = f->match->max_advance_width;

	if ( fontheight )
		f->height = fontheight;
	else
		f->height = f->ascent + f->descent;

	f->width += fontspacing;
	if ( f->width < 1 )
		f->width=1;

	return 0;
}

FcPattern *get_fcpattern( const char* fontstr ){
	if (fontstr[0] == '-')
		return( XftXlfdParse(fontstr, False, False) );
	return( FcNameParse((FcChar8 *)fontstr) );
}

// load the configured fonts, with (optional) fontsize
void xloadfonts(double fontsize) {
	FcPattern *pattern;
	double fontval;

 	// get fontstr, from commandline or config.h 
	char* fontstr = (opt_font == NULL) ? regular_font : opt_font;
	
	pattern = get_fcpattern(fontstr);

	if (!pattern)
		die("can't open font %s\n", fontstr);

	// determine the fontsize
	if (fontsize > 1) {
		FcPatternDel(pattern, FC_PIXEL_SIZE);
		FcPatternDel(pattern, FC_SIZE);
		FcPatternAddDouble(pattern, FC_PIXEL_SIZE, (double)fontsize);
		usedfontsize = fontsize;
	} else {
		if (FcPatternGetDouble(pattern, FC_PIXEL_SIZE, 0, &fontval) ==
				FcResultMatch) {
			usedfontsize = fontval;
		} else if (FcPatternGetDouble(pattern, FC_SIZE, 0, &fontval) ==
				FcResultMatch) {
			usedfontsize = -1;
		} else {
			/*
			 * Default font size, if none given. This is to
			 * have a known usedfontsize value.
			 */
			FcPatternAddDouble(pattern, FC_PIXEL_SIZE, default_font_pixelsize);
			usedfontsize = default_font_pixelsize;
		}
		defaultfontsize = usedfontsize;
	}


	// names of temporary font files
	char fname[4][32] = { {0},{0},{0},{0} };
	
#if EMBEDFONT == 1
  	 #warning embedding fonts
  	 #include "embed/embed_font.h"

  	 int fd[4] = { 0,0,0,0 };
  	  uchar *embfont[4] = { 
  	 	slterm_font_ttfz, 
  	 	slterm_font_bold_ttfz, 
  	 	slterm_font_italic_ttfz, 
  	 	slterm_font_bold_italic_ttfz };
  	  unsigned int embfontlenz[4] = { 
  	 	slterm_font_ttfz_len, 
  	 	slterm_font_bold_ttfz_len, 
  	 	slterm_font_italic_ttfz_len, 
  	 	slterm_font_bold_italic_ttfz_len };
  	  unsigned int embfontlen[4] = { 
  	 	slterm_font_ttf_len, 
  	 	slterm_font_bold_ttf_len, 
  	 	slterm_font_italic_ttf_len, 
  	 	slterm_font_bold_italic_ttf_len };
  	 

	  size_t bufsize = 128;
  	 for ( int i = 0; i<4 ; i++ )
		 if ( embfontlen[i]+128 > bufsize )
		 	bufsize = embfontlen[i] + 128;

	 {
		struct { 
			unsigned char buf[bufsize];
		 	unsigned long canary;
		} s; 
		s.canary = 0xa23df1223;
		
		 // prevent compiler "optimizations" by using (portable) assembly,
		 // demonstrating the power of void
		 // a warning: volatile is useless. the keyword just doesn't work. (sometimes..)
		 asm( "" :"+m"(s.canary) );

		 for ( int i = 0; i<4 ; i++ ){
			 if ( embfontlenz[i]){ 
#ifdef DEBUG
				 printf("Decompress Font: %d -> %d\n",embfontlenz[i],embfontlen[i]);
#endif
				 strcpy( fname[i], "/tmp/slterm_XXXXXX.ttf" );
				 fd[i] = mkstemps( fname[i], 4 );
				 if ( fd[i] <= 0 ){
					 fprintf(stderr,"Cannot create temporary file\n");
					 fd[i] = 0;
					 *fname[i] = 0;
				 } else {
					 uint len = sinflate( s.buf, bufsize, embfont[i], embfontlenz[i] );
					 if ( len != embfontlen[i] ){
						 fprintf(stderr, "Error decompressing embedded font #%d\n",i);
						 close( fd[i] );
						 unlink(fname[i]);
						 *fname[i] = 0;
					 } else {
						 write( fd[i], s.buf, embfontlen[i] );
						 fsync( fd[i] );
					 }
				 }
			 }
		 } 
		 if ( s.canary != 0xa23df1223 ){
			 die("buffer overflow");
		 }
	 }

#endif

	// load regular font
	if (xloadfont(&dc.font, pattern, 0, fname[0] ))
		die("x can't open font %s\n", fontstr);

	if (usedfontsize < 0) { // determine the used fontsize as pixelsize of the regular font
		FcPatternGetDouble(dc.font.match->pattern, FC_PIXEL_SIZE, 0, &fontval);
		usedfontsize = fontval;
		if (fontsize == 0)
			defaultfontsize = fontval;
	}

	/* Setting character width and height. */
	twin.cw = ceilf(dc.font.width * cwscale);
	twin.ch = ceilf(dc.font.height * chscale);

	borderpx = ceilf(((float)borderperc / 100) * twin.cw);

	if ( useboldfont ){
		if ( bold_font ){
			FcPatternDestroy(pattern);
			pattern = get_fcpattern(bold_font);
		} else {
			FcPatternDel(pattern, FC_WEIGHT);
			FcPatternAddInteger(pattern, FC_WEIGHT, FC_WEIGHT_BOLD);
		}
		if (xloadfont(&dc.bfont, pattern, usedfontsize, fname[1] ))
			die("can't open font %s\n", fontstr);
		FcPatternDel(pattern, FC_WEIGHT);
	}


	if ( useitalicfont ){
		if ( italic_font ){
			FcPatternDestroy(pattern);
			pattern = get_fcpattern(italic_font);
		} else {
			FcPatternDel(pattern, FC_SLANT);
			FcPatternAddInteger(pattern, FC_SLANT, FC_SLANT_ITALIC);
		}
		if (xloadfont(&dc.ifont, pattern, usedfontsize, fname[2] ))
			die("can't open font %s\n", fontstr);
	}


	if ( usebolditalicfont ){
		if ( bolditalic_font ){
			FcPatternDestroy(pattern);
			pattern = get_fcpattern(bolditalic_font);
		} else {
			FcPatternDel(pattern, FC_WEIGHT);
			FcPatternAddInteger(pattern, FC_WEIGHT, FC_WEIGHT_BOLD);
		}
		if (xloadfont(&dc.ibfont, pattern, usedfontsize, fname[3]))
			die("can't open font %s\n", fontstr);
	}


	FcPatternDestroy(pattern);

	#if EMBEDFONT == 1
		// remove fonts from tmp, if written
		for ( int i = 0; i<4; i++ ){
			if (fd[i])
				close (fd[i]);
			if ( *fname[i] )
				unlink( fname[i] );
		}
	#endif

}

void xunloadfont(Font *f) {
	XftFontClose(xwin.dpy, f->match);
	FcPatternDestroy(f->pattern);
	if (f->set)
		FcFontSetDestroy(f->set);
}

void xunloadfonts(void) {
	/* Free the loaded fonts in the font cache.  */
	while (frclen > 0)
		XftFontClose(xwin.dpy, frc[--frclen].font);

	xunloadfont(&dc.font);
	if ( useboldfont ) 
		xunloadfont(&dc.bfont);
	if ( useitalicfont ) 
		xunloadfont(&dc.ifont);
	if ( usebolditalicfont ) 
		xunloadfont(&dc.ibfont);
}

int xmakeglyphfontspecs(XftGlyphFontSpec *specs, const Glyph *glyphs, int len,
		int x, int y) {
	float winx = twin.hborderpx + x * twin.cw, winy = twin.vborderpx + y * twin.ch,
	xp, yp;
	ushort mode, prevmode = USHRT_MAX;
	Font *font = &dc.font;
	int frcflags = FRC_NORMAL;
	float runewidth = twin.cw;
	//Rune rune;
	uint rune;
	FT_UInt glyphidx;
	FcResult fcres;
	FcPattern *fcpattern, *fontpattern;
	FcFontSet *fcsets[] = {NULL};
	FcCharSet *fccharset;
	int i, f, numspecs = 0;

	for (i = 0, xp = winx, yp = winy + font->ascent; i < len; ++i) {
		/* Fetch rune and mode for current glyph. */
		rune = charmap_convert( glyphs[i].u,glyphs[i].mode );
		mode = glyphs[i].mode;

#ifdef UTF8
		/* Skip dummy wide-character spacing. */
		if (mode == ATTR_WDUMMY)
			continue;
#endif

		/* Determine font for glyph if different from previous glyph. */
		if (prevmode != mode) {
			prevmode = mode;
			font = &dc.font;
			frcflags = FRC_NORMAL;
#ifdef UTF8
			runewidth = twin.cw * ((mode & ATTR_WIDE) ? 2.0f : 1.0f);
#else
			runewidth = twin.cw;// * ((mode & ATTR_WIDE) ? 2.0f : 1.0f);
#endif
			if ((mode & ATTR_ITALIC) && (mode & ATTR_BOLD) && usebolditalicfont ) {
				font = &dc.ibfont;
				frcflags = FRC_ITALICBOLD;
			} else if (mode & ATTR_ITALIC && useitalicfont ) {
				font = &dc.ifont;
				frcflags = FRC_ITALIC;
			} else if (mode & ATTR_BOLD && useboldfont) {
				font = &dc.bfont;
				frcflags = FRC_BOLD;
			}
			yp = winy + font->ascent;
		}
		//if ( rune>=0x80 )
		//printf("rune: %x  \n",rune,glyphidx);
		/* Lookup character index with default font. */

		// todo: cache that. ( besser auch die charmap )
		glyphidx = XftCharIndex(xwin.dpy, font->match, rune);
		if (glyphidx) {
			//if ( rune>0x80 )
			//printf("rune: %x  idx: %x\n",rune,glyphidx);
			specs[numspecs].font = font->match;
			specs[numspecs].glyph = glyphidx;
			specs[numspecs].x = (short)xp;
			specs[numspecs].y = (short)yp;
			xp += runewidth;
			numspecs++;
			continue;
		} else {
			// todo: load chars when loading the font, spare that
			// the table is limited to ascii 255
		}

		/* Fallback on font cache, search the font cache for match. */
		// TODO: reverse that. "cache" fonts in a table indexed by ascii
		for (f = 0; f < frclen; f++) {
			glyphidx = XftCharIndex(xwin.dpy, frc[f].font, rune);
			/* Everything correct. */
			if (glyphidx && frc[f].flags == frcflags)
				break;
			/* We got a default font for a not found glyph. */
			if (!glyphidx && frc[f].flags == frcflags && frc[f].unicodep == rune) {
				break;
			}
		}

		/* Nothing was found. Use fontconfig to find matching font. */
		if (f >= frclen) {
			// not in cache
			fprintf(stderr,"rune missing in font, unicode: %x  ascii: %x\n",rune,glyphs[i].u);

			if (!font->set)
				font->set = FcFontSort(0, font->pattern, 1, 0, &fcres);
			fcsets[0] = font->set;

			/*
			 * Nothing was found in the cache. Now use
			 * some dozen of Fontconfig calls to get the
			 * font for one single character.
			 *
			 * Xft and fontconfig are design failures.
			 */
			fcpattern = FcPatternDuplicate(font->pattern);
			fccharset = FcCharSetCreate();

			FcCharSetAddChar(fccharset, rune);
			FcPatternAddCharSet(fcpattern, FC_CHARSET, fccharset);
			//FcPatternAddBool(fcpattern, FC_SCALABLE, 1);

			FcConfigSubstitute(0, fcpattern, FcMatchPattern);
			FcDefaultSubstitute(fcpattern);

		//FcPatternDel(fcpattern, FC_FAMILY);
			//FcPatternDel(fcpattern, FC_WEIGHT);

			fontpattern = FcFontSetMatch(0, fcsets, 1, fcpattern, &fcres);
			//fontpattern = FcFontMatch(NULL, fcpattern, &fcres);

			/* Allocate memory for the new cache entry. */
			if (frclen >= frccap) {
				frccap += 16;
				frc = xrealloc(frc, frccap * sizeof(Fontcache));
			}

			frc[frclen].font = XftFontOpenPattern(xwin.dpy, fontpattern);
			if (!frc[frclen].font)
				die("XftFontOpenPattern failed seeking fallback font: %s\n",
						strerror(errno));
			frc[frclen].flags = frcflags;
			frc[frclen].unicodep = rune;

			glyphidx = XftCharIndex(xwin.dpy, frc[frclen].font, rune);

			fprintf(stderr,"found idx: %d\nfrclen: %d\n",glyphidx,frclen);
			// misc. ok. memeory leak. frclen, if runes aren't found.
			// todo: write a rune buffer for codepoints and fonts.

			if ( glyphidx ){
				//FcPatternPrint( fcpattern );

			}

			f = frclen;
			frclen++;

			FcPatternDestroy(fcpattern);
			FcCharSetDestroy(fccharset);
		}

		specs[numspecs].font = frc[f].font;
		specs[numspecs].glyph = glyphidx;
		specs[numspecs].x = (short)xp;
		specs[numspecs].y = (short)yp;
		xp += runewidth;
		numspecs++;
	}

	return numspecs;
}


