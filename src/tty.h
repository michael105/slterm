#ifndef stTTY_H
#define stTTY_H

extern char *stty_args;

extern int iofd; 
extern int cmdfd;


void ttyresize(int, int);
void stty(char **);
void sigchld(int);
void ttywriteraw(const utfchar *, size_t);
void sendbreak(const Arg *);
void tprinter(char *s, size_t len);

#endif
