#ifndef colors_h
#define colors_h
// color related functions and definitions


#define FOR_RGB(_do) _do(red);_do(green);_do(blue)

#define INVERT_COLOR(_color) _color.red = ~_color.red; _color.green = ~_color.green; _color.blue = ~_color.blue

#ifndef UTF8
#define IS_TRUECOL(x)(0)
#define TRUECOLOR(r,g,b) (0)
#else
#define TRUECOLOR(r, g, b) (1 << 24 | (r) << 16 | (g) << 8 | (b))
#define IS_TRUECOL(x) (1 << 24 & (x))
#endif




#endif

