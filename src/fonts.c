// font loading and handling
//
//


#include "includes.h"
#include "fonts.h"
#include "x.h"
#include "config.h"


char *usedfont = NULL;
double usedfontsize = 0;
double defaultfontsize = 0;

/* Fontcache is an array now. A new font will be appended to the array. */
Fontcache *frc = NULL;
int frclen = 0;
int frccap = 0;


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
		xloadfonts(usedfont, arg->f);
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



int xloadfont(Font *f, FcPattern *pattern) {
		FcPattern *configured;
		FcPattern *match;
		FcResult result;
		XGlyphInfo extents;
		int wantattr, haveattr;

		/*
		 * Manually configure instead of calling XftMatchFont
		 * so that we can use the configured pattern for
		 * "missing glyph" lookups.
		 */
		configured = FcPatternDuplicate(pattern);
		if (!configured)
				return 1;

		FcConfigSubstitute(NULL, configured, FcMatchPattern);
		XftDefaultSubstitute(xw.dpy, xw.scr, configured);

		match = FcFontMatch(NULL, configured, &result);
		if (!match) {
				FcPatternDestroy(configured);
				return 1;
		}

		if (!(f->match = XftFontOpenPattern(xw.dpy, match))) {
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
						fputs("font slant does not match\n", stderr);
				}
		}

		if ((XftPatternGetInteger(pattern, "weight", 0, &wantattr) ==
								XftResultMatch)) {
				if ((XftPatternGetInteger(f->match->pattern, "weight", 0, &haveattr) !=
										XftResultMatch) ||
								haveattr != wantattr) {
						f->badweight = 1;
						fputs("font weight does not match\n", stderr);
				}
		}

//		XftTextExtentsUtf8(xw.dpy, f->match, (const FcChar8 *)ascii_printable,
//						strlen(ascii_printable), &extents);
		char printable[255];
		int p = 0;
		for ( int a = 32; a<127; a++) 
			printable[p++] = a;
		for ( int a = 128; a<256; a++) 
			printable[p++] = a;
		printable[p] = 0;


		XftTextExtentsUtf8(xw.dpy, f->match, (const FcChar8 *)printable,
						sizeof(printable), &extents);



		f->set = NULL;
		f->pattern = configured;

		f->ascent = f->match->ascent;
		f->descent = f->match->descent;
		f->lbearing = 0;
		f->rbearing = f->match->max_advance_width;

		f->height = f->ascent + f->descent;
#ifdef UTF8
		f->width = DIVCEIL(extents.xOff, strlen(ascii_printable));
		//f->width = DIVCEIL(extents.xOff,190 );
#else
		//f->width=8;
		f->width = DIVCEIL(extents.xOff, 96);
		//f->width = DIVCEIL(extents.xOff, 96);
#endif

		f->width += fontspacing;
		if ( f->width < 1 )
				f->width=1;

		return 0;
}

void xloadfonts(char *fontstr, double fontsize) {
		FcPattern *pattern;
		double fontval;

		if (fontstr[0] == '-')
				pattern = XftXlfdParse(fontstr, False, False);
		else
				pattern = FcNameParse((FcChar8 *)fontstr);

		if (!pattern)
				die("can't open font %s\n", fontstr);

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
						 * Default font size is 12, if none given. This is to
						 * have a known usedfontsize value.
						 */
						FcPatternAddDouble(pattern, FC_PIXEL_SIZE, 12);
						usedfontsize = 12;
				}
				defaultfontsize = usedfontsize;
		}

		if (xloadfont(&dc.font, pattern))
				die("can't open font %s\n", fontstr);

		if (usedfontsize < 0) {
				FcPatternGetDouble(dc.font.match->pattern, FC_PIXEL_SIZE, 0, &fontval);
				usedfontsize = fontval;
				if (fontsize == 0)
						defaultfontsize = fontval;
		}

		/* Setting character width and height. */
		win.cw = ceilf(dc.font.width * cwscale);
		win.ch = ceilf(dc.font.height * chscale);

		borderpx = ceilf(((float)borderperc / 100) * win.cw);

		FcPatternDel(pattern, FC_SLANT);
		FcPatternAddInteger(pattern, FC_SLANT, FC_SLANT_ITALIC);
		if (xloadfont(&dc.ifont, pattern))
				die("can't open font %s\n", fontstr);

		FcPatternDel(pattern, FC_WEIGHT);
		FcPatternAddInteger(pattern, FC_WEIGHT, FC_WEIGHT_BOLD);
		if (xloadfont(&dc.ibfont, pattern))
				die("can't open font %s\n", fontstr);

		FcPatternDel(pattern, FC_SLANT);
		FcPatternAddInteger(pattern, FC_SLANT, FC_SLANT_ROMAN);
		if (xloadfont(&dc.bfont, pattern))
				die("can't open font %s\n", fontstr);

		FcPatternDestroy(pattern);
}

void xunloadfont(Font *f) {
		XftFontClose(xw.dpy, f->match);
		FcPatternDestroy(f->pattern);
		if (f->set)
				FcFontSetDestroy(f->set);
}

void xunloadfonts(void) {
		/* Free the loaded fonts in the font cache.  */
		while (frclen > 0)
				XftFontClose(xw.dpy, frc[--frclen].font);

		xunloadfont(&dc.font);
		xunloadfont(&dc.bfont);
		xunloadfont(&dc.ifont);
		xunloadfont(&dc.ibfont);
}

static const unsigned short cp850[128] = {
  /* 0x80 */
  0x00c7, 0x00fc, 0x00e9, 0x00e2, 0x00e4, 0x00e0, 0x00e5, 0x00e7,
  0x00ea, 0x00eb, 0x00e8, 0x00ef, 0x00ee, 0x00ec, 0x00c4, 0x00c5,
  /* 0x90 */
  0x00c9, 0x00e6, 0x00c6, 0x00f4, 0x00f6, 0x00f2, 0x00fb, 0x00f9,
  0x00ff, 0x00d6, 0x00dc, 0x00f8, 0x00a3, 0x00d8, 0x00d7, 0x0192,
  /* 0xa0 */
  0x00e1, 0x00ed, 0x00f3, 0x00fa, 0x00f1, 0x00d1, 0x00aa, 0x00ba,
  0x00bf, 0x00ae, 0x00ac, 0x00bd, 0x00bc, 0x00a1, 0x00ab, 0x00bb,
  /* 0xb0 */
  0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x00c1, 0x00c2, 0x00c0,
  0x00a9, 0x2563, 0x2551, 0x2557, 0x255d, 0x00a2, 0x00a5, 0x2510,
  /* 0xc0 */
  0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c, 0x00e3, 0x00c3,
  0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c, 0x00a4,
  /* 0xd0 */
  0x00f0, 0x00d0, 0x00ca, 0x00cb, 0x00c8, 0x0131, 0x00cd, 0x00ce,
  0x00cf, 0x2518, 0x250c, 0x2588, 0x2584, 0x00a6, 0x00cc, 0x2580,
  /* 0xe0 */
  0x00d3, 0x00df, 0x00d4, 0x00d2, 0x00f5, 0x00d5, 0x00b5, 0x00fe,
  0x00de, 0x00da, 0x00db, 0x00d9, 0x00fd, 0x00dd, 0x00af, 0x00b4,
  /* 0xf0 */
  0x00ad, 0x00b1, 0x2017, 0x00be, 0x00b6, 0x00a7, 0x00f7, 0x00b8,
  0x00b0, 0x00a8, 0x00b7, 0x00b9, 0x00b3, 0x00b2, 0x25a0, 0x00a0,
};


static const unsigned short cp437[128] = {
  /* 0x80 */
  0x00c7, 0x00fc, 0x00e9, 0x00e2, 0x00e4, 0x00e0, 0x00e5, 0x00e7,
  0x00ea, 0x00eb, 0x00e8, 0x00ef, 0x00ee, 0x00ec, 0x00c4, 0x00c5,
  /* 0x90 */
  0x00c9, 0x00e6, 0x00c6, 0x00f4, 0x00f6, 0x00f2, 0x00fb, 0x00f9,
  0x00ff, 0x00d6, 0x00dc, 0x00a2, 0x00a3, 0x00a5, 0x20a7, 0x0192,
  /* 0xa0 */
  0x00e1, 0x00ed, 0x00f3, 0x00fa, 0x00f1, 0x00d1, 0x00aa, 0x00ba,
  0x00bf, 0x2310, 0x00ac, 0x00bd, 0x00bc, 0x00a1, 0x00ab, 0x00bb,
  /* 0xb0 */
  0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
  0x2555, 0x2563, 0x2551, 0x2557, 0x255d, 0x255c, 0x255b, 0x2510,
  /* 0xc0 */
  0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c, 0x255e, 0x255f,
  0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c, 0x2567,
  /* 0xd0 */
  0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256b,
  0x256a, 0x2518, 0x250c, 0x2588, 0x2584, 0x258c, 0x2590, 0x2580,
  /* 0xe0 */
  0x03b1, 0x00df, 0x0393, 0x03c0, 0x03a3, 0x03c3, 0x00b5, 0x03c4,
  0x03a6, 0x0398, 0x03a9, 0x03b4, 0x221e, 0x03c6, 0x03b5, 0x2229,
  /* 0xf0 */
  0x2261, 0x00b1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00f7, 0x2248,
  0x00b0, 0x2219, 0x00b7, 0x221a, 0x207f, 0x00b2, 0x25a0, 0x00a0,
};


static const unsigned short cp1250[128] = {
  /* 0x80 */
  0x20ac, 0xfffd, 0x201a, 0xfffd, 0x201e, 0x2026, 0x2020, 0x2021,
  0xfffd, 0x2030, 0x0160, 0x2039, 0x015a, 0x0164, 0x017d, 0x0179,
  /* 0x90 */
  0xfffd, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
  0xfffd, 0x2122, 0x0161, 0x203a, 0x015b, 0x0165, 0x017e, 0x017a,
  /* 0xa0 */
  0x00a0, 0x02c7, 0x02d8, 0x0141, 0x00a4, 0x0104, 0x00a6, 0x00a7,
  0x00a8, 0x00a9, 0x015e, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x017b,
  /* 0xb0 */
  0x00b0, 0x00b1, 0x02db, 0x0142, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
  0x00b8, 0x0105, 0x015f, 0x00bb, 0x013d, 0x02dd, 0x013e, 0x017c,
  /* 0xc0 */
  0x0154, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x0139, 0x0106, 0x00c7,
  0x010c, 0x00c9, 0x0118, 0x00cb, 0x011a, 0x00cd, 0x00ce, 0x010e,
  /* 0xd0 */
  0x0110, 0x0143, 0x0147, 0x00d3, 0x00d4, 0x0150, 0x00d6, 0x00d7,
  0x0158, 0x016e, 0x00da, 0x0170, 0x00dc, 0x00dd, 0x0162, 0x00df,
  /* 0xe0 */
  0x0155, 0x00e1, 0x00e2, 0x0103, 0x00e4, 0x013a, 0x0107, 0x00e7,
  0x010d, 0x00e9, 0x0119, 0x00eb, 0x011b, 0x00ed, 0x00ee, 0x010f,
  /* 0xf0 */
  0x0111, 0x0144, 0x0148, 0x00f3, 0x00f4, 0x0151, 0x00f6, 0x00f7,
  0x0159, 0x016f, 0x00fa, 0x0171, 0x00fc, 0x00fd, 0x0163, 0x02d9,
};

static const unsigned short cp1252[32] = {
  /* 0x80 */
  0x20ac, 0xfffd, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
  0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0xfffd, 0x017d, 0xfffd,
  /* 0x90 */
  0xfffd, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
  0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0xfffd, 0x017e, 0x0178,
};


uint convert(uint rune){
	// 1252 / dec-mcs / ansi
	if ( rune > 0x7f && (rune < 0xA0) )
		return( cp1252[rune-0x80] );
	//if ( rune > 0x7f && (rune < 0x100) )
	//	return( cp1250[rune-0x80] );

	return(rune);
}

int xmakeglyphfontspecs(XftGlyphFontSpec *specs, const Glyph *glyphs, int len,
				int x, int y) {
		float winx = win.hborderpx + x * win.cw, winy = win.vborderpx + y * win.ch,
		xp, yp;
		ushort mode, prevmode = USHRT_MAX;
		Font *font = &dc.font;
		int frcflags = FRC_NORMAL;
		float runewidth = win.cw;
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
				rune = convert( glyphs[i].u );
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
						runewidth = win.cw * ((mode & ATTR_WIDE) ? 2.0f : 1.0f);
#else
						runewidth = win.cw;// * ((mode & ATTR_WIDE) ? 2.0f : 1.0f);
#endif
						if ((mode & ATTR_ITALIC) && (mode & ATTR_BOLD)) {
								font = &dc.ibfont;
								frcflags = FRC_ITALICBOLD;
						} else if (mode & ATTR_ITALIC) {
								font = &dc.ifont;
								frcflags = FRC_ITALIC;
						} else if (mode & ATTR_BOLD) {
								font = &dc.bfont;
								frcflags = FRC_BOLD;
						}
						yp = winy + font->ascent;
				}
					//if ( rune>=0x80 )
					//printf("rune: %x  \n",rune,glyphidx);
				/* Lookup character index with default font. */
				glyphidx = XftCharIndex(xw.dpy, font->match, rune);
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
				}

				/* Fallback on font cache, search the font cache for match. */
				for (f = 0; f < frclen; f++) {
						glyphidx = XftCharIndex(xw.dpy, frc[f].font, rune);
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
						FcPatternAddBool(fcpattern, FC_SCALABLE, 1);

						FcConfigSubstitute(0, fcpattern, FcMatchPattern);
						FcDefaultSubstitute(fcpattern);

						fontpattern = FcFontSetMatch(0, fcsets, 1, fcpattern, &fcres);

						/* Allocate memory for the new cache entry. */
						if (frclen >= frccap) {
								frccap += 16;
								frc = xrealloc(frc, frccap * sizeof(Fontcache));
						}

						frc[frclen].font = XftFontOpenPattern(xw.dpy, fontpattern);
						if (!frc[frclen].font)
								die("XftFontOpenPattern failed seeking fallback font: %s\n",
												strerror(errno));
						frc[frclen].flags = frcflags;
						frc[frclen].unicodep = rune;

						glyphidx = XftCharIndex(xw.dpy, frc[frclen].font, rune);

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


