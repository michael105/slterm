#if 0
COMPILE PRINTF

return
#endif

/* dump a chartable to the terminal */

#ifndef MLIB
#include <stdio.h>
#endif

int main(int argc, char **argv){
	for ( int a = 32; a<256; a++ ){
		//if ( a>0x7f && a<0xa0 )
		//	continue;
		printf("%2x: %c  ", a, a );
		if ( !((a+1)%8) )
			printf("\n");
	}
	return(0);
}
