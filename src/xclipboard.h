#pragma once 


// clipboard
typedef struct {
		Atom xtarget;
		char *primary, *clipboard;
		struct timespec tclick1;
		struct timespec tclick2;
} XSelection;


extern XSelection xsel;


//clipboard events
void selnotify(XEvent *);
void selclear_(XEvent *);
void selrequest(XEvent *);

void clipcopy(const Arg *);
void clippaste(const Arg *);
void selpaste(const Arg *);

