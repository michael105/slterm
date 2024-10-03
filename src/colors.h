#ifndef colors_h
#define colors_h
// color related functions and definitions


#define FOR_RGB(_do) _do(red);_do(green);_do(blue)

#define INVERT_COLOR(_color) _color.red = ~_color.red; _color.green = ~_color.green; _color.blue = ~_color.blue




#endif

