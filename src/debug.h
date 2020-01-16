#if 0
# /*
# make -f debug.h [target]
# targets below


check: 
		gcc -Wall -Wextra -DFULLDEBUG -DDEBUG_INCLUDESRC -std=c9x -fsyntax-only -Werror debug.h 

#(dump save flags)
flags:  
		@echo -ne '-DENABLEDEBUG -DDEBUG_INCLUDESRC' 

test: debug.h
		gcc -DENABLEDEBUG -DDEBUG_INCLUDESRC -DTEST_DEBUG -o testdebug -x c debug.h

testfile: debug.h
		gcc -DENABLEDEBUG -DDEBUG_INCLUDESRC -DTEST_DEBUG -DTEST_DEBUG_FILE -o testdebug -x c debug.h


all: debug.h.txt html

debug.h.txt: debug.h.3

README.rst: debug.h
		$(file > README.rst,$(README))

manpage: debug.h.3

debug.h.3: README.rst
		rst2man README.rst > debug.h.3

html: README.rst
	pandoc -f rst -t html -s README.rst > debug.html


define README =

=========
 debug.h
=========
 
debugging functions, colored, file logging


SYNOPSIS
========
 
 #define DEBUG 
 
 #define FULLDEBUG

 #define DEBUG_FILELEVEL [0..5]

 #define DEBUG_INCLUDESRC
 
 #include "debug.h"

**setdebugtarget(debugtarget,filename)**
	set the debug target to one of:

	- STDOUT
	- STDERR
	- TOFILE (filename)

**dbg[1..5]("message [fmt]",arg1,arg2,..)**:
  debug, when [1..5] < debug_filelevel

**dbgif[1..5]( if , "message [fmt]",arg1,arg2,..)**:
  debug, when "if" evaluates to true

**dbgf[1..5]("message [fmt]",arg1,arg2,..)**:
	debug with additional info: sourcefile, line

**warn("message [fmt]",arg1,arg2,..)**:
  Dump a warning, independent of debug switches

**warnif( if, "message [fmt]",arg1,arg2,..)**:
  Dump a warning, when "if" is true, independent of debug switches

**error("message [fmt]",arg1,arg2,..)**:
  Dump an error, independent of debug switches

**errorif( if, "message [fmt]",arg1,arg2,..)**:
  Dump an error, when "if" is true, independent of debug switches

**fatal("message [fmt]",arg1,arg2,..)**:
  Dump an error, independent of debug switches, and quit


OVERVIEW
========

Include debug.h into all sources, where you need debug macros.
In one (and only one) sourcefile define DEBUG_INCLUDESRC,
before the include.

debug (and dbg) macros can be enabled and disabled by #define DEBUG
globally.

The debugtarget can be set by the function setdebugtarget at runtime.
possible targets: STDERR, STDOUT, TOFILE

error and warning macros are not affected by the DEBUG switch.

dbg(..) calls will dump the message, 
		when debug_filelevel is not set, or set to 1..5

dbg[x] will log only, when debug_filelevel is set to any value > x


DESCRIPTION
===========
 

Hopefully uploading this to github prevents me of doing this for the X'th time,
again and again. (this file might date back to ~2003,2004.
And I was about to do this work today, again.)

The intention of this file is to have several debug macros, dbg, and dbg[2..5]
dbg messages will always be logged, except DEBUG is not defined or
debug_filelevel is set to 0
dbg2 and dbg messages will be logged, when debug is set to a value >= 2
dbg3, dbg2 and dbg messages will be logged, when debug is set to a value >= 3
.... until dbg5

When FULLDEBUG is defined, the filename and line is also dumped.

So spread dbg instructions wildly around, and simply disable them when you
are going to do a release. When bugs arise, you are able to ask for a recompile
with DEBUG enabled, and for the resulting log file.
This also means sort of transparency.
When releasing with the source code, 
the debug functions give much insight into the development process.
(I do not believe, revealing, where and why you did have to look for bugs
would show bad skills. Whoever has the skills to understand the debug
messages and debug functions spreading patterns, will most possibly
also be able to read your code. If not, it doesn't matter anyways)


My workflow is mostly related with debugging: separate a problem into as many different parts as possible,
and implement them part for part.
After every implementation of a part, check for the wished functionality, via debugging functions,
also with unexpected input.
Sometimes this can involve changing only a single source line,
and check after that.
This might seem like a slowly approach. 
But it isn't, in my experience. 
When fully in flow, I put out up to thousands of (tested) source lines
per day. The big advantage of this approach might also be the continuing
success feelings. Which keeps me flowing, in turn. Anyways.



Include this header into all sources, where you need debug macros.
In one (and only one) sourcefile define DEBUG_INCLUDESRC,
before the include.


debug (and dbg) macros can be enabled and disabled by #define DEBUG
globally.

The debugtarget can be set by the function setdebugtarget at runtime.
possible targets: STDERR, STDOUT, TOFILE

error and warning macros are not affected by the DEBUG switch.

The debuglevel (-1..5) can be set differently for every source file, 
either at runtime via dbg_filelevel = level,
and/or once by '#define DEBUG_FILEVEL', before the includement of debug.h
It's set to 5 per default, meaning, all dbg[1..5] calls will debug output.


LICENSE
=======

BSD License


AUTHOR
======

Michael (misc) Myer misc.myer at zoho.com


endef
ifdef UNDEF
*/
#endif


#ifndef debug_h
#define debug_h


#ifdef __cplusplus
extern "C" {
#endif

#ifndef bool
#define bool int
#define true 1
#define false 0
#endif

#if ( ENABLEDEBUG == NOCOLOR )
#define AC_NORM
#define AC_BLACK 
#define AC_RED 
#define AC_GREEN 
#define AC_BROWN 
#define AC_BLUE 
#define AC_MAGENTA 
#define AC_MARINE 
#define AC_LGREY 
#define AC_WHITE 

#define AC_GREY 
#define AC_LRED 
#define AC_LGREEN
#define AC_YELLOW 
#define AC_LBLUE 
#define AC_LMAGENTA 
#define AC_LMARINE 
#define AC_LWHITE 
#else
// ansicolor escape sequences
#define AC_NORM      "\033[0m"
#define AC_BOLD      "\033[1m"
#define AC_FAINT     "\033[2m"
#define AC_CURSIV    "\033[3m"
#define AC_UNDERLINE "\033[4m"
#define AC_BLINK     "\033[5m"
#define AC_INVERSE   "\033[7m"
#define AC_BLACK     "\033[0;30m"
#define AC_RED       "\033[0;31m"
#define AC_GREEN     "\033[0;32m"
#define AC_BROWN     "\033[0;33m"
#define AC_BLUE      "\033[0;34m"
#define AC_MAGENTA   "\033[0;35m"
#define AC_MARINE    "\033[0;36m"
#define AC_LGREY     "\033[0;37m"
#define AC_WHITE     "\033[0;38m"

#define AC_GREY      "\033[1;30m" 
#define AC_LRED      "\033[1;31m" 
#define AC_LGREEN    "\033[1;32m" 
#define AC_YELLOW    "\033[1;33m"
#define AC_LBLUE     "\033[1;34m"
#define AC_LMAGENTA  "\033[1;35m"
#define AC_LMARINE   "\033[1;36m"
#define AC_LWHITE    "\033[1;37m"

#ifdef SHORTCOLORNAMES

#define NORM     AC_NORM
#define INVERSE  AC_INVERSE
#define BLINK    AC_BLINK
#define BLACK    AC_BLACK
#define RED      AC_RED
#define GREEN    AC_GREEN
#define BROWN    AC_BROWN
#define BLUE     AC_BLUE
#define MAGENTA  AC_MAGENTA
#define MARINE   AC_MARINE
#define LGREY    AC_LGREY
#define WHITE    AC_WHITE
#define GREY     AC_GREY
#define LRED     AC_LRED
#define LGREEN   AC_LGREEN
#define YELLOW   AC_YELLOW
#define LBLUE    AC_LBLUE
#define LMAGENTA AC_LMAGENTA
#define LMARINE  AC_LMARINE
#define LWHITE   AC_LWHITE

#endif

#ifdef SHRTSHORTCOLORNAMES

#define cN   AC_NORM
#define cBL  AC_BLACK
#define cR   AC_RED
#define cG   AC_GREEN
#define cBR  AC_BROWN
#define cB   AC_BLUE
#define cMG  AC_MAGENTA
#define cM   AC_MARINE
#define cLG  AC_LGREY
#define cW   AC_WHITE
#define cG   AC_GREY
#define cLR  AC_LRED
#define cLGN AC_LGREEN
#define cY   AC_YELLOW
#define cLB  AC_LBLUE
#define cLMG AC_LMAGENTA
#define cLM  AC_LMARINE
#define cLW  AC_LWHITE

#endif


#endif

#ifdef FULLDEBUG
#ifdef ENABLEDEBUG
#undef ENABLEDEBUG
#endif
// set to 5 - everything
#define ENABLEDEBUG 5
#endif




		// where the debug should go
		enum _debugtarget { STDERR, STDOUT, TOFILE };

		bool _setdebugtarget( enum _debugtarget t, const char *file );

		// Set a target for debug (defaults to stderr)
		// returns false, if the stream could not be opened
#define setdebugtarget(debugtarget,filename) _setdebugtarget( debugtarget, filename )



		// Error and warning severity
		//enum _severity { DEBUG, MINOR, UNUSUAL, WARNING, SEVERE, FATAL};
		enum _severity { FATAL, SEVERE, WARNING, UNUSUAL, MINOR, DEBUG };
#ifndef DEBUG_FILELEVEL
		// local file debug level (everything by default );
		static int dbg_filelevel = DEBUG;
#else 
		static int dbg_filelevel = DEBUG_FILELEVEL;
#endif


		void _warning(enum _severity sev, const char* file, const int line, const char* function, const char* fmt, ... );
		void _error( enum _severity sev, const char* file, const int line, const char* function, const char* fmt, ... );
		void _dbg_full( int level, int filelevel, const char* file, const int line, const char* function, const char* fmt, ... );
		void _dbg( int level, int filelevel, const char* fmt, ... );

#define warning(severity,...) _warning( severity, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )
#define warn(...) _warning( WARNING, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )
#define warnif(when,...) {if ( when ) warn(__VA_ARGS__);}

#define error(...) _error( SEVERE, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )
#define fatal(...) _error( FATAL, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )
#define errorif(when,...) {if ( when ) warning(__VA_ARGS__)}
#define fatalif(when,...) {if ( when ) fatal(__VA_ARGS__)}


#ifdef ENABLEDEBUG 
#if ( ENABLEDEBUG>0 )

#define _DBG_FULL_(level,...) _dbg_full( level,  dbg_filelevel,  __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )

#define dbgf(...)  _DBG_FULL_( 0,  __VA_ARGS__ )
#define dbgf1(...) _DBG_FULL_( 1,  __VA_ARGS__ )
#define dbgf2(...) _DBG_FULL_( 2,  __VA_ARGS__ )
#define dbgf3(...) _DBG_FULL_( 3,  __VA_ARGS__ )
#define dbgf4(...) _DBG_FULL_( 4,  __VA_ARGS__ )
#define dbgf5(...) _DBG_FULL_( 5,  __VA_ARGS__ )

#define dbgi(arg) _dbg(1, dbg_filelevel, "DBG: L.%d, %s, int %s:   %d", __LINE__, __FILE__, #arg , arg )

#ifdef FULLDEBUG
#define dbg(...)  _DBG_FULL_( 0, __VA_ARGS__ )
#define dbg1(...) _DBG_FULL_( 1, __VA_ARGS__ )
#define dbg2(...) _DBG_FULL_( 2, __VA_ARGS__ )
#define dbg3(...) _DBG_FULL_( 3, __VA_ARGS__ )
#define dbg4(...) _DBG_FULL_( 4, __VA_ARGS__ )
#define dbg5(...) _DBG_FULL_( 5, __VA_ARGS__ )
#define dbgif(when,...) {if ( when ) dbg(__VA_ARGS__)}
#else
#define dbg(...)  _dbg( 0, dbg_filelevel, __VA_ARGS__ )
#define dbg1(...) _dbg( 1, dbg_filelevel, __VA_ARGS__ )
#define dbg2(...) _dbg( 2, dbg_filelevel, __VA_ARGS__ )
#define dbg3(...) _dbg( 3, dbg_filelevel, __VA_ARGS__ )
#define dbg4(...) _dbg( 4, dbg_filelevel, __VA_ARGS__ )
#define dbg5(...) _dbg( 5, dbg_filelevel, __VA_ARGS__ )
#define dbgif(when,...) {if ( when ) dbg(__VA_ARGS__)}
#endif //FULLDEBUG

#else //ENABLEDEBUG not defined
#define dbg(...) {}
#define dbgif(...) {}
#define dbg1(...) {}
#define dbg2(...) {}
#define dbg3(...) {}
#define dbg4(...) {}
#define dbg5(...) {}
#define dbgf(...) {}
#define dbgf1(...) {}
#define dbgf2(...) {}
#define dbgf3(...) {}
#define dbgf4(...) {}
#define dbgf5(...) {}

#define dbgi(_int) {}


#endif //enabledebug>0
#endif //ENABLEDEBUG




#ifdef __cplusplus
}
#endif


#ifdef DEBUG_INCLUDESRC

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

// Implementation

static FILE* target;

static void init(){
		if ( !target )
				target = stderr;
}

bool _setdebugtarget( enum _debugtarget t , const char * file)
{
		switch(t){
				case STDOUT: target = stdout;
										 break;

				case TOFILE: target = fopen(file,"w");
										 if ( ferror( target ) ){
												 target = stderr;
												 warn("Couldn't open debug target: %s", file);
												 return(false);
										 }
										 break;
				default: 
										 target = stderr;
										 break;

		}

		return(true);
}



void _warning( enum _severity sev, const char* file, const int line, const char* function, const char*fmt, ... ){
		init();
		fprintf(target, AC_BROWN "WARNING:" AC_NORM " %s  line %d, in %s\n   ", file, line, function);
		va_list ap;
		va_start(ap,fmt);
		vfprintf(target, fmt, ap );
		fprintf(target,"\n");
		va_end(ap);
		fflush(target);
		if (sev==FATAL){
				exit(1);
		}
}
void _error( enum _severity sev, const char* file, const int line, const char* function, const char*fmt, ... ){
		init();
		fprintf(target, AC_RED "ERROR:"AC_NORM" %s  line %d, in %s    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n", file, line, function);
		va_list ap;
		va_start(ap,fmt);
		vfprintf(target, fmt, ap );
		fprintf(target,"\n");
		va_end(ap);
		fflush(target);
		if (sev==FATAL){
				fprintf(target, AC_LRED "FATAL"AC_NORM"\n");
				if ( !( target == stderr || target == stdout ) ) // TOFILE
						fprintf( stderr, AC_LRED "FATAL error in %s line %d\n"AC_NORM"Quit\n", file, line );
				fflush(target);
				fflush( STDERR );
				exit(1);
		}
}
void _dbg_full( int level, int filelevel, const char* file, const int line, const char* function, const char*fmt, ... ){
		if ( level < filelevel )
				return;
		init();
		fprintf(target, "   %s   %d, in %s\n", file, line, function );
		va_list ap;
		va_start(ap,fmt);
		vfprintf(target, fmt,  ap );
		fprintf(target,"\n");
		va_end(ap);
		fflush(target);
}
void _dbg( int level, int filelevel, const char*fmt, ... ){
		if ( level < filelevel )
				return;
		init();
		va_list ap;
		va_start(ap,fmt);
		vfprintf(target, fmt,  ap );
		fprintf(target,"\n");
		va_end(ap);
		fflush(target);
}




#endif //DEBUG_INCLUDESRC

#endif //IFNDEF debug_h

#ifdef TEST_DEBUG

// testing, and also serves as example

int main(){
#ifdef TEST_DEBUG_FILE
		setdebugtarget( TOFILE, "test.log" );
#endif

		int a = 3;
		dbg( "Checking debug functions." );
		dbg5( "dbg5" );
		dbg2( "dbg2, a value: %d", 42 );

		dbg(""); // Empty line

		dbg_filelevel = 2;
		dbg3(" Shouldn't show up" );

		dbg("this is higher than the debuglevel");
		dbg2("level 2, too");

		dbg5("this not."); // not
		
		dbgf2(" A \"full\" debug message by dbgf2");

		dbg(""); // Empty line
		warnif( a>0, "a gt 0 ");
		warnif( a>3, "a gt 3 ");

		dbg(""); // Empty line
		error( "Raising an error" );

		fatal( "Good bye" ); // Quits

		dbg("This shouldn't show up");

		return(0);
}

#endif //testdebug

#if 0
endif
#endif
