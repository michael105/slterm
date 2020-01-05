### st-asc

Fork of the st terminal. (suckless.org)

Stripped of unicode support.
Unicode encoding needs 4 Bytes per char within st,
and I nearly never need unicode chars in the terminal.
(I'd even propose, unicode isn't needed for system administration/
development. You are going to get into all sort of troubles,
when, e.g. you're naming files and directories in the root fs
with unicode characters..)


So, in my quest to slim down all programs I'm using,
I'm about to strip unicode and utf8 support.

Yet I managed to get a memory footprint of around 8MB. 
(>20MB before)
I'm always keeping more than 10 terminals open,
so that sums up.

The smaller memory footprint also pays out in a more responsive
system overall, improving st's speed as well. (3x here, and close to urxvt now)

It's however a crude hack, much (unused) utf8 supporting code is left yet.
Hopefully the compiles does it's job eliminating unneeded potions.

Me, I'm going to strip of the rgb color support. 
Which, again, count's with 2 integers ( 8 Bytes ) per char.

256 colors might be enough for everyone.
(Am I confusing something..?..)


(misc 2020)

---- snip ----

#### roadmap

- strip unicode (done partially)
- strip rgb colors
- gain earth domination
- quench vogones
- stifle this laughter
-

xxxx
---- snip snip -----



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

