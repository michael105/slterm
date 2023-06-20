#ifndef scroll_h
#define scroll_h

#include "xevent.h"

// SCROLL and LESSMODE are to be or'ed and given to lessmode_toggle
#define LESSMODE(mode) ({ enum { off=(1<<29), toggle=((1<<29)|(1<<30)), on=(1<<30) }; mode; })
#define SCROLL(x) ({ enum { bottom=0, pagedown=(1<<31), \
		pageup=((1<<31)|(1<<32), top=(1<<32) }; x; })
// SCROLL can also  begiven the number of lines to be scrolled.
// -1 = scroll up 1 line, 3 scroll down 3 lines
// SCROLL is also the argument for scroll()
#define LESSMODEMASK (3<<29)
#define SCROLLMASK ((3<<31)|((1<<29)-1))

#define ISSCROLLDOWN(x) ( !(x&(1<<32)) && ( (x&SCROLLMASK)>0 )) 
//#define ISSCROLLDOWN(x) ( x>0 && x<(1<<31) )
//#define ISSCROLLUP(x) ( x<0 && x> -(1UI<<32UI) ) 
#define ISSCROLLUP(x) ( x&(1<<32) ) 
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

