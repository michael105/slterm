
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


