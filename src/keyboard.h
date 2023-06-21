
// keyboard input handling.

/* types used in config.h */
typedef struct {
		unsigned int mod;
		KeySym keysym;
		void (*func)(const Arg *);
		const Arg arg;
		unsigned int inputmode;
} Shortcut;


typedef struct {
		KeySym k;  // this is unsigned int (32) here
		unsigned int mask;
		char *s;
		unsigned char len;
		/* three-valued logic variables: 0 indifferent, 1 on, -1 off */
		signed char appkey;    /* application keypad */
		signed char appcursor; /* application cursor */
} Key;


// save different inputmodes
extern int inputmode;


/* X modifiers */
#define XK_ANY_MOD UINT_MAX
#define XK_NO_MOD 0
#define XK_SWITCH_MOD (1 << 13)



int kmap(KeySym k, unsigned int state);
void numlock(const Arg *);
void kpress(XEvent *ev);

int match(unsigned int mask, unsigned int state);


// callbacks . Used in config.h
void ttysend(const Arg *);

void dummy( const Arg *a);
