

Date: 2024/10/08

Well. better keeping my notes here.

Funnny with those colors.
I'm unhappy with the xterm palette.

The first 8 colors I've been able to change to a xterm compatible palette.
So, there are already 32 hand selected colors, since all colors
come in normal, faint, bold and bold_faint.

However, the 6x6x6 color cube of the colors 16-232 isn't that bright.

Looking around, there is this site.

https://lospec.com/palette-list

and, for 256 colors.

https://lospec.com/palette-list/srb2 - 256 colors


Now, well, eventually I'll stick to a 64 color palette. 
First 16 colors, with attributes.

I would need to know, how many apps actually use those extended indexed colorset.

64 colors ought to be enough for everyone!



-- 2020/01/11


Copying the git log.
To be exact, copying all log messages, 
having a informative message.
Going to need something doing this automatic.


Date:   Sat Jan 11 2020 

    tag this 0.9rc3. Seems to be stable.

    Debugging switch in config.h.in.
    
    Enable (global) debugging now with the debug flag in config.h.in
    Might be useful for reporting bugs.
    (when encountering a bug, please compile with debugging
    set to level 1, and save the output.
    st-asc > log.txt

    Optimization for non Utf8.
    
    Copy int32 at once, instead of byte for byte.
    Could be further optimized; but I don't
    want it too machine dependent.
    (Now this depends on unsigned int being 32bit long.
    And could be adapted easily.)
    Byte for Byte, eye for eye. :) I guess,
    this doesn]t work in English.

Date:   Fri Jan 10 2020 

    save compiler info in the binary

    Edited the config "system".
    
    Now only one file, config.h.in, needs to be edited.

    Dynamic allocation of the history buffer
    
    Made the allocation of the memory for the hist buf dynamic.
    for, e.g., 16k lines of history, this results in around 6MB Memory
    (depending on the terminal's width);
    which are now allocated dynamically.

Notes:
    if utf8 is enabled, the saving is more than 30/40MB,
    depending on the terminal's width and the history len.

    configuration via one file only

    v0.9rc2

    cleanup

Date:   Fri Jan 10 2020 

    utf8 compileswitch seems to work

    Optimizations. Option to build a shared version
    
    The optimizations finally start to give better results.
    To name them: No temporary vars for swapping of values,
    instead xor exchange
    No division/modulo, replaced by bit operations

Date:   Thu Jan 9 2020 

    History scrollback limited to the history (Not the buf)

    Compileswitch UTF8.
    
    After thinking about it, it's not so much code for utf8.
    A cleaner codebase would be nicer.
    But on the other hand, having the option to switch to utf8
    might also be nice.
    Has to be implemented as compile time switch.

    Color option for the cursor in unfocused windows added

    Added a nicer cursor for unfocused windows
    
    The default empty rect has been an annoyance to me,
    since I met Linux the first time.

Date:   Wed Jan 8 2020 

    Replaced division by bitshift
    
    division is the most expensive operation.
    Anyways. doesn't seem to be a big difference.
    1E9 equals 1.000.000.000
    2^30 might be close enough

Date:   Tue Jan 7 2020 

    ansicolor test added

    utf8 file added for tests

    vt102 ascii control char table

    Ascii code tables added for reference

    Added debug macros.
    
    "Debugged" the handling of German Umlauts.
    Recognized, the problem has been with the shell (zsh)..
    I'll keep the dbg macros. Might need them later again.

    utf8. Compiler warnings silenced

    Added compile time switch XRESOURCES
    
    needs to have the switch XRESOURCES defined at compile time
    (somehow I don't have a good feeling with this patch

    Patches applied. Set "Unicode" chars to 1 Byte. rgb stripped

Date:   Sat Jan 4 2020 

    Initial commit



.. 2020/01/06

Patched the existing scrollback patch in.
I did find a terminal scroller, and somehow this would be nicer.
This would be more consistent, despite what terminal or -emulation
is used. But this thing slowed down st quite a bit.

Ok. At the moment, the ripped unicode and rgb support
really show some effect.
But the scrollback patch somehow is quite memory consuming.

Possibly I should either come back to the terminal scroller,
and see, whether this thing could be optimized for better speed.
Or, the scrollback buffer within st is, well, not exactly 
well dessigned. It's a static buffer,
even the variable is made globally static.
so. For me, always keeping at least 10 terminals open,
that's not so good.
Have to look at it.

For now I'll upload my changes, again.
And it seems, like I should do a real fork of st.

I already changed quite a bit, and merging all my changes
might be too much work.
We'll see.

..


Thinking about it - it might be better keeping the scrollback
buffer within st. The problems arising by separating the scrollback 
would be, especially: 
- Ansi control characters either being resent when scrolling back,
	or they would have to be parsed
- Color codes and textattributes, same problem;
	worse, when scrolling back they would have to be parsed again.
- this also prevents the scrollback from being accordingly seekable

However, something is bloating st, when setting the scrollback buffer
to larger values (>10k lines)

Ok. So. Found it. It's obvious.
The history within st needs to keep track of every glyph,
meaning not only the displayed chars, with it's attributes, 
but also the "empty" Glyphs. (Which also do have attributes, 
e.g. the backgroundcolor.

So. What to do. 
An external scrollback buffer has the advantage, 
of holding only chars, which are displayed, and control sequences.
But - e.g., when scrolling backwards, there might be colors
and text attributes displayed wrong.
Copying also could give problems.
Either the control sequences are parsed within st AND the external scrollback.
or. :/ dunno. I like a colored scrollback buffer.

Seems I should see, whether I'm able to optimize the scrollback buffer
of st. 
The current implementation: 
  - allocates a complete line of glyphs
  - for every single line. Does this even for completely empty, 
    or yet unusedlines.
  - reallocates every single line of the "history" on win resize
	- The "history" is more like a preallocated buffer, 
	  where scrolled lines are copied into.

So. Instead of doing this, I'm now going to prepare 0.9rc1.
There still is some utf8 stuff left to clean.
And further testing would be useful.

About adding patches, and so on: When you guard your changes 
with an #ifdef YOURPATCHNAME / #endif, 
I'll add patches in each case. 
Just do a fork, and consequently file a pull request. 

You can grep for the switch "XRESOURCES",
where I did exactly this, guard a patch 
to be optional.

;) It's also your showcase, bookmarks, about what you added.

---

copied this from README. Its irrelevant.


A rant against some sort of scientists, btw.
So, someone is sure, and "prooves" this 
by a study, humans cannot perceipt a latency &lower 200ms,
since that's our reaction time. 

That's. Bullshit. 
One can think, what this scientist did believe.
But - just ask an musician. A system, used for plaing live e.g. synthesizer,
is really hard to play with a latency of 200ms. 

I'd say, it's going to be hard to get a medium timing with anything bigger than 40ms.

The latency is feelable at, say, 5ms. at least. I did never really check,
since even 20ms are hard to get with nowadays systems.
But you can tell, in each case, the difference to a "real" instrument. 
Which also doe's have some latency. But, amongst others, this latency is steady.

Oh. There's another example, showing this scientist being wrong. 
It's our capability to perceive the location of an audible source.
What needs a perception and combination of both ears in the scale of microseconds.
Maybe even nanoseconds. But 200ms is. Painful slow.

.. 2020/01

scrolling - is this the job of the terminal emulator,
or for a separate tool?

not sure. 
Ok. I try to break this down - 
it's also the question, whether one likes to have (graphical) scrollbars.


