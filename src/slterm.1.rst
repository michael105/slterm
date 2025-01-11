========
 slterm
========

(sl)im (term)inal 


SYNOPSIS
========


**slterm** **-I** **-H** **-L** **-h** 

**slterm** [**-aivVx**] [**-c** *class*] [**-f** *font*] [**-g** *geometry*]
[**-n** *name*] [**-o** *iofile*] [**-T** *title*] [**-t** *title*]
[**-l** *line*] [**-w** *windowid*] [**-X**] 
[[**-e**] *command*] [*arguments*...]]

**slterm** [**-aivVx**] [**-c** *class*] [**-f** *font*] [**-g** *geometry*]
[**-n** *name*] [**-o** *iofile*] [**-T** *title*] [**-t** *title*]
[**-w** *windowid*] -l *line* [*stty_args*...]


DESCRIPTION
===========

**slterm** is a virtual terminal emulator for X

slterm is highly optimized for fast text output and slim memory usage,
while having several addons like switchable codepages or advanced
history browsing.

In favor of compatibility with terminal programs and performance utf8
support is stripped. Communication with the x clipboard is however
automatically converted from and to utf8.


USAGE
=====

The most important key combination might be

**Ctrl+F1** 
  Show a help screen with all shortcuts.


The usage of slterm differs to other terminal emulators by its three different main modes.


normalmode
----------

The codepage can be changed via hotkeys. (`Ctrl+Shift+0..9`).

Paste from the clipbard with `Shift+Insert`.

Set Bookmarks to the current line with `Ctrl+Shift+0..9`
Scroll back to a bookmark with `Ctrl+0..9`

Retmarks are atomatically stored, and can be browsed to later.
Every time, a command is entered with return, a retmark is stored.
Switch into lessmode, and browse to the location of the
last entered command with `Shift+backspace`.

`Ctrl+Shift` + `Up`/`Down`/`PageUp`/`PageDown` switch to lessmode.


lessmode
--------

In lessmode it is possible to browse the scrollback buffer via 
the standard less like keybindings.

`Ctrl+Shift Up/Down/PgUp/PdDown` enter lessmode 
`Shift+Backspace` enters lessmode and browses back to the last line, 
a command has been entered.

In lessmode, it is possible to browse between the marks of commands,
comfirmed with `Enter` in the shell via `Backspace`/`Tab left` and `Tab`.
`1` to `9` scroll back to the according numbered retmark.


`Shift+Enter` in Normal mode enters lessmode, if the output
of a command is more than a screen.

`q` leaves lessmode and switches to normalmode again.



selectionmode
-------------

In selection mode it is possible to select lines or areas (either by lines,
or rectangular areas).
enter with `Alt+S`, start selecting with `v`, `s` or `V` (rectangular selection) 
and the cursor keys, 
switch between rectangular selection and line selection with `t`,
copy to the clipboard with `Enter`.
`y` selects the current line, `y` with a active selection yanks the
selection to the clipboard and exits selection mode,
`yy` (sort of vim binding) selects the current line
and yanks it to the clipboard.
`p` copies the selection and pastes it into the terminal.
`q` or `ESC` exit selection mode.



all modes
---------
::

   Ctrl+Shift + I: Inverse colors
   Ctrl+F1:    Show this help
   
   Set font width/size:
   Alt+Shift + Insert/Delete:   Enlarge/Shrink width
   Alt+Shift + PageUp/PageDown: Zoom in / out
   Alt+Shift + Home:            Reset font display
   
   Select Codepage: `Ctrl+Win` + 
   0 CP1250
   1 CP1251
   2 CP1252 (this is mostly ANSI, and the 1. page of Unicode)
   3 CP1253
   4 CP437  (Old IBM codetable, borders and tables)
   5 CP850  (DOS Standard table)
   6 CP4002 (Custom table, mix of 1252 and 437)


TERMINAL
========


slterm is mostly VT100 compatible, please look in question for the according manuals.
Here only some of the vt100 or slterm additions are described.

slterm starts either the env variable SHELL, /bin/sh, or a command supplied with
`slterm -e command`.

Several Variables are exported into the environment.


Colors
------

The basic 8 colors, accessable in the shell as:
$BLACK, $RED, $GREEN, $YELLOW, $BLUE, $MAGENTA, $CYAN, $WHITE.

> echo $RED red text


The can be combined with prefixes: L(ight), D(ark), LD(light-dark), BD(background).
L: $LGREEN, D: $DRED, $LD: $LDCYAN, BG: $BGBLUE.

These Colors cannot be combined: ORANGE, ORANGERED, BROWN, BGBROWN, PURPLE, GRAY, MINT, TURQUOISE

Other text attributes are: $BOLD, $FAINT, $CURSIVE, $UNDERLINE, $BLINK, $REVERSE, $STRIKETHROUGH, $DOUBLEUNDERLINE.

The text attributes can be combined, with some special combinations:
BLINK and REVERSE blinks by reversing colors.
STRIKETHROUGH and UNDERLINE get a double underline.


The default foreground and background color and attributes can be reset with $NORMAL.


255 Colors can be set with: 

- foreground: echo -e "\e[38;5;XXm", XX one of 0 - 255.
- background: echo -e "\e[48;5;XXm", XX one of 0 - 255.


Cursor
------

There are several cursor shapes, set with: echo -e "\e[X q".
X one of 0..12:

* 1,2: block cursor
* 3,4: underline
* 5,6: vertical bar


slterm additions:
~~~~~~~~~~~~~~~~~


* 7:   'X'
* 7;Y: Y is the ascii code of the char, used as cursor
* 8:   double underline
* 9:   empty block
* 10:  underline, two lines at the sides
* 11:  underline and overline, lines right and left
* 12:  overline, lines right and left



Bell
----

Sending a bell to the terminal (echo -e "\007") sends 
the according notification (XBell) to the window manager.




 
INSTALL
=======

If obtained from source, edit the files config.make and config.h
to customize slterm. Type `make`, and `make install`.

If you downloaded the statically linked binary,

1. copy the binary to a suitable place (/usr/local/bin)

2. install the terminal info file: (for curses) `slterm -I | tic -sx -` 
   ( the netbsd version of tic, the terminal info compiler,  is supplied as source in 
   tools/tic, and should be compatible with other curses versions )

3. If needed, download this man page in its man format (slterm.1) 
   from github (github.com/michael105/slterm), 
   copy into the appropiate directory (/usr/local/share/man/man1)


Interestingly, the statically linked binary seems to use even less memory 
than the shared binary. 


CURSES
------
     
   To be used with curses, the installation of the terminfo database file is needed.
   slterm.terminfo is supplied in the sources, within the folder src.
   It can be installed with `tic -sx slterm.terminfo`.
   Alternatively, the termcap database "linux" is mostly compatible.
   Set with `export TERM=linux`

   The terminfo database of slterm is also displayed, when slterm was compiled with
   EMBEDRESOURCES. Type `slterm -I`, to install: `slterm -I | tic -sx -`.
   The key combination `Ctrl+Shift+Win+ALT+I` will dump the terminal info to
   the terminal as well, and can be used, to install the terminal info within
   a remote shell. ( type `tic -sx -`, hit `Ctrl+Shift+Win+Alt+I`, and `Ctrl+D` )
   

   Tic is the terminfo compiler, available from the curses distributions,
   the netbsd tic implementation is supplied within tools/tic.
   There is a statically linked binary for linux, 64bit of tic at
   github.com/michael105/static-bin 
   Sources of tic and netbsd curses: github.com/oasislinux/netbsd-curses/


OPTIONS
=======

-h show short option usage

-H Display this manpage as text

-L show license

-a
   disable alternate screens in terminal

-c class
   defines the window class (default $TERM).

-f font
   defines the font to use when slterm is run.
   example: slterm -f 'Liberation Mono:Bold:pixelsize=13:antialias=true:autohint=true'
   the parameters are described in the fontconfig documentation, 
   an overview is supplied in doc/fontconfig.txt

-g geometry
   defines the X11 geometry string. The form is
   [=][<cols>{xX}<rows>][{+-}<xoffset>{+-}<yoffset>]. See
   XParseGeometry (3) for further details.

-i
   will fixate the position given with the -g option.

-n name
   defines the window instance name (default $TERM).

-o iofile
   writes all the I/O to iofile. This feature is useful when recording
   slterm sessions. A value of "-" means standard output.

-T title
   defines the window title (default 'slterm').

-t title
   defines the window title (default 'slterm').

-w windowid
   embeds slterm within the window identified by windowid

-l line
   use a tty line instead of a pseudo terminal. line should be a
   (pseudo-)serial device (e.g. /dev/ttyS0 on Linux for serial port 0).
   When this flag is given remaining arguments are used as flags for
   stty(1). By default slterm initializes the serial line to 8 bits, no
   parity, 1 stop bit and a 38400 baud rate. The speed is set by
   appending it as last argument (e.g. 'slterm -l /dev/ttyS0 115200').
   Arguments before the last one are stty(1) flags. If you want to
   set odd parity on 115200 baud use for example 'slterm -l /dev/ttyS0
   parenb parodd 115200'. Set the number of bits by using for example
   'slterm -l /dev/ttyS0 cs7 115200'. See stty(1) for more arguments and
   cases.

-v
   prints version information, then exits.

-V 
   prints version and compile information, then exits

-e command [ arguments ... ]
   slterm executes command instead of the shell. If this is used it must
   be the last option on the command line, as in xterm / rxvt. This
   option is only intended for compatibility, and all the remaining
   arguments are used as a command even without it.

-x
   enable reading of the XResources database for the configuration
   slterm must had been compiled with the XRESOURCES flag in config.make set to 1
   
-X
   lock all memory pages into memory, prevent swapping.
   Secrets could be revealed, also years later, if the memory
   is swapped to disk. Worse, with flash disks also erasing
   the contents will not necessarily erase the written cells.
   This option locks all memory pages into ram.


AUTHORS
=======

(2020-2025) Michael (misc147), www.github.com/michael105

The code is based on st, the suckless terminal emulator,
fetched from git 1.1.2020, which was based on code from Aurelien Aptel.

The included patches to st had been provided by: 

Tonton Couillon,
dcat, 
Jochen Sprickerhof,
M Farkas-Dyck,
Ivan Tham,
Ori Bernstein,
Matthias Schoth,
Laslo Hunhold,
Paride Legovini,
Lorenzo Bracco,
Kamil Kleban,
Avi Halachmi,
Jacob Prosser,
Augusto Born de Oliveira,
Kai Hendry,
Laslo Hunhold,
Matthew Parnell,
Doug Whiteley,
Aleksandrs Stier,
Devin J. Pohly,
Sai Praneeth Reddy
 


LICENSE
=======

MIT, see the LICENSE file for the terms of redistribution or type slterm -L

SEE ALSO
========

**tabbed**\ (1), **utmp**\ (1), **stty**\ (1)

BUGS
====

See the README in the distribution.


