#ifndef termdraw_h
#define termdraw_h




void ttywrite(const utfchar *, size_t, int);
void tsetdirtattr(int);
int tattrset(int);
int twrite(const utfchar *, int, int);
void tsetdirt(int, int);

void tdeletechar(int);
void tdeleteline(int);
void tinsertblank(int);
void tinsertblankline(int);
void tdumpline(int);
void tdump(void);
void tclearregion(int, int, int, int);

void tfulldirt(void);


#endif

