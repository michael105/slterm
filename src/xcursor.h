// cursor drawing and
// cursor color functions

#pragma once


int xsetcursor(int cursor);

int xgetcursor();

Color *getcursorcolor( Glyph g, int cx, int cy );

void xdrawcursor(int cx, int cy, Glyph g, int ox, int oy, Glyph og);



