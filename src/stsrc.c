// Including all sources into a single file
// leaves more possibilities for optimizations to the compiler.
// Did try to split the huge st.c file,
// but there are many globals left.
// beware. it wasn't me.. ;misc 2021

//#define DEBUG_FILELEVEL 5
//#define FULLDEBUG 5
#include "debug.h"



#include "includes.h"

#include "st.h"
#include "x.h"
#include "system.h"
#include "tty.h"
#include "selection.h"
#include "xevent.h"
#include "fonts.h"
#include "statusbar.h"
#include "scroll.h"
#include "xdraw.h"
#include "termdraw.h"


#include "statusbar.c"
#include "st.c"
#include "fonts.c"
#include "mem.c"
#include "base64.c"
#include "selection.c"
#include "x.c"
#include "xevent.c"
#include "main.c"
#include "system.c"
#include "tty.c"
#include "utf8.c"
#include "scroll.c"
#include "xdraw.c"
#include "termdraw.c"





