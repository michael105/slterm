#ifndef input_h
#define input_h


// clipboard
typedef struct {
		Atom xtarget;
		char *primary, *clipboard;
		struct timespec tclick1;
		struct timespec tclick2;
} XSelection;


// save different inputmodes
extern int inputmode;
// selction
extern XSelection xsel;

extern void (*handler[LASTEvent])(XEvent *); 



// keyboard input handling.

/* types used in config.h */
typedef struct {
		uint mod;
		KeySym keysym;
		void (*func)(const Arg *);
		const Arg arg;
		uint inputmode;
} Shortcut;


typedef struct {
		KeySym k;
		uint mask;
		char *s;
		/* three-valued logic variables: 0 indifferent, 1 on, -1 off */
		signed char appkey;    /* application keypad */
		signed char appcursor; /* application cursor */
} Key;


/* X modifiers */
#define XK_ANY_MOD UINT_MAX
#define XK_NO_MOD 0
#define XK_SWITCH_MOD (1 << 13)


int match(uint mask, uint state);
char *kmap(KeySym k, uint state);
void kpress(XEvent *ev);

// callbacks
void numlock(const Arg *);

// mouse events handling
typedef struct {
		uint mod;
		uint button;
		void (*func)(const Arg *);
		const Arg arg;
		uint release;
		int altscrn; /* 0: don't care,  -1: not alt screen,  1: alt screen */
} MouseShortcut;

// callbacks
int mouseaction(XEvent *, uint);
void brelease(XEvent *);
void bpress(XEvent *);
void bmotion(XEvent *);

void mousereport(XEvent *);

/* function definitions used in config.h */

void clipcopy(const Arg *);
void clippaste(const Arg *);
void selpaste(const Arg *);
void zoom(const Arg *);
void zoomabs(const Arg *);
void zoomreset(const Arg *);
void ttysend(const Arg *);



#endif

