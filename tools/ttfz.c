/*
Compress font files (or other files)
Error checking should be done separately,
intended only for the build process of slterm.

Limits: no error checking, will not work very well with bigger files,
 there might be more trouble.
 Works for the intended usage, however.

  
usage: ttfz infile outfile

misc 2025, Public Domain
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
*/

#include <stdio.h>
#include <stdlib.h>


#define SDEFL_IMPLEMENTATION
#include "sdefl.h"


static struct sdefl sdefl;


int main( int argc, char **argv ){
	if ( argc!=3 ){
		fprintf(stderr,"Usage: ttfz infile outfile\n");
		exit(22);
	}

	void *in,*out;

	FILE* fp = fopen(argv[1], "r");
	if (!fp) exit(2);
	fseek(fp, 0, SEEK_END);
	size_t size = (size_t)ftell(fp);
	fseek(fp, 0, SEEK_SET);
	in = calloc(size, 1);
	fread(in, 1, (size_t)size, fp);
	fclose(fp);


	out = calloc( size*2,1 );

   size_t len = sdeflate(&sdefl, out, in, (int)size, 7);
	
	//printf("len: %d\n",len);

	fp = fopen( argv[2], "r" );
	if ( fp ){
		fprintf( stderr, "outfile exists: %s\nAbort\n",argv[2] );
		fclose(fp);
		exit(1);
	}

	fp = fopen( argv[2], "w" );

	if (!fp){
		fprintf(stderr, "Cannot write to %s\n", argv[2] );
		exit(1);
	}
	fwrite( out, 1, len, fp );
	fclose(fp);

	printf("Compress:\n%s -> %s\n%d -> %d\n", argv[1], argv[2], size, len );

	return(0);
}



