#ifndef st_system_h
#define st_system_h


void die(const char *errstr, ...);
void execsh(char *, char **);
void sigchld(int a);



#endif

