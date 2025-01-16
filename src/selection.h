#ifndef selection_h
#define selection_h

#include "includes.h"
#include <stddef.h>

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


enum selection_mode { SEL_IDLE = 0, SEL_EMPTY = 1, SEL_READY = 2 };

enum selection_type { SEL_REGULAR = 1, SEL_RECTANGULAR = 2 };

enum selection_snap { SNAP_WORD = 1, SNAP_LINE = 2 };


enum selection_mode { SEL_IDLE = 0, SEL_EMPTY = 1, SEL_READY = 2 };

enum selection_type { SEL_REGULAR = 1, SEL_RECTANGULAR = 2 };

enum selection_snap { SNAP_WORD = 1, SNAP_LINE = 2 };


// selction

extern Selection sel;



//  callbacks
void keyboard_select(const Arg *);

void mousesel(XEvent *, int);

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
