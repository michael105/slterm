####   Mon May 4 20:19:14 2020 +0200
(Michael Myer)

    history doesn't shrink on resize
    
    The history doesn't shrink on resize anymore.
    The content of a larger screen keeps saved.

Notes:

    Not resizing the history to a lower width
    seems to be far more stable.
    Although I guess, the crashes on frequent size changes before
    have to do with internals of the xserver,
    I got quite bored by this.
    I'm using i3, and therefore resize events are happening quite often here.
    Seems stable now.
    I'm still testing. But might be the time for a release now.

####   Mon May 4 19:17:50 2020 +0200
(Michael Myer)

    Mon May  4 19:17:49 CEST 2020

####   Wed Apr 22 09:55:26 2020 +0200
(Michael Myer)

    Scrolling with Shift+Up/Down
    
    Enabled Shift+Up/Down. Scroll Up, down 3 lines.
    (Set in config.h.in)

####   Mon Feb 10 07:50:35 2020 +0100
(Michael Myer)

    minor changes

####   Mon Feb 10 07:45:10 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 21 12:43:01 2020 +0100
(Michael Myer)

    Tue Jan 21 12:43:01 CET 2020

####   Thu Jan 16 22:57:32 2020 +0100
(Michael Myer)

    minor changes

####   Thu Jan 16 16:21:04 2020 +0100
(Michael Myer)

    minor

####   Thu Jan 16 16:19:54 2020 +0100
(Michael Myer)

    Cleanup

####   Thu Jan 16 15:23:59 2020 +0100
(Michael Myer)

    splitting sources

Notes:

    Oh man. Tydied the whole day.
    Now the sun sets down.
    I should take a picture of my workrooms view here.
    I like it. And it's nicely quit. Just the birds.
    Possibly I should think about getting more flexible again.
    Being always in the city is not that good.
    On the other hand, I obviously need the city's life.
    It really is closely related with my life itself.
    IF there's something left. What is not really certain.

####   Thu Jan 16 15:15:44 2020 +0100
(Michael Myer)

    Splitting sources

####   Thu Jan 16 10:03:20 2020 +0100
(Michael Myer)

    remove compile time header

####   Thu Jan 16 10:02:25 2020 +0100
(Michael Myer)

    Cleanup. Split sources

Notes:

    Still cleaning up.
    The former main file, st.c, gets more easy to look through.
    But still is close to 2000 loc's.
    I'm going to remove the usage of globals as far as possible.
    
    I'd like to merge two buf's, the history buf, and the current screen buf.
    Atm, the screen is copied to the history, when scrolling,
    and vice versa.
    That's unneccessary. Would be much nicer to have a "virtual" screen buf just pointing to
    a line in the history buf.
    
    As soon, I have managed to split the according source files further,
    this might be quite more easy to change to.
    The rule, try to keep all your source files below 1000 loc's -
    Albeit I include the source files itself into the main source file,
    splitting the files also into different "namespaces" (virtually)
    is really helpful.
    
    Maybe it's my experience (and love) with perl - there you don't have any rules at all obliged.
    But very soon you'll see, you should give yourself the rules. Of, e.g., "virtual"
    namespaces. It's like working with goto's. Goto's are there, behind
    every language. And in most languages you are able use them.
    And, when optimizing close to hardware level, you should use them.
    The compiler is not always as good as it's being told.
    However, use gotos wisely .. ;) The spaggeti monster isn't only a fairy tale,
    it snoozed you out, and when not being careful, will crunch you again.

####   Thu Jan 16 09:37:09 2020 +0100
(Michael Myer)

    cleanup. utf8 macros.

####   Thu Jan 16 09:26:09 2020 +0100
(Michael Myer)

    cleanup. dir src created

####   Thu Jan 16 09:07:48 2020 +0100
(Michael Myer)

    Cleanup. Indentation

####   Thu Jan 16 09:06:45 2020 +0100
(Michael Myer)

    Splitting sources

####   Thu Jan 16 08:58:07 2020 +0100
(Michael Myer)

    cleanup. splitting sources

####   Wed Jan 15 20:43:17 2020 +0100
(Michael Myer)

    minor changes

####   Sun Jan 12 22:45:39 2020 +0100
(Michael Myer)

    sources separated. working on resize()

####   Sun Jan 12 18:48:24 2020 +0100
(Michael Myer)

    Splitted the source files. Optimizing flag
    
    This is interesting.
    By including all source files into one file,
    and adding the option -fwhole-program to the gcc options,
    the whole thing is quite more responsive.
    So, somehow it seems to me,
    the traditional approach of splitting all sources
    into source (.c) and header file (.h) is a relict
    of times, when every single object file took
    at least several seconds to compile.
    Nowadays, and my development system is quite old -
    this splitting is not needed anymore.
    So it seems to me now.
    At least for "smaller" projects.

####   Sun Jan 12 17:41:17 2020 +0100
(Michael Myer)

    debugging header

####   Sun Jan 12 17:40:39 2020 +0100
(Michael Myer)

    splitting sourcefiles

####   Sun Jan 12 09:01:25 2020 +0100
(Michael Myer)

    tagging 0.9.rc4
    
    Everything seems to work.
    I'm still about to add changes for rebuilding the history
    on resize events, so linewraps and so on will show up correctly.
    Somehow, it's a bit tricky. So I'm going to clean the sources now, also
    separating into more different files.
    Before I tag this 0.9.rc4.
    Somehow I'm fixed on having the linewrap actualization work.
    Yet, all these mallocs and free's give segfaults. :/
    As usual. I personally would have written some things in another way,
    from the beginning. Maybe I should do some further changes.
    Now some inconsistencies show up.
    Having the history and the current screen's line buf separated is one of
    the annoyances to me. It's simply unneccessary.
    But deeply in the sources.
    Anyways - I'm going to do some tidying work now.

####   Sun Jan 12 01:23:09 2020 +0100
(Michael Myer)

    documentation

####   Sun Jan 12 01:23:05 2020 +0100
(Michael Myer)

    documentation

####   Sun Jan 12 01:20:19 2020 +0100
(Michael Myer)

    minor changes

####   Sun Jan 12 01:16:31 2020 +0100
(Michael Myer)

    minor changes

####   Sun Jan 12 01:07:41 2020 +0100
(Michael Myer)

    docu, manpage

####   Sun Jan 12 00:02:52 2020 +0100
(Michael Myer)

    minor changes

####   Sat Jan 11 23:48:05 2020 +0100
(Michael Myer)

    minor changes

####   Sat Jan 11 23:45:07 2020 +0100
(Michael Myer)

    minor changes

####   Sat Jan 11 23:23:44 2020 +0100
(Michael Myer)

    minor changes

####   Sat Jan 11 23:23:02 2020 +0100
(Michael Myer)

    minor changes

####   Sat Jan 11 23:18:35 2020 +0100
(Michael Myer)

    gcc flags

####   Sat Jan 11 23:17:38 2020 +0100
(Michael Myer)

    Cleanup

####   Sat Jan 11 23:16:31 2020 +0100
(Michael Myer)

    minor changes

####   Sat Jan 11 23:16:06 2020 +0100
(Michael Myer)

    minor changes

####   Sat Jan 11 23:12:07 2020 +0100
(Michael (misc) Myer)

    Update README.md

####   Sat Jan 11 23:09:20 2020 +0100
(Michael Myer)

    minor changes

####   Sat Jan 11 23:07:42 2020 +0100
(Michael Myer)

    minor changes

####   Sat Jan 11 23:03:58 2020 +0100
(Michael Myer)

    minor changes

####   Sat Jan 11 23:02:28 2020 +0100
(Michael Myer)

    logging

####   Sat Jan 11 22:31:05 2020 +0100
(Michael Myer)

    Tagging 0.9.rc3 (b)

####   Sat Jan 11 22:11:36 2020 +0100
(Michael Myer)

    needed for shared library

####   Sat Jan 11 22:09:09 2020 +0100
(Michael Myer)

    tag this 0.9rc3. Seems to be stable.

####   Sat Jan 11 21:34:28 2020 +0100
(Michael Myer)

    backup

####   Sat Jan 11 14:11:23 2020 +0100
(Michael Myer)

    Dump more verbose info on version and compile spec

####   Sat Jan 11 12:50:29 2020 +0100
(Michael Myer)

    Debugging switch in config.h.in.
    
    Enable (global) debugging now with the debug flag in config.h.in
    Might be useful for reporting bugs.
    (when encountering a bug, please compile with debugging
    set to level 1, and save the output.
    st-asc > log.txt

####   Sat Jan 11 11:06:36 2020 +0100
(Michael Myer)

    docu

####   Sat Jan 11 11:02:34 2020 +0100
(Michael Myer)

    Optimization for non Utf8.
    
    Copy int32 at once, instead of byte for byte.
    Could be further optimized; but I don't
    want it too machine dependent.
    (Now this depends on unsigned int being 32bit long.
    And could be adapted easily.)
    Byte for Byte, eye for eye. :) I guess,
    this doesn]t work in English.

####   Fri Jan 10 22:55:36 2020 +0100
(Michael Myer)

    save compiler info in the binary

####   Fri Jan 10 22:54:25 2020 +0100
(Michael Myer)

    README

####   Fri Jan 10 19:41:41 2020 +0100
(Michael Myer)

    x.h added, needed for the creation of st-asc as shared library

####   Fri Jan 10 19:39:56 2020 +0100
(Michael Myer)

    Edited the config "system".
    
    Now only one file, config.h.in, needs to be edited.

####   Fri Jan 10 19:36:52 2020 +0100
(Michael Myer)

    Dynamic allocation of the history buffer
    
    Made the allocation of the memory for the hist buf dynamic.
    for, e.g., 16k lines of history, this results in around 6MB Memory
    (depending on the terminal's width);
    which are now allocated dynamically.

Notes:

    if utf8 is enabled, the saving is more than 30/40MB,
    depending on the terminal's width and the history len.

####   Fri Jan 10 17:18:18 2020 +0100
(Michael Myer)

    configuration via one file only

####   Fri Jan 10 16:46:29 2020 +0100
(Michael Myer)

    v0.9rc2

####   Fri Jan 10 16:44:38 2020 +0100
(Michael Myer)

    cleanup

####   Fri Jan 10 16:24:25 2020 +0100
(Michael Myer)

    utf8 compileswitch seems to work

####   Fri Jan 10 02:35:46 2020 +0100
(Michael Myer)

    Optimizations. Option to build a shared version
    
    The optimizations finally start to give better results.
    To name them: No temporary vars for swapping of values,
    instead xor exchange
    No division/modulo, replaced by bit operations

####   Thu Jan 9 21:42:10 2020 +0100
(Michael Myer)

    SWAP via xor

####   Thu Jan 9 19:14:16 2020 +0100
(Michael Myer)

    before changes to history

####   Thu Jan 9 14:13:24 2020 +0100
(Michael Myer)

    scrollback

####   Thu Jan 9 13:47:26 2020 +0100
(Michael Myer)

    History scrollback limited to the history (Not the buf)

####   Thu Jan 9 12:56:47 2020 +0100
(Michael Myer)

    minor changes

####   Thu Jan 9 12:43:45 2020 +0100
(Michael Myer)

    UTF8 Switch

####   Thu Jan 9 10:44:57 2020 +0100
(Michael Myer)

    Compileswitch UTF8.
    
    After thinking about it, it's not so much code for utf8.
    A cleaner codebase would be nicer.
    But on the other hand, having the option to switch to utf8
    might also be nice.
    Has to be implemented as compile time switch.

####   Thu Jan 9 01:05:14 2020 +0100
(Michael Myer)

    Color option for the cursor in unfocused windows added

####   Thu Jan 9 00:12:05 2020 +0100
(Michael Myer)

    Added a nicer cursor for unfocused windows
    
    The default empty rect has been an annoyance to me,
    since I met Linux the first time.

####   Wed Jan 8 22:00:56 2020 +0100
(Michael Myer)

    Replaced division by bitshift
    
    division is the most expensive operation.
    Anyways. doesn't seem to be a big difference.
    1E9 equals 1.000.000.000
    2^30 might be close enough

####   Wed Jan 8 17:34:14 2020 +0100
(Michael Myer)

    update

####   Wed Jan 8 17:34:10 2020 +0100
(Michael Myer)

    update

####   Wed Jan 8 12:25:19 2020 +0100
(Michael Myer)

    documented applied patches

####   Wed Jan 8 10:24:54 2020 +0100
(Michael Myer)

    renamed

####   Wed Jan 8 00:29:01 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 23:57:27 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 23:56:16 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 22:44:14 2020 +0100
(Michael Myer)

    moved pictures to ./images

####   Tue Jan 7 22:43:40 2020 +0100
(Michael Myer)

    moved pictures to ./images

####   Tue Jan 7 22:43:22 2020 +0100
(Michael Myer)

    moved pictures to ./images

####   Tue Jan 7 22:41:37 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 22:28:19 2020 +0100
(Michael Myer)

    vogons

####   Tue Jan 7 21:56:20 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 21:55:47 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 21:55:16 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 21:54:13 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 21:53:41 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 21:53:06 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 21:52:48 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 21:51:42 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 21:49:28 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 21:48:06 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 21:42:40 2020 +0100
(Michael Myer)

    screenshot of i3 and st-asc

####   Tue Jan 7 21:27:13 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 20:56:26 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 20:55:44 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 20:52:11 2020 +0100
(Michael Myer)

    resize

####   Tue Jan 7 20:48:16 2020 +0100
(Michael Myer)

    vt102 picture added

####   Tue Jan 7 20:26:22 2020 +0100
(Michael Myer)

    replaced config.h with symbolic link

####   Tue Jan 7 20:05:58 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 20:01:46 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 19:58:14 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 19:45:43 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 19:43:48 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 19:43:16 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 19:42:30 2020 +0100
(Michael Myer)

    ascii table

####   Tue Jan 7 19:33:25 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 19:32:09 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 19:30:27 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 19:29:21 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 19:28:33 2020 +0100
(Michael Myer)

    minor changes

####   Tue Jan 7 19:26:16 2020 +0100
(Michael Myer)

    ansicolor test added

####   Tue Jan 7 18:35:19 2020 +0100
(Michael Myer)

    Code reformatted

####   Tue Jan 7 17:58:20 2020 +0100
(Michael Myer)

    readme

####   Tue Jan 7 17:57:47 2020 +0100
(Michael Myer)

    readme

####   Tue Jan 7 17:56:51 2020 +0100
(Michael Myer)

    readme

####   Tue Jan 7 17:55:14 2020 +0100
(Michael Myer)

    readme

####   Tue Jan 7 17:54:33 2020 +0100
(Michael Myer)

    readme

####   Tue Jan 7 17:54:02 2020 +0100
(Michael Myer)

    readme (not)

####   Tue Jan 7 17:37:10 2020 +0100
(Michael Myer)

    minor. documentation.

####   Tue Jan 7 17:06:56 2020 +0100
(Michael Myer)

    moved to ./test/

####   Tue Jan 7 17:03:38 2020 +0100
(Michael Myer)

    simple tests added

####   Tue Jan 7 17:03:11 2020 +0100
(Michael Myer)

    simple tests added

####   Tue Jan 7 16:43:11 2020 +0100
(Michael Myer)

    minor

####   Tue Jan 7 16:43:04 2020 +0100
(Michael Myer)

    log

####   Tue Jan 7 14:41:54 2020 +0100
(Michael Myer)

    ISCONTROL simplified

####   Tue Jan 7 14:26:01 2020 +0100
(Michael Myer)

    utf8 file added for tests

####   Tue Jan 7 14:22:13 2020 +0100
(Michael Myer)

    vt102 ascii control char table

####   Tue Jan 7 14:17:10 2020 +0100
(Michael Myer)

    Ascii code tables added for reference

####   Tue Jan 7 13:48:20 2020 +0100
(Michael Myer)

    Makefile simplification

####   Tue Jan 7 13:47:44 2020 +0100
(Michael Myer)

    Makefile simplification

####   Tue Jan 7 13:40:17 2020 +0100
(Michael Myer)

    Makefile cleanup

####   Tue Jan 7 13:40:01 2020 +0100
(Michael Myer)

    man page

####   Tue Jan 7 13:36:02 2020 +0100
(Michael Myer)

    Makefile fix

####   Tue Jan 7 13:23:13 2020 +0100
(Michael Myer)

    Added debug macros.
    
    "Debugged" the handling of German Umlauts.
    Recognized, the problem has been with the shell (zsh)..
    I'll keep the dbg macros. Might need them later again.

####   Tue Jan 7 11:40:34 2020 +0100
(Michael Myer)

    minor

####   Tue Jan 7 11:40:19 2020 +0100
(Michael Myer)

    log

####   Tue Jan 7 05:13:35 2020 +0100
(Michael Myer)

    utf8. Compiler warnings silenced

####   Tue Jan 7 04:47:25 2020 +0100
(Michael Myer)

    regression bug fixed

####   Tue Jan 7 04:06:21 2020 +0100
(Michael Myer)

    minor

####   Tue Jan 7 02:42:21 2020 +0100
(Michael Myer)

    Makefile switch XRESOURCES

####   Tue Jan 7 02:36:14 2020 +0100
(Michael Myer)

    documentation

####   Tue Jan 7 02:30:53 2020 +0100
(Michael Myer)

    Added compile time switch XRESOURCES
    
    needs to have the switch XRESOURCES defined at compile time
    (somehow I don't have a good feeling with this patch

####   Tue Jan 7 02:00:43 2020 +0100
(Michael Myer)

    docu

####   Tue Jan 7 02:00:24 2020 +0100
(Michael Myer)

    Patches applied. Set "Unicode" chars to 1 Byte. rgb stripped

####   Tue Jan 7 01:59:58 2020 +0100
(Michael Myer)

    Applied patches

####   Tue Jan 7 01:59:18 2020 +0100
(Michael Myer)

    update

####   Tue Jan 7 01:59:10 2020 +0100
(Michael Myer)

    docu

####   Tue Jan 7 01:58:54 2020 +0100
(Michael Myer)

    Patches applied. Set "Unicode" chars to 1 Byte. rgb stripped

####   Tue Jan 7 01:57:51 2020 +0100
(Michael Myer)

    Makefile - config.h replaced by link

####   Tue Jan 7 01:45:01 2020 +0100
(Michael Myer)

    Last commit before codebase change

####   Sun Jan 5 20:58:46 2020 +0100
(Michael Myer)

    docu

####   Sun Jan 5 20:34:26 2020 +0100
(Michael Myer)

    removing useless config file

####   Sun Jan 5 20:33:53 2020 +0100
(Michael Myer)

    removing useless config file

####   Sun Jan 5 20:33:31 2020 +0100
(Michael Myer)

    docu

####   Sun Jan 5 14:07:58 2020 +0100
(Michael Myer)

    minor changes

####   Sun Jan 5 12:24:35 2020 +0100
(Michael Myer)

    terminfo file

####   Sun Jan 5 12:03:32 2020 +0100
(Michael Myer)

    minor changes

####   Sun Jan 5 11:56:56 2020 +0100
(Michael Myer)

    minor changes

####   Sun Jan 5 11:56:13 2020 +0100
(Michael Myer)

    minor changes

####   Sun Jan 5 11:54:57 2020 +0100
(Michael Myer)

    minor changes

####   Sun Jan 5 11:48:25 2020 +0100
(Michael Myer)

    minor changes

####   Sun Jan 5 11:46:42 2020 +0100
(Michael Myer)

    minor changes

####   Sun Jan 5 11:43:04 2020 +0100
(Michael Myer)

    minor changes

####   Sun Jan 5 11:32:49 2020 +0100
(Michael Myer)

    minor changes

####   Sun Jan 5 11:31:51 2020 +0100
(Michael Myer)

    minor changes

####   Sun Jan 5 11:27:09 2020 +0100
(Michael Myer)

    minor changes

####   Sun Jan 5 11:24:29 2020 +0100
(Michael Myer)

    minor changes

####   Sat Jan 4 23:50:13 2020 +0100
(Michael Myer)

    minor changes

####   Sat Jan 4 23:47:51 2020 +0100
(Michael Myer)

    minor changes

####   Sat Jan 4 23:42:47 2020 +0100
(Michael Myer)

    Initial and crude commit

####   Sat Jan 4 23:25:20 2020 +0100
(Michael Myer)

    minor changes

####   Sat Jan 4 23:13:36 2020 +0100
(Michael Myer)

    documenting

####   Sat Jan 4 23:01:40 2020 +0100
(Michael Myer)

    Readme added

####   Sat Jan 4 22:27:01 2020 +0100
(Michael (misc) Myer)

    Initial commit
