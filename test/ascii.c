#if 0
COMPILE prints printl itohex printf fmtd strcpy strcat ewrites atoi
SHRINKELF

return
#endif

#ifndef MLIB
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define writes(msg) write(1,msg,sizeof(msg))
#define ewrites(msg) write(2,msg,sizeof(msg))
#define printl() write(1,"\n",1)

#endif


const struct { char *shortname; char *desc; } ctrl[] = {
	{ "NUL", "Null, End of file" },
	{ "SOH", "Start of Heading" },
	{ "STX", "Start of Text" },
	{ "ETX", "End of Text" },
	{ "EOT", "End of Transmission" },
	{ "ENQ", "Enquiry" },
	{ "ACK", "Acknowledge (Telnet)" },
	{ "BEL", "Bell (audible or attention signal)" },
	{ "BS" , "Backspace (Delete)" },
	{ "TAB", "Horizontal Tabulation" },
	{ "LF" , "Line Feed" },
	{ "VT" , "Vertical Tabulation" },
	{ "FF" , "Form Feed" },
	{ "CR" , "Carriage Return, Enter" },
	{ "SO" , "Shift Out" },
	{ "SI" , "Shift In" },
	{ "DLE", "Data Link Escape" },
	{ "DC1", "Device Control 1" },
	{ "DC2", "Device Control 2" },
	{ "DC3", "Device Control 3" },
	{ "DC4", "Device Control 4" },
	{ "NAK", "Negative Acknowledge" },
	{ "SYN", "Synchronous Idle" },
	{ "ETB", "End of Transmission Block" },
	{ "CAN", "Cancel" },
	{ "EM" , "End of Medium" },
	{ "SUB", "Substitute" },
	{ "ESC", "Escape, VT100 Initiate Control Sequence" },
	{ "FS" , "File Separator" },
	{ "GS" , "Group Separator" },
	{ "RS" , "Record Separator" },
	{ "US" , "Unit Separator" },
};

void usage(){
	ewrites( R"(ascii [-dhxct] [-f num] [-t num]
Dump an ascii table to stdout.
Options:
	-s num : start with num
	-e num : end at num
	-c     : colorize
	-x     : show hexadecimal numbers
	-d     : decimal numbers
	-t     : dump a table and descriptions of the control chars 0-31
	-F fmt : printf format. printf is callen with 3 times c (current char)
	-T num : Linebreak after num columns (default 8)
	-h     : show this help.

(c) 2023, BSD 3-clause, misc (github.com/michael105)
)" );
	exit(0);
}

int main(int argc, char **argv){
	int opt=0;
	int from = 32;
	int to = 255;
	int lb = 8; // linebreak after 8 columns
	char fmtb[32]; 
	char *fmt = fmtb;
	strcpy(fmt,"%2x:");
	for ( *argv++; *argv; *argv++ ){
		if ( argv[0][0] == '-' ){
			for ( char *o = *argv + 1; *o; o++ )
			switch ( *o ){
				case 'd':
					opt |= 1;
					strcpy(fmt,"%3d:");
					break;
				case 'x':
					opt |= 2;
					break;
				case 'c':
					opt |= 4;
					break;
				case 't':
					for ( int a = 0; a<32; a++ ){
						printf("%2d - %2x: %3s  %s\n",a,a,ctrl[a].shortname, ctrl[a].desc );
					}
					exit(0);
				case 's':
					*argv++;
					from = atoi( *argv );
					break;
				case 'e':
					*argv++;
					to = atoi( *argv );
					break;
				case 'T':
					*argv++;
					lb = atoi( *argv );
					break;
				case 'F':
					opt = -1;
					*argv++;
					if ( *argv )
						fmt = *argv;
					break;

				case 'h':
					usage();

				default:
					writes("ascii [-hdxct] [-s num] [-e num]\n");
					exit(0);
			}
		}
	}
	if ( opt >= 0 ){
		if ( (opt&3) == 3 )
			strcpy(fmt,"%3d(%2x):");
		if ( (opt&4) )
			strcat(fmt," \e[36m%c\e[37m  ");
		else
			strcat(fmt," %c  ");
	}

	int col = 1;
	for ( int a = from; a<=to; a++ ){
		printf(fmt, a, a, a );

		if ( col++ == lb ){
			printf("\n");
			col=1;
		}
	}
	exit(0);
}
