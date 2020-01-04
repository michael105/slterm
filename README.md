#### st-aes

Fork of the st terminal. (suckless.org)

Stripped of unicode support.
Unicode encoding needs 4 Bytes per char within st,
and I nearly never need unicode chars in the terminal.

So, in my quest to slim down all programs I'm using,
I'm about to strip unicode and utf8 support.

Yet I managed to get a memory footprint of around 8MB. 
(>20MB before)
I'm always keeping more than 10 terminals open,
so that sums up.

The smaller memory footprint also pays out in a more responsive
system overall, st's printing speed gets far higher as well.

(Close to urxvt now).





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

