#ifndef MEM_C
#define MEM_C

// memory related functions

#include "mem.h"

// Set glyphs (non utf8 - one glyph is one int32 )
// misc - could be optimized ( either copy int64, or simd )
void memset32( uint32_t* i, uint32_t value, int count ){
	for ( int a=0; a<count; a++ )
		i[a] = value;
}


ssize_t xwrite(int fd, const char *s, size_t len) {
	size_t aux = len;
	ssize_t r;

	while (len > 0) {
		r = write(fd, s, len);
		if (r < 0) {
			return r;
		}
		len -= r;
		s += r;
	}

	return aux;
}
// opt - power of 8? -> memset32 -> memset64
void *xmalloc(size_t len) {
	void *p;

	if (!(p = malloc(len))) {
		die("malloc: %s\n", strerror(errno));
	}

	return p;
}

void *xrealloc(void *p, size_t len) {
	if ((p = realloc(p, len)) == NULL) {
		die("realloc: %s\n", strerror(errno));
	}

	return p;
}

char *xstrdup(char *s) {
	if ((s = strdup(s)) == NULL) {
		die("strdup: %s\n", strerror(errno));
	}

	return s;
}


#endif

