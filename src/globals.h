#pragma once

#include "fonts.h"

// Globals, used across several source files, 
// and some global typedefs (type aliases)
// (It's been not my idea.)


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





