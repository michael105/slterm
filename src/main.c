// function main
// option parsing, config init.
//
//

#include "xwindow.h"
#include "xevent.h"
#include "includes.h"
#include "scroll.h"
#include "selection.h"
#include "system.h"
#include "compile.h"
#include "config.h"

#include <sys/mman.h>
#include <error.h>


#ifdef XRESOURCES
int resource_load(XrmDatabase db, char *name, enum resource_type rtype,
		void *dst) {
	char **sdst = dst;
	int *idst = dst;
	float *fdst = dst;

	char fullname[256];
	char fullclass[256];
	char *type;
	XrmValue ret;

	snprintf(fullname, sizeof(fullname), "%s.%s", opt_name ? opt_name : "st",
			name);
	snprintf(fullclass, sizeof(fullclass), "%s.%s", opt_class ? opt_class : "St",
			name);
	fullname[sizeof(fullname) - 1] = fullclass[sizeof(fullclass) - 1] = '\0';

	XrmGetResource(db, fullname, fullclass, &type, &ret);
	if (ret.addr == NULL || strncmp("String", type, 64))
		return 1;

	switch (rtype) {
		case STRING:
			*sdst = ret.addr;
			break;
		case INTEGER:
			*idst = strtoul(ret.addr, NULL, 10);
			break;
		case FLOAT:
			*fdst = strtof(ret.addr, NULL);
			break;
	}
	return 0;
}



void config_init(void) {
	char *resm;
	XrmDatabase db;
	ResourcePref *p;

	XrmInitialize();
	resm = XResourceManagerString(xwin.dpy);
	if (!resm)
		return;

	db = XrmGetStringDatabase(resm);
	for (p = resources; p < resources + LEN(resources); p++)
		resource_load(db, p->name, p->type, p->dst);
}
#endif

void printversion(){
	printf( "slterm version " VERSION "\n" );
}

void printhelp(){
}

void usage(void) {
	printversion();
	die(	"\nusage:\n\n"
			" slterm -H: show help\n"
		  	"        -I: dump terminfo file\n"
			"        -L display license\n\n"
			" slterm [-aiv] [-c class] [-f font] [-g geometry]"
			" [-n name] [-o file]\n"
			"        [-T title] [-t title] [-w windowid]"
			" [[-e] command [args ...]]\n\n"
			" slterm [-aiv] [-c class] [-f font] [-g geometry]"
			" [-n name] [-o file]\n"
			"[-x] [-v] [-V] [-X]\n"
			"        [-T title] [-t title] [-w windowid] -l line"
			" [stty_args ...]\n\n",
			argv0, argv0);
}


void missingfontname( const char* option ){
	fprintf(stderr, "Missing font name for option %s\n"
			"Usage: slterm [-f fontname] [-fb boldname] [-fi italicname] [-fI bolditalicname] [other options]\n",
			option );
	fprintf(stderr, "\nThe fontname format is specified in the fontconfig documentation,\n"
			"http://freedesktop.org/software/fontconfig/fontconfig-user.html\n"
			"A list of attributes is in doc/fontconfig.txt.\n"
			"Supply 0 to disable bold, italic or bolditalic fonts,\n"
			"using colors only for the text rendering of the different attributes.\n"
			"\n");
	exit(1);
}


#ifdef shared
// share the whole text segment, including main
int shared_main(int argc, char *argv[]) {
		iofd = 1; // 
#else
int main(int argc, char *argv[]) {
#endif
	xwin.l = xwin.t = 0;
	xwin.isfixed = False;
	twin.cursor = cursorshape;

#define EARGF(_unneeded)  ({ if ( ! *++argv ){ fprintf(stderr, "missing option for %s\n", argv[-1] ); usage(); }; *argv; }) 


#define ARGFONT(_type) ({ if ( ! *++argv ) missingfontname( argv[-1] ); \
		if ( **argv == '0' ){ _type##_font = 0; use##_type##font=0; } \
		else { _type##_font = *argv; use##_type##font=1; } \
		*argv; })

	int useregularfont; // dummy

	while ( *++argv && argv[0][0] == '-'  ){
		for ( char *opt = *argv+1; *opt; opt++ ){
			switch ( *opt ){
#ifdef INCLUDETERMINFO
				case 'I': // dump terminfo
					write(STDOUT_FILENO, slterm_terminfo, strlen(slterm_terminfo) );
					exit(0);
#endif
#ifdef INCLUDELICENSE
				case 'L':
					write(STDOUT_FILENO, slterm_license, strlen(slterm_license) );
					exit(0);
#endif
#ifdef INCLUDEMANPAGE
				case 'H':
					write(STDOUT_FILENO, slterm_man, strlen(slterm_man) );
					exit(0);
#endif
				case 'X':
					if ( mlockall(MCL_CURRENT|MCL_FUTURE) ){
						perror("Unable to lock memory pages into memory");
						exit(errno);
					}
					break;
				case 'a':
					allowaltscreen = 0;
					break;
				case 'c':
					opt_class = EARGF(usage());
					break;
				case 'e':
					//if (argc > 0)
					//	--argc, ++argv;
					*argv++;
					goto run;
				case 'f':
					switch ( *++opt ){
						case 'R':
						case 'b': ARGFONT(bold); break;
						case 'i': ARGFONT(italic); break;
						case 'I': ARGFONT(bolditalic); break;

						case 'r':
						default:
						opt_font = ARGFONT(regular);
					}
					break;
				case 'g':
					xwin.gm = XParseGeometry(EARGF(usage()), &xwin.l, &xwin.t, &cols, &rows);
					break;
				case 'i':
					xwin.isfixed = 1;
					break;
				case 'o':
					opt_io = EARGF(usage());
					break;
				case 'l':
					opt_line = EARGF(usage());
					break;
				case 'n':
					opt_name = EARGF(usage());
					break;
				case 't':
				case 'T':
					opt_title = EARGF(usage());
					break;
				case 'w':
					opt_embed = EARGF(usage());
					break;
				case 'x':
					opt_xresources = 1;
					break;
				case 'v':
					printversion();
					exit(0);
				case 'V':
					printversion();
					printf( "\nCompiled " __COMPILEDATE__ "\n"
							__UNAME__ "\n"
							"CC: "__CC__" "__CC_VERSION__"\n\n"
							"Compileflags:\n"
							__OPT_FLAG__ "\n"
							"HISTORY: %d\n"
							"DEBUGLEVEL: "__ENABLEDEBUG__"\n"
							"XRESOURCES: "__XRESOURCES__"\n"
							"UTF8: "__UTF8__"\n"
							__COMPILECOMMAND__ "\n", ( 1<<HISTSIZEBITS) );
					exit(0);
				default:
					usage();
			}
		}
	}

run:
	//if (argc > 0) /* eat all remaining arguments */
	if ( *argv )
		opt_cmd = argv;

	if (!opt_title)
		opt_title = (opt_line || !opt_cmd) ? "slterm" : opt_cmd[0];

	//setlocale(LC_CTYPE, "");
	XSetLocaleModifiers("");

#ifdef XRESOURCES
	if (!(xwin.dpy = XOpenDisplay(NULL)))
		die("Can't open display\n");

	if (opt_xresources)
		config_init();
#endif

	cols = MAX(cols, 1);
	rows = MAX(rows, 1);
	tnew(cols, rows);
	xinit(cols, rows);
	xsetenv();
	selinit();

	create_unicode_table(); // unicode to current cp table
	sort_shortcuts();	

	run();

	return 0;
}
