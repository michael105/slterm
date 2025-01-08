
#include "statusbar.h"
#include "term.h"
#include "mem.h"
#include "config.h"


int statusvisible;
Glyph *statusbar = NULL;
char* p_status = NULL;
static int statuswidth = 0;
static int focusdraw = 1;


void drawstatus(){
	term->dirty[term->bot] = 1;
	drawregion(0, term->bot, term->col, term->bot + 1);
}

void statusbar_focusin(){
	if ( !focusdraw && statusvisible && statusbar ){
		focusdraw = 1;
		for ( Glyph *g = statusbar; g < &statusbar[statuswidth]; g++) {
			g->mode = statusattr;
			g->fg = statusfg;
			g->bg = statusbg;

		}
		drawstatus();
	}
}


void statusbar_focusout(){
	if ( focusdraw && statusvisible && statusbar ){
		focusdraw = 0;
		for ( Glyph *g = statusbar; g < &statusbar[statuswidth]; g++) {
			g->mode = ATTR_REVERSE; 
			g->fg = defaultfg,
			g->bg = defaultbg;
		}
		drawstatus();
	}
}




// updates the statusbar with current line, etc., when visible.
void updatestatus(){

	if ( statusvisible ){

		// currently shown number of cols
		int stwidth = statuswidth;
		if ( term->col != statuswidth )
			stwidth = term->col;

		char buf[512];
		memset( buf, ' ', 256 );
		//bzero(buf+256,256);

		//int p = sprintf(buf,"  %s  %5d-%2d %5d %5d %3d%% (%3d%%)   RM:%3d", p_status,
		int p = sprintf(buf+256,"%5d-%2d %5d %5d %3d%% (%3d%%)   RM:%3d",
				term->histi-term->scr,term->histi-term->scr+term->row, 
				term->histi+term->row, term->histi+term->row-(term->histi-term->scr+term->row),
				((term->histi-term->scr)*100)/((term->histi)?term->histi:1),
				((term->histi-term->scr-term->scrollmarks[0]+1)*100)/((term->histi-term->scrollmarks[0]+1)?term->histi-term->scrollmarks[0]+1:1),
				term->retmark_scrolled
				);

		if ( stwidth > p+10 ){
			buf[p++]=' ';

			for ( int a=1; a<10; a++ ){
				if ( term->scrollmarks[a] )
					buf[p++] = a+'0';
				else
					buf[p++] = ' ';
			}
			if ( term->scrollmarks[0] )
				buf[p++] = '0';
			else 
				buf[p++] = ' ';
		}

		buf[p] = 0;

		int bp = 256 - stwidth + p;
		if ( bp <0 ) bp = 0;
		if ( 256-bp > strlen(p_status) +3 )
			memcpy( buf+bp+3, p_status, strlen(p_status) );

		setstatus(buf+bp);
	}
}


void setstatus(char* status){
	Glyph *deb, *fin;

	if ( term->colalloc != statuswidth ){
		free(statusbar);
		statusbar = xmalloc(term->colalloc * sizeof(Glyph));
		//statusbar = xrealloc(statusbar, term->colalloc * sizeof(Glyph));
		statuswidth = term->colalloc;
	}
	if ( term->col != statuswidth )
		statuswidth = term->col;

#ifndef UTF8
	Glyph g = { .fg = statusfg, .bg = statusbg, .mode = statusattr, .u = ' ' };
#endif

	for (deb = statusbar,fin=&statusbar[statuswidth]; (deb < fin);	deb++) {
#ifdef UTF8
		deb->mode = statusattr;
		deb->fg = statusfg;
		deb->bg = statusbg;
#else
		deb->intG = g.intG;
#endif
		if ( *status ){
			deb->u = *status;
			status++;
		}
	}

}

void showstatus(int show, char *status){
	if ( p_status ){
		free(p_status);
		p_status = NULL;
	}

	if ( show ){
		p_status = strdup(status);
		if ( !statusvisible ){
			statusvisible = 1;
			Arg a = { .i=0 };
			kscrollup(&a); // (clears the statusbar). I know. But works.
			setstatus(status);
			// paint status
			redraw();
		}

	} else {
		if ( statusvisible ){
			statusvisible = 0;
			// clear status
			term->dirty[term->bot] = 1;
			drawregion(0, term->bot, term->col, term->bot + 1);
			//term->row++;
			//tresize(term->col,term->row+1);
		}
	}
}

// shows a small message at the bottom right corner
void set_notifmode(int type, KeySym ksym) {
	static char *lib[] = {" MOVE ", "SELECT"," LESS " };
	static Glyph *g, *deb, *fin;
	static int col, bot;
	col = term->col, bot = term->bot;

	if (ksym == -1) { // show
		free(g);
		g = xmalloc(term->colalloc * sizeof(Glyph));
		memcpy(g, TLINE(bot), term->colalloc * sizeof(Glyph));
		//tresize(term->col,term->row-1);
	} else if (ksym == -2) { // hide
		memcpy(TLINE(bot), g, term->colalloc * sizeof(Glyph));
		//tresize(term->col,term->row+1);
	}

	if (type < 3) {
		char *z = lib[type];
		for (deb = &TLINE(bot)[col - 6], fin = &TLINE(bot)[col]; deb < fin;
				z++, deb++) {
			deb->mode = ATTR_REVERSE, deb->u = *z, deb->fg = defaultfg,
				deb->bg = defaultbg;
		}
	} else if (type < 5) {
		memcpy(TLINE(bot), g, term->colalloc * sizeof(Glyph));
		//memcpy(TLINE(bot), g, term->colalloc * sizeof(Glyph));
	} else {
		for (deb = &TLINE(bot)[0], fin = &TLINE(bot)[col]; deb < fin;
				deb++) {
			deb->mode = ATTR_REVERSE, deb->u = ' ', deb->fg = defaultfg,
				deb->bg = defaultbg;
		}
		TLINE(bot)[0].u = ksym;
	}

	term->dirty[bot] = 1;
	drawregion(0, bot, col, bot + 1);
}


void statusbar_kpress( KeySym *ks, char *buf ){

}


