// function main
// option parsing, config init.
//
//

#include "x.h"
#include "xevent.h"
#include "includes.h"
#include "scroll.h"
#include "selection.h"
#include "system.h"
#include "arg.h"
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
	resm = XResourceManagerString(xw.dpy);
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
	die("\nusage: %s [-aiv] [-c class] [-f font] [-g geometry]"
			" [-n name] [-o file]\n"
			"          [-T title] [-t title] [-w windowid]"
			" [[-e] command [args ...]]\n"
			"       %s [-aiv] [-c class] [-f font] [-g geometry]"
			" [-n name] [-o file]\n"
			"          [-T title] [-t title] [-w windowid] -l line"
			" [stty_args ...] [-x] [-v] [-V] [-X]\n",
			argv0, argv0);
}



#ifdef shared
// share the whole text segment, including main
int shared_main(int argc, char *argv[]) {
#else
int main(int argc, char *argv[]) {
#endif
		xw.l = xw.t = 0;
		xw.isfixed = False;
		win.cursor = cursorshape;
		iofd = 1;

		ARGBEGIN {
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
				if (argc > 0)
					--argc, ++argv;
				goto run;
			case 'f':
				opt_font = EARGF(usage());
				break;
			case 'g':
				xw.gm = XParseGeometry(EARGF(usage()), &xw.l, &xw.t, &cols, &rows);
				break;
			case 'i':
				xw.isfixed = 1;
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
		ARGEND;

run:
		if (argc > 0) /* eat all remaining arguments */
			opt_cmd = argv;

		if (!opt_title)
			opt_title = (opt_line || !opt_cmd) ? "st" : opt_cmd[0];

		//setlocale(LC_CTYPE, "");
		XSetLocaleModifiers("");

#ifdef XRESOURCES
		if (!(xw.dpy = XOpenDisplay(NULL)))
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
