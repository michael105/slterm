// cursor drawing and
// cursor color functions

#pragma once


int _xsetcursor(int cursor, int attr);
#define __xsetcursor(_a,_b,...) _xsetcursor(_a,_b)
#define xsetcursor(_cursor,...) __xsetcursor(_cursor,__VA_OPT__(__VA_ARGS__,) 0 )

int xgetcursor();

Color *getcursorcolor( Glyph g, int cx, int cy );

void xdrawcursor(int cx, int cy, Glyph g, int ox, int oy, Glyph og);



