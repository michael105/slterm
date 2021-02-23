#ifndef mem_h
#define mem_h



#include "includes.h"


void memset32( uint32_t* i, uint32_t value, int count );

ssize_t xwrite(int fd, const char *s, size_t len);
void *xmalloc(size_t len);
void *xrealloc(void *p, size_t len);
char *xstrdup(char *s);





#endif

