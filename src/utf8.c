#include <sys/types.h>

#include "utf8.h"
#include "term.h"

#ifdef UTF8
static uchar utfbyte[UTF_SIZ + 1] = {0x80,    0, 0xC0, 0xE0, 0xF0};
static uchar utfmask[UTF_SIZ + 1] = {0xC0, 0x80, 0xE0, 0xF0, 0xF8};
static Rune utfmin[UTF_SIZ + 1] = {       0,    0,  0x80,  0x800,  0x10000};
static Rune utfmax[UTF_SIZ + 1] = {0x10FFFF, 0x7F, 0x7FF, 0xFFFF, 0x10FFFF};
#else
static uchar utfbyte[UTF_SIZ + 4] = {0x80, 0, 0xC0, 0xE0, 0xF0};
static uchar utfmask[UTF_SIZ + 4] = {0xC0, 0x80, 0xE0, 0xF0, 0xF8};
static Rune utfmin[UTF_SIZ + 4] = {0, 0, 0x80, 0x0, 0x0};
static Rune utfmax[UTF_SIZ + 4] = {0xFF, 0x7F, 0xFF, 0xFF, 0xFF};
#endif


size_t utf8validate(Rune *u, size_t i) {
		if (!BETWEEN(*u, utfmin[i], utfmax[i]) || BETWEEN(*u, 0xD800, 0xDFFF)) {
				*u = UTF_INVALID;
		}
		for (i = 1; *u > utfmax[i]; ++i) {
		}

		return i;
}

char utf8encodebyte(Rune u, size_t i) { return utfbyte[i] | (u & ~utfmask[i]); }

#ifdef UTF8
char *utf8strchr(char *s, Rune u) {
		Rune r;
		size_t i, j, len;

		len = strlen(s);
		for (i = 0, j = 0; i < len; i += j) {
				if (!(j = utf8decode(&s[i], &r, len - i)))
						break;
				if (r == u)
						return &(s[i]);
		}

		return NULL;
}
#endif


size_t utf8decode(const char *c, Rune *u, size_t clen) {
		size_t i, j, len, type;
		Rune udecoded;

		*u = UTF_INVALID;
		if (!clen) {
				return 0;
		}
		udecoded = utf8decodebyte(c[0], &len);
		if (!BETWEEN(len, 1, UTF_SIZ)) {
				return 1;
		}
		for (i = 1, j = 1; i < clen && j < len; ++i, ++j) {
				udecoded = (udecoded << 6) | utf8decodebyte(c[i], &type);
				if (type != 0) {
						return j;
				}
		}
		if (j < len) {
				return 0;
		}
		*u = udecoded;
		utf8validate(u, len);

		return len;
}

Rune utf8decodebyte(char c, size_t *i) {
		for (*i = 0; *i < LEN(utfmask); ++(*i)) {
				if (((uchar)c & utfmask[*i]) == utfbyte[*i]) {
						return (uchar)c & ~utfmask[*i];
				}
		}

		return 0;
}

size_t utf8encode(Rune u, char *c) {
#ifndef UTF8
		c[0] = u;
		return 1;
#else
		size_t len, i;

		len = utf8validate(&u, 0);
		if (len > UTF_SIZ) {
				return 0;
		}

		for (i = len - 1; i != 0; --i) {
				c[i] = utf8encodebyte(u, 0);
				u >>= 6;
		}
		c[0] = utf8encodebyte(u, len);

		return len;
#endif
}


