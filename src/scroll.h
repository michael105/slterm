#ifndef scroll_h
#define scroll_h

#include "xevent.h"

// SCROLL and LESSMODE are to be or'ed and given to lessmode_toggle
#define LESSMODE(mode) ({ enum { off=(1<<28), toggle=((1<<28)|(1<<29)), on=(1<<29) }; mode; })
#define LESSMODE_OFF (1<<28)
#define LESSMODE_ON (1<<29)
#define LESSMODE_TOGGLE (LESSMODE_OFF|LESSMODE_ON)
#define SCROLL_BOTTOM (0)
#define SCROLL_PAGEDOWN (1<<30)
#define SCROLL_PAGEUP ((1<<30)|(1<<31))
#define SCROLL_TOP (1<<31)
// SCROLL can also  begiven the number of lines to be scrolled.
// -1 = scroll up 1 line, 3 scroll down 3 lines
// SCROLL is also the argument for scroll()
#define LESSMODEMASK (3<<28)
#define SCROLLMASK ((3<<30)|((1<<28)-1))

#define ISSCROLLDOWN(x) ( !(x&(1<<31)) && ( (x&SCROLLMASK)>0 )) 
//#define ISSCROLLDOWN(x) ( x>0 && x<(1<<31) )
//#define ISSCROLLUP(x) ( x<0 && x> -(1UI<<32UI) ) 
#define ISSCROLLUP(x) ( x&(1<<31) ) 
//#define ISSCROLLUP(x) ( ( x&(SCROLLMASK&&(~(1<<32))) ) && (x&(1<<32)) ) 

extern int scrollmarks[12];
extern int retmarks[10];

// callbacks
void kscrolldown(const Arg *);
void kscrollup(const Arg *);
void lessmode_toggle(const Arg*);

void set_scrollmark(const Arg *a);
void set_retmark();
void retmark(const Arg *a);
void scrollmark(const Arg *a);
void leavescroll(const Arg *a);
void enterscroll(const Arg *a);

// scroll up i lines (-1,..) down (1,..), i must be < (1<<30) (actually > as well )
// or SCROLL(bottom), SCROLL(pagedown), SCROLL(pageup) SCROLL(top)
void scroll(const Arg *a);

void tnewline(int);
void scrolltobottom();
void scrolltotop();

void tsetscroll(int, int);
void tscrollup(int, int, int);
void tscrolldown(int, int, int);


#endif

