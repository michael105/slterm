#ifndef fonts_h
#define fonts_h


#include <X11/Xft/Xft.h>

#include "xevent.h"
#include "term.h"

typedef XftGlyphFontSpec GlyphFontSpec;


//extern char *usedfont;


/* Font structure */
#define Font Font_
typedef struct {
#ifndef UTF8
		int asciitable[256]; // cache codepoints
#endif
		int height;
		int width;
		int ascent;
		int descent;
		int badslant;
		int badweight;
		short lbearing;
		short rbearing;
		XftFont *match;
		FcFontSet *set;
		FcPattern *pattern;
} Font;

// font cache. growing array. no ring. as stated below.
// in theory, this shouldn't be global, the cache depends
// on the used font.
enum { FRC_NORMAL, FRC_ITALIC, FRC_BOLD, FRC_ITALICBOLD };

typedef struct {
		XftFont *font;
		int flags;
		Rune unicodep;
} Fontcache;

/* Fontcache is an array now. A new font will be appended to the array. */
Fontcache *frc = NULL;
int frclen = 0;
int frccap = 0;


// callbacks
void set_fontwidth( const Arg *a );
void set_fontheight( const Arg *a );
void zoom(const Arg *);
void zoomabs(const Arg *);
void zoomreset(const Arg *);


int noutf8_xmakeglyphfontspecs(XftGlyphFontSpec *, const Glyph *, int, int, int);
int xmakeglyphfontspecs(XftGlyphFontSpec *, const Glyph *, int, int, int);


int xloadfont(Font *, FcPattern *, int pixelsize, const char* filename);

void xloadfonts(double);
void xunloadfont(Font *);
void xunloadfonts(void);


void fill_font_asciitable(Font *f);

#endif

