/*
Decompress font file
intended for testing only.

usage: ttfdz infile outfile

misc 2025, MIT License
*/

#include <stdio.h>
#include <stdlib.h>


#define SINFL_IMPLEMENTATION
#include "sinfl.h"



int main( int argc, char **argv ){
	if ( argc!=3 ){
		fprintf(stderr,"Usage: ttfdz infile outfile\n");
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


	out = calloc( size*8,1 ); // well. should work, for ttf fonts.

   size_t len = sinflate( out, size*8, in, size ); 
	if ( len == size*8 ){
		fprintf( stderr, "buffer too small, internal error\n" );
		exit(1);
	}
	
	printf("len: %d\n",len);

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

	printf("Decompressed: %s -> %s\n,%d -> %d\n", argv[1], argv[2], size, len );

	return(0);
}



