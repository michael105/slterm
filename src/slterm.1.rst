========
 slterm
========

The 'slim terminal'


SYNOPSIS
========

**slterm** [**-aivVx**] [**-c** *class*] [**-f** *font*] [**-g** *geometry*]
[**-n** *name*] [**-o** *iofile*] [**-T** *title*] [**-t** *title*]
[**-l** *line*] [**-w** *windowid*] [[**-e**] *command*
[*arguments*...]]

**slterm** [**-aivVx**] [**-c** *class*] [**-f** *font*] [**-g** *geometry*]
[**-n** *name*] [**-o** *iofile*] [**-T** *title*] [**-t** *title*]
[**-w** *windowid*] -l *line* [*stty_args*...]


DESCRIPTION
===========

**slterm** is a virtual terminal emulator for X.
It is originally based on st (suckless.org),
utf-8 and rgb support stripped for better performance,
with several additions and patches.


KEYS
====


**(Anymod)+F1**  
   Show the reference of keybindings
   'Anymod' can be any combination of modification keys (Ctrl, Alt,..)
   Added to show the internal help, also when F1 has been bound to
   e.g. the window manager

**Ctrl+Shift+c**
   Copy the selected text to the clipboard selection.

**Ctrl+Shift+v**
   Paste from the clipboard selection.

**Ctrl+Shift+y**
   Paste from primary selection (middle mouse button).

**Ctrl+Shift+l**
   Toggle "less mode": when enabled, Cursor Up/Down, PageUp/PageDown, Home/End
   will scroll up / down, without holding Shift

**Shift+Backspace**
   Enable lessmode and scroll back to the line, 
   the last command has been entered in the shell

**Shift+Enter** 
   Execute command, enter lessmode when more than
   one screen is displayed by the command.

**Ctrl+Shift + [Up/Down/PageUp/Pagedown]**
   Scroll, and enter lessmode

**Ctrl+Alt + [0..9]** 
   Set Scrollmark 0 - 9

**Ctrl + [0..9]**     
   Scroll to mark 0 - 9

**Ctrl+Shift+s** 
   Enter keyboard selection mode
   (Described below, with a different key mapping)

**Shift+PgUp/PgDown**
   Scroll Up/Down a Page

**Shift+Up/Down**
   Scroll Up/Down three lines

**Shift+Home/End**
   Scroll to top / bottom

**Break**
   Send a break in the serial line. Break key is obtained in PC
   keyboards pressing at the same time control and pause.

**Ctrl+Print Screen**
   Toggle if st should print to the *iofile.*

**Shift+Print Screen**
   Print the full screen to the *iofile.*

**Print Screen**
   Print the selection to the *iofile.*

**Alt+Shift + Insert/Delete**   
   Enlarge/Shrink font width

**Alt+Shift + PageUp/PageDown** 
   Zoom in / out

**Ctrl+Shift + I** 
   Inverse colors

**Ctrl+Shift+Home**
   Reset to default font size.

**Ctrl+Win + [0..9]**
   Switch charmaps


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
   slterm must have been compiled with the XRESOURCES flag in config.h.in set to 1


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

**slterm** can be customized by editing config.in and (re)compiling
the source code, or by editing the Xresources init files and 
compiling slterm with Xresources enabled.


AUTHORS
=======

Based on Aurelien APTEL <aurelien dot aptel at gmail dot com> bt source code.

The code has been hosted and maintained by the suckless project.

Applied patches are written by:

  - Tonton Couillon - \<la dot luge at free dot fr\>
  - Jochen Sprickerhof - <st@jochen.sprickerhof.de>
  - M Farkas-Dyck - <strake888@gmail.com>
  - Ivan Tham - <pickfire@riseup.net> (mouse scrolling)
  - Ori Bernstein - <ori@eigenstate.org> (fix memory bug)
  - Matthias Schoth - <mschoth@gmail.com> (auto altscreen scrolling)
  - Laslo Hunhold - <dev@frign.de> (unscrambling, git port)
  - Paride Legovini - <pl@ninthfloor.org> (don't require the Shift
    modifier when using the auto altscreen scrolling)
  - Lorenzo Bracco - <devtry@riseup.net> (update base patch, use static
    variable for config)
  - Kamil Kleban - <funmaker95@gmail.com> (fix altscreen detection)
  - Avi Halachmi - <avihpit@yahoo.com> (mouse + altscreen rewrite after
    `a2c479c`)
  - Jacob Prosser - <geriatricjacob@cumallover.me>
  - Augusto Born de Oliveira - <augustoborn@gmail.com>
  - Kai Hendry - <hendry@iki.fi>
  - Laslo Hunhold - <dev@frign.de> (git port)
  - Matthew Parnell - <matt@parnmatt.co.uk> (0.7, git ports)
  - Doug Whiteley - <dougwhiteley@gmail.com>
  - Aleksandrs Stier
  - @dcat on [Github](https://github.com/dcat/st-xresources)
  - Devin J. Pohly - <djpohly@gmail.com> (git port)
  - Sai Praneeth Reddy - <spr.mora04@gmail.com> (read borderpx from
    xresources)


All other additions, performance optimizations, 
and the reorganization of the source files
has done Michael (misc) Myer. 
(2020-23 / misc.myer@zoho.com / https://github.com/michael105)

(My apologies for not pushing the work back to suckless,
but the heavy changes and the not so simple additions
let me seem this neither easy nor following the suckless philosophy;
and it wouldn't be possible to submit "patches" anymore)


LICENSE
=======

MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


BUGS
====

Clipboard copy/paste of characters > 127 currently doesn't communicate correctly
with Xorg programs. 

The history ringbuffer could get problematic in conjunction with the scrollmarks when circled. (atm, the default history has 65536 lines, so it's not at the top of the todo list)

Under special circumstances the alternate buffer crashes. Yet, I couldn't reproduce
the problem, when I looked for it. If someone is able to spot the factors, please drop me a note.


