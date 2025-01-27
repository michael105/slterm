
#include "scroll.h"

void tsetscroll(int t, int b) {
	LIMIT(t, 0, term->rows - 1);
	LIMIT(b, 0, term->rows - 1);
	if (t > b) {
		//		SWAPint( t,b );
		term->top = b;
		term->bot = t;
	} else {
		term->top = t;
		term->bot = b;
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
	term->scr=HISTSIZE;
	if ( (term->circledhist==0) && (term->scr>term->histi ) )
		term->scr=term->histi; // 
	selscroll(0, term->scr);
	retmark_scrolledup();
	tfulldirt();
	updatestatus();
}

// scroll upwards (Shift+up)_
void kscrollup(const Arg *a) {
	int n = a->i;

	DBG2("kscrollup, n: %d, term->histi: %d, term->rows: %d scr: %d\n",
			n, term->histi, term->rows, term->scr);
	if (n < 0) { // scroll a page upwards
					 //n = term->rows + n;
		n = term->rows;
	}
	DBG2("kscrollup2, n: %d\n",n);

	if ( term->scr <= HISTSIZE-n ) { 
		term->scr += n;

		if ( (term->circledhist==0) && (term->scr>term->histi ) )
			term->scr=term->histi; // at the top

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
	term->scrollmarks[a->i] = term->histi - term->scr+1;	
	updatestatus();
	//DBG("Setscrollmark: n:%d histi:%d scr:%d\n", a->i, term->histi, term->scr );
}


void scrollmark(const Arg *a){
	if (term==p_alt) return;
	//DBG("Scrollmark: n:%d scrm:%d histi:%d scr:%d\n", a->i, term->scrollmarks[a->i],term->histi, term->scr );
	//	if ( term->scrollmarks[a->i] ){
	if ( term->scrollmarks[a->i] )
		term->scr=term->histi-term->scrollmarks[a->i]+1;
	else 
		term->scr=term->histi-term->scrollmarks[a->i];
	selscroll(0, term->scr);
	tfulldirt();
	updatestatus();
	//	}
}

void tscrolldown(int orig, int n, int copyhist) {
	int i;


	DBG("===== tscrolldown, orig:%d n:%d , histi: %d  scr: %d copyhist: %d term->bot: %d\n",orig,n, term->histi, term->scr, copyhist, term->bot);
	if ( term->histi == 0 && IS_SET(MODE_ALTSCREEN) ){ //xxx bug patch. alt screen 
																		// else segfaults. reproduce: man man; and scroll with a (now, since this is patched rotfl) unknown combination of commands.
																		//DBG("RETURN\n"); // xxx
																		//return; // ? strange. no segfaults anymore. 
	}
	//LIMIT(n, 0, term->bot - orig ); //xxx
	LIMIT(n, 0, term->bot - orig + 1);

	if (copyhist) {
		term->histi = (term->histi - 1 ) & (HISTSIZE-1); 
		SWAPp( term->hist[term->histi], term->line[term->bot] );
		///DBG("copyhist: term->histi %d   %p <->  %p  \n", term->histi, term->hist[term->histi], term->line[term->bot] );

		if (  term->line[term->bot] == 0 ){
			///DBG("newline .");
			//term->circledhist=1; //?
			term->line[term->bot]  = xmalloc( term->colalloc * sizeof(Glyph));
			memset( term->line[term->bot]  ,0,term->colalloc * sizeof(Glyph));
		}

	}

	tsetdirt(orig, term->bot - n);
	tclearregion(0, term->bot - n + 1, term->cols - 1, term->bot);

	for (i = term->bot; i >= orig + n; i--) {
		SWAPp( term->line[i], term->line[i-n] );
	}

	selscroll(orig, n);
}


// Scroll downwards (And append lines)
// doesn't neccessarily scroll the view.
// Thanks for the names. Afterall, upwards is downwards, its just only the opposite.
void tscrollup(int orig, int n, int copyhist) {
	int i;

	DBG("===== tscrollup, orig:%d n:%d , histi: %d  scr: %d copyhist: %d \n",orig,n, term->histi, term->scr, copyhist);
	//LIMIT(n, 0, term->bot - orig ); //xxx
	LIMIT(n, 0, term->bot - orig + 1);

	if (copyhist) {
		DBG2("term->histi: %d\n", term->histi);
		term->histi = (term->histi + 1) & (HISTSIZE-1);
		///DBG("term->histi: %d, \n", term->histi);
		if ( term->histi == 0 ){
			if ( term->circledhist == 0 ){
				term->circledhist=1;
				///DBG("circledhist = 1");
				// dirty bugfix below. didn't find the real problem
				term->hist[0] = xmalloc( term->colalloc * sizeof(Glyph));
				memset(term->hist[0],0,term->colalloc * sizeof(Glyph));
			}
		}


		if ( term->hist[term->histi] ){
			///DBG("SWAP cthist %d, histi %d, orig %d\n", term->cthist, term->histi, orig);

			if (  term->line[term->bot] == 0 ){
				///DBG("newline 2 .");
				//term->circledhist=1; //?
				term->line[term->bot]  = xmalloc( term->colalloc * sizeof(Glyph));
				memset( term->line[term->bot]  ,0,term->colalloc * sizeof(Glyph));
			}
			SWAPp( term->hist[term->histi], term->line[orig] );

		}	 else {
			///DBG("New line, cthist %d, term->histi: %d, term->col: %d\n", term->cthist, term->histi, term->cols );
			term->hist[term->histi] = term->line[orig];
			term->line[orig] = xmalloc( term->colalloc * sizeof(Glyph));
			memset(term->line[orig],0,term->colalloc * sizeof(Glyph));
		}
		// (candidate for swap or, malloc hist here) done.
		// "compression" might take place here, as well.
		// sort of count*glyph for adjacent equal glyphs.
		// Maybe another text attribute. Then, the next glyph
		// as union int gives the count. Giving for, e.g. an empty line
		// with 200 cols a compression ratio of 200/2 (misc)
	}

	if (term->scr > 0 && term->scr < HISTSIZE) {
		term->scr = MIN(term->scr + n, HISTSIZE - 1);
	}

	tclearregion(0, orig, term->colalloc - 1, orig + n - 1);
	tsetdirt(orig + n, term->bot);

	///DBG("swap\n");
	for (i = orig; i <= term->bot - n; i++) {
		SWAPp(term->line[i],term->line[i+n]);
	}

	selscroll(orig, -n);
	///DBG("scrd: %d %d %d %d %d %d\n", orig, n, term->histi, term->scr, term->scrollmarks[0], term->rows);
	if ( enterlessmode ){ // scroll down until next line.
		if ( term->histi > term->scrollmarks[0]){
			//DBG("Scroll\n");
			term->scr=(term->histi)-term->scrollmarks[0];
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
	//DBG("tnewline: %d, term->scr: %d  histi: %d\n",first_col, term->scr,term->histi );
	if (y == term->bot) {
		tscrollup(term->top, 1, 1);
	} else {
		y++;
	}
	tmoveto(first_col ? 0 : term->cursor.x, y);
	updatestatus();
}

void enterscroll(const Arg *a){
	if (term==p_alt) return;
	//DBG("enterscroll: %d %d %d %d\n",term->histi,term->rows,term->scr,term->cursor.y);

	term->scrollmarks[0] = term->histi+ term->rows - ( term->rows - term->cursor.y );
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

