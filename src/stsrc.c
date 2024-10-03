// Including all sources into a single file
// leaves more possibilities for optimizations to the compiler.
// Did try to split the huge st.c file,
// but there are many globals left.
// beware. it wasn't me.. ;misc 2021

//#define DEBUG_FILELEVEL 5
//#define FULLDEBUG 5
#include "debug.h"



#include "includes.h"
#include "arg.h"
#include "st.h"
#include "x.h"
#include "system.h"
#include "tty.h"
#include "selection.h"
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


#include "statusbar.c"
#include "arg.c"
#include "st.c"
#include "charmaps.c"
// termdraw needs to go after st.c .?
// otherweise delete and cursor keys do make trouble.
// Not looking for the source now.
#include "termdraw.c"

#include "fonts.c"
#include "mem.c"
#include "base64.c"
#include "selection.c"
#include "colors.c"
#include "x.c"
#include "xevent.c"
#include "mouse.c"
#include "keyboard.c"
#include "main.c"
#include "system.c"
#include "tty.c"
#include "utf8.c"
#include "scroll.c"
#include "xdraw.c"





