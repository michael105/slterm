#ifndef BASE64_C
#define BASE64_C
// base64 coding/decoding


#include "mem.h"
#include "includes.h"

static const char base64_digits[] = {
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  62, 0,  0,  0,  63, 52, 53, 54, 55, 56, 57, 58, 59, 60,
		61, 0,  0,  0,  -1, 0,  0,  0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
		11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,
		0,  0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
		43, 44, 45, 46, 47, 48, 49, 50, 51, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0};

char base64dec_getc(const char **src) {
		while (**src && !isprint(**src)) {
				(*src)++;
		}
		return **src ? *((*src)++) : '='; /* emulate padding if string ends */
}

char *base64dec(const char *src) {
		size_t in_len = strlen(src);
		char *result, *dst;

		if (in_len % 4) {
				in_len += 4 - (in_len % 4);
		}
		result = dst = xmalloc(in_len / 4 * 3 + 1);
		while (*src) {
				int a = base64_digits[(unsigned char)base64dec_getc(&src)];
				int b = base64_digits[(unsigned char)base64dec_getc(&src)];
				int c = base64_digits[(unsigned char)base64dec_getc(&src)];
				int d = base64_digits[(unsigned char)base64dec_getc(&src)];

				/* invalid input. 'a' can be -1, e.g. if src is "\n" (c-str) */
				if (a == -1 || b == -1) {
						break;
				}

				*dst++ = (a << 2) | ((b & 0x30) >> 4);
				if (c == -1) {
						break;
				}
				*dst++ = ((b & 0x0f) << 4) | ((c & 0x3c) >> 2);
				if (d == -1) {
						break;
				}
				*dst++ = ((c & 0x03) << 6) | d;
		}
		*dst = '\0';
		return result;
}


#endif

