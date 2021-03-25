
#include "scroll.h"


int scrollmarks[12];
int retmarks[10];
static int cret=0;
static int circledret=0;
static int pret=0;


void tsetscroll(int t, int b) {
		LIMIT(t, 0, term.row - 1);
		LIMIT(b, 0, term.row - 1);
		if (t > b) {
				//		SWAPint( t,b );
				term.top = b;
				term.bot = t;
		} else {
				term.top = t;
				term.bot = b;
		}
}


void kscrolldown(const Arg *a) {
		int n = a->i;
		//printf("kscrolldown, histi: %d  scr: %d\n",term.histi,term.scr );

		dbg2("kscrolldown, n: %d, guard: %x\n",n, term.guard);
		if (n < 0) {
				n = term.row + n;
		}

		if (n > term.scr) {
				n = term.scr;
		}
		dbg2("kscrolldown2, n: %d\n",n);

		if (term.scr > 0) {
				term.scr -= n;
				selscroll(0, -n);
				tfulldirt();
				updatestatus();
		}
}

void scrolltobottom(){
		if ( term.scr ){
				term.scr=0;
				selscroll(0, 0);
				tfulldirt();	
				updatestatus();
		}
}

void scrolltotop(){
		term.scr=HISTSIZE;
		if ( (term.circledhist==0) && (term.scr>term.histi ) )
				term.scr=term.histi;
		selscroll(0, term.scr);
		tfulldirt();
		updatestatus();
}


void kscrollup(const Arg *a) {
		int n = a->i;

		//printf("kscrollup, histi: %d  scr: %d\n",term.histi,term.scr );
		dbg2("kscrollup, n: %d, term.histi: %d, term.row: %d scr: %d\n",
						n, term.histi, term.row, term.scr);
		if (n < 0) {
				n = term.row + n;
		}
		dbg2("kscrollup2, n: %d\n",n);

		if ( term.scr <= HISTSIZE-n ) {
				term.scr += n;

				if ( (term.circledhist==0) && (term.scr>term.histi ) )
						term.scr=term.histi;

				selscroll(0, n);
				tfulldirt();
				updatestatus();
		}
}


void set_scrollmark(const Arg *a) {
	scrollmarks[a->i] = term.histi-term.scr+1;	
	updatestatus();
	//printf("Setscrollmark: n:%d histi:%d scr:%d\n", a->i, term.histi, term.scr );
}

void set_retmark() {
	retmarks[0] = term.histi;//-term.scr;	
	//updatestatus();
	//printf("Setretmark: n:%d histi:%d scr:%d\n", 0, term.histi, term.scr );
}

void retmark(const Arg* a){
	//printf("Retmark: n:%d scrm:%d histi:%d scr:%d\n", term.row, retmarks[0],term.histi, term.scr );
//	if ( scrollmarks[a->i] ){
	if ( term.histi<term.row){
			scrolltotop();
			return;
	}

	term.scr=(term.histi-retmarks[0])-term.row+1;
	printf("scr: %d\n", term.scr );
	if ( term.scr<0 ){
			// TODO: circledhist
			term.scr=0;
	};
	selscroll(0, term.scr);
	tfulldirt();
	updatestatus();
//	}
}



void scrollmark(const Arg *a){
	//printf("Scrollmark: n:%d scrm:%d histi:%d scr:%d\n", a->i, scrollmarks[a->i],term.histi, term.scr );
//	if ( scrollmarks[a->i] ){
	term.scr=term.histi-scrollmarks[a->i]+1;
	selscroll(0, term.scr);
	tfulldirt();
	updatestatus();
//	}
}

void tscrolldown(int orig, int n, int copyhist) {
		int i;


		//printf("===== tscrolldown, orig:%d n:%d , histi: %d  scr: %d copyhist: %d\n",orig,n, term.histi, term.scr, copyhist);
		LIMIT(n, 0, term.bot - orig + 1);

		if (copyhist) {
				term.histi = ((term.histi - 1 ) ^ HISTSIZE) & (HISTSIZE-1); 
				//term.histi = (term.histi - 1 + HISTSIZE) % HISTSIZE; //??? uh. negative number, I guess
				SWAPp( term.hist[term.cthist][term.histi], term.line[term.bot] );
		}

		tsetdirt(orig, term.bot - n);
		tclearregion(0, term.bot - n + 1, term.col - 1, term.bot);

		for (i = term.bot; i >= orig + n; i--) {
				SWAPp( term.line[i], term.line[i-n] );
		}

		selscroll(orig, n);
}



void tscrollup(int orig, int n, int copyhist) {
		int i;

		//printf("===== tscrollup, orig:%d n:%d , histi: %d  scr: %d copyhist: %d \n",orig,n, term.histi, term.scr, copyhist);
		LIMIT(n, 0, term.bot - orig + 1);

		if (copyhist) {
				dbg2("term.histi: %d\n", term.histi);
				term.histi = ((term.histi + 1) ^ HISTSIZE ) & (HISTSIZE-1);
				dbg2("term.histi: %d, \n", term.histi);
				if ( term.histi == 0 ){
						if ( term.circledhist == 0 ){
								term.circledhist=1;
								dbg("circledhist = 1");
								// dirty bugfix below. didn't find the real problem
								term.hist[0][0] = xmalloc( term.colalloc * sizeof(Glyph));
								memset(term.hist[0][0],0,term.colalloc * sizeof(Glyph));
						}
				}


				if ( term.hist[term.cthist][term.histi] ){
						dbg2("SWAP cthist %d, histi %d, orig %d\n", term.cthist, term.histi, orig);
						SWAPp( term.hist[term.cthist][term.histi], term.line[orig] );
				}	 else {
						dbg2("New line, cthist %d, term.histi: %d, term.col: %d\n", term.cthist, term.histi, term.col);
						term.hist[term.cthist][term.histi] = term.line[orig];
						term.line[orig] = xmalloc( term.colalloc * sizeof(Glyph));
						memset(term.line[orig],0,term.colalloc * sizeof(Glyph));
				}
				// (candidate for swap or, malloc hist here) done.
				// "compression" might take place here, as well.
				// sort of count*glyph for adjacent equal glyphs.
				// Maybe another text attribute. Then, the next glyph
				// as union int gives the count. Giving for, e.g. an empty line
				// with 200 cols a compression ratio of 200/2 (misc)
		}

		if (term.scr > 0 && term.scr < HISTSIZE) {
				term.scr = MIN(term.scr + n, HISTSIZE - 1);
		}

		tclearregion(0, orig, term.colalloc - 1, orig + n - 1);
		tsetdirt(orig + n, term.bot);

		for (i = orig; i <= term.bot - n; i++) {
				SWAPp(term.line[i],term.line[i+n]);
		}

		selscroll(orig, -n);
		dbgf("scrd: %d %d %d %d %d %d\n", orig, n, term.histi, term.scr, scrollmarks[0], term.row);
		if ( enterlessmode ){ // scroll down until next line.
				if ( term.histi > scrollmarks[0]){
						//printf("Scroll\n");
						term.scr=(term.histi)-scrollmarks[0];
						selscroll(0, term.scr);
						//tfulldirt();
						Arg a; 
						 a.i=2;
					  lessmode_toggle(&a);
						enterlessmode = 0;
						//a.i=0;
						//scrollmark(&a);

						//inputmode |= MODE_LESS;
						//set_notifmode( 2, -1 ); // show message "less"
				}
		}

}

void tnewline(int first_col) {
		int y = term.c.y;

		dbgf("tnewline: %d, term.scr: %d  histi: %d\n",first_col, term.scr,term.histi );
		if (y == term.bot) {
				tscrollup(term.top, 1, 1);
		} else {
				y++;
		}
		tmoveto(first_col ? 0 : term.c.x, y);
		updatestatus();
}

void enterscroll(const Arg *a){
		//printf("enterscroll\n");
		//set_scrollmark( a );
		scrollmarks[0] = term.histi+term.row;
		enterlessmode = term.row;
		ttywrite("\n",1,1);
}

void leavescroll(const Arg *a){
		enterlessmode = 0;
		ttywrite("\n",1,1);
}


void lessmode_toggle(const Arg *a){
		if (abs(a->i) == 2 ){ // enable
				inputmode |= MODE_LESS;
				//selscroll(0,0);
				tfulldirt();
				//set_notifmode( 2, -1 ); // show message "less"
		} else {
				if (abs(a->i) == 1 ){ // enable
						inputmode |= MODE_LESS;
						Arg d = { .i=0 };
						kscrolldown(&d);
				} else {
						if ( a->i == -3 ) //disable 
								inputmode &= ~MODE_LESS;
						else // toggle - i==0
								inputmode ^= MODE_LESS;
				}
		}

		if ( inputmode & MODE_LESS ){ // enable
				//set_notifmode( 2, -1 ); // show message "less"
				showstatus(1," -LESS- ");
				updatestatus();
		} else { // disable
				//set_notifmode( 4,-2 ); // hide message
				showstatus(0,0);
				scrolltobottom();
		}
}

