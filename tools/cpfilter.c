// Conversion betweeen the different charmaps.


// github.com/michael105/slterm
// miSc Mychael Myer, 23, BSD 3clause
//
// Don't blame me for erratic conversions.

#define DEFAULT_CP CP437

// max unicode point to convert.
#define UNIMAX 4096
#define BUF 64000


typedef char Arg;
typedef unsigned int uint;
typedef unsigned char uchar;


#include "../src/charmaps.h"


typedef struct { 
	const unsigned short *map;
	const char *name;
	const unsigned char *chars;
} charmap;

#define MAP(name,chars) { name, #name, (unsigned char*) chars }


// chars, which are likely to be in literal texts.
// Used to guess a codepage.
// I wrote this first hand for german texts,
// there are only few mathematical and special chars included.
// For other languages, other chars might need to go here.
// It would be helpful using a dictionary.
// But this works out as well, for my needs.
const charmap cp[] = {
	MAP( cpe4002,"\x81\x84\x8e\x94\x9a\x99" // umlaute
	             "\xe1\xe3\xe4\x9b\x9c\xe6" ),
	MAP( cp850,  "\x81\x84\x8e\x94\x9a\x99" // umlaute
   	          "\xe1\x9c\xe6\xf4\xf5\xf8\xb8" ),
	MAP( cp437,  "\x81\x84\x8e\x94\x9a\x99" // umlaute
	             "\xe1\xe3\xe4\x9b\x9c\xe6" ),
	MAP( cp1252, "\xe4\xf6\xfc\xdc\xd5\xc4" // "������" // Umlauts
                "\x80\xb6\xa7\x93\x94\xa9"
                "\xf7\xa9\xb0\xb1\xd7\xb5\xae" ),

	{0,0,0},
};

#define NUMCP (sizeof(cp)/sizeof(charmap)-1)

enum codepage { CP1252, CP850, CP437, CPE4002, UTF8 };
const char* cps[] = { "cp1252", "cp850", "cp437", "cpe4002", "utf8",0 };
const unsigned short* map[] = { cp1252,cp850,cp437,cpe4002 };


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define W(msg) write(2,msg,sizeof(msg))


void usage(){
	W("convert stdin to stdout\n"
			"Usage: convert [-h] [fromcp] [tocp]\n"
			"Example: cat text.txt | convert cp1252 cp850\n\n"
			"Whithout any options, try to guess the charset and convert to cpe4002\n"
			"(change the defaults in the source, if needed)\n"
			"\nmiSc 23, BSD 3clause\n"
			);
	exit(1);
}


enum codepage guess_charmap(const unsigned char *buf, int len){
	int guess[NUMCP];
	memset(guess,0,NUMCP*sizeof(int));
	int n = 0; // ascii 32-127
	int utf=0; // utf8 
	int so=0; // chars, extended ascii ( 128-255 )
	int co = 0; // control chars.

	// try to guess by the used characters.
	// this eventually works best with german and english literal texts,
	// I only added few mathematical and special other chars
	// ( (C),  +-, quotation marks, Euro, ..)
	// For other languages, the typically used characters (and cp's) would need to be added below.
	unsigned char *gcp[8];
	gcp[CP1252] = (uchar*) "������" // Umlauts
					          "\x80\xb6\xa7\x93\x94\xa9"
					          "\xf7\xa9\xb0\xb1\xd7\xb5\xae";
	gcp[CP437] = (uchar*) "\x81\x84\x8e\x94\x9a\x99" // umlaute
					          "\xe1\xe3\xe4\x9b\x9c\xe6";
	gcp[CP850] = (uchar*) "\x81\x84\x8e\x94\x9a\x99" // umlaute
					          "\xe1\x9c\xe6\xf4\xf5\xf8"
					          "\xb8";
	gcp[CPE4002] = (uchar*)"";



	int a = 0;
	while ( a<len ){
		switch (buf[a]) {
			case 0 ... 31:
				co++;
				break;
			case 128 ... 255:

				//printf("c: %d\n",buf[a]);
				for ( int b = 0; b<NUMCP; b++ ){
					//printf("BBB\n");
					for ( unsigned char *c = gcp[b]; *c; c++ ){
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
	if ( n<len-len/4 ){
		W("This looks like a binary file.\nContinuing anyways\n");
	}

	int max = 0;
	int guessed = -1;
	for ( int a = 0; a<NUMCP; a++ )
		if ( guess[a] > max ){
			max = guess[a];
			guessed = a;
		}

	fprintf(stderr,"Guess: %s\n(%d chars match)\n",cps[guessed],max);
	fprintf(stderr,"utf: %d  so: %d  n: %d\n",utf,so,n);

	return(guessed);
}


int main(int argc, char **argv ){

	if ( (argc>1 && argv[1][0] == '-' && argv[1][1] == 'h') ||
			argc > 3 )
		usage();

	unsigned char buf[BUF],obuf[BUF];
	int len;
	int from = -1;
	int to = -1;

	while ( argc>1 ){
		to = from;
		 const char **c = cps;
		 while ( strcmp(*c,argv[argc-1]) != 0 ){
			 *c++;
			 if ( ! *c ){
				 fprintf(stderr,"Unknown codepage: %s\n",argv[argc-1]);
				 exit(1);
			 }
		 }
		 from = c - cps;
		 argc--;
	}

	if ( to==-1 )
		to = DEFAULT_CP;

	// to table.
	char ocp[UNIMAX];
	memset( ocp, 0 , UNIMAX );

	for ( int a=0; a<128; a++ ){
		ocp[ map[to][a] ] = a;
	}


	while (( len = read(0,buf,BUF) )){
		if ( from == -1 ){
			W("Guessing charset\n");
			from = guess_charmap(buf,len);
		}
		for ( int a = 0, p = 0; a<len; a++,p++ ){
			if ( buf[a] <128 )
				obuf[p] = buf[a];
			else {
				if ( ocp[ map[from][buf[a]-128] ] )
					obuf[p] =  ocp[ map[from][buf[a]-128] ]+128; 
				else { // no conversion
					obuf[p] = 255;
				}
			}


		}

		write(1,obuf,len);

	}






	exit(0);
}




