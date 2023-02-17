// Conversion betweeen the different charmaps.


// github.com/michael105/slterm
// miSc Mychael Myer, 23, BSD 3clause
//
// Don't blame me for erratic conversions.

#define DEFAULT_CP 2

// max unicode point to convert.
#define UNIMAX 4096
#define BUF 64000


typedef char Arg;
typedef unsigned int uint;
typedef unsigned char uchar;


#include "../src/charmaps.h"


typedef struct { 
	const unsigned short *map;
	const char esign; // used for unconvertible chars
	const char *name;
	const unsigned char *chars;
} charmap;

#define MAP(name,esign,chars) { name, 0x##esign, #name, (unsigned char*) chars }


// chars, which are likely to be in literal texts.
// It would be helpful using a dictionary.
// But this works out as well, for my needs.
//
// Try to guess by the used characters.
// This eventually works best with german and english literal texts,
// I only added few mathematical and special other chars
// ( (C),  +-, quotation marks, Euro, ..)
// For other languages, the typically used characters (and cp's) would need to be added below.
const charmap cp[] = {
	MAP( cpe4002,a8,  "\x81\x84\x8e\x94\x9a\x99" // umlaute
	                  "\xe1\xe3\xe4\x9b\x9c\xe6" ),
	MAP( cp850,  a8,  "\x81\x84\x8e\x94\x9a\x99" // umlaute
   	               "\xe1\x9c\xe6\xf4\xf5\xf8\xb8" ),
	MAP( cp437,  a8,  "\x81\x84\x8e\x94\x9a\x99" // umlaute
	                  "\xe1\xe3\xe4\x9b\x9c\xe6" ),
	MAP( cp1252, b6,  "\xe4\xf6\xfc\xdc\xd5\xc4" // "üöäÖÜÄ" // Umlauts
                     "\x80\xb6\xa7\x93\x94\xa9"
                     "\xf7\xa9\xb0\xb1\xd7\xb5\xae" ),

	{0,0,0,0},
};


#define NUMCP (sizeof(cp)/sizeof(charmap)-1)


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define W(msg) write(2,msg,sizeof(msg))


int opts;
// verbose
#define V(msg) ({ if ( opts&1 ) W(msg); 1; })
#define v(...) { if ( opts&1 ) fprintf(stderr,__VA_ARGS__); }
// silent
#define E(msg) { if ( !(opts&2) ) W(msg); }


void usage(){
	W("convert stdin to stdout\n"
			"Usage: convert [-hs] [tocp] [fromcp]\n"
			"Example: cat text.txt | convert cp1252 cp850\n\n"
			"Whithout any options, try to guess the charset and convert to cp437\n"
			"(change the defaults in the source, if needed)\n"
			"options: -s : silent\n"
			"         -l : list codepages\n"
			"\nmiSc 23, BSD 3clause\n"
			);
	exit(1);
}


int guess_charmap(const unsigned char *buf, int len){
	int guess[NUMCP];
	memset(guess,0,NUMCP*sizeof(int));
	int n = 0; // ascii 32-127
	int utf=0; // utf8 
	int so=0; // chars, extended ascii ( 128-192 )
	int co = 0; // control chars.
	int ext = 0; // chars 128-255


	int a = 0;
	while ( a<len ){
		switch (buf[a]) {
			case 0 ... 31:
				co++;
				break;
			case 128 ... 255:

				ext++;
				//printf("c: %d\n",buf[a]);
				for ( int b = 0; b<NUMCP; b++ ){
					//printf("BBB\n");
					for ( const unsigned char *c = cp[b].chars; *c; c++ ){
						//printf("b: %d c: %d\n",b,*c);
						if ( *c == buf[a] ){
							guess[b] = guess[b] + 1;
							//printf("guess, b: %d = %d\n",b,guess[b]);
							break;
						}
					}
				}

				if ( buf[a] < 192 ){ // 128-192 = no utf8
					so++;
					//utf -= 4; // I'm uncertain about mixed encodings
				} else { // coul'd be  utf8
					if ( (buf[a] & 0xe0) == 0xc0 ){ // initial Byte 2Byte utf8
						if ( (a+1<len) && ( (buf[a+1] & 0xc0) == 0x80 ) ){ 
							utf ++;
							a++;
						}
					} else if ( (buf[a] & 0xf0) == 0xe0 ){ // 3byte
						if ( (a+2 < len ) && ( (buf[a+1] & 0xc0) == 0x80 ) && 
								( (buf[a+2] & 0xc0) == 0x80 )){
							utf+=2;
							a+=2;
						}
					} else if ( (buf[a] & 0xf8) == 0xf0 ){ //4byte
						if ( ( a+3<len ) && ( (buf[a+1] & 0xc0) == 0x80 ) &&
								( (buf[a+2] & 0xc0) == 0x80 ) &&
								( (buf[a+3] & 0xc0) == 0x80 ) ){
							utf+=3;
							a+=3;
						}
					}

				}

			break;

			default: // ascii char 32-127
				n++;
		}

		a++;
	}

	if ( !ext ){
		V("No extended Ascii/UTF present.\n");
		return(-1);
	}

	if ( n<len-len/4 ){
		V("This looks like binary data.\nContinuing anyways\n");
	}

	int max = 0;
	int guessed = -1;
	for ( int a = 0; a<NUMCP; a++ )
		if ( guess[a] > max ){
			max = guess[a];
			guessed = a;
		}

	v("Guess: %s\n(%d chars match)\n",cp[guessed].name,max);
	v("utf: %d\nchars:   %d\n   0-31: %d\n 32-127: %d\n128-191: %d\n192-255: %d\n",utf,len,co,n,so,ext);

	return(guessed);
}

void listcodepages(){
	W("Supported charmaps:\n\n");
	for ( const charmap *cm = cp; cm->name != 0; cm++ )
		printf("%s\n",cm->name);

	exit(0);
}

int main(int argc, char **argv ){
	opts = 1; // verbose

	//if ( (argc>1 && argv[1][0] == '-' && argv[1][1] == 'h') ||
	//		argc > 3 )
	//	usage();
	
	// parse options
	for ( *argv++; *argv && *argv[0] == '-'; *argv++ ){
		argc--;
		for ( char *o = *argv+1; *o; o++ ){
			switch (*o) {
				case 'h':
					usage();
				case 'l':
					listcodepages();
				case 's':
					opts=2;
			}
		}
	}
	argc--;

	unsigned char buf[BUF],obuf[BUF*4];
	int len;
	int from = -1;
	int to = -1;

	while ( argc>0 ){
		from = to;
		 const charmap *c = cp;
		 while ( strcmp(c->name,argv[argc-1]) != 0 ){
			 c++;
			 if ( c->name == 0 ){
				 fprintf(stderr,"Unknown codepage: %s\n",argv[argc-1]);
				 exit(1);
			 }
		 }
		 to = c - cp;
		 argc--;
	}

	if ( to==-1 )
		to = DEFAULT_CP;

	// to table.
	char ocp[UNIMAX];
	memset( ocp, 0 , UNIMAX );

	// create reverse table
	for ( int a=0; a<128; a++ ){
		ocp[ cp[to].map[a] ] = a;
	}


	len = read(0,buf,BUF);

	if ( from == -1 ){
		V("Guessing charset\n");
		from = guess_charmap(buf,len);
	}

	if ( (from == -1) || // no conversion possible, no extended ascii
			( (from==to) && ( V("Source and destination codepage are equal\n")) ) ){ 		
		// write stdin to stdout (we are a filter)
		do{ write(1,buf,len); } while (( len=read(0,buf,BUF) ));
		exit(0);
	}

	v("Convert from %s to %s\n", cp[from].name,cp[to].name);

	do {
		for ( int a = 0, p = 0; a<len; a++,p++ ){
			if ( buf[a] <128 )
				obuf[p] = buf[a];
			else {
				if ( ocp[ cp[from].map[buf[a]-128] ] )
					obuf[p] =  ocp[ cp[from].map[buf[a]-128] ]+128; 
				else { // no conversion possible
					obuf[p] = cp[to].esign;
				}
			}


		}

		write(1,obuf,len);

	} while (( len = read(0,buf,BUF) ));
	



	exit(0);
}




