
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

