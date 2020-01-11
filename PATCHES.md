## applied Patches


<div id="TOC">

  - [keyboard\_select](#keyboard_select)
  - [scrollback](#scrollback)
  - [anysize](#anysize)
  - [one clipboard](#one-clipboard)
  - [relativeborder](#relativeborder)
  - [selectioncolors](#selectioncolors)
  - [xresources](#xresources)

</div>

omitted:

newterm ( doesn't detach->parent killed->child too )
	
vertcenter ( somehow doesn't look right to me)


---
### keyboard\_select

#### Description

This patch allows you to select and copy text to primary buffer with
keyboard shortcuts like the perl extension keyboard-select for urxvt.

#### Instructions

The patch changes the config.def.h. Delete your config.h or add the
shortcut below if you use a custom one.

    Shortcut shortcuts[] = {
        ...
        { TERMMOD, XK_s, keyboard_select, { 0 } },
    };

#### Notes

When you run "keyboard\_select", you have 3 modes available :

  - move mode : to set the start of the selection;
  - select mode : to activate and set the end of the selection;
  - input mode : to enter the search criteria.

Shortcuts for move and select modes :

``` 
 h, j, k, l:    move cursor left/down/up/right (also with arrow keys)
 !, _, *:       move cursor to the middle of the line/column/screen
 Backspace, $:  move cursor to the beginning/end of the line
 PgUp, PgDown : move cursor to the beginning/end of the column
 Home, End:     move cursor to the top/bottom left corner of the screen
 /, ?:          activate input mode and search up/down
 n, N:          repeat last search, up/down
 s:             toggle move/selection mode
 t:             toggle regular/rectangular selection type
 Return:        quit keyboard_select, keeping the highlight of the selection
 Escape:        quit keyboard_select
```

With h,j,k,l (also with arrow keys), you can use a quantifier. Enter a
number before hitting the appropriate key.

Shortcuts for input mode :

``` 
 Return:       Return to the previous mode
```

#### Authors

  - Tonton Couillon - \<la dot luge at free dot fr\>



---
### scrollback

#### Description

Scroll back through terminal output using Shift+{PageUp, PageDown}.

patch on top of the previous to allow scrolling using
`Shift+MouseWheel`.

patch on top of the previous two to allow scrollback using mouse wheel
only when not in `MODE_ALTSCREEN`. For example the content is being
scrolled instead of the scrollback buffer in `less`. Consequently the
Shift modifier for scrolling is not needed anymore. **Note: patches
before `20191024-a2c479c` might break mkeys other than scrolling
functions.**

patch on top of the first two to allow changing how fast the mouse
scrolls.

#### Notes

  - Patches modify config.def.h, you need to add mkeys to your own
    config.h
  - With patches before `20191024-a2c479c`: you can not have a mshortcut
    for the same mkey so remove Button4 and Button5 from mshortcuts in
    config.h
  - The mouse and altscreen patches `20191024-a2c479c` (and later) are
    simpler and more robust because st gained better support for
    customized mouse shortcuts. As a result, the altscreen patch doesn't
    really need the mouse patch. However to keep it simple the
    instructions stay the same: the alrscreen patch still applies on top
    of the (now very minimal) mouse patch.

#### Authors

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




---
### anysize

#### Description

By default, st's window size always snaps to the nearest multiple of the
character size plus a fixed inner border (set with borderpx in
config.h). When the size of st does not perfectly match the space
allocated to it (when using a tiling WM, for example), unsightly gaps
will appear between st and other apps, or between instances of st.

This patch allows st to resize to any pixel size, makes the inner border
size dynamic, and centers the content of the terminal so that the
left/right and top/bottom borders are balanced. With this patch, st on a
tiling WM will always fill the entire space allocated to it.

#### Authors

  - Augusto Born de Oliveira - <augustoborn@gmail.com>

---
### one clipboard

Free Desktop mandates the user to remember which of two clipboards you
are keeping selections in. If you switch between a terminal and browser,
you might this UX jarring. This patch modifies st to work from one
CLIPBOARD, the same as your browser.

#### Description

st by default only sets PRIMARY on selection since
[March 2015](http://git.suckless.org/st/commit/?id=28259f5750f0dc7f52bbaf8b746ec3dc576a58ee)
according to the [Freedesktop
standard](http://standards.freedesktop.org/clipboards-spec/clipboards-latest.txt).

This patch makes st set CLIPBOARD on selection. Furthermore from
`st-clipboard-0.8.2.diff` middle click pastes from CLIPBOARD.

You may want to replace selpaste with clippaste in your config.h to
complete the affect.

#### Authors

  - Kai Hendry - <hendry@iki.fi>
  - Laslo Hunhold - <dev@frign.de> (git port)
  - Matthew Parnell - <matt@parnmatt.co.uk> (0.7, git ports)


---
### relativeborder

#### Description

When working with a mixture of different DPI scales on different
monitors, you need to use a flexible font that will size correctly no
matter the DPI - for example, `DejaVu Sans Mono-10`. If you have a
border set in pixels, this border will look vastly different in size
depending on the DPI of your display.

This patch allows you to specify a border that is relative in size to
the width of a cell in the terminal.

#### Authors

  - Doug Whiteley - <dougwhiteley@gmail.com>



---
### selectioncolors

#### Description

This patch adds the two color-settings *selectionfg* and *selectionbg*
to config.def.h. Those define the fore- and background colors which are
used when text on the screen is selected with the mouse. This removes
the default behaviour which would simply reverse the colors.

Additionally, a third setting *ingnoreselfg* exists. If true then the
setting *selectionfg* is ignored and the original foreground-colors of
each cell are not changed during selection. Basically only the
background-color would change. This might be more visually appealing to
some folks.

#### Authors

  - Aleksandrs Stier



---
### xresources

#### Description

This patch adds the ability to configure st via Xresources. At startup,
st will read and apply the resources named in the `resources[]` array in
config.h.

#### Authors

  - @dcat on [Github](https://github.com/dcat/st-xresources)
  - Devin J. Pohly - <djpohly@gmail.com> (git port)
  - Sai Praneeth Reddy - <spr.mora04@gmail.com> (read borderpx from
    xresources)
