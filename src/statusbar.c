
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
	drawregion(0, term->bot, term->cols, term->bot + 1);
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

// text entering, textfield
// besser als struct, objektorientiert.
uchar tfbuf[512];
int tfbuflen = 512;

int tfpos = 0; // is not necessarily the pos of the cursor.
int tftextlen = 0;
int tfvisible = 0;

uchar tfinput[512];
int tfinputlen = 512;

// callbacks
//
// xkeyboard -> keypress
// verarbeitet input, auch keycombos
//
// tfevent -> receiver (searchwidget)
// sendet:
// textentered (buf)
// search (buf)
// complete (buf)
//
// setcursor(x) -> relative cursor position
//
// redraw() -> receiver (statusbar)
// ->redraw() beide richtungen. d.h. in statusbarupdate: setwidth(), paint(), getcursor()
//
// ->setwidth(x)
//
// ->view,hide
// ->setfocus()
//
//
// eventfilterlist. (Ctrl+F,Tab,ESC, . hm. evtl zu weit.)
// benoetige ein sinnvolles format fuewr die glyphen.
// vermutlich am besten "doppelt". textstring, und glyphlist fuer farbe und attribute.
//
// auch moeglich: die callbacks von sprintf in minilib fuer colorierung schreiben.
// ->sprintf: marker fuer farbe. zb ein Test in %ROT%Farbe
// hatte ja schonmal ein konzept. 
//
// sinnvoll, hier, hab keine zeit: variablen und callbacks als struktur.
//
// Rest koennte ich immer noch aufbauen. bspw sender- verteiler -receiver
// usw.
// 
// besser: slots.
// in der strukt slot registerkey[], drin struct: key,callback(key,sender)
// -> structs muessen fuer alle widgets eine selbe grundstruktur am anfang haben.
// -> = "parent" class.
// -> sogar die syntax wird nett: struct _textinput { struct widget; ...
// laesst sich dann auch polymorph gestalten.
// create_widget( typ )... setzt callbacks, initialisiert.
//
// kann natuerlich auch macros machen:
// struct widget; wird zu PARENTCLASS widget;
// "public" scheint unpassend.
// genaugenommen: struct _textinput{ CLASS widget; 
// und struct _widget{ CLASS object; 
// ...
// nicht mal casten notwendig.
// nur upcasten, von object auf widget, usw.
//
// kann ich natuerlich auch gleich mit ids arbeiten.
// fuer jede instanz, und jede klasse.
//



// for the mode MODE_ENTERSTRING
void statusbar_kpress( KeySym *ks, char *buf ){

}




// updates the statusbar with current line, etc., when visible.
void updatestatus(){

	if ( statusvisible ){

		// currently shown number of cols
		int stwidth = statuswidth;
		if ( term->cols != statuswidth )
			stwidth = term->cols;

		char buf[512];
		memset( buf, ' ', 256 );
		//bzero(buf+256,256);

		//int p = sprintf(buf,"  %s  %5d-%2d %5d %5d %3d%% (%3d%%)   RM:%3d", p_status,
		int p = 0;

		int fstwidth = stwidth;

		stwidth -= strlen(p_status)+3; // try to keep that much free space at the left

		// scrollinfo
		if ( stwidth > 32 + 11 ){
			p = sprintf(buf+256,"%5d-%2d %5d %5d %3d%% (%3d%%)   RM:%3d ",
					term->histi-term->scr,term->histi-term->scr+term->rows, 
					term->histi+term->rows, term->histi+term->rows-(term->histi-term->scr+term->rows),
					((term->histi-term->scr)*100)/((term->histi)?term->histi:1),
					((term->histi-term->scr-term->scrollmarks[0]+1)*100)/((term->histi-term->scrollmarks[0]+1)?term->histi-term->scrollmarks[0]+1:1),
					term->scrolled_retmark
					);

			if ( stwidth > p+10 ){
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

		} else if ( stwidth > 20 + 6 ){ //TODO: other values (line number)
			p = sprintf(buf+256,"%5d %5d %3d%%   RM:%3d ",
					term->histi+term->rows, term->histi+term->rows-(term->histi-term->scr+term->rows),
					((term->histi-term->scr)*100)/((term->histi)?term->histi:1),
					term->scrolled_retmark );
		} else {
			p = sprintf(buf+256,"%5d %3d%% ",
					term->histi+term->rows-(term->histi-term->scr+term->rows),
					((term->histi-term->scr)*100)/((term->histi)?term->histi:1) );
		}

		//printf("p: %d\n",p);
		buf[p] = 0;

		int bp = 256 - fstwidth + p;
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
	int stwidth = statuswidth;
	if ( term->cols != stwidth )
		stwidth = term->cols;

#ifndef UTF8
	Glyph g = { .fg = statusfg, .bg = statusbg, .mode = statusattr, .u = ' ' };
#endif

	for (deb = statusbar,fin=&statusbar[stwidth]; (deb < fin);	deb++) {
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
			drawregion(0, term->bot, term->cols, term->bot + 1);
			//term->rows++;
			//tresize(term->cols,term->rows+1);
		}
	}
}

// shows a small message at the bottom right corner
void set_notifmode(int type, KeySym ksym) {
	static char *lib[] = {" MOVE ", "SELECT"," LESS " };
	static Glyph *g, *deb, *fin;
	static int col, bot;
	col = term->cols, bot = term->bot;

	if (ksym == -1) { // show
		free(g);
		g = xmalloc(term->colalloc * sizeof(Glyph));
		memcpy(g, TLINE(bot), term->colalloc * sizeof(Glyph));
		//tresize(term->cols,term->rows-1);
	} else if (ksym == -2) { // hide
		memcpy(TLINE(bot), g, term->colalloc * sizeof(Glyph));
		//tresize(term->cols,term->rows+1);
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

