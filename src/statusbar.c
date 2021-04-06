
#include "statusbar.h"
#include "st.h"
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
				char buf[256];
				//int p = sprintf(buf," -LESS-  %5d-%2d %5d %3d%% (%3d%%)", 
				int p = sprintf(buf,"  %s  %5d-%2d %5d %3d%% (%3d%%)", p_status,
						term->histi-term->scr,term->histi-term->scr+term->row, 
						term->histi+term->row, 
						((term->histi-term->scr)*100)/((term->histi)?term->histi:1),
						((term->histi-term->scr-scrollmarks[0]+1)*100)/((term->histi-scrollmarks[0]+1)?term->histi-scrollmarks[0]+1:1)
						);
				buf[p]=' ';

				for ( int a=1; a<10; a++ ){
						if ( scrollmarks[a] )
										buf[a+p] = a+'0';
								else
										buf[a+p] = ' ';
				}
				if ( scrollmarks[0] )
						buf[p+10] = '0';
				else 
						buf[p+10] = ' ';
				buf[p+11] = 0;

				setstatus(buf);
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
		static char *lib[] = {" MOVE ", " SEL  "," LESS " };
		static Glyph *g, *deb, *fin;
		static int col, bot;

		if (ksym == -1) { // show
				free(g);
				col = term->col, bot = term->bot;
				g = xmalloc(term->colalloc * sizeof(Glyph));
				memcpy(g, term->line[bot], term->colalloc * sizeof(Glyph));
				//tresize(term->col,term->row-1);
		} else if (ksym == -2) { // hide
				memcpy(term->line[bot], g, term->colalloc * sizeof(Glyph));
				//tresize(term->col,term->row+1);
		}

		if (type < 3) {
				char *z = lib[type];
				for (deb = &term->line[bot][col - 6], fin = &term->line[bot][col]; deb < fin;
								z++, deb++) {
						deb->mode = ATTR_REVERSE, deb->u = *z, deb->fg = defaultfg,
								deb->bg = defaultbg;
				}
		} else if (type < 5) {
				memcpy(term->line[bot], g, term->colalloc * sizeof(Glyph));
				//memcpy(term->line[bot], g, term->colalloc * sizeof(Glyph));
		} else {
				for (deb = &term->line[bot][0], fin = &term->line[bot][col]; deb < fin;
								deb++) {
						deb->mode = ATTR_REVERSE, deb->u = ' ', deb->fg = defaultfg,
								deb->bg = defaultbg;
				}
				term->line[bot][0].u = ksym;
		}

		term->dirty[bot] = 1;
		drawregion(0, bot, col, bot + 1);
}


void statusbar_kpress( KeySym *ks, char *buf ){

}


