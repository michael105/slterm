// Main configuration file

#ifndef CONFIG_H
#define CONFIG_H

#ifndef extract_keyref

// Silence my syntaxcheckerplugin
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

// Config options

/*
 font: see http://freedesktop.org/software/fontconfig/fontconfig-user.html
       and doc/fontconfig.txt

 the default regular font can be overwritten via command line 
 e.g.: slterm -f 'Monospace:pixelsize=13:antialias=true'

 To use embedded fonts, set EMBEDFONT to 1 in config.make,
 The attributes below are still used.
 To change the embedded fonts, you need to change the files in src/embed/

*/

// regular font
static char *regular_font = "Liberation Mono:Bold:pixelsize=13:antialias=true:autohint=true";
//static char *font = "Monospace:pixelsize=13:antialias=true:autohint=true";

//if size not set 
int default_font_pixelsize = 14;

/* if fonts below are set, they are used, no matter of xresources or command line options
 else, if set to 0, the appropiate weight and slant are added to "regular_font" 
 (italic,bold,bold italic) when usexxxfont below is set to 1.
The pixelsize as well as charwidth and height are set according to the regular font,
pixelsize (if unset) defaults to 14.
*/
static char *italic_font = "Liberation Mono:Bold:Italic:pixelsize=13:antialias=true:autohint=true";
static char *bold_font = 0;
static char *bolditalic_font = 0;

// Whether to use another font, for bold/italic/bolditalic
#if 1
int useboldfont = 0;
int useitalicfont = 1;
int usebolditalicfont = 0;
#else
int useboldfont = 1;
int useitalicfont = 1;
int usebolditalicfont = 1;
#endif



// more/less font width spacing
// here, with utf8 enabled, -1 looks much better.
int fontspacing = 0;
//int fontspacing = -1;


//static int borderpx = 4;

// The terminal's borderwidth in percent of a char's width
int borderperc = 40;

/*
 What program is execed by slterm depends of these precedence rules:
 1: program passed with -e
 2: utmp option
 3: SHELL environment variable
 4: value of shell in /etc/passwd
 5: value of shell in config.h
 */
static char *shell = "/bin/sh";
char *utmp = NULL;
char *stty_args = "stty raw pass8 nl -echo -iexten -cstopb 38400";

/* identification sequence returned in DA and DECID */
char *vtiden = "\033[?6c";

/* Kerning / character bounding-box multipliers */
static float cwscale = 1.0;
static float chscale = 1.0;

/*
 word delimiter string
 More advanced example: L" `'\"()[]{}"
 */
wchar_t *worddelimiters = L" ";

/* selection timeouts (in milliseconds) */
static unsigned int doubleclicktimeout = 300;
static unsigned int tripleclicktimeout = 600;

/* alt screens */
int allowaltscreen = 1;

/* frames per second slterm should at maximum draw to the screen */
/* set to log(framerate) ->   log(128) = 7; log(64) = 6; log(32) = 5 */
#define xfps_shift 7
#define actionfps_shift 5

/*
 blinking timeout (set to 0 to disable blinking) for the terminal blinking
 attribute.
 */
static unsigned int blinktimeout = 800;

/* thickness of underline and bar cursors */
static unsigned int cursorthickness = 2;

/* bell volume. It must be a value between -100 and 100. 0 to disable */
static int bellvolume = 50;

/* 
 default TERM value 
 used also for the identification of the capabilities to curses
 this can be overriden in the shell by exporting a known terminal name,
 e.g.: export TERM=xterm-256color
 To make slterm known to curses et al, the terminfo capability file
 (tic) needs to be installed: tix -sx slterm.terminfo or
 slterm -I | tic -sx -
 */
char *termname = "slterm-256color";

/*
 spaces per tab

 When you are changing this value, don't forget to adapt the 'it' value in
 the slterm.terminfo and appropriately install the slterm.terminfo in the environment where
 you use this slterm version.

	it#$tabspaces,

 Secondly make sure your kernel is not expanding tabs. When running `stty
 -a` tab0 should appear. You can tell the terminal to not expand tabs by
 running following command:
	stty tabs
 */
unsigned int tabspaces = 8;


/* 
 The table of the first 8 (32) colors, numbered 30..37
 Names are defined by xorg, and (should be) conformant with css names.
 a table of colornames is in doc/colornames.html, and colornames_gray.html.
 RGB in hexadecimal (#RRGGBB) is also possible.
 The table is oriented at the de facto xterm standard.


 each color is: "normal", "bold", "faint", "bold+faint"
 They are used via the ansi sequences, e.g "\033[31m" for red. 
 > echo -e "\e[33;1m Text" shows text in red (bold)
 > echo -e "\e[33;1;2m Text" shows text in red (bold_faint)
 (30-37 is foreground,40-47 background color, according to 0..7)
 foregroundcolor 8-15 (90..97) gets color 0-7, bold.

 colors 16 - 255 are colors used from xterm and calculated in colors.c/xdraw.c
  the algorithm to display faint and bold_faint colors is slightly modified
  for better contrast.
 (todo: use colors 8-15 to switch to betacode (-> switch via vi syntax colorizing ))
*/
static const char* colortablenames[8][4] = { 
#define GRADIENT(_normal,_bold,_faint,_bold_faint) { #_normal, #_bold, #_faint, #_bold_faint }

//	   0m          	1m    		2m           	1;2m	
//	   normal      	bold      	faint       	bold_faint

	{ "black",     	"gray50", 	"gray11",   	"darkslategray" },
	{ "red3",      	"red",   	"darkred",  	"orangered" },
	{ "green3", 		"green", 	"darkgreen",	"PaleGreen" },
	//{ "green3", 		"green", 	"darkgreen",	"olive" },
	{ "saddlebrown",	"yellow",	"#531818",  	"orange" },
	//{ "saddlebrown",	"yellow",	"#531818",  	"chocolate" },
	{ "blue2",     	"#5050ff", 	"darkblue", 	"#00aaea" }, //"deepskyblue" },
	{ "magenta3",  	"magenta", 	"darkmagenta",	"blueviolet" },
	//{ "cyan3",     	"cyan", 		"#321680", 	"aquamarine" },
	{ "cyan3",     	"cyan", 		"darkcyan", 	"aquamarine" },
	{ "gray90",    	"white", 	"darkkhaki", 	"SlateGray3"  } 
	// The default text color is "gray90" on "black"

};

// background colors (40..47, and 100..107 )
static const char* bgcolornames[16] = { 

	"black", "DarkRed", "Green4", "saddlebrown", 
	//"black", "OrangeRed", "Green4", "saddlebrown", 
	"blue4" , "magenta4", "#321680", "silver",
	//"blue4" , "magenta4", "turquoise4", "silver",

	"darkslategray", "red", "green", "yellow", 
	"blue", "magenta", "cyan", "white"

};

    //"brown3", // sort of brown (orange). brown is "brown" and faint..
	//"#592a1d",
	 //"#532020", // brown
	 //"#531818", // brown
	 //"#562215", // brown
	
// Outdated. the current table is colortablenames.
// dgreen 036209
// lgreen 00ff10
// dblue 070060
// dfblue 3c33aa
static const char *colorname[] = {
    /* 8 normal colors */
    "black", "red3", "green3", //"brown3", // sort of brown (orange). brown is "brown" and faint..
	"#592a1d", //"#532020", // brown //"#531818", // brown //"#562215", // brown //"yellow3",
    "blue2", "magenta3", "cyan3", "gray90",
    /* 8 bright colors */
    "gray50", "red", "green", "yellow",
    //"#5050ff", // light blue
	 "#00aaea", // schweinchenblau
    //"#5c5cff",
    "magenta", "cyan", "white",
    [255] = 0,
    /* more colors can be added after 255 to use with DefaultXX */
    "#cccccc", "#2e3440",
};


/*
 Default colors (colorname index)
 foreground, background, cursor, reverse cursor
*/
unsigned char defaultfg = 7;
unsigned char defaultbg = 0;
static unsigned char defaultcs = 15;
//static unsigned char defaultrcs = 202;
// Unfocused window
static unsigned char unfocusedrcs = 46; //118;//226; 

// colors and attribute for the statusbar
unsigned char statusfg = 75; //21;
unsigned char statusbg = 21; //82;
unsigned char statusattr = ATTR_BOLD; // ATTR_REVERSE

/* Colors used for selection */
unsigned int selectionbg = 19;
//unsigned int selectionbg = 257;
unsigned int selectionfg = 7;
/* If 0 use selectionfg as foreground in order to have a uniform foreground-color */
/* Else if 1 keep original foreground-color of each cell => more colors :) */
//static int ignoreselfg = 1;

/*
 Default shape of cursor
 2: Block 
 4: Underline ("_")
 6: Bar ("|")
 7: Block, and (X)
 8: double underline
 9: empty block
 10: underline with sides
 11: under- and overline with sides
 12: overline with sides
*/
unsigned int cursorshape = 4;

/* Default columns and rows numbers */
static unsigned int cols = 80;
static unsigned int rows = 24;

/* Default colour and shape of the mouse cursor */
static unsigned int mouseshape = XC_xterm;
static unsigned int mousefg = 7;
static unsigned int mousebg = 0;

/*
 Xresources preferences to load at startup
 TODO: update this with colortablenames
*/
#ifdef XRESOURCES
ResourcePref resources[] = {
    { "font", STRING, &font },
    { "color0", STRING, &colorname[0] },
    { "color1", STRING, &colorname[1] },
    { "color2", STRING, &colorname[2] },
    { "color3", STRING, &colorname[3] },
    { "color4", STRING, &colorname[4] },
    { "color5", STRING, &colorname[5] },
    { "color6", STRING, &colorname[6] },
    { "color7", STRING, &colorname[7] },
    { "color8", STRING, &colorname[8] },
    { "color9", STRING, &colorname[9] },
    { "color10", STRING, &colorname[10] },
    { "color11", STRING, &colorname[11] },
    { "color12", STRING, &colorname[12] },
    { "color13", STRING, &colorname[13] },
    { "color14", STRING, &colorname[14] },
    { "color15", STRING, &colorname[15] },
    { "background", STRING, &colorname[256] },
    { "foreground", STRING, &colorname[257] },
    { "cursorColor", STRING, &colorname[258] },
    { "termname", STRING, &termname },
    { "shell", STRING, &shell },
    //{ "xfps", INTEGER, &xfps },
    //{ "actionfps", INTEGER, &actionfps },
    { "blinktimeout", INTEGER, &blinktimeout },
    { "bellvolume", INTEGER, &bellvolume },
    { "tabspaces", INTEGER, &tabspaces },
    { "borderpx", INTEGER, &borderpx },
    { "cwscale", FLOAT, &cwscale },
    { "chscale", FLOAT, &chscale },
};
#endif
/*
 Color used to display font attributes when fontconfig selected a font which
 doesn't match the ones requested.
*/
//static unsigned int defaultattr = 11;

/*
 Force mouse select/shortcuts while mask is active (when MODE_MOUSE is set).
 Note that if you want to use ShiftMask with selmasks, set this to an other
 modifier, set to 0 to not use it.
*/
static uint forcemousemod = ShiftMask;

/*
 Internal mouse shortcuts.
 Beware that overloading Button1 will disable the selection.
*/
const unsigned int mousescrollincrement = 3;
static MouseShortcut mshortcuts[] = {
    /* mask                 button   function        argument       release */
    { ShiftMask, Button4, kscrollup, {.i = mousescrollincrement*3}, 0, /* !alt */ -1 },
    { XK_ANY_MOD, Button4, kscrollup, {.i = mousescrollincrement}, 0, /* !alt */ -1 },
    { ShiftMask, Button5, kscrolldown, {.i = mousescrollincrement*3}, 0, /* !alt */ -1 },
    { XK_ANY_MOD, Button5, kscrolldown, {.i = mousescrollincrement}, 0, /* !alt */ -1 },
    { XK_ANY_MOD, Button2, selpaste, {.i = 0}, 1 },
    { XK_ANY_MOD, Button4, ttysend, {.s = "\031"} },
    { XK_ANY_MOD, Button5, ttysend, {.s = "\005"} },
};

/* Internal keyboard shortcuts. */
// inputmodes
#define ALLMODES 0xffffffff
#define MODE_DEFAULT 0x01
#define MODE_LESS 0x02
#define IMODE_HELP 0x04 // 0x4 | 0x2 , keys for lessmode

#define ALL_KEYS UINT_MAX-1
// all comments, starting with //K are parsed into the keystroke shortref

#define I(value) { .i=value }
#define BIND(mask,keysym,function,argument,inputmode,...) { mask,keysym,function,argument,inputmode }

#else  // ifndef extract_keyref
		 // definitions to extract a key reference
#define BIND(mask,keysym,function,argument,inputmode,...) inputmode; mask ; keysym ; function ; __VA_ARGS__
#define ControlMask Control
#define ShiftMask Shift
#define Mod1Mask Alt
#define Mod4Mask Win
#define XK_ANY_MOD All
#define ALLMODES All
#define MODE_LESS Less
#define IMODE_HELP Help
#endif

// masks: Mod1Mask .. Mod5Mask, ControlMask, ShiftMask, LockMask
// mod1 = alt, mod2 = win , mod3 = Capslock (here)
// mod4 = win (!) 
#define MODKEY Mod1Mask
#define TERMMOD (ControlMask|ShiftMask)
#define SETBOOKMARKMASK (ControlMask|Mod1Mask)
// CapsLock
//#define BOOKMARKMASK Mod2Mask
#define BOOKMARKMASK ControlMask
// Ctrl+Shift+Win
#define SETFONTMASK ShiftMask|Mod1Mask


/*
 The table of bound shortcuts and keys
 Can be changed. 
 The key names are in the according header file of xorg,
 here: /usr/include/X11/keysymdef.h 
 
 The first matching binding will be used,
 the table is matched in its order here.
 e.g.
 Match Shift + PageUp
 Match PageUp
 */
#ifndef extract_keyref
Shortcut shortcuts[] = {
#endif
/*  { mask,       keysym,   function,  argument, INPUTMODE } */

BIND( ControlMask, XK_F1, showhelp, { 0},ALLMODES ),

BIND( XK_ANY_MOD, XK_Break, sendbreak, {.i = 0},ALLMODES ),
BIND( ControlMask, XK_Print, toggleprinter, {.i = 0},ALLMODES ),
BIND( ShiftMask, XK_Print, printscreen, {.i = 0},ALLMODES ),
BIND( XK_ANY_MOD, XK_Print, printsel, {.i = 0},ALLMODES ),
BIND( TERMMOD, XK_I, inverse_screen, {},ALLMODES ),

BIND(ControlMask|ShiftMask|Mod1Mask|Mod4Mask, XK_I, dump_terminfo, {}, MODE_DEFAULT ),
		// Change font size/width
BIND( SETFONTMASK, XK_Page_Up, zoom, {.f = -1},ALLMODES ),
    //{ SETFONTMASK, XK_Prior, zoom, {.f = +1},ALLMODES ),
BIND( SETFONTMASK, XK_Page_Down, zoom, {.f = +1},ALLMODES ),
    //{ SETFONTMASK, XK_Next, zoom, {.f = -1},ALLMODES ),
BIND( SETFONTMASK, XK_Home, zoomreset, {.f = 0},ALLMODES ),

BIND( SETFONTMASK, XK_Insert, set_fontwidth, {.i = -1},ALLMODES ),
BIND( SETFONTMASK, XK_Delete, set_fontwidth, {.i = 1},ALLMODES ),
BIND( SETFONTMASK, XK_End, set_fontwidth, {.i = -1},ALLMODES ),

// Scrolling
BIND( ShiftMask, XK_Page_Up, kscrollup, {.i = -1},ALLMODES ),
BIND( ShiftMask, XK_Page_Down, kscrolldown, {.i = -1},ALLMODES ),
BIND( ShiftMask, XK_End, scrolltobottom, { },ALLMODES ),
BIND( ShiftMask, XK_Home, scrolltotop, { },ALLMODES ),
 		// Shift+Up/Down: Scroll Up/Down 3 lines
BIND( ShiftMask, XK_Up, kscrollup, {.i = 3},ALLMODES ),
BIND( ShiftMask, XK_Down, kscrolldown, {.i = 3},ALLMODES ),

// lessmode
BIND( XK_ANY_MOD, XK_Up, kscrollup, {.i = 1},MODE_LESS ),
BIND( XK_ANY_MOD, XK_Down, kscrolldown, {.i = 1},MODE_LESS ),
BIND( XK_ANY_MOD, XK_Page_Up, kscrollup, {.i = -1},MODE_LESS ),
BIND( XK_ANY_MOD, XK_Page_Down, kscrolldown, {.i = -1},MODE_LESS ),
BIND( XK_ANY_MOD, XK_space, kscrolldown, {.i = -1},MODE_LESS ),
BIND( XK_ANY_MOD, XK_End, scrolltobottom, { },MODE_LESS ),
BIND( XK_ANY_MOD, XK_Home, scrolltotop, { },MODE_LESS ),


// help mode
BIND( XK_ANY_MOD, XK_q, quithelp, { 0},IMODE_HELP ),
BIND( XK_ANY_MOD, XK_Escape, quithelp, { 0},IMODE_HELP ),
BIND( XK_ANY_MOD, XK_Up, kscrollup, {.i = 1},IMODE_HELP ),
BIND( XK_ANY_MOD, XK_Down, kscrolldown, {.i = 1},IMODE_HELP ),
BIND( XK_ANY_MOD, XK_Page_Up, kscrollup, {.i = -1},IMODE_HELP ),
BIND( XK_ANY_MOD, XK_Page_Down, kscrolldown, {.i = -1},IMODE_HELP ),
BIND( XK_ANY_MOD, XK_space, kscrolldown, {.i = -1},IMODE_HELP ),
BIND( XK_ANY_MOD, XK_End, scrolltobottom, { },IMODE_HELP ),
BIND( XK_ANY_MOD, XK_Home, scrolltotop, { },IMODE_HELP ),


		// abort precessing when in the help view. 
BIND( XK_ANY_MOD, ALL_KEYS, dummy, {}, IMODE_HELP ),


// clipboard
BIND( TERMMOD, XK_C, clipcopy, {.i = 0},ALLMODES ),
BIND( TERMMOD, XK_V, clippaste, {.i = 0},ALLMODES ),
BIND( TERMMOD, XK_Y, selpaste, {.i = 0},ALLMODES ),
BIND( ShiftMask, XK_Insert, selpaste, {.i = 0},ALLMODES ),
BIND( TERMMOD, XK_S, keyboard_select, { 0 },ALLMODES ),
BIND( Mod1Mask, XK_s, keyboard_select, { 0 },ALLMODES ),

BIND( TERMMOD, XK_Num_Lock, numlock, {.i = 0},ALLMODES ),


// scrollmarks
BIND( SETBOOKMARKMASK, XK_1, set_scrollmark, { .i=1 },ALLMODES ),
BIND( SETBOOKMARKMASK, XK_2, set_scrollmark, { .i=2 },ALLMODES ),
BIND( SETBOOKMARKMASK, XK_3, set_scrollmark, { .i=3 },ALLMODES ),
BIND( SETBOOKMARKMASK, XK_4, set_scrollmark, { .i=4 },ALLMODES ),
BIND( SETBOOKMARKMASK, XK_5, set_scrollmark, { .i=5 },ALLMODES ),
BIND( SETBOOKMARKMASK, XK_6, set_scrollmark, { .i=6 },ALLMODES ),
BIND( SETBOOKMARKMASK, XK_7, set_scrollmark, { .i=7 },ALLMODES ),
BIND( SETBOOKMARKMASK, XK_8, set_scrollmark, { .i=8 },ALLMODES ),
BIND( SETBOOKMARKMASK, XK_9, set_scrollmark, { .i=9 },ALLMODES ),
BIND( SETBOOKMARKMASK, XK_0, set_scrollmark, { .i=0 },ALLMODES ),
		// set scrollmark 0 on Return
    //{ XK_ANY_MOD, XK_Return, set_scrollmark, { .i=0 },ALLMODES ),

// here Capslock (mapped with xmodmap)
BIND( BOOKMARKMASK, XK_1, scrollmark, { .i=1 },ALLMODES ),
BIND( BOOKMARKMASK, XK_2, scrollmark, { .i=2 },ALLMODES ),
BIND( BOOKMARKMASK, XK_3, scrollmark, { .i=3 },ALLMODES ),
BIND( BOOKMARKMASK, XK_4, scrollmark, { .i=4 },ALLMODES ),
BIND( BOOKMARKMASK, XK_5, scrollmark, { .i=5 },ALLMODES ),
BIND( BOOKMARKMASK, XK_6, scrollmark, { .i=6 },ALLMODES ),
BIND( BOOKMARKMASK, XK_7, scrollmark, { .i=7 },ALLMODES ),
BIND( BOOKMARKMASK, XK_8, scrollmark, { .i=8 },ALLMODES ),
BIND( BOOKMARKMASK, XK_9, scrollmark, { .i=9 },ALLMODES ),
BIND( BOOKMARKMASK, XK_0, scrollmark, { .i=0 },ALLMODES ),

/* replaced with retmarks
BIND( XK_ANY_MOD, XK_1, scrollmark, { .i=1 },MODE_LESS ),
BIND( XK_ANY_MOD, XK_2, scrollmark, { .i=2 },MODE_LESS ),
BIND( XK_ANY_MOD, XK_3, scrollmark, { .i=3 },MODE_LESS ),
BIND( XK_ANY_MOD, XK_4, scrollmark, { .i=4 },MODE_LESS ),
BIND( XK_ANY_MOD, XK_5, scrollmark, { .i=5 },MODE_LESS ),
BIND( XK_ANY_MOD, XK_6, scrollmark, { .i=6 },MODE_LESS ),
BIND( XK_ANY_MOD, XK_7, scrollmark, { .i=7 },MODE_LESS ),
BIND( XK_ANY_MOD, XK_8, scrollmark, { .i=8 },MODE_LESS ),
BIND( XK_ANY_MOD, XK_9, scrollmark, { .i=9 },MODE_LESS ),
BIND( XK_ANY_MOD, XK_0, scrollmark, { .i=0 },MODE_LESS ),*/

BIND( XK_ANY_MOD, XK_1, retmark, { .i=1 },MODE_LESS ),
BIND( XK_ANY_MOD, XK_2, retmark, { .i=2 },MODE_LESS ),
BIND( XK_ANY_MOD, XK_3, retmark, { .i=3 },MODE_LESS ),
BIND( XK_ANY_MOD, XK_4, retmark, { .i=4 },MODE_LESS ),
BIND( XK_ANY_MOD, XK_5, retmark, { .i=5 },MODE_LESS ),
BIND( XK_ANY_MOD, XK_6, retmark, { .i=6 },MODE_LESS ),
BIND( XK_ANY_MOD, XK_7, retmark, { .i=7 },MODE_LESS ),
BIND( XK_ANY_MOD, XK_8, retmark, { .i=8 },MODE_LESS ),
BIND( XK_ANY_MOD, XK_9, retmark, { .i=9 },MODE_LESS ),
BIND( XK_ANY_MOD, XK_0, retmark, { .i=10 },MODE_LESS ),



BIND( SETBOOKMARKMASK, XK_Return, enterscroll, { .i=11 },ALLMODES ),
    //{ ControlMask, XK_Return, enterscroll, { .i=11 },ALLMODES ),
BIND( ShiftMask, XK_Return, enterscroll, { .i=11 },ALLMODES ),
    //{ XK_ANY_MOD, XK_Return, leavescroll, { 0 },ALLMODES ),

	// doesnt ork ??
	// { TERMMOD, XK_E, ttysend, { .s="\x80" }, ALLMODES ),

BIND( ShiftMask, XK_BackSpace, retmark , { },ALLMODES ),
BIND( ShiftMask, XK_ISO_Left_Tab, retmark, {}, ALLMODES ), // tab left <- to enter lessmode
BIND( XK_ANY_MOD,XK_BackSpace, retmark , { },MODE_LESS ),
BIND( XK_ANY_MOD,XK_Tab, retmark , { .i=-1 },MODE_LESS ), // tab -> to scroll down
	// tab left or backspace and right cycle between set retmarks.

// "less mode" enter with Ctrl+shift+ Cursor/Page up/down 
//  Up and PageUp also scroll upwards
// toggle with Ctrl+Shift + L / down
// quit with q or Escape
	// switch on.
BIND( TERMMOD, XK_Up, lessmode_toggle, I( LESSMODE_ON | SCROLLUP(3) ) ,ALLMODES ),
BIND( TERMMOD, XK_Page_Up, lessmode_toggle, I( LESSMODE_ON | SCROLL_PAGEUP) ,ALLMODES ),
BIND( TERMMOD, XK_Page_Down, lessmode_toggle, I( LESSMODE_ON | SCROLL_PAGEDOWN),ALLMODES ),
BIND( TERMMOD, XK_Down, lessmode_toggle,I( LESSMODE_ON | SCROLLDOWN(3)),ALLMODES ),
BIND( TERMMOD, XK_Home, lessmode_toggle,I( LESSMODE_ON | SCROLL_TOP ),ALLMODES ),

	// toggle
BIND( TERMMOD, XK_L, lessmode_toggle, I(LESSMODE_TOGGLE),ALLMODES ),
BIND( XK_ANY_MOD, XK_Scroll_Lock, lessmode_toggle, I(LESSMODE_TOGGLE),ALLMODES ),
	// switchoff
BIND( XK_ANY_MOD, XK_Escape, lessmode_toggle,  I(LESSMODE_OFF | SCROLL_BOTTOM),MODE_LESS ),
BIND( XK_ANY_MOD, XK_q, lessmode_toggle, I(LESSMODE_OFF | SCROLL_BOTTOM),MODE_LESS ),
BIND( ShiftMask, XK_Return, lessmode_toggle, I(LESSMODE_TOGGLE),MODE_LESS ),
		
// select charmap
BIND( ControlMask|Mod4Mask, XK_1, set_charmap, { .i=1 },ALLMODES ),
BIND( ControlMask|Mod4Mask, XK_2, set_charmap, { .i=2 },ALLMODES ),
BIND( ControlMask|Mod4Mask, XK_3, set_charmap, { .i=3 },ALLMODES ),
BIND( ControlMask|Mod4Mask, XK_4, set_charmap, { .i=4 },ALLMODES ),
BIND( ControlMask|Mod4Mask, XK_5, set_charmap, { .i=5 },ALLMODES ),
BIND( ControlMask|Mod4Mask, XK_6, set_charmap, { .i=6 },ALLMODES ),
BIND( ControlMask|Mod4Mask, XK_7, set_charmap, { .i=7 },ALLMODES ),
BIND( ControlMask|Mod4Mask, XK_8, set_charmap, { .i=8 },ALLMODES ),
BIND( ControlMask|Mod4Mask, XK_9, set_charmap, { .i=9 },ALLMODES ),
BIND( ControlMask|Mod4Mask, XK_0, set_charmap, { .i=0 },ALLMODES ),


    //{ TERMMOD, XK_T, temp, {.i = 0},ALLMODES },

#ifndef extract_keyref
};
#undef I
#undef BIND

/*
 Special keys (change & recompile slterm.terminfo accordingly)

 Mask value:
   Use XK_ANY_MOD to match the key no matter modifiers state
   Use XK_NO_MOD to match the key alone (no modifiers)
 appkey value:
   0: no value
   > 0: keypad application mode enabled
   = 2: term.numlock = 1
   < 0: keypad application mode disabled
 appcursor value:
   0: no value
   > 0: cursor application mode enabled
   < 0: cursor application mode disabled

 Be careful with the order of the definitions because st searches in
 this table sequentially, so any XK_ANY_MOD must be in the last
 position for a key.
*/

/*
 If you want keys other than the X11 function keys (0xFD00 - 0xFFFF)
 to be mapped below, add them to this array.
*/
static KeySym mappedkeys[] = { -1 };

/*
 State bits to ignore when matching key or button events.  By default,
 numlock (Mod2Mask) and keyboard layout (XK_SWITCH_MOD) are ignored.
*/
static uint ignoremod = Mod2Mask|XK_SWITCH_MOD;

#define K(key,mod,str,appkey,appcursor) { key,mod,str,sizeof(str)-1,appkey,appcursor }

static Key key[] = {
    /* keysym           mask            string      appkey appcursor */

	 // Linux compatibility definitions below
	 K( XK_Return, Mod1Mask, "\033\r", 0, 0),
    K( XK_Return, XK_ANY_MOD, "\r", 0, 0),
    K( XK_Up, ShiftMask, "\033[1;2A", 0, 0),
    K( XK_Up, Mod1Mask, "\033[1;3A", 0, 0),
    K( XK_Up, ShiftMask|Mod1Mask, "\033[1;4A", 0, 0),
    K( XK_Up, ControlMask, "\033[1;5A", 0, 0),
    K( XK_Up, ShiftMask|ControlMask, "\033[1;6A", 0, 0),
    K( XK_Up, ControlMask|Mod1Mask, "\033[1;7A", 0, 0),
    K( XK_Up, ShiftMask|ControlMask|Mod1Mask, "\033[1;8A", 0, 0),
    K( XK_Up, XK_ANY_MOD, "\033[A", 0, -1),
    K( XK_Up, XK_ANY_MOD, "\033OA", 0, +1),
    K( XK_Down, ShiftMask, "\033[1;2B", 0, 0),
    K( XK_Down, Mod1Mask, "\033[1;3B", 0, 0),
    K( XK_Down, ShiftMask|Mod1Mask, "\033[1;4B", 0, 0),
    K( XK_Down, ControlMask, "\033[1;5B", 0, 0),
    K( XK_Down, ShiftMask|ControlMask, "\033[1;6B", 0, 0),
    K( XK_Down, ControlMask|Mod1Mask, "\033[1;7B", 0, 0),
    K( XK_Down, ShiftMask|ControlMask|Mod1Mask, "\033[1;8B", 0, 0),
    K( XK_Down, XK_ANY_MOD, "\033[B", 0, -1),
    K( XK_Down, XK_ANY_MOD, "\033OB", 0, +1),
    K( XK_Left, ShiftMask, "\033[1;2D", 0, 0),
    K( XK_Left, Mod1Mask, "\033[1;3D", 0, 0),
    K( XK_Left, ShiftMask|Mod1Mask, "\033[1;4D", 0, 0),
    K( XK_Left, ControlMask, "\033[1;5D", 0, 0),
    K( XK_Left, ShiftMask|ControlMask, "\033[1;6D", 0, 0),
    K( XK_Left, ControlMask|Mod1Mask, "\033[1;7D", 0, 0),
    K( XK_Left, ShiftMask|ControlMask|Mod1Mask, "\033[1;8D", 0, 0),
    K( XK_Left, XK_ANY_MOD, "\033[D", 0, -1),
    K( XK_Left, XK_ANY_MOD, "\033OD", 0, +1),
    K( XK_Right, ShiftMask, "\033[1;2C", 0, 0),
    K( XK_Right, Mod1Mask, "\033[1;3C", 0, 0),
    K( XK_Right, ShiftMask|Mod1Mask, "\033[1;4C", 0, 0),
    K( XK_Right, ControlMask, "\033[1;5C", 0, 0),
    K( XK_Right, ShiftMask|ControlMask, "\033[1;6C", 0, 0),
    K( XK_Right, ControlMask|Mod1Mask, "\033[1;7C", 0, 0),
    K( XK_Right, ShiftMask|ControlMask|Mod1Mask, "\033[1;8C", 0, 0),
    K( XK_Right, XK_ANY_MOD, "\033[C", 0, -1),
    K( XK_Right, XK_ANY_MOD, "\033OC", 0, +1),


    K( XK_Insert, ShiftMask, "\033[4l", -1, 0),
    K( XK_Insert, ShiftMask, "\033[2;2~", +1, 0),
    K( XK_Insert, ControlMask, "\033[L", -1, 0),
    K( XK_Insert, ControlMask, "\033[2;5~", +1, 0),
    K( XK_Insert, XK_ANY_MOD, "\033[4h", -1, 0),
    K( XK_Insert, XK_ANY_MOD, "\033[2~", +1, 0),
    K( XK_Delete, ControlMask, "\033[M", -1, 0),
    K( XK_Delete, ControlMask, "\033[3;5~", +1, 0),
    K( XK_Delete, ShiftMask, "\033[2K", -1, 0),
    K( XK_Delete, ShiftMask, "\033[3;2~", +1, 0),
    K( XK_Delete, XK_ANY_MOD, "\033[P", -1, 0),
    K( XK_Delete, XK_ANY_MOD, "\033[3~", +1, 0),
    K( XK_BackSpace, XK_NO_MOD, "\177", 0, 0),
    K( XK_BackSpace, Mod1Mask, "\033\177", 0, 0),
    K( XK_Home, ShiftMask, "\033[2J", 0, -1),
    K( XK_Home, ShiftMask, "\033[1;2H", 0, +1),
    K( XK_Home, XK_ANY_MOD, "\033[H", 0, -1),
    K( XK_Home, XK_ANY_MOD, "\033[1~", 0, +1),
    K( XK_End, ControlMask, "\033[J", -1, 0),
    K( XK_End, ControlMask, "\033[1;5F", +1, 0),
    K( XK_End, ShiftMask, "\033[K", -1, 0),
    K( XK_End, ShiftMask, "\033[1;2F", +1, 0),
    K( XK_End, XK_ANY_MOD, "\033[4~", 0, 0),
    K( XK_Prior, ControlMask, "\033[5;5~", 0, 0),
    K( XK_Prior, ShiftMask, "\033[5;2~", 0, 0),
    K( XK_Prior, XK_ANY_MOD, "\033[5~", 0, 0),
    K( XK_Next, ControlMask, "\033[6;5~", 0, 0),
    K( XK_Next, ShiftMask, "\033[6;2~", 0, 0),
    K( XK_Next, XK_ANY_MOD, "\033[6~", 0, 0),
    K( XK_ISO_Left_Tab, ShiftMask, "\033[Z", 0, 0),

    K( XK_F1, XK_NO_MOD, "\033OP", 0, 0),
    K( XK_F1, /* F13 */ ShiftMask, "\033[1;2P", 0, 0),
    K( XK_F1, /* F25 */ ControlMask, "\033[1;5P", 0, 0),
    K( XK_F1, /* F37 */ Mod4Mask, "\033[1;6P", 0, 0),
    K( XK_F1, /* F49 */ Mod1Mask, "\033[1;3P", 0, 0),
    K( XK_F1, /* F61 */ Mod3Mask, "\033[1;4P", 0, 0),
    K( XK_F2, XK_NO_MOD, "\033OQ", 0, 0),
    K( XK_F2, /* F14 */ ShiftMask, "\033[1;2Q", 0, 0),
    K( XK_F2, /* F26 */ ControlMask, "\033[1;5Q", 0, 0),
    K( XK_F2, /* F38 */ Mod4Mask, "\033[1;6Q", 0, 0),
    K( XK_F2, /* F50 */ Mod1Mask, "\033[1;3Q", 0, 0),
    K( XK_F2, /* F62 */ Mod3Mask, "\033[1;4Q", 0, 0),
    K( XK_F3, XK_NO_MOD, "\033OR", 0, 0),
    K( XK_F3, /* F15 */ ShiftMask, "\033[1;2R", 0, 0),
    K( XK_F3, /* F27 */ ControlMask, "\033[1;5R", 0, 0),
    K( XK_F3, /* F39 */ Mod4Mask, "\033[1;6R", 0, 0),
    K( XK_F3, /* F51 */ Mod1Mask, "\033[1;3R", 0, 0),
    K( XK_F3, /* F63 */ Mod3Mask, "\033[1;4R", 0, 0),
    K( XK_F4, XK_NO_MOD, "\033OS", 0, 0),
    K( XK_F4, /* F16 */ ShiftMask, "\033[1;2S", 0, 0),
    K( XK_F4, /* F28 */ ControlMask, "\033[1;5S", 0, 0),
    K( XK_F4, /* F40 */ Mod4Mask, "\033[1;6S", 0, 0),
    K( XK_F4, /* F52 */ Mod1Mask, "\033[1;3S", 0, 0),
    K( XK_F5, XK_NO_MOD, "\033[15~", 0, 0),
    K( XK_F5, /* F17 */ ShiftMask, "\033[15;2~", 0, 0),
    K( XK_F5, /* F29 */ ControlMask, "\033[15;5~", 0, 0),
    K( XK_F5, /* F41 */ Mod4Mask, "\033[15;6~", 0, 0),
    K( XK_F5, /* F53 */ Mod1Mask, "\033[15;3~", 0, 0),
    K( XK_F6, XK_NO_MOD, "\033[17~", 0, 0),
    K( XK_F6, /* F18 */ ShiftMask, "\033[17;2~", 0, 0),
    K( XK_F6, /* F30 */ ControlMask, "\033[17;5~", 0, 0),
    K( XK_F6, /* F42 */ Mod4Mask, "\033[17;6~", 0, 0),
    K( XK_F6, /* F54 */ Mod1Mask, "\033[17;3~", 0, 0),
    K( XK_F7, XK_NO_MOD, "\033[18~", 0, 0),
    K( XK_F7, /* F19 */ ShiftMask, "\033[18;2~", 0, 0),
    K( XK_F7, /* F31 */ ControlMask, "\033[18;5~", 0, 0),
    K( XK_F7, /* F43 */ Mod4Mask, "\033[18;6~", 0, 0),
    K( XK_F7, /* F55 */ Mod1Mask, "\033[18;3~", 0, 0),
    K( XK_F8, XK_NO_MOD, "\033[19~", 0, 0),
    K( XK_F8, /* F20 */ ShiftMask, "\033[19;2~", 0, 0),
    K( XK_F8, /* F32 */ ControlMask, "\033[19;5~", 0, 0),
    K( XK_F8, /* F44 */ Mod4Mask, "\033[19;6~", 0, 0),
    K( XK_F8, /* F56 */ Mod1Mask, "\033[19;3~", 0, 0),
    K( XK_F9, XK_NO_MOD, "\033[20~", 0, 0),
    K( XK_F9, /* F21 */ ShiftMask, "\033[20;2~", 0, 0),
    K( XK_F9, /* F33 */ ControlMask, "\033[20;5~", 0, 0),
    K( XK_F9, /* F45 */ Mod4Mask, "\033[20;6~", 0, 0),
    K( XK_F9, /* F57 */ Mod1Mask, "\033[20;3~", 0, 0),
    K( XK_F10, XK_NO_MOD, "\033[21~", 0, 0),
    K( XK_F10, /* F22 */ ShiftMask, "\033[21;2~", 0, 0),
    K( XK_F10, /* F34 */ ControlMask, "\033[21;5~", 0, 0),
    K( XK_F10, /* F46 */ Mod4Mask, "\033[21;6~", 0, 0),
    K( XK_F10, /* F58 */ Mod1Mask, "\033[21;3~", 0, 0),
    K( XK_F11, XK_NO_MOD, "\033[23~", 0, 0),
    K( XK_F11, /* F23 */ ShiftMask, "\033[23;2~", 0, 0),
    K( XK_F11, /* F35 */ ControlMask, "\033[23;5~", 0, 0),
    K( XK_F11, /* F47 */ Mod4Mask, "\033[23;6~", 0, 0),
    K( XK_F11, /* F59 */ Mod1Mask, "\033[23;3~", 0, 0),
    K( XK_F12, XK_NO_MOD, "\033[24~", 0, 0),
    K( XK_F12, /* F24 */ ShiftMask, "\033[24;2~", 0, 0),
    K( XK_F12, /* F36 */ ControlMask, "\033[24;5~", 0, 0),
    K( XK_F12, /* F48 */ Mod4Mask, "\033[24;6~", 0, 0),
    K( XK_F12, /* F60 */ Mod1Mask, "\033[24;3~", 0, 0),

	 K( XK_KP_Enter, XK_ANY_MOD, "\033OM", +2, 0),
    K( XK_KP_Enter, XK_ANY_MOD, "\r", -1, 0),

    K( XK_KP_Home, ShiftMask, "\033[2J", 0, -1),
    K( XK_KP_Home, ShiftMask, "\033[1;2H", 0, +1),
    K( XK_KP_Home, XK_ANY_MOD, "\033[H", 0, -1),
    K( XK_KP_Home, XK_ANY_MOD, "\033[1~", 0, +1),
    K( XK_KP_Up, XK_ANY_MOD, "\033Ox", +1, 0),
    K( XK_KP_Up, XK_ANY_MOD, "\033[A", 0, -1),
    K( XK_KP_Up, XK_ANY_MOD, "\033OA", 0, +1),
    K( XK_KP_Down, XK_ANY_MOD, "\033Or", +1, 0),
    K( XK_KP_Down, XK_ANY_MOD, "\033[B", 0, -1),
    K( XK_KP_Down, XK_ANY_MOD, "\033OB", 0, +1),
    K( XK_KP_Left, XK_ANY_MOD, "\033Ot", +1, 0),
    K( XK_KP_Left, XK_ANY_MOD, "\033[D", 0, -1),
    K( XK_KP_Left, XK_ANY_MOD, "\033OD", 0, +1),
    K( XK_KP_Right, XK_ANY_MOD, "\033Ov", +1, 0),
    K( XK_KP_Right, XK_ANY_MOD, "\033[C", 0, -1),
    K( XK_KP_Right, XK_ANY_MOD, "\033OC", 0, +1),
    K( XK_KP_Prior, ShiftMask, "\033[5;2~", 0, 0),
    K( XK_KP_Prior, XK_ANY_MOD, "\033[5~", 0, 0),
    K( XK_KP_Begin, XK_ANY_MOD, "\033[E", 0, 0),
    K( XK_KP_End, ControlMask, "\033[J", -1, 0),
    K( XK_KP_End, ControlMask, "\033[1;5F", +1, 0),
    K( XK_KP_End, ShiftMask, "\033[K", -1, 0),
    K( XK_KP_End, ShiftMask, "\033[1;2F", +1, 0),
    K( XK_KP_End, XK_ANY_MOD, "\033[4~", 0, 0),
    K( XK_KP_Next, ShiftMask, "\033[6;2~", 0, 0),
    K( XK_KP_Next, XK_ANY_MOD, "\033[6~", 0, 0),
    K( XK_KP_Insert, ShiftMask, "\033[2;2~", +1, 0),
    K( XK_KP_Insert, ShiftMask, "\033[4l", -1, 0),
    K( XK_KP_Insert, ControlMask, "\033[L", -1, 0),
    K( XK_KP_Insert, ControlMask, "\033[2;5~", +1, 0),
    K( XK_KP_Insert, XK_ANY_MOD, "\033[4h", -1, 0),
    K( XK_KP_Insert, XK_ANY_MOD, "\033[2~", +1, 0),
    K( XK_KP_Delete, ControlMask, "\033[M", -1, 0),
    K( XK_KP_Delete, ControlMask, "\033[3;5~", +1, 0),
    K( XK_KP_Delete, ShiftMask, "\033[2K", -1, 0),
    K( XK_KP_Delete, ShiftMask, "\033[3;2~", +1, 0),
    K( XK_KP_Delete, XK_ANY_MOD, "\033[P", -1, 0),
    K( XK_KP_Delete, XK_ANY_MOD, "\033[3~", +1, 0),
    K( XK_KP_Multiply, XK_ANY_MOD, "\033Oj", +2, 0),
    K( XK_KP_Add, XK_ANY_MOD, "\033Ok", +2, 0),
    K( XK_KP_Subtract, XK_ANY_MOD, "\033Om", +2, 0),
    K( XK_KP_Decimal, XK_ANY_MOD, "\033On", +2, 0),
    K( XK_KP_Divide, XK_ANY_MOD, "\033Oo", +2, 0),
    K( XK_KP_0, XK_ANY_MOD, "\033Op", +2, 0),
    K( XK_KP_1, XK_ANY_MOD, "\033Oq", +2, 0),
    K( XK_KP_2, XK_ANY_MOD, "\033Or", +2, 0),
    K( XK_KP_3, XK_ANY_MOD, "\033Os", +2, 0),
    K( XK_KP_4, XK_ANY_MOD, "\033Ot", +2, 0),
    K( XK_KP_5, XK_ANY_MOD, "\033Ou", +2, 0),
    K( XK_KP_6, XK_ANY_MOD, "\033Ov", +2, 0),
    K( XK_KP_7, XK_ANY_MOD, "\033Ow", +2, 0),
    K( XK_KP_8, XK_ANY_MOD, "\033Ox", +2, 0),
    K( XK_KP_9, XK_ANY_MOD, "\033Oy", +2, 0),


//#ifdef F13_35
#if 1
    K( XK_F13, XK_NO_MOD, "\033[1;2P", 0, 0),
    K( XK_F14, XK_NO_MOD, "\033[1;2Q", 0, 0),
    K( XK_F15, XK_NO_MOD, "\033[1;2R", 0, 0),
    K( XK_F16, XK_NO_MOD, "\033[1;2S", 0, 0),
    K( XK_F17, XK_NO_MOD, "\033[15;2~", 0, 0),
    K( XK_F18, XK_NO_MOD, "\033[17;2~", 0, 0),
    K( XK_F19, XK_NO_MOD, "\033[18;2~", 0, 0),
    K( XK_F20, XK_NO_MOD, "\033[19;2~", 0, 0),
    K( XK_F21, XK_NO_MOD, "\033[20;2~", 0, 0),
    K( XK_F22, XK_NO_MOD, "\033[21;2~", 0, 0),
    K( XK_F23, XK_NO_MOD, "\033[23;2~", 0, 0),
    K( XK_F24, XK_NO_MOD, "\033[24;2~", 0, 0),
    K( XK_F25, XK_NO_MOD, "\033[1;5P", 0, 0),
    K( XK_F26, XK_NO_MOD, "\033[1;5Q", 0, 0),
    K( XK_F27, XK_NO_MOD, "\033[1;5R", 0, 0),
    K( XK_F28, XK_NO_MOD, "\033[1;5S", 0, 0),
    K( XK_F29, XK_NO_MOD, "\033[15;5~", 0, 0),
    K( XK_F30, XK_NO_MOD, "\033[17;5~", 0, 0),
    K( XK_F31, XK_NO_MOD, "\033[18;5~", 0, 0),
    K( XK_F32, XK_NO_MOD, "\033[19;5~", 0, 0),
    K( XK_F33, XK_NO_MOD, "\033[20;5~", 0, 0),
    K( XK_F34, XK_NO_MOD, "\033[21;5~", 0, 0),
    K( XK_F35, XK_NO_MOD, "\033[23;5~", 0, 0),
#endif
};
#undef K

/*
 Selection types' masks.
 Use the same masks as usual.
 Button1Mask is always unset, to make masks match between ButtonPress.
 ButtonRelease and MotionNotify.
 If no match is found, regular selection is used.
*/
static uint selmasks[] = {
    [SEL_RECTANGULAR] = Mod1Mask,
};


/*
 select codepages
 can be: cp437, cp850, cp1250,cp1251,cp1252,cp1253,cp1255,
         cp1256,cp1257,cp1258,cpe4000,cpe4002a, 
         macintosh, atarist, mac_centraleurope
 (macintosh = MacRoman)
 other codepages could be added in charmaps.h
 use Ctrl+Win+ [1..9] to switch to the other codepages on the fly.

 cp1252 is at the same time the DEC-MCS, ANSI, and Windows1252 default codepage.

 cp437 is the old ibm codepage, with those signs to draw borders and boxes.
 (cp437 is uncomplete here, the "graphics mode" signs 0..0x1f are missing.
 it's the old issue of the IBM PC1. Reinvented.)

 Me, I'm happy with CP1252, it has got umlauts, and yet every application
 was able to work with the ansi code table without modifications. 

 Now, I did write a personal codepage, named cpe4000
 There are Umlauts, sz, box drawing chars ( at the places of the cp437 / 850),
 the greek alphabet and several mathematical and logical chars.
 It's what I do need for writing. 
 There's a filter for the conversion to and from utf8 and between codepages,
 within tools. 
 Numbered the cp e4000, according to the rfc's 0xExxxx is reserved for private 
 assignments. 

 Finally, I created another codepage. Naming this cpe4002a,
 but I regard the codepage as alpha, I'm going to change some things.
 Got the box drawing chars at the same location as with cp437.
 umlauts are there, at the locations of cp437/cp850
 Several logical and mathematical operators and signs
*/

// assign to Ctrl+Win +                    0,       1,    2        3      4      5      6
const short unsigned int* codepage[] = { cp1250, cp1251, cp1252, cp1253, cp437, cp850, cpe4002a,
//    7                    8      9
	mac_centraleurope, macintosh    };
// the default codepage is cp1252
// and assigned to Ctrl+Win+2

#ifdef MISC //my local copy
int selected_codepage = 6;
#else
// the default codepage (cp1252)
int selected_codepage = 2;
#endif

// convert the x clipboard to and from utf8, when yanking/pasting
#ifndef UTF8 // wouldn't be useful
#define UTF8_CLIPBOARD
#endif


// environment variables, exported to the shell
const char *export_env[][2] = {
   { "SHELL", "sh"},
		// display chars 128-255 in less
   { "LESSCHARSET", "dos"}, 

		// "save" default. better than unset.
		// is set in the shell normally
   { "LANG", "C"},
   { "LC_ALL", "C"},
		

   { "NORM", "\e[0;37;40m"},

   { "BLACK", "\e[30m"},
   { "RED", "\e[31m"},
   { "GREEN", "\e[32m"},
   { "YELLOW", "\e[33m"},
   { "BLUE", "\e[34m"},
   { "MAGENTA", "\e[35m"},
   { "CYAN", "\e[36m"},
   { "WHITE", "\e[37m"},

   { "BROWN", "\e[33m"},
   { "BGBROWN", "\e[43m"},
   { "ORANGE", "\e[1;2;33m"},
   { "ORANGERED", "\e[1;2;31m"},
   { "GRAY", "\e[1;2;30m"},
   { "PURPLE", "\e[1;2;35m"},
   { "MINT", "\e[1;2;32m"},
   { "TURQUOISE", "\e[1;2;36m"},

   { "LBLACK", "\e[90m"},
   { "LRED", "\e[91m"},
   { "LGREEN", "\e[92m"},
   { "LYELLOW", "\e[93m"},
   { "LBLUE", "\e[94m"},
   { "LMAGENTA", "\e[95m"},
   { "LCYAN", "\e[96m"},
   { "LWHITE", "\e[97m"},

   { "DBLACK", "\e[2;30m"},
   { "DRED", "\e[2;31m"},
   { "DGREEN", "\e[2;32m"},
   { "DYELLOW", "\e[2;33m"},
   { "DBLUE", "\e[2;34m"},
   { "DMAGENTA", "\e[2;35m"},
   { "DCYAN", "\e[2;36m"},
   { "DWHITE", "\e[2;37m"},

   { "LDBLACK", "\e[1;2;30m"},
   { "LDRED", "\e[1;2;31m"},
   { "LDGREEN", "\e[1;2;32m"},
   { "LDYELLOW", "\e[1;2;33m"},
   { "LDBLUE", "\e[1;2;34m"},
   { "LDMAGENTA", "\e[1;2;35m"},
   { "LDCYAN", "\e[1;2;36m"},
   { "LDWHITE", "\e[1;2;37m"},


   { "BGBLACK", "\e[40m"},
   { "BGRED", "\e[41m"},
   { "BGGREEN", "\e[42m"},
   { "BGYELLOW", "\e[43m"},
   { "BGBLUE", "\e[44m"},
   { "BGMAGENTA", "\e[45m"},
   { "BGCYAN", "\e[46m"},
   { "BGWHITE", "\e[47m"},

   { "BGLBLACK", "\e[100m"},
   { "BGLRED", "\e[101m"},
   { "BGLGREEN", "\e[102m"},
   { "BGLYELLOW", "\e[103m"},
   { "BGLBLUE", "\e[104m"},
   { "BGLMAGENTA", "\e[105m"},
   { "BGLCYAN", "\e[106m"},
   { "BGLWHITE", "\e[107m"},


   { "BOLD","\e[1m" },
   { "FAINT","\e[2m" },
   { "CURSIVE","\e[3m" },
   { "UNDERLINE","\e[4m" },
   { "BLINK","\e[6m" },
   { "REVERSE","\e[7m" },
   { "STRIKETHROUGH","\e[9m" },
   { "DOUBLEUNDERLINE","\e[21m" },
	{0} };



#endif

#endif

