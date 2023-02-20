#ifndef input_h
#define input_h
#include <X11/Xlib.h>


// save different inputmodes
extern int inputmode;

extern void (*handler[LASTEvent])(XEvent *); 


//void tty_send_unicode(const Arg *arg);

// callback argument
typedef union {
  int i;
  unsigned int ui;
  float f;
  const void *v;
  const char *s;
} Arg;


/* XEMBED messages */
#define XEMBED_FOCUS_IN 4
#define XEMBED_FOCUS_OUT 5





// the main event loop
void run();

// callbacks
//void temp(const Arg *);


// event handling


// xwindow related
void expose(XEvent *);
void visibility(XEvent *);
void unmap(XEvent *);
void cmessage(XEvent *);
void resize(XEvent *);
void focus(XEvent *);
void propnotify(XEvent *);




#endif

