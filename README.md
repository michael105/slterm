### st-asc



Fork of the st terminal. (suckless.org)

Stripped of unicode support and rgb colors.

<img align="right" src="images/vt-102-1984.jpg"> 

Applied Patches:

- anysize
- clipboard
- keyboard_select
- relative_border
- scrollback
- scrollback-mouse
- scrollback-mouse-increment
- selectioncolors
- xresources <br>
		added commandline switch -x to enable reading the xresources,<br>
		compile time switch "XRESOURCES"


Unicode encoding needs 4 Bytes per char within st,
and I nearly never need unicode chars in the terminal.

>(I'd even propose, unicode isn't needed for system administration/
development at all. You are going to get into all sort of troubles,
when, e.g., you'd be naming files and directories in the root fs
using unicode characters. Furthermore, let's assume, there are, say,
100.000 instances of st running. Now, on the world. Multiply this
with, umm, 10 MB, multiply with cpu cycles of 100.000 (swapping, and so on),
and . It's not theorethical anymore. Ok. The calculation might be a little bit wrong,
Admittedly, you shouldn't multiply memory and cpu cycles.
Anyways, my point is valid. And there is the computing time to add,
generated by typos. Adding a unicode character in the wrong script could create.. uuh
never mind. But that's the reason to stick to KISS (keep it simple, stupid..) principles. 
Murphy's law will strike, anyways. That's the law..)


So, in my quest to slim down all programs I'm using,
I'm about to strip unicode and utf8 support.

Yet I managed to get a memory footprint of around 8MB.
Ok. Added all patches, and with the current history of 10k lines,
it's at 12MB.
(>20MB before)
I'm always keeping more than 10 terminals open,
so that sums up. What now has two meanings. 
I have to look for what exactly blows that much.
Every Glyph (char at the screen, with attributes and colors) now
needs 4 Bytes. ( around 16Bytes before stripping unicode and rgb)


The smaller memory footprint also pays out in a more responsive
system overall, improving st's speed as well. (3x here) Ok.
I tried hard, to get "benchmarks", the removed unicode support
doesn't profit of. Still, this shows up with a gain of 2x.

I checked several emulators, closest in terms of performance would be 
urxvt. However, testing and comparing more, the speed of urxvt is given
by a (neat) trick. When dumping many chars into the terminal, eg. with cat,
the screen doesn't show every single character. It's more sort of an animation,
showing only enough chars to give the perception of a continous scrolling
terminal. When confronted with, e.g., a `dd if=data bs=1000`;
this cheat doesn't work anymore, dumping the data takes (depending on the size, etc)
up to 20times longer. 
So st seems to be the fastest terminal emulator available. 




It's however a crude hack, much (unused) utf8 supporting code is left yet.
Hopefully the compiler does it's job eliminating unneeded potions.

The rgb color support is stripped.
Counting 2 integers ( 8 Bytes ) per glyph.

;) 256 colors might be enough for everyone.
(Am I confusing something..?..)

![](doc/colors.png?raw=true)

"256" colors (with attributes faint, normal, bold) and the ascii table without control characters as displayed by st-asc.<br>
The scripts for creating the output are within ./test

![](doc/ascii.png)


#### Links

About utf8 
http://doc.cat-v.org/bell_labs/utf-8_history

A comparison on latency, speed and memory consumption of
different terminal emulators. st and urxvt standing out.
So my feelings on st being suitable as base for a slimmed down terminal emulator
have been right.
https://anarc.at/blog/2018-05-04-terminal-emulators-2/

Latency. https://danluu.com/term-latency/

Latency, comparing old (1980) and nowadays systems.
https://danluu.com/input-lag/



---- snip ----

#### roadmap

- strip ~~unicode~~ (done partially)
- ~~strip rgb colors~~
- ~~add patches~~
- gain earth domination
- quench vogones
- stifle laughter

xxxx
---- SNIP -----

(misc 2020, misc.myer@zoho.com )


====================



st - simple terminal
--------------------
st is a simple terminal emulator for X which sucks less.


Requirements
------------
In order to build st you need the Xlib header files.


Installation
------------
Edit config.mk to match your local setup (st is installed into
the /usr/local namespace by default).

Afterwards enter the following command to build and install st (if
necessary as root):

    make clean install


Running st
----------
If you did not install st with make clean install, you must compile
the st terminfo entry with the following command:

    tic -sx st.info

See the man page for additional details.

Credits
-------
Based on Aurélien APTEL <aurelien dot aptel at gmail dot com> bt source code.

