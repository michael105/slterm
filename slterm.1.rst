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

It is a slimmed down descendant of st (suckless.org)
with several additions and patches.

utf-8 and rgb support are stripped for better performance and less memory usage.


USAGE
=====

The most important key combination might be

**Ctrl+F1** 
  Show a help screen with all shortcuts.




The usage of slterm differs from other terminal emulators by its different modes.

In selection mode it is possible to select lines or areas (either by lines,
or rectangular areas).
enter with Alt+S, start selecting with v or s and the cursor keys, 
switch between rectangular selection and line selection with t,
copy to the clipboard with Enter.
y selects the current line, yy (sort of vim binding) selects the current line
and yanks it to the clipboard.

In lessmode it is possible to browse in the scrollback buffer via 
the standard less like keybindings.

Ctrl+Shift Up/Down/PgUp/PdDown enter lessmode 
Shift+Backspace enters lessmode and browses back to the last line, 
a command has been entered.

in lessmode, it is possible to browse between the marks of commands,
comfirmed via enter, via Backspace or Tab left and Tab.

Shift + Enter in Normal mode enters lessmode, if the output
of a command is more than a screen.

 
INSTALL
=======

   If obtained from source, type make and make install.

   If you downloaded the statically linked binary,
   - copy the binary to a suitable place (/usr/local/bin)
   - install the terminal info file, if needed: (for curses) slterm -I > slterm.info; tic -sx slterm.info
   If needed, download the man page (slterm.1) 
   of the current version from github (github.com/michael105/slterm)
   and copy into the appropiate directory.



OPTIONS
=======

**-a**
   disable alternate screens in terminal

**-c** *class*
   defines the window class (default $TERM).

**-f** *font*
   defines the *font* to use when slterm is run.

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
   slterm sessions. A value of "-" means standard output.

**-T** *title*
   defines the window title (default 'slterm').

**-t** *title*
   defines the window title (default 'slterm').

**-w** *windowid*
   embeds slterm within the window identified by *windowid*

**-l** *line*
   use a tty *line* instead of a pseudo terminal. *line* should be a
   (pseudo-)serial device (e.g. /dev/ttyS0 on Linux for serial port 0).
   When this flag is given remaining arguments are used as flags for
   **stty(1).** By default slterm initializes the serial line to 8 bits, no
   parity, 1 stop bit and a 38400 baud rate. The speed is set by
   appending it as last argument (e.g. 'slterm -l /dev/ttyS0 115200').
   Arguments before the last one are **stty(1)** flags. If you want to
   set odd parity on 115200 baud use for example 'slterm -l /dev/ttyS0
   parenb parodd 115200'. Set the number of bits by using for example
   'slterm -l /dev/ttyS0 cs7 115200'. See **stty(1)** for more arguments and
   cases.

**-v**
   prints version information, then exits.

**-V** 
   prints version and compile information, then exits

**-e** *command* **[** *arguments* **... ]**
   slterm executes *command* instead of the shell. If this is used it **must
   be the last option** on the command line, as in xterm / rxvt. This
   option is only intended for compatibility, and all the remaining
   arguments are used as a command even without it.

**-x**
   enable reading of the XResources database for the configuration
   slterm must have been compiled with the XRESOURCES flag in config.h.in set to 1
   
**-X**
   lock all memory pages into memory, prevent swapping.
   Secrets could be revealed, also years later, if the memory
   is swapped to disk. Worse, with flash disks also erasing
   the contents will not necessarily erase the written cells.
   This option locks all memory pages into ram.



CUSTOMIZATION
=============

**slterm** can be customized by editing config.make and src/config.h,
afterwards (re)compiling the source code, or by editing the Xresources init files and 
compiling slterm with Xresources enabled.

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
