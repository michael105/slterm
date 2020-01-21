#ifndef st_system_h
#define st_system_h

// pid of shell 
extern pid_t pid;
extern int iofd; // set to 1 in main.c
extern int cmdfd;



void die(const char *errstr, ...);
void execsh(char *, char **);
void sigchld(int a);



#endif

