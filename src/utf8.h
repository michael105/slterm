#ifndef utf8_h
#define utf8_h

#include <stddef.h>


#ifdef UTF8
typedef uint_least32_t Rune;
#else
typedef unsigned char Rune;
#endif


size_t utf8decode(const char *c, Rune *u, size_t clen);
size_t utf8encode(Rune u, char *c);
size_t utf8validate(Rune *u, size_t i);
char utf8encodebyte(Rune u, size_t i);
Rune utf8decodebyte(char c, size_t *i);

#endif
