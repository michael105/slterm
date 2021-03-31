
#include "statusbar.h"
#include "st.h"
#include "mem.h"


int statusvisible;
Glyph *statusbar;
char* p_status = NULL;


void drawstatus(){
		term->dirty[term->bot] = 1;
		drawregion(0, term->bot, term->col, term->bot + 1);
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

				setstatus(buf);
		}
}

void setstatus(char* status){
		static Glyph *deb, *fin;
		static int col, bot;

		free(statusbar);
		col = term->col, bot = term->bot;
		statusbar = xmalloc(term->colalloc * sizeof(Glyph));
		char *z = status;

		for (deb = statusbar,fin=&statusbar[col]; (deb < fin) && (*status);
						status++, deb++) {
				deb->mode = ATTR_REVERSE, deb->u = *status, deb->fg = defaultfg,
						deb->bg = defaultbg;
		}

		for (; (deb < fin); deb++) {
				deb->mode = ATTR_REVERSE, deb->u = ' ', deb->fg = defaultfg,
						deb->bg = defaultbg;
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


