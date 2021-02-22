#ifndef fonts_h
#define fonts_h

typedef XftGlyphFontSpec GlyphFontSpec;


/* Font structure */
#define Font Font_
typedef struct {
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

/* Font Ring Cache */
enum { FRC_NORMAL, FRC_ITALIC, FRC_BOLD, FRC_ITALICBOLD };

typedef struct {
		XftFont *font;
		int flags;
		Rune unicodep;
} Fontcache;



int xmakeglyphfontspecs(XftGlyphFontSpec *, const Glyph *, int, int, int);


int xloadfont(Font *, FcPattern *);

void xloadfonts(char *, double);
void xunloadfont(Font *);
void xunloadfonts(void);

#endif

