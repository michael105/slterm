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


extern Selection sel;
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


#endif
