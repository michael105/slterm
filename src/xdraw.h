#ifndef xdraw_h
#define xdraw_h


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

