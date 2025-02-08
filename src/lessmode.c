
#define TFBLEN 256


// text entering, textfield
// besser als struct, objektorientiert.
uchar tfbuf[TFBLEN];
int tfbuflen = TFBLEN;

int tfpos = 0; // is not necessarily the pos of the cursor.
int tftextlen = 0;
int tfvisible = 0;

uchar tfinput[TFBLEN];
int tfinputlen = TFBLEN;



// toggle search
void lessmode_search( const Arg *a ){
	


}




// for the mode MODE_ENTERSTRING
int statusbar_kpress( XKeyEvent *ke, KeySym *ks, char *buf ){

	return(0);
}




// Argument Arg.i is one of LESSMODE_ON, LESSMODE_OFF, LESSMODE_TOGGLE
// can be or'ed with SCROLL (all definitions), then scrolls also
void lessmode_toggle(const Arg *a){
	if (term!=p_term) return;

	switch( a->i & LESSMODEMASK ){
		case LESSMODE_ON:
			inputmode |= MODE_LESS;
			//tfulldirt();
			break;
		case LESSMODE_OFF:
			inputmode &= ~MODE_LESS;
			break;
		case LESSMODE_TOGGLE:
			inputmode ^= MODE_LESS;
			break;
	}

	if ( a->i & ~LESSMODEMASK ){
		//DBG("scroll %d\n",a->i);
		//DBG("scroll %d\n",d.i);
		scroll(ARGP(i = SCROLLMASK & a->i ));
	}

	if ( inputmode & MODE_LESS ){ // enabled
		showstatus(1,"-LESS-");
		updatestatus();
	} else { // disable
		showstatus(0,0);
		//scrolltobottom();
	}
}

