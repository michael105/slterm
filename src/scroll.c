
#include "scroll.h"


void tsetscroll(int t, int b) {
		LIMIT(t, 0, term->row - 1);
		LIMIT(b, 0, term->row - 1);
		if (t > b) {
				//		SWAPint( t,b );
				term->top = b;
				term->bot = t;
		} else {
				term->top = t;
				term->bot = b;
		}
}


void kscrolldown(const Arg *a) {
		int n = a->i;

		dbg2("kscrolldown, n: %d, guard: %x\n",n, term->guard);
		if (n < 0) {
				n = term->row + n;
		}

		if (n > term->scr) {
				n = term->scr;
		}
		dbg2("kscrolldown2, n: %d\n",n);

		if (term->scr > 0) {
				term->scr -= n;
				selscroll(0, -n);
				tfulldirt();
				updatestatus();
		}
}

void scrolltobottom(){
	//printf("scrolltobottom\n"); // xxx
		if ( term->scr ){
				term->scr=0;
				selscroll(0, 0);
				tfulldirt();	
				updatestatus();
		}
}

void scrolltotop(){
	//printf("totop\n");
		term->scr=HISTSIZE;
		if ( (term->circledhist==0) && (term->scr>term->histi ) )
				term->scr=term->histi;
		selscroll(0, term->scr);
		tfulldirt();
		updatestatus();
}


void kscrollup(const Arg *a) {
		int n = a->i;

		dbg2("kscrollup, n: %d, term->histi: %d, term->row: %d scr: %d\n",
						n, term->histi, term->row, term->scr);
		if (n < 0) {
				n = term->row + n;
		}
		dbg2("kscrollup2, n: %d\n",n);

		if ( term->scr <= HISTSIZE-n ) {
				term->scr += n;

				if ( (term->circledhist==0) && (term->scr>term->histi ) )
						term->scr=term->histi;

				selscroll(0, n);
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
	term->scrollmarks[a->i] = term->histi-term->scr+1;	
	updatestatus();
	//printf("Setscrollmark: n:%d histi:%d scr:%d\n", a->i, term->histi, term->scr );
}

void set_retmark() {
	if (term==p_alt) return;
	
	//if ( term->histi + term->cursor.y < term->row )
	//	return; // scrolled less than a page.

	term->retmarks[ term->current_retmark ] = term->histi + term->cursor.y;
	term->current_retmark = (term->current_retmark + 1) & (RETMARKCOUNT-1);
	term->scroll_retmark = term->current_retmark;
	//updatestatus();
	//printf("Setretmark: n:%d histi:%d scr:%d\n", 0, term->histi, term->scr );
}

void retmark(const Arg* a){
	if (term==p_alt) return; // not usable in alt screen
	//printf("Retmark: n:%d scrm:%d histi:%d scr:%d   scroll_mark %d  current_mark %d\n", term->row, term->retmarks[0],term->histi, term->scr, term->scroll_retmark, term->current_retmark );


	if ( a->i == -1 ){ // tab right in lessmode -> scrolling down

		// scanning could be optimized. (skip, and divide..)
		int b = 1;
		for ( int t = (term->current_retmark +1 ) & (RETMARKCOUNT-1); t!=term->current_retmark; 
				t = (t+1) & ( RETMARKCOUNT-1 ) ){
			//printf("mark: %d   %d\n",t, term->retmarks[t] );
			if ( (term->histi - term->retmarks[t] < term->scr) ){
				term->scr=(term->histi-term->retmarks[t]);
				term->retmark_scrolled = (term->current_retmark-t) & ( RETMARKCOUNT-1);
				b = 0;
				break;
			}
		}
		if ( b ){
			term->retmark_scrolled = 0;
			scrolltobottom();
		}

	} else if ( a->i >= 1 ){ // number, scroll to retmark number x
		int t = (term->current_retmark - a->i ) & (RETMARKCOUNT-1); 
		term->scr=(term->histi-term->retmarks[t]);
		term->retmark_scrolled = (term->current_retmark-t) & ( RETMARKCOUNT-1);

	} else { // scroll backward
		if ( term->histi<term->row){ // at the top
			scrolltotop();
			lessmode_toggle( ARGPi(LESSMODE_ON) );	
			return;
		}
		
		for ( int t = (term->current_retmark -1 ) & (RETMARKCOUNT-1); t!=term->current_retmark; 
				t = (t-1) & ( RETMARKCOUNT-1 ) ){
			//printf("mark: %d   %d\n",t, term->retmarks[t] );
			if ( (term->retmarks[t]==0) || (term->histi - term->retmarks[t] > term->scr) ){
				term->scr=(term->histi-term->retmarks[t]);
				term->retmark_scrolled = (term->current_retmark-t) & ( RETMARKCOUNT-1);
				break;
			}
		}

		/*
		int t = term->scroll_retmark;
		term->scroll_retmark = (term->scroll_retmark-1) & (RETMARKCOUNT-1);
		if ( term->scroll_retmark == ((term->current_retmark) &(RETMARKCOUNT-1)) ||
				(term->retmarks[t]==0 && term->retmarks[term->scroll_retmark] == 0 )  )
			term->scroll_retmark = t;

		term->scr=(term->histi-term->retmarks[term->scroll_retmark]);
		*/
	}
	//term->scr=(term->histi-term->retmarks[0])-term->row+1;
	//printf("scr: %d\n", term->scr );
	if ( term->scr<0 ){
			// TODO: circledhist
		term->scr=0;
		//term->scr&=(HISTSIZE-1);
	};
	//printf("Retmark OUT: n:%d scrm:%d histi:%d scr:%d   scroll_mark %d  current_mark %d\n", term->row, term->retmarks[0],term->histi, term->scr, term->scroll_retmark, term->current_retmark );

	selscroll(0, term->scr);
	tfulldirt();
	//updatestatus();
	lessmode_toggle( ARGPi(LESSMODE_ON) );
//	}
}



void scrollmark(const Arg *a){
	if (term==p_alt) return;
	//printf("Scrollmark: n:%d scrm:%d histi:%d scr:%d\n", a->i, term->scrollmarks[a->i],term->histi, term->scr );
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


		//printf("===== tscrolldown, orig:%d n:%d , histi: %d  scr: %d copyhist: %d term->bot: %d\n",orig,n, term->histi, term->scr, copyhist, term->bot);
		if ( term->histi == 0 && IS_SET(MODE_ALTSCREEN) ){ //xxx bug patch. alt screen 
		// else segfaults. reproduce: man man; and scroll with a (now, since this is patched rotfl) unknown combination of commands.
			printf("RETURN\n"); // xxx
			//return; // ? strange. no segfaults anymore. 
		}
		//LIMIT(n, 0, term->bot - orig ); //xxx
		LIMIT(n, 0, term->bot - orig + 1);

		if (copyhist) {
				//term->histi = (term->histi - 1 ) & ~(HISTSIZE-1); //xxx
				term->histi = ((term->histi - 1 ) ^ HISTSIZE) & (HISTSIZE-1); 
				//term->histi = (term->histi - 1 + HISTSIZE) % HISTSIZE; //??? uh. negative number, I guess
				SWAPp( term->hist[term->histi], term->line[term->bot] );
				///printf("copyhist: term->histi %d   %p <->  %p  \n", term->histi, term->hist[term->histi], term->line[term->bot] );

				if (  term->line[term->bot] == 0 ){
					///printf("newline .");
						//term->circledhist=1; //?
					 term->line[term->bot]  = xmalloc( term->colalloc * sizeof(Glyph));
					memset( term->line[term->bot]  ,0,term->colalloc * sizeof(Glyph));
				}

		}

		tsetdirt(orig, term->bot - n);
		tclearregion(0, term->bot - n + 1, term->col - 1, term->bot);

		for (i = term->bot; i >= orig + n; i--) {
				SWAPp( term->line[i], term->line[i-n] );
		}

		selscroll(orig, n);
}



void tscrollup(int orig, int n, int copyhist) {
		int i;

		//	printf("===== tscrollup, orig:%d n:%d , histi: %d  scr: %d copyhist: %d \n",orig,n, term->histi, term->scr, copyhist);
		//LIMIT(n, 0, term->bot - orig ); //xxx
		LIMIT(n, 0, term->bot - orig + 1);

		if (copyhist) {
				dbg2("term->histi: %d\n", term->histi);
				term->histi = ((term->histi + 1) ^ HISTSIZE ) & (HISTSIZE-1);
				///printf("term->histi: %d, \n", term->histi);
				if ( term->histi == 0 ){
						if ( term->circledhist == 0 ){
								term->circledhist=1;
								///printf("circledhist = 1");
								// dirty bugfix below. didn't find the real problem
								term->hist[0] = xmalloc( term->colalloc * sizeof(Glyph));
								memset(term->hist[0],0,term->colalloc * sizeof(Glyph));
						}
				}


				if ( term->hist[term->histi] ){
						///printf("SWAP cthist %d, histi %d, orig %d\n", term->cthist, term->histi, orig);

						if (  term->line[term->bot] == 0 ){
							///printf("newline 2 .");
							//term->circledhist=1; //?
							term->line[term->bot]  = xmalloc( term->colalloc * sizeof(Glyph));
							memset( term->line[term->bot]  ,0,term->colalloc * sizeof(Glyph));
						}
						SWAPp( term->hist[term->histi], term->line[orig] );

				}	 else {
						///printf("New line, cthist %d, term->histi: %d, term->col: %d\n", term->cthist, term->histi, term->col);
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

		///printf("swap\n");
		for (i = orig; i <= term->bot - n; i++) {
				SWAPp(term->line[i],term->line[i+n]);
		}

		selscroll(orig, -n);
		///printf("scrd: %d %d %d %d %d %d\n", orig, n, term->histi, term->scr, term->scrollmarks[0], term->row);
		if ( enterlessmode ){ // scroll down until next line.
				if ( term->histi > term->scrollmarks[0]){
						//printf("Scroll\n");
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
		//printf("tnewline: %d, term->scr: %d  histi: %d\n",first_col, term->scr,term->histi );
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
		//printf("enterscroll: %d %d %d %d\n",term->histi,term->row,term->scr,term->cursor.y);
		
		term->scrollmarks[0] = term->histi+ term->row - ( term->row - term->cursor.y );
		enterlessmode = term->row;
		ttywrite("\n",1,1);
}

void leavescroll(const Arg *a){
		enterlessmode = 0;
		ttywrite("\n",1,1);
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
		//printf("scroll %d\n",a->i);
		//printf("scroll %d\n",d.i);
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

