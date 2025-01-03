========
 slterm
========

(sl)im (term)inal 


SYNOPSIS
========


**slterm** **-I** **-H** **-L** **-h** 

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

It is a slimmed down descendant of st (suckless.org)
with several additions and patches.

utf-8 and rgb support are stripped for better performance and less memory usage.


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


lessmode
--------

In lessmode it is possible to browse the scrollback buffer via 
the standard less like keybindings.

`Ctrl+Shift Up/Down/PgUp/PdDown` enter lessmode 
`Shift+Backspace` enters lessmode and browses back to the last line, 
a command has been entered.

In lessmode, it is possible to browse between the marks of commands,
comfirmed with `Enter` in the shell via `Backspace` or `Tab left` and `Tab`.
`1` to `9` scroll back to the according numbered retmark.


`Shift+Enter` in Normal mode enters lessmode, if the output
of a command is more than a screen.


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



TERMINAL
========


slterm is mostly VT100 compatible, please look in question for the according manuals.
Here only some of the vt100 or slterm additions are described.

slterm starts either the env variable SHELL, /bin/sh, or a command supplied with
`slterm -e command`.

Several Variables are exported into the environment.


Colors
------

The basic 8 colors, accesible in the shell as:
$BLACK, $RED, $GREEN, $YELLOW, $BLUE, $MAGENTA, $CYAN, $WHITE.

The can be combined with prefixes: L(ight), D(ark), LD(light-dark), BD(background).
L: $LGREEN, D: $DRED, $LD: $LDCYAN, BG: $BGBLUE.

These Colors cannot be combined: ORANGE, ORANGERED, BROWN, BGBROWN, PURPLE, GRAY, MINT, TURQUOISE

Other text attributes are: $BOLD, $FAINT, $CURSIVE, $UNDERLINE, $BLINK, $REVERSE, $STRIKETHROUGH, $DOUBLEUNDERLINE.

The text attributes can be combined, with some special combinations:
BLINK and REVERSE blinks by reversing colors.
STRIKETHROUGH and UNDERLINE get a double underline.


The default foreground and background color and attributes can be reset with $NORMAL.


255 Colors can be set with: 

- foreground: "\e[38;5;XXm", XX one of 0 - 255.
- background: "\e[48;5;XXm", XX one of 0 - 255.


Cursor
------

There are several cursor shapes, set with: "\e[X q".
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
3. If needed, download this man page in its man format (slterm.1) 
   from github (github.com/michael105/slterm), 
   copy into the appropiate directory (/usr/local/share/man/man1)



CURSES
------
     
   To be used with curses, the installation of the terminfo database file is needed.
   slterm.terminfo is supplied in the sources, within the folder src.
   It can be installed with `tic -sx slterm.terminfo`.
   Alternatively, the termcap database "linux" seems to be mostly compatible.
   Set with `export TERM=linux`

   The terminfo database of slterm is also displayed, when slterm was compiled with
   EMBEDRESOURCES. Type `slterm -I`, to install: `slterm -I | tic -sx -`.
   The key combination `Ctrl+Shift+Win+ALT+I` will dump the terminal info to
   the terminal as well, and can be used, to install the terminal info within
   a remote shell. ( type `tic -sx -`, hit `Ctrl+Shift+Win+Alt+I`, and `Ctrl+D` )
   

   Tic is the terminfo compiler, available from the curses distributions.
   There is a statically linked binary for linux, 64bit of tic at
   github.com/michael105/static-bin 
   (125kB, sha3sum: 510f25bdb35c437c0bc28690a6d292f128113144fee93cf37b01381c)
   Sources of tic and netbsd curses: github.com/oasislinux/netbsd-curses/


OPTIONS
=======

-a
   disable alternate screens in terminal

-c class
   defines the window class (default $TERM).

-f font
   defines the font to use when slterm is run.

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
   slterm must have been compiled with the XRESOURCES flag in config.h.in set to 1
   
-X
   lock all memory pages into memory, prevent swapping.
   Secrets could be revealed, also years later, if the memory
   is swapped to disk. Worse, with flash disks also erasing
   the contents will not necessarily erase the written cells.
   This option locks all memory pages into ram.


AUTHORS
=======

(2020-2024) Michael (misc147), www.github.com/michael105

The code is based on st, the suckless terminal emulator,
fetched from git 1.1.2020, which was based on code from Aurelien Aptel.

The patches to slterm had been provided by: 

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


