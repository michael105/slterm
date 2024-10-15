
// mouse events handling
typedef struct {
		unsigned int mod;
		unsigned int button;
		void (*func)(const Arg *);
		const Arg arg;
		unsigned int release;
		int altscrn; /* 0: don't care,  -1: not alt screen,  1: alt screen */
} MouseShortcut;



// event handling

int mouseaction(XEvent *, unsigned int);
void brelease(XEvent *);
void bpress(XEvent *);
void bmotion(XEvent *);
void mousereport(XEvent *);

int evcol(XEvent *);
int evrow(XEvent *);


