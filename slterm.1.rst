========
 slterm
========

(sl)im (term)inal 


SYNOPSIS
========

**slterm** [**-aivVx**] [**-c** *class*] [**-f** *font*] [**-g** *geometry*]
[**-n** *name*] [**-o** *iofile*] [**-T** *title*] [**-t** *title*]
[**-l** *line*] [**-w** *windowid*] [[**-e**] *command*] [**-X**] 
[*arguments*...]]

**slterm** [**-aivVx**] [**-c** *class*] [**-f** *font*] [**-g** *geometry*]
[**-n** *name*] [**-o** *iofile*] [**-T** *title*] [**-t** *title*]
[**-w** *windowid*] -l *line* [*stty_args*...]


DESCRIPTION
===========

**slterm** is a virtual terminal emulator for X
It is a slimmed down fork of st (suckless.org),
utf-8 and rgb support stripped for better performance,
but with several additions and patches.


KEYS
====

**Ctrl-Shift-c**
   Copy the selected text to the clipboard selection.

**Ctrl-Shift-v**
   Paste from the clipboard selection.

**Ctrl-Shift-y**
   Paste from primary selection (middle mouse button).

**Ctrl+Shift+l**
   Toggle "less mode": when enabled, Cursor Up/Down, PageUp/PageDown, Home/End
   will scroll up / down, without holding Shift

**Ctrl-Shift-s** 
   Enter keyboard selection mode
   (Described below, with a different key mapping)

**Shift-PgUp/PgDown**
   Scroll Up/Down a Page

**Shift-Up/Down**
   Scroll Up/Down three lines

**Shift-Home/End**
   Scroll to top / bottom

**Break**
   Send a break in the serial line. Break key is obtained in PC
   keyboards pressing at the same time control and pause.

**Ctrl-Print Screen**
   Toggle if st should print to the *iofile.*

**Shift-Print Screen**
   Print the full screen to the *iofile.*

**Print Screen**
   Print the selection to the *iofile.*

**Ctrl-Shift-Page Up**
   Increase font size.

**Ctrl-Shift-Page Down**
   Decrease font size.

**Ctrl-Shift-Home**
   Reset to default font size.

   
Keybinding reference
====================


The keycombinations can be changed in config.h(recompile needed)


Keystrokes 
----------

There are 3 different modes in slterm;
regular mode, lessmode and selection mode.
slterm is started in the regular mode.


All modes:

  * Ctrl+Shift + I: Inverse colors
  * (Anymod)+F1:    Show this help


Set font width/size:

  * Alt+Shift + Insert/Delete:   Enlarge/Shrink width
  * Alt+Shift + PageUp/PageDown: Zoom in / out


Select Codepage:

  Ctrl+Win + 0 CP1250 
           + 1 CP1251
           + 2 CP1252 (this is mostly ANSI, and the 1. page of Unicode)
           + 3 CP1253
           + 4 CP437  (Old IBM codetable, borders and tables)
           + 5 CP850  (DOS Standard table)
           + 6 CP4002 (DOS Standard table)


Regular mode:

Scrolling:

  * Shift + Up/Down/PageUp/Pagedown: Scroll up/down
  * Shift + Home/End: Scroll to top/bottom
  * Shift + Backspace: Scroll to the location of the last command (shell)


Clipboard:

  * Shift + Insert / Ctrl+Shift + y: Paste
  * Ctrl+Shift + c: Copy 


Scrollmarks:

  * Ctrl+Alt + [0..9]: Set Scrollmark 0 - 9
  * Ctrl + [0..9]:     Scroll to mark 0 - 9
  * Shift + Backspace: Scroll to the last entered command (in shell)



Lessmode:

  Ctrl+Shift + Up/PageUp/l: Enter lessmode. 

  Scroll around with cursor keys, Home, End.
  Backspace goes to the location of the last command in shell.
  Exit with q/ESC

  Shift+Backspace: Scroll to the location of the last entered command,
    enter lessmode

  Shift+Enter: Execute command, enter lessmode when more than
    one screen is displayed by the command.

  * Ctrl+Alt + [0..9]: Set Scrollmark 0 - 9
  *            [0..9]: Goto Scrollmark 0 - 9



Selection Mode:

  Ctrl+Shift + S: Enter selection mode

  There are 3 submodes in selection mode:
    - move mode : to set the start of the selection;
    - select mode : to activate and set the end of the selection;
    - input mode : to enter the search criteria.
	

  Shortcuts for move and select modes :
 
 *    h, j, k, l:    move cursor left/down/up/right (also with arrow keys)
 *    !, _, \*:       move cursor to the middle of the line/column/screen
 *    Backspace, $:  move cursor to the beginning/end of the line
 *    PgUp, PgDown:  move cursor to the beginning/end of the column
 *    Home, End:     move cursor to the top/bottom left corner of the screen
 *    /, ?:          activate input mode and search up/down
 *    n, N:          repeat last search, up/down
 *    s:             toggle move/selection mode
 *    t:             toggle regular/rectangular selection type
 *    Return:        quit keyboard_select, keeping the highlight of the selection
 *    Escape:        quit keyboard_select
    
    With h,j,k,l (also with arrow keys), you can use a quantifier.
    Enter a number before hitting the appropriate key.
    

  Shortcuts for input mode :
 
 Return:       Return to the previous mode
 
 

Full shortcut list 
==================


====      =========              ===            ========                
Mode      Modifiers              Key            Function                 
-----------------------------------------------------------------------------
All	 Control+Alt        	 0          	 set_scrollmark 	
All	 Control+Alt        	 1          	 set_scrollmark 	
All	 Control+Alt        	 2          	 set_scrollmark 	
All	 Control+Alt        	 3          	 set_scrollmark 	
All	 Control+Alt        	 4          	 set_scrollmark 	
All	 Control+Alt        	 5          	 set_scrollmark 	
All	 Control+Alt        	 6          	 set_scrollmark 	
All	 Control+Alt        	 7          	 set_scrollmark 	
All	 Control+Alt        	 8          	 set_scrollmark 	
All	 Control+Alt        	 9          	 set_scrollmark 	
All	 Control+Alt        	 Return     	 enterscroll 	
All	 Control+Shift      	 C          	 clipcopy 	
All	 Control+Shift      	 Down       	 lessmode_toggle 	
All	 Control+Shift      	 Home       	 lessmode_toggle 	
All	 Control+Shift      	 I          	 inverse_screen 	
All	 Control+Shift      	 L          	 lessmode_toggle 	
All	 Control+Shift      	 Num_Lock   	 numlock 	
All	 Control+Shift      	 Page_Down  	 lessmode_toggle 	
All	 Control+Shift      	 Page_Up    	 lessmode_toggle 	
All	 Control+Shift      	 S          	 keyboard_select 	
All	 Control+Shift      	 Up         	 lessmode_toggle 	
All	 Control+Shift      	 V          	 clippaste 	
All	 Control+Shift      	 Y          	 selpaste 	
All	 All                	 Break      	 sendbreak 	
All	 All                	 Print      	 printsel 	
All	 All                	 Scroll_Lock 	 lessmode_toggle 	
All	 Control            	 0          	 scrollmark 	
All	 Control            	 1          	 scrollmark 	
All	 Control            	 2          	 scrollmark 	
All	 Control            	 3          	 scrollmark 	
All	 Control            	 4          	 scrollmark 	
All	 Control            	 5          	 scrollmark 	
All	 Control            	 6          	 scrollmark 	
All	 Control            	 7          	 scrollmark 	
All	 Control            	 8          	 scrollmark 	
All	 Control            	 9          	 scrollmark 	
All	 Control            	 F1         	 showhelp 	
All	 Control            	 Print      	 toggleprinter 	
All	 Control+Win        	 0          	 set_charmap 	
All	 Control+Win        	 1          	 set_charmap 	
All	 Control+Win        	 2          	 set_charmap 	
All	 Control+Win        	 3          	 set_charmap 	
All	 Control+Win        	 4          	 set_charmap 	
All	 Control+Win        	 5          	 set_charmap 	
All	 Control+Win        	 6          	 set_charmap 	
All	 Control+Win        	 7          	 set_charmap 	
All	 Control+Win        	 8          	 set_charmap 	
All	 Control+Win        	 9          	 set_charmap 	
All	 Shift              	 BackSpace  	 retmark 	
All	 Shift              	 Down       	 kscrolldown 	
All	 Shift              	 End        	 scrolltobottom 	
All	 Shift              	 Home       	 scrolltotop 	
All	 Shift              	 Insert     	 selpaste 	
All	 Shift              	 Page_Down  	 kscrolldown 	
All	 Shift              	 Page_Up    	 kscrollup 	
All	 Shift              	 Print      	 printscreen 	
All	 Shift              	 Return     	 enterscroll 	
All	 Shift              	 Up         	 kscrollup 	
All	 Shift+Alt          	 Delete     	 set_fontwidth 	
All	 Shift+Alt          	 End        	 set_fontwidth 	
All	 Shift+Alt          	 Home       	 zoomreset 	
All	 Shift+Alt          	 Insert     	 set_fontwidth 	
All	 Shift+Alt          	 Page_Down  	 zoom 	
All	 Shift+Alt          	 Page_Up    	 zoom 	
Help	 All                	 ALL_KEYS   	 dummy 	
Help	 All                	 Escape     	 showhelp 	
Help	 All                	 q          	 showhelp 	
Less	 All                	 0          	 scrollmark 	
Less	 All                	 1          	 scrollmark 	
Less	 All                	 2          	 scrollmark 	
Less	 All                	 3          	 scrollmark 	
Less	 All                	 4          	 scrollmark 	
Less	 All                	 5          	 scrollmark 	
Less	 All                	 6          	 scrollmark 	
Less	 All                	 7          	 scrollmark 	
Less	 All                	 8          	 scrollmark 	
Less	 All                	 9          	 scrollmark 	
Less	 All                	 BackSpace  	 retmark 	
Less	 All                	 Down       	 kscrolldown 	
Less	 All                	 End        	 scrolltobottom 	
Less	 All                	 Escape     	 lessmode_toggle 	
Less	 All                	 Home       	 scrolltotop 	
Less	 All                	 Page_Down  	 kscrolldown 	
Less	 All                	 Page_Up    	 kscrollup 	
Less	 All                	 Up         	 kscrollup 	
Less	 All                	 q          	 lessmode_toggle 	
Less	 Shift              	 Return     	 lessmode_toggle 	



OPTIONS
=======

**-a**
   disable alternate screens in terminal

**-c** *class*
   defines the window class (default $TERM).

**-f** *font*
   defines the *font* to use when st is run.

**-g** *geometry*
   defines the X11 geometry string. The form is
   [=][<cols>{xX}<rows>][{+-}<xoffset>{+-}<yoffset>]. See
   **XParseGeometry** (3) for further details.

**-i**
   will fixate the position given with the -g option.

**-n** *name*
   defines the window instance name (default $TERM).

**-o** *iofile*
   writes all the I/O to *iofile.* This feature is useful when recording
   st sessions. A value of "-" means standard output.

**-T** *title*
   defines the window title (default 'st').

**-t** *title*
   defines the window title (default 'st').

**-w** *windowid*
   embeds st within the window identified by *windowid*

**-l** *line*
   use a tty *line* instead of a pseudo terminal. *line* should be a
   (pseudo-)serial device (e.g. /dev/ttyS0 on Linux for serial port 0).
   When this flag is given remaining arguments are used as flags for
   **stty(1).** By default st initializes the serial line to 8 bits, no
   parity, 1 stop bit and a 38400 baud rate. The speed is set by
   appending it as last argument (e.g. 'st -l /dev/ttyS0 115200').
   Arguments before the last one are **stty(1)** flags. If you want to
   set odd parity on 115200 baud use for example 'st -l /dev/ttyS0
   parenb parodd 115200'. Set the number of bits by using for example
   'st -l /dev/ttyS0 cs7 115200'. See **stty(1)** for more arguments and
   cases.

**-v**
   prints version information, then exits.

**-V** 
   prints version and compile information, then exits

**-e** *command* **[** *arguments* **... ]**
   st executes *command* instead of the shell. If this is used it **must
   be the last option** on the command line, as in xterm / rxvt. This
   option is only intended for compatibility, and all the remaining
   arguments are used as a command even without it.

**-x**
   enable reading of the XResources database for the configuration
   st-asc must have been compiled with the XRESOURCES flag in config.h.in set to 1
   
**-X**
   lock all memory pages into memory, prevent swapping.
   Secrets could be revealed, also years later, if the memory
   is swapped to disk. Worse, with flash disks also erasing
   the contents will not necessarily erase the written cells.
   This option locks all memory pages into ram.


Keyboard selection mode
=======================

(Patch by Tonton Couillon - la dot luge at free dot fr)
   
When you run "keyboard\_select", you have 3 modes available:

  - move mode:    to set the start of the selection;
  - select mode:  to activate and set the end of the selection;
  - input mode:   to enter the search criteria.

Shortcuts for move and select modes:
    
   
:h, j, k, l:      move cursor left/down/up/right (also with arrow keys)
:!, _, \*:        move cursor to the middle of the line/column/screen
:Backspace, $:    move cursor to the beginning/end of the line
:PgUp, PgDown:    move cursor to the beginning/end of the column
:Home, End:       move cursor to the top/bottom left corner of the screen
:/, ?:            activate input mode and search up/down
:n, N:            repeat last search, up/down
:s:               toggle move/selection mode
:t:               toggle regular/rectangular selection type
:Return:          quit keyboard_select, keeping the highlight of the selection
:Escape:          quit keyboard_select
 
      

With h,j,k,l (also with arrow keys), you can use a quantifier. Enter a
number before hitting the appropriate key.

Shortcuts for input mode:

Return:       Return to the previous mode



CUSTOMIZATION
=============

**slterm** can be customized by editing makefile.config and src/config.h,
afterwards (re)compiling the source code, or by editing the Xresources init files and 
compiling slterm with Xresources enabled.

AUTHORS
=======

Michael (misc) Myer, www.github.com/michael105

See README and PATCHES for other authors.

LICENSE
=======

See the LICENSE file for the terms of redistribution.

SEE ALSO
========

**tabbed**\ (1), **utmp**\ (1), **stty**\ (1)

BUGS
====

See the README in the distribution.
