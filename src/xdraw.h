#ifndef xdraw_h
#define xdraw_h


typedef XftColor Color;
typedef XftDraw *Draw;

/* Drawing Context */
typedef struct {
		Color *col;
		size_t collen;
		Font font, bfont, ifont, ibfont;
		GC gc;
} DC;



extern DC dc;



void xdrawglyphfontspecs(const XftGlyphFontSpec *, Glyph, int, int, int);
void xdrawglyph(Glyph, int, int);
void xclear(int, int, int, int);

int xstartdraw(void);

void xdrawcursor(int, int, Glyph, int, int, Glyph);
void xdrawline(Line, int, int, int);
void xfinishdraw(void);



#endif

