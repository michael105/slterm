// Including all sources into a single file
// leaves more possibilities for optimizations to the compiler.
// Did try to split the huge term.c file,
// but there are many globals left.
// beware. it wasn't me.. ;misc 2021

//#define DEBUG_FILELEVEL 5
//#define FULLDEBUG 5
#include "debug.h"

#include "includes.h"
#include "globals.h"
//#include "arg.h"
#include "term.h"
#include "xwindow.h"
#include "xcursor.h"
#include "system.h"
#include "tty.h"
#include "selection.h"
#include "xclipboard.h"
#include "xevent.h"
#include "mouse.h"
#include "keyboard.h"
#include "fonts.h"
#include "statusbar.h"
#include "scroll.h"
#include "xdraw.h"
#include "termdraw.h"
#include "charmaps.h"
#include "colors.h"

// embedded resources
#ifdef INCLUDETERMINFO
#include "slterm_terminfo.h"
#endif
#ifdef INCLUDELICENSE
#include "slterm_license.h"
#endif

#ifdef INCLUDEMANPAGE
#include "slterm_man.h"
#endif


#include "globals.c"
#include "statusbar.c"
//#include "arg.c"
#include "term.c"
#include "controlchars.c"
#include "charmaps.c"
// termdraw needs to go after term.c .?
// otherweise delete and cursor keys do make trouble.
// Not looking for the source now.
#include "termdraw.c"

#include "fonts.c"
#include "mem.c"
#include "base64.c"
#include "selection.c"
#include "xclipboard.c"
#include "colors.c"
#include "xwindow.c"
#include "xcursor.c"
#include "xevent.c"
#include "mouse.c"
#include "keyboard.c"
#include "main.c"
#include "system.c"
#include "tty.c"
#include "utf8.c"
#include "scroll.c"
#include "xdraw.c"





