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
		XftTextExtentsUtf8(xw.dpy, f->match, (const FcChar8 *)ascii_printable,
						sizeof(ascii_printable), &extents);



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


int xmakeglyphfontspecs(XftGlyphFontSpec *specs, const Glyph *glyphs, int len,
				int x, int y) {
		float winx = win.hborderpx + x * win.cw, winy = win.vborderpx + y * win.ch,
		xp, yp;
		ushort mode, prevmode = USHRT_MAX;
		Font *font = &dc.font;
		int frcflags = FRC_NORMAL;
		float runewidth = win.cw;
		Rune rune;
		FT_UInt glyphidx;
		FcResult fcres;
		FcPattern *fcpattern, *fontpattern;
		FcFontSet *fcsets[] = {NULL};
		FcCharSet *fccharset;
		int i, f, numspecs = 0;

		for (i = 0, xp = winx, yp = winy + font->ascent; i < len; ++i) {
				/* Fetch rune and mode for current glyph. */
				rune = glyphs[i].u;
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

				/* Lookup character index with default font. */
				glyphidx = XftCharIndex(xw.dpy, font->match, rune);
				if (glyphidx) {
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


