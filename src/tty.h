#ifndef stTTY_H
#define stTTY_H

void stty(char **);
void sigchld(int);
void ttywriteraw(const utfchar *, size_t);

#endif
