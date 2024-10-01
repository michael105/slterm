#ifndef xdraw_h
#define xdraw_h


typedef XftColor Color;
typedef XftDraw *Draw;

/* Drawing Context */
typedef struct {
		Color *col; // Pointer to an array of 256 colors
		size_t collen;
		Color *colortable; // Colors 0..7 in normal, bold, faint, bold|faint 
								 // Pointer to an array
		Color *bgcolors; // Background colors 0..15
		Font font, bfont, ifont, ibfont;
		GC gc;
} DC;



extern DC dc;


// Returns the used background color
Color* xdrawglyphfontspecs(const XftGlyphFontSpec *, Glyph, int, int, int);
// Returns the used background color
Color* xdrawglyph(Glyph, int, int);
void xclear(int, int, int, int);

int xstartdraw(void);

void xdrawcursor(int, int, Glyph, int, int, Glyph);
void xdrawline(Line, int, int, int);
void xfinishdraw(void);



#endif

