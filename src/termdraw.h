#ifndef termdraw_h
#define termdraw_h




void ttywrite(const utfchar *, size_t, int);
void tsetdirtattr(int);
int tattrset(int);
int twrite(const utfchar *, int, int);
void tsetdirt(int, int);




#endif

