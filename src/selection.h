#ifndef selection_h
#define selection_h

typedef struct {
  int mode;
  int type;
  int snap;
  /*
   * Selection variables:
   * nb – normalized coordinates of the beginning of the selection
   * ne – normalized coordinates of the end of the selection
   * ob – original coordinates of the beginning of the selection
   * oe – original coordinates of the end of the selection
   */
  struct {
    int x, y;
  } nb, ne, ob, oe;

  int alt;
} Selection;


// clipboard
typedef struct {
		Atom xtarget;
		char *primary, *clipboard;
		struct timespec tclick1;
		struct timespec tclick2;
} XSelection;


// selction
extern XSelection xsel;

extern Selection sel;


//clipboard events
void selnotify(XEvent *);
void selclear_(XEvent *);
void selrequest(XEvent *);
void mousesel(XEvent *, int);

// clipboard callbacks
void clipcopy(const Arg *);
void clippaste(const Arg *);
void selpaste(const Arg *);
void keyboard_select(const Arg *);


void selnormalize(void);
void selscroll(int, int);
void selsnap(int *, int *, int);

void setsel(char *, Time);
void xsetsel(char *);

void selclear(void);
void selinit(void);
void selstart(int, int, int);
void selextend(int, int, int, int);
int selected(int, int);
char *getsel(void);

int trt_kbdselect(KeySym, char *, int);

static void tdumpsel(void);

#endif
