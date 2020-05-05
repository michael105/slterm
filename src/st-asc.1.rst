========
 st-asc
========

(s)imple (t)erminal - (asc)ii fork


SYNOPSIS
========

**st-asc** [**-aivVx**] [**-c** *class*] [**-f** *font*] [**-g** *geometry*]
[**-n** *name*] [**-o** *iofile*] [**-T** *title*] [**-t** *title*]
[**-l** *line*] [**-w** *windowid*] [[**-e**] *command*
[*arguments*...]]

**st-asc** [**-aivVx**] [**-c** *class*] [**-f** *font*] [**-g** *geometry*]
[**-n** *name*] [**-o** *iofile*] [**-T** *title*] [**-t** *title*]
[**-w** *windowid*] -l *line* [*stty_args*...]

DESCRIPTION
===========

**st-asc** is a virtual terminal emulator for X
It is a slimmed down clone of st (suckless.org),
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
   Toggle "less mode": when enabled, Cursor Up/Down and PageUp/PageDown
   will scroll up / down, without having hold Shift

**Ctrl-Shift-s** 
   Enter keyboard selection mode
   (Described below, with a different key mapping)

**Shift-PgUp/PgDown**
   Scroll Up/Down a Page

**Shift-Up/Down**
   Scroll Up/Down three lines

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

**st-asc** can be customized by editing config.h.in and (re)compiling
the source code, or by editing the Xresources init files and 
compiling st-asc with Xresources enabled.

AUTHORS
=======

See README and PATCHES for the authors.

LICENSE
=======

See the LICENSE file for the terms of redistribution.

SEE ALSO
========

**tabbed**\ (1), **utmp**\ (1), **stty**\ (1)

BUGS
====

See the README in the distribution.
