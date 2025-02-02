
#include "scroll.h"

void tsetscroll(int t, int b) {
	LIMIT(t, 0, term->rows - 1);
	LIMIT(b, 0, term->rows - 1);
	if (t > b) {
		//		SWAPint( t,b );
		term->scroll_top = b;
		term->scroll_bottom = t;
	} else {
		term->scroll_top = t;
		term->scroll_bottom = b;
	}
}

// scroll downwards
void kscrolldown(const Arg *a) {
	int n = a->i;

	DBG("kscrolldown, n: %d, guard: %x\n",n, term->guard);
	if (n < 0) { // scroll a page
		n = term->rows + n;
	}

	if (n > term->scr) {  // at the bottom ( bottom: scr=0 )
		n = term->scr;
	}
	DBG2("kscrolldown2, n: %d\n",n);

	if (term->scr > 0) {
		term->scr -= n; 
		selscroll(0, -n);
		retmark_scrolleddown();

		tfulldirt();
		updatestatus();
	}
}

void scrolltobottom(){
	DBG("scrolltobottom\n"); // xxx
	if ( term->scr ){
		term->scr=0;
		selscroll(0, 0);
		retmark_scrolleddown();
		tfulldirt();	
		updatestatus();
	}
}

void scrolltotop(){
	DBG("totop\n");
	term->scr=term->histsize + 1; //HSTSIZE
	if ( (term->circledhist==0) && (term->scr>term->histindex ) )
		term->scr=term->histindex; // 
	selscroll(0, term->scr);
	retmark_scrolledup();
	tfulldirt();
	updatestatus();
}

// scroll upwards (Shift+up)_
void kscrollup(const Arg *a) {
	int n = a->i;

	DBG2("kscrollup, n: %d, term->histindex: %d, term->rows: %d scr: %d\n",
			n, term->histindex, term->rows, term->scr);
	if (n < 0) { // scroll a page upwards
					 //n = term->rows + n;
		n = term->rows;
	}
	DBG2("kscrollup2, n: %d\n",n);

	if ( term->scr <= term->histsize - n + 1) { //HSTSIZE
		term->scr += n;

		if ( (term->circledhist==0) && (term->scr>term->histindex ) )
			term->scr=term->histindex; // at the top

		DBG2("kscrollup3, scr: %d\n",term->scr);
		selscroll(0, n);
		retmark_scrolledup();
		tfulldirt();
		updatestatus();
	}
}

void scroll( const Arg *a){
	Arg d;
	switch ( a->i ){ // scrolling and the history make me dizzy. misc. 
						  // At some time I'm going to rewrite that 
		case SCROLL_BOTTOM:
			scrolltobottom();
			break;
		case SCROLL_TOP:
			scrolltotop();
			break;
		case SCROLL_PAGEUP:
			d.i = -1;
			kscrollup( &d );
			break;
		case SCROLL_PAGEDOWN:
			d.i = -1;
			kscrolldown( &d );
			break;
		default:
			if ( ISSCROLLDOWN(a->i) ){
				kscrolldown( a );
			} else if ( ISSCROLLUP(a->i) ){
				d.i = a->i & SCROLL_LINEMASK;
				kscrollup( &d );
			}
	}
}

void set_scrollmark(const Arg *a) {
	if (term==p_alt) return;
	term->scrollmarks[a->i] = term->histindex - term->scr+1;	
	updatestatus();
	//DBG("Setscrollmark: n:%d histindex:%d scr:%d\n", a->i, term->histindex, term->scr );
}


void scrollmark(const Arg *a){
	if (term==p_alt) return;
	//DBG("Scrollmark: n:%d scrm:%d histindex:%d scr:%d\n", a->i, term->scrollmarks[a->i],term->histindex, term->scr );
	//	if ( term->scrollmarks[a->i] ){
	if ( term->scrollmarks[a->i] )
		term->scr=term->histindex-term->scrollmarks[a->i]+1;
	else 
		term->scr=term->histindex-term->scrollmarks[a->i];
	selscroll(0, term->scr);
	tfulldirt();
	updatestatus();
	//	}
}

void tscrolldown(int orig, int n, int copyhist) {
	int i;


	DBG("===== tscrolldown, orig:%d n:%d , histindex: %d  scr: %d copyhist: %d term->scroll_bottom: %d\n",orig,n, term->histindex, term->scr, copyhist, term->scroll_bottom);
	if ( term->histindex == 0 && IS_SET(MODE_ALTSCREEN) ){ //xxx bug patch. alt screen 
																		// else segfaults. reproduce: man man; and scroll with a (now, since this is patched rotfl) unknown combination of commands.
																		//DBG("RETURN\n"); // xxx
																		//return; // ? strange. no segfaults anymore. 
	}
	//LIMIT(n, 0, term->scroll_bottom - orig ); //xxx
	LIMIT(n, 0, term->scroll_bottom - orig + 1);

	if (copyhist) {
		term->histindex = (term->histindex - 1 ) & (term->histsize); 
		SWAPp( term->hist[term->histindex], term->line[term->scroll_bottom] );
		///DBG("copyhist: term->histindex %d   %p <->  %p  \n", term->histindex, term->hist[term->histindex], term->line[term->scroll_bottom] );

		if (  term->line[term->scroll_bottom] == 0 ){
			///DBG("newline .");
			//term->circledhist=1; //?
			term->line[term->scroll_bottom]  = xmalloc( term->colalloc * sizeof(Glyph));
			memset( term->line[term->scroll_bottom]  ,0,term->colalloc * sizeof(Glyph));
		}

	}

	tsetdirt(orig, term->scroll_bottom - n);
	tclearregion(0, term->scroll_bottom - n + 1, term->cols - 1, term->scroll_bottom);

	for (i = term->scroll_bottom; i >= orig + n; i--) {
		SWAPp( term->line[i], term->line[i-n] );
	}

	selscroll(orig, n);
}


// Scroll downwards (And append lines)
// doesn't neccessarily scroll the view.
// Thanks for the names. Afterall, upwards is downwards, its just only the opposite.
void tscrollup(int orig, int n, int copyhist) {
	int i;

	DBG("===== tscrollup, orig:%d n:%d , histindex: %d  scr: %d copyhist: %d \n",orig,n, term->histindex, term->scr, copyhist);
	//LIMIT(n, 0, term->scroll_bottom - orig ); //xxx
	LIMIT(n, 0, term->scroll_bottom - orig + 1);

	if (copyhist) {
		DBG("term->histindex: %d\n", term->histindex);
		term->histindex = (term->histindex + 1) & (term->histsize);
		DBG("term->histindex: %d, \n", term->histindex);
		if ( term->histindex == 0 ){
			if ( term->circledhist == 0 ){
				term->circledhist=1;
				DBG("circledhist = 1");
				// dirty bugfix below. didn't find the real problem
				term->hist[0] = xmalloc( term->colalloc * sizeof(Glyph));
				memset(term->hist[0],0,term->colalloc * sizeof(Glyph));
			}
		}

		if ( term->hist[term->histindex] ){
			DBG("SWAP cthist %p, histindex %d, orig %d\n", term->hist[term->histindex], term->histindex, orig);

			if (  term->line[term->scroll_bottom] == 0 ){
				DBG("newline 2 .");
				//term->circledhist=1; //?
				term->line[term->scroll_bottom]  = xmalloc( term->colalloc * sizeof(Glyph));
				memset( term->line[term->scroll_bottom]  ,0,term->colalloc * sizeof(Glyph));
			}
			SWAPp( term->hist[term->histindex], term->line[orig] );

		}	 else {
			DBG("New line, cthist %d, term->histindex: %d, term->col: %d\n", 1, term->histindex, term->cols );
			term->hist[term->histindex] = term->line[orig];
			term->line[orig] = xmalloc( term->colalloc * sizeof(Glyph) );
			memset(term->line[orig],0,term->colalloc * sizeof(Glyph));
		}
		// (candidate for swap or, malloc hist here) done.
		// "compression" might take place here, as well.
		// sort of count*glyph for adjacent equal glyphs.
		// Maybe another text attribute. Then, the next glyph
		// as union int gives the count. Giving for, e.g. an empty line
		// with 200 cols a compression ratio of 200/2 (misc)
	}

	DBG("1\n");
	if (term->scr > 0 && term->scr < term->histsize + 1 ) { //HSTSIZE
		term->scr = MIN(term->scr + n, term->histsize );  //HISTSIZE - 1);
	}

	DBG("1a\n");
	tclearregion(0, orig, term->colalloc - 1, orig + n - 1);
	DBG("2\n");
	tsetdirt(orig + n, term->scroll_bottom);

	DBG("swap: %d   %d\n",orig,term->scroll_bottom);
	for (i = orig; i <= term->scroll_bottom - n; i++) {
		SWAPp(term->line[i],term->line[i+n]);
	}

	selscroll(orig, -n);
	DBG("scrd: %d %d %d %d %d %d\n", orig, n, term->histindex, term->scr, term->scrollmarks[0], term->rows);
	if ( enterlessmode ){ // scroll down until next line.
		if ( term->histindex > term->scrollmarks[0]){
			//DBG("Scroll\n");
			term->scr=(term->histindex)-term->scrollmarks[0];
			selscroll(0, term->scr);
			//tfulldirt();
			lessmode_toggle(ARGP(i=LESSMODE_ON));
			enterlessmode = 0;
			//a.i=0;
			//scrollmark(&a);

			//inputmode |= MODE_LESS;
			//set_notifmode( 2, -1 ); // show message "less"
		}
	}

}

void tnewline(int first_col) {
	int y = term->cursor.y;

	//xxx
	//DBG("tnewline: %d, term->scr: %d  histindex: %d\n",first_col, term->scr,term->histindex );
	if (y == term->scroll_bottom) {
		tscrollup(term->scroll_top, 1, 1);
	} else {
		y++;
	}
	tmoveto(first_col ? 0 : term->cursor.x, y);
	updatestatus();
}

void enterscroll(const Arg *a){
	if (term==p_alt) return;
	//DBG("enterscroll: %d %d %d %d\n",term->histindex,term->rows,term->scr,term->cursor.y);

	term->scrollmarks[0] = term->histindex+ term->rows - ( term->rows - term->cursor.y );
	enterlessmode = term->rows;
	ttywrite((utfchar*)"\n",1,1);
}

void leavescroll(const Arg *a){
	enterlessmode = 0;
	ttywrite((utfchar*)"\n",1,1);
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

