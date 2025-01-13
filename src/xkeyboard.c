
// keyboard handling

// inputmode. switchable via lessmode_toggle GLBLMARK
int inputmode = 1;
int help_storedinputmode = 0;



int match(uint mask, uint state) {
	return mask == XK_ANY_MOD || mask == (state & ~ignoremod);
}

int kmap(KeySym k, uint state) {
	Key *kp;
	int i;
	//KeySym k2 = k;

	dbg("Key: %d %c\n", k,k);
	/* Check for mapped keys out of X11 function keys. */
	for (i = 0; i < LEN(mappedkeys); i++) { //? misc.
														 // Better compare first with a bitfield,
														 // by the or'ed mapped keys
		if (mappedkeys[i] == k)
			break;
	}

	if (i == LEN(mappedkeys)) { // no mapped key
		KeySym tk = k;
			if ( tk>0x1000000 ) 
				tk-= 0x1000000; // convert to unicode

		if ((tk & 0xFFFF) < 0xFD00) { // No control/function/mod key
											  // dbg ("Here\n");// -> no multibyte key. no match. ret.
			dbg("Key2: %x %c\n", tk,tk);
			/* Check for mapped keys out of X11 function keys. */
			if ( (tk > 0x80) && ( tk<0xf000 ) ){ //unicode translation to current codepage needed
				if ( tk >= UNITABLE ){ // greater than the cached table	 
					for ( uchar a = 0; a<128; a++ ){
						if ( codepage[selected_codepage][a] == tk ){
							ttywrite( &a,1,1 );
							return(1);
							//k = codepage[selected_codepage][a];
							//goto CONT;
						}
					}
				} else {
					if ( uni_to_cp[tk] ){
						dbg("uni to cp: %x\n",uni_to_cp[tk]);
						ttywrite( &uni_to_cp[tk],1,1 );
						return(1);
						//k = uni_to_cp[tk];
						//goto CONT;
					}
				}
				ttywrite((utfchar*)"~",1,1); // drop utf8 here
			}

			return(0);
		}
	}
//CONT:

// key ist the key binding array (!) defined in config.h. man.
// doesn't seem useful to sort, normally a key is within the array, when 
// arrived here.
	for (kp = key; kp < key + LEN(key); kp++) {
		if (kp->k != k)
			continue;

		if (!match(kp->mask, state))
			continue;
		if (IS_SET(MODE_APPKEYPAD) ? kp->appkey < 0 : kp->appkey > 0)
			continue;
		if (IS_SET(MODE_NUMLOCK) && kp->appkey == 2)
			continue;

		if (IS_SET(MODE_APPCURSOR) ? kp->appcursor < 0 : kp->appcursor > 0)
			continue;

		ttywrite((utfchar*)kp->s, kp->len, 1);
		return(1); // abort further handling
	}
//	if ( k2 != k ){ // translated from unicode to codepage
//		ttywrite((uchar*)&k, 1, 1);
//		return(1);
//	}
	//printf("looking for: %x\n",kp->k);
	return(0);
}

// sort the shortcuts, 
// to be able to abort scanning through all of them for each key
// gets callen at startup
void sort_shortcuts(){
	// Do a stable sort. Elements of the same
	// keysym value stay in the same order. 	
	// This is callen only once at startup. for about 100 elements.
#define SIZE (sizeof(shortcuts)/sizeof(Shortcut))
	//uint cp[SIZE];
	KeySym bing; //
	uint pt = 0;  // 

	for ( int a = 0; a<SIZE; a++ )
		if ( !shortcuts[a].keysym )
			die("Invalid configuration, shortcut number %d, keysym is 0\n",a+1);

	 while (1){
		bing = UINT_MAX;
		for ( int a = pt; a<SIZE; a++ ){
			if ( shortcuts[a].keysym < bing   )
				bing = shortcuts[a].keysym;
		}
		if ( bing == UINT_MAX )
			break;
		for ( int a = pt; a<SIZE; a++ )
			if ( bing == shortcuts[a].keysym ){
				Shortcut tmp;
				memcpy(&tmp,shortcuts+a,sizeof(Shortcut));
				memcpy(shortcuts+a,shortcuts+pt,sizeof(Shortcut));
				memcpy(shortcuts+pt,&tmp,sizeof(Shortcut));
				/*char *l1 = (char*)(shortcuts+a), *l2= (char*)(shortcuts+pt); 
				while( l1<(char*)(shortcuts+a)+sizeof(Shortcut) ){
				asm("XXXXX:");
				asm("lea %0,%%ecx\n"
				    "lea %1,%%eax\n"
						//"xor %%rax,%%rcx\nxor %%rcx,%%rax\nxor %%rax,%%rcx\n" 
					"mov %%ecx,%1\n"
					"mov %%eax,%0\n": 
					"+m"(l1), "+m"(l2) : : "cc", "memory", "rcx", "rax" );
					*/
				/*	*l1^=*l2;
					*l2^=*l1;
					*l1^=*l2; 
					l1++; l2++;
				} */ // doesnt work. ??? :( finally. my processor is faulty. 
					  // (misc 24) - there seems to be a bug with the order of instruction execution
					  // did have more trouble, also with rep mov et al
				pt++;
			}
	};

			
//	Shortcut tmp[SIZE];
//	memcpy( tmp, shortcuts, sizeof(shortcuts) );

//	for ( int a = 0; a<SIZE; a++ )
//		memcpy( shortcuts+a, tmp+cp[a], sizeof(Shortcut) );

#if 0
	 printf("=== sort_shortcuts\n");
	for ( int a = 0; a<SIZE; a++ ){
		printf("%d %x  \n",a,shortcuts[a].keysym); 
	}
#endif

#undef SIZE
}

// Keystrokes are handled here.
void kpress(XEvent *ev) {
	XKeyEvent *e = &ev->xkey;
	KeySym ksym;
	unsigned char buf[32];
	int len;
	Rune c;
	Status status;
	Shortcut *bp;

	if (IS_SET(MODE_KBDLOCK))
		return;

	len = XmbLookupString(xwin.xic, e, (char*)buf, sizeof buf, &ksym, &status);

	if (IS_SET(MODE_KBDSELECT)) {
		if (match(XK_NO_MOD, e->state) || (XK_Shift_L | XK_Shift_R) & e->state)
			twin.mode ^= trt_kbdselect(ksym, (char*)buf, len);
		return;
	}


	if ( IS_SET( MODE_ENTERSTRING ) ){
		statusbar_kpress( &ksym, (char*)buf );
		return;
	}

	dbg("key: %x, keycode: %x, state: %x\n",ksym, e->keycode, e->state );

	// handle return, add retmark 
	if ( ( ksym == XK_Return ) && ( inputmode == MODE_REGULAR ) && !( twin.mode & MODE_KBDSELECT ) ){
		//if ( (!IS_SET(MODE_ALTSCREEN)) && ( ksym == XK_Return ) ){
		set_retmark();
	}


	/* 1. shortcuts */
	for (bp = shortcuts; bp < shortcuts + LEN(shortcuts); bp++) {
		 if ( ksym < bp->keysym ){  // bp->keysym > ksym
			break;
		}

		if ( (( ksym == bp->keysym ) || ( bp->keysym == ALL_KEYS )) && 
				match(bp->mod, e->state) && 
				(bp->inputmode & inputmode)) {

			bp->func(&(bp->arg));
			return;
		}
	}


	if ( term->mode & TMODE_HELP ) // in the help browser. Only shortcuts used
											 //if ( inputmode & MODE_HELP )
		return;

	/* 2. keys to translate, defined in config.h */
	if (kmap(ksym, e->state)) {
		//ttywrite(customkey, strlen(customkey), 1);
		return;
	}

	dbg("Key2: %d %c, state:%x, mod1: %x, len %d\n", ksym, ksym, e->state,
			Mod1Mask, len);
	/* 3. composed string from input method */
	if (len == 0)
		return;
	if (len == 1 && e->state & Mod1Mask) {
		dbg("K\n");
		if (IS_SET(MODE_8BIT)) {
			if (*buf < 0177) {
				c = *buf | 0x80;
				len = utf8encode(c, (char*)buf);
			}
		} else {
			buf[1] = buf[0];
			buf[0] = '\033';
			len = 2;
		}
	}
	ttywrite(buf, len, 1);
}


void numlock(const Arg *dummy) {
	twin.mode ^= MODE_NUMLOCK; 
}

/*
void temp(const Arg *dummy){
	for ( int a = 0; a<128; a++){//2190
			  //cpe4002a[a] = a+0x238a;
	}
} */


// does nothing. Aborts shortcut scanning and key processing
void dummy( const Arg *a){
}

void dump_terminfo( const Arg *a){
#ifdef INCLUDETERMINFO
				ttywrite((utfchar*)slterm_terminfo, strlen(slterm_terminfo),1 );
#endif
}




