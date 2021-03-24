#ifndef scroll_h
#define scroll_h

#include "xevent.h"


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



void tnewline(int);
void scrolltobottom();
void scrolltotop();

void tsetscroll(int, int);
void tscrollup(int, int, int);
void tscrolldown(int, int, int);


#endif

