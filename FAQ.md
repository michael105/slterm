(git version 2020/01 suckless.org)

#### Why does st not handle utmp entries?

Use the excellent tool of [utmp](https://git.suckless.org/utmp/) for this task.

#### Some _random program_ complains that st is unknown/not recognised/unsupported/whatever!

It means that st does not have any terminfo entry on your system. Chances are
you did not `make install`. If you just want to test it without installing it,
you can manually run `tic -sx st.info`. (misc24: the file is named slterm.info)

#### Nothing works, and nothing is said about an unknown terminal!

* Some programs just assume they’re running in xterm i.e. they don’t rely on
  terminfo. What you see is the current state of the “xterm compliance”.
* Some programs don’t complain about the lacking st description and default to
  another terminal. In that case see the question about terminfo.

#### How do I scroll back up?

Using a terminal multiplexer.

* `st -e tmux` using C-b [
* `st -e screen` using C-a ESC

#### Why doesn't the Del key work in some programs?

Taken from the terminfo manpage:

	If the terminal has a keypad that transmits codes when the keys
	are pressed, this information can be given. Note that it is not
	possible to handle terminals where the keypad only works in
	local (this applies, for example, to the unshifted HP 2621 keys).
	If the keypad can be set to transmit or not transmit, give these
	codes as smkx and rmkx. Otherwise the keypad is assumed to
	always transmit.

In the st case smkx=E[?1hE= and rmkx=E[?1lE>, so it is mandatory that
applications which want to test against keypad keys send these
sequences.

But buggy applications (like bash and irssi, for example) don't do this. A fast
solution for them is to use the following command:

	$ printf '\033[?1h\033=' >/dev/tty

or
	$ tput smkx

In the case of bash, readline is used. Readline has a different note in its
manpage about this issue:

	enable-keypad (Off)
		When set to On, readline will try to enable the
		application keypad when it is called. Some systems
		need this to enable arrow keys.

Adding this option to your .inputrc will fix the keypad problem for all
applications using readline.

If you are using zsh, then read the zsh FAQ
<http://zsh.sourceforge.net/FAQ/zshfaq03.html#l25>:

	It should be noted that the O / [ confusion can occur with other keys
	such as Home and End. Some systems let you query the key sequences
	sent by these keys from the system's terminal database, terminfo.
	Unfortunately, the key sequences given there typically apply to the
	mode that is not the one zsh uses by default (it's the "application"
	mode rather than the "raw" mode). Explaining the use of terminfo is
	outside of the scope of this FAQ, but if you wish to use the key
	sequences given there you can tell the line editor to turn on
	"application" mode when it starts and turn it off when it stops:

		function zle-line-init () { echoti smkx }
		function zle-line-finish () { echoti rmkx }
		zle -N zle-line-init
		zle -N zle-line-finish

Putting these lines into your .zshrc will fix the problems.

#### How can I use meta in 8bit mode?

St supports meta in 8bit mode, but the default terminfo entry doesn't
use this capability. If you want it, you have to use the 'st-meta' value
in TERM.

#### I cannot compile st in OpenBSD

OpenBSD lacks librt, despite it being mandatory in POSIX
<http://pubs.opengroup.org/onlinepubs/9699919799/utilities/c99.html#tag_20_11_13>.
If you want to compile st for OpenBSD you have to remove -lrt from config.mk, and
st will compile without any loss of functionality, because all the functions are
included in libc on this platform.

#### The Backspace Case

St is emulating the Linux way of handling backspace being delete and delete being
backspace.

This is an issue that was discussed in suckless mailing list
<https://lists.suckless.org/dev/1404/20697.html>. Here is why some old grumpy
terminal users wants its backspace to be how he feels it:

	Well, I am going to comment why I want to change the behaviour
	of this key. When ASCII was defined in 1968, communication
	with computers was done using punched cards, or hardcopy
	terminals (basically a typewriter machine connected with the
	computer using a serial port).  ASCII defines DELETE as 7F,
	because, in punched-card terms, it means all the holes of the
	card punched; it is thus a kind of 'physical delete'. In the
	same way, the BACKSPACE key was a non-destructive backspace,
	as on a typewriter.  So, if you wanted to delete a character,
	you had to BACKSPACE and then DELETE.  Another use of BACKSPACE
	was to type accented characters, for example 'a BACKSPACE `'.
	The VT100 had no BACKSPACE key; it was generated using the
	CONTROL key as another control character (CONTROL key sets to
	0 b7 b6 b5, so it converts H (code 0x48) into BACKSPACE (code
	0x08)), but it had a DELETE key in a similar position where
	the BACKSPACE key is located today on common PC keyboards.
	All the terminal emulators emulated the difference between
	these keys correctly: the backspace key generated a BACKSPACE
	(^H) and delete key generated a DELETE (^?).

	But a problem arose when Linus Torvalds wrote Linux. Unlike
	earlier terminals, the Linux virtual terminal (the terminal
	emulator integrated in the kernel) returned a DELETE when
	backspace was pressed, due to the VT100 having a DELETE key in
	the same position.  This created a lot of problems (see [1]
	and [2]). Since Linux has become the king, a lot of terminal
	emulators today generate a DELETE when the backspace key is
	pressed in order to avoid problems with Linux. The result is
	that the only way of generating a BACKSPACE on these systems
	is by using CONTROL + H. (I also think that emacs had an
	important point here because the CONTROL + H prefix is used
	in emacs in some commands (help commands).)

	From point of view of the kernel, you can change the key
	for deleting a previous character with stty erase. When you
	connect a real terminal into a machine you describe the type
	of terminal, so getty configures the correct value of stty
	erase for this terminal. In the case of terminal emulators,
	however, you don't have any getty that can set the correct
	value of stty erase, so you always get the default value.
	For this reason, it is necessary to add 'stty erase ^H' to your
	profile if you have changed the value of the backspace key.
	Of course, another solution is for st itself to modify the
	value of stty erase.  I usually have the inverse problem:
	when I connect to non-Unix machines, I have to press CONTROL +
	h to get a BACKSPACE. The inverse problem occurs when a user
	connects to my Unix machines from a different system with a
	correct backspace key.

	[1] http://www.ibb.net/~anne/keyboard.html
	[2] http://www.tldp.org/HOWTO/Keyboard-and-Console-HOWTO-5.html

#### But I really want the old grumpy behaviour of my terminal

Apply [1].

[1] https://st.suckless.org/patches/delkey

#### Why do images not work in st (in programs such as w3m)?

This is a terrible hack that overdraws an image on top of the terminal emulator
window. It also relies on a very specific way the terminal draws it's contents.

A more proper (but limited way) would be using sixels. Which st doesn't
support.

#### BadLength X error in Xft when trying to render emoji

Xft makes st crash when rendering color emojis with the following error:

"X Error of failed request:  BadLength (poly request too large or internal Xlib length error)"
  Major opcode of failed request:  139 (RENDER)
  Minor opcode of failed request:  20 (RenderAddGlyphs)
  Serial number of failed request: 1595
  Current serial number in output stream:  1818"

This is a known bug in Xft (not st) which happens on some platforms and
combination of particular fonts and fontconfig settings.

See also:
https://gitlab.freedesktop.org/xorg/lib/libxft/issues/6
https://bugs.freedesktop.org/show_bug.cgi?id=107534
https://bugzilla.redhat.com/show_bug.cgi?id=1498269

The solution is to remove color emoji fonts or disable this in the fontconfig
XML configuration.  As an ugly workaround (which may work only on newer
fontconfig versions (FC_COLOR)), the following code can be used to mask color
fonts:

	FcPatternAddBool(fcpattern, FC_COLOR, FcFalse);

Please don't bother reporting this bug to st, but notify the upstream Xft
developers about fixing this bug.
