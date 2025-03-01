// the help content.
// It's shown on F1, and compiled into st.
// I guess, it's better to have a help screen always there,
// when to load it (hopefully, when nothing did fuck up).
// Especially, it's maybe 1/100 of the memory usage,
// and furthermore quite possibly shared anyways.
static char* helpcontents = "\
\n\r\
[36m     slterm - [00;35msl[36mim [00;35mt[36merminal emulator for X [0;0m\n\r\
\n\r\
http://github.com/michael105/slterm\n\r\
Version: 0.99.6.0\n\r\
\n\r\
\n\r\
[01;33m Help [0;0m\n\r\
\n\r\
\n\r\
Colors\n\r\
\n\r\
\e[1;30mecho -e \"\\e[A;Nm\" text ..\n\r\
N: fg color 30 - 37, and bg color 40 - 47\n\r\
\n\r\
255 colors\n\r\
echo -e \"\\e[A;38;5;Nm\" text ...\n\r\
echo -e \"\\e[0,48,5,Nm text\" for colored background.\n\r\
N: color 0-255\n\r\
A: 0=normal, 1=light, 2=dark\n\r\
\n\r\
\n\r\
\n\r\
[01;33m Keybinding reference [0;0m\n\r\
\n\r\
The keycombinations can be changed in config.h(recompile needed)\n\r\
\n\r\
For other information please have a look into the man page,\n\r\
also accessible with 'slterm -H'\n\r\
\n\r\
\n\r\
[01;33m Keystrokes [0;0m\n\r\
\n\r\
[01;30mThere are 4 different modes in slterm;\n\r\
regular mode, lessmode, selection mode and alt mode(alt screen).\n\r\
slterm is started in the regular mode.\n\r\
When the alt mode is used, e.g. by man or links, \n\r\
scrollmarks and lessmode are disabled.\n\r\
[0;0m\n\r\
\n\r\
[4m[01;32mAll modes:[0;0m\n\r\
\n\r\
  Ctrl+Shift + I: Inverse colors\n\r\
  (Anymod)+F1:    Show this help\n\r\
\n\r\
\n\r\
[1mSet font width/size:[0;0m\n\r\
\n\r\
  Ctrl+Shift + Insert/Delete:   Enlarge/Shrink width\n\r\
  Ctrl+Shift + Home/End:        Enlarge/Shrink height\n\r\
  Ctrl+Shift + PageUp/PageDown: Zoom in / out\n\r\
  Ctrl+Shift + Backspace:       Reset font display\n\r\
\n\r\
\n\r\
[1mSelect Codepage:[0;0m\n\r\
\n\r\
  Ctrl+Win + 0 CP1250 \n\r\
             1 CP1251\n\r\
             2 CP1252 [01;30m(this is mostly ANSI, and the 1. page of Unicode)[0;0m\n\r\
             3 CP1253\n\r\
             4 CP437  [01;30m(Old IBM codetable, borders and tables)[0;0m\n\r\
             5 CP850  [01;30m(DOS Standard table)[0;0m\n\r\
             6 CP4002 [01;30m(Custom table, mix of 1252 and 437)[0;0m\n\r\
\n\r\
\n\r\
[1m[4m[01;32mRegular mode:[0;0m\n\r\
\n\r\
[1mScrolling:[0;0m\n\r\
\n\r\
  Shift + Up/Down/PageUp/Pagedown: Scroll up/down\n\r\
  Shift + Home/End:  Scroll to top/bottom\n\r\
  Shift + Backspace: Scroll to the location of the last command (shell)\n\r\
                     and enter lessmode\n\r\
  Shift+Enter:       Execute command, enter lessmode if more than\n\r\
                     one screen is displayed by the command.\n\r\
\n\r\
\n\r\
[1mClipboard:[0;0m\n\r\
\n\r\
  Shift + Insert / Ctrl+Shift + y:  Paste\n\r\
  Ctrl+Shift + c / mouse selection: Copy \n\r\
\n\r\
\n\r\
[1mScrollmarks:[0;0m\n\r\
\n\r\
  Ctrl+Alt + [0..9]: Set Scrollmark 0 - 9\n\r\
  Ctrl + [0..9]:     Scroll to mark 0 - 9\n\r\
  Shift + Backspace: Scroll to the last entered command (in shell)\n\r\
\n\r\
\n\r\
\n\r\
[1m[4m[01;32mLessmode:[0;0m\n\r\
\n\r\
  Alt+Shift + Down/PageDown/Up/PageUp/l, ScrollLock:  Enter lessmode. \n\r\
  Scroll around with cursor keys, Home, End.\n\r\
  Backspace/Tab left go to the location of the previous command in shell,\n\r\
  Tab scrolls down to the next entered command location.\n\r\
  Exit with q/ESC\n\r\
\n\r\
  Shift+Backspace/Tab left: Scroll to the location of the last entered command,\n\r\
    enter lessmode\n\r\
\n\r\
  Tab: Scroll downwards to the next entered command line\n\r\
\n\r\
  Ctrl+Alt + [0..9]: Set Scrollmark 0 - 9\n\r\
      Ctrl + [0..9]: Goto Scrollmark 0 - 9\n\r\
  Lessmode:  [0..9]: Goto Retmark 1 - 10\n\r\
\n\r\
\n\r\
\n\r\
[1m[4m[01;32mSelection Mode:[0;0m\n\r\
\n\r\
  Ctrl+Shift + S, Alt + S: Enter selection mode\n\r\
\n\r\
  There are 3 submodes in selection mode:\n\r\
    - move mode : to set the start of the selection;\n\r\
    - select mode : to activate and set the end of the selection;\n\r\
    - input mode : to enter the search criteria.\n\r\
	\n\r\
\n\r\
  Shortcuts for move and select modes :\n\r\
 \n\r\
 h, j, k, l:    move cursor left/down/up/right (also with arrow keys)\n\r\
 !, _, *:       move cursor to the middle of the line/column/screen\n\r\
 Backspace,Home move cursor to the beginning of line\n\r\
 $,End          move cursor to end of line\n\r\
 PgUp, PgDown:  move cursor to the beginning/end of the column\n\r\
 g, G:          move cursor to the top/bottom left corner of the screen\n\r\
\n\r\
 y:             In move mode, select the current line.\n\r\
                In select mode, copy the selection to the clipboard, quit selection\n\r\
					 (yy means yank the current line)\n\r\
 /, ?:          activate input mode and search up/down\n\r\
 n, N:          repeat last search, up/down\n\r\
\n\r\
 s:             toggle move/selection mode\n\r\
 v/V:           toggle move/selection mode, select regular/rectangular type\n\r\
 t:             toggle regular/rectangular selection type\n\r\
\n\r\
 p:             quit keyboard_select, copy and paste selection\n\r\
 Return:        quit keyboard_select, copy selection to clipboard\n\r\
 Escape:        quit keyboard_select\n\r\
 \n\r\
 With h,j,k,l (also with arrow keys), you can use a quantifier.\n\r\
 Enter a number before hitting the appropriate key.\n\r\
 \n\r\
\n\r\
  Shortcuts for input mode :\n\r\
 \n\r\
 Return:       Return to the previous mode\n\r\
 \n\r\
 \n\r\
\n\r\
[01;33m Shortcut list, without selection mode bindings [0;0m\n\r\
\n\r\
\n\r\
Mode\t Modifiers\t\t Key\t\t Function\t Info\n\r\
\n\r\
\n\r\
All	 All                	 Break      	 sendbreak 	\n\r\
All	 All                	 Print      	 printsel 	\n\r\
All	 All                	 Scroll_Lock 	 lessmode_toggle 	\n\r\
All	 Alt                	 s          	 keyboard_select 	\n\r\
All	 Alt+Shift          	 Down       	 lessmode_toggle 	\n\r\
All	 Alt+Shift          	 Home       	 lessmode_toggle 	\n\r\
All	 Alt+Shift          	 L          	 lessmode_toggle 	\n\r\
All	 Alt+Shift          	 Page_Down  	 lessmode_toggle 	\n\r\
All	 Alt+Shift          	 Page_Up    	 lessmode_toggle 	\n\r\
All	 Alt+Shift          	 Up         	 lessmode_toggle 	\n\r\
All	 Control            	 0          	 scrollmark 	\n\r\
All	 Control            	 1          	 scrollmark 	\n\r\
All	 Control            	 2          	 scrollmark 	\n\r\
All	 Control            	 3          	 scrollmark 	\n\r\
All	 Control            	 4          	 scrollmark 	\n\r\
All	 Control            	 5          	 scrollmark 	\n\r\
All	 Control            	 6          	 scrollmark 	\n\r\
All	 Control            	 7          	 scrollmark 	\n\r\
All	 Control            	 8          	 scrollmark 	\n\r\
All	 Control            	 9          	 scrollmark 	\n\r\
All	 Control            	 F1         	 showhelp 	\n\r\
All	 Control            	 Print      	 toggleprinter 	\n\r\
All	 Control+Alt        	 0          	 set_scrollmark 	\n\r\
All	 Control+Alt        	 1          	 set_scrollmark 	\n\r\
All	 Control+Alt        	 2          	 set_scrollmark 	\n\r\
All	 Control+Alt        	 3          	 set_scrollmark 	\n\r\
All	 Control+Alt        	 4          	 set_scrollmark 	\n\r\
All	 Control+Alt        	 5          	 set_scrollmark 	\n\r\
All	 Control+Alt        	 6          	 set_scrollmark 	\n\r\
All	 Control+Alt        	 7          	 set_scrollmark 	\n\r\
All	 Control+Alt        	 8          	 set_scrollmark 	\n\r\
All	 Control+Alt        	 9          	 set_scrollmark 	\n\r\
All	 Control+Alt        	 Return     	 enterscroll 	\n\r\
All	 Control+Shift      	 C          	 clipcopy 	\n\r\
All	 Control+Shift      	 I          	 inverse_screen 	\n\r\
All	 Control+Shift      	 Num_Lock   	 numlock 	\n\r\
All	 Control+Shift      	 S          	 keyboard_select 	\n\r\
All	 Control+Shift      	 V          	 clippaste 	\n\r\
All	 Control+Shift      	 Y          	 selpaste 	\n\r\
All	 Control+Win        	 0          	 set_charmap 	\n\r\
All	 Control+Win        	 1          	 set_charmap 	\n\r\
All	 Control+Win        	 2          	 set_charmap 	\n\r\
All	 Control+Win        	 3          	 set_charmap 	\n\r\
All	 Control+Win        	 4          	 set_charmap 	\n\r\
All	 Control+Win        	 5          	 set_charmap 	\n\r\
All	 Control+Win        	 6          	 set_charmap 	\n\r\
All	 Control+Win        	 7          	 set_charmap 	\n\r\
All	 Control+Win        	 8          	 set_charmap 	\n\r\
All	 Control+Win        	 9          	 set_charmap 	\n\r\
All	 Shift              	 BackSpace  	 retmark 	\n\r\
All	 Shift              	 Down       	 kscrolldown 	\n\r\
All	 Shift              	 End        	 scrolltobottom 	\n\r\
All	 Shift              	 Home       	 scrolltotop 	\n\r\
All	 Shift              	 ISO_Left_Tab 	 retmark 	\n\r\
All	 Shift              	 Insert     	 selpaste 	\n\r\
All	 Shift              	 Page_Down  	 kscrolldown 	\n\r\
All	 Shift              	 Page_Up    	 kscrollup 	\n\r\
All	 Shift              	 Print      	 printscreen 	\n\r\
All	 Shift              	 Return     	 enterscroll 	\n\r\
All	 Shift              	 Up         	 kscrollup 	\n\r\
All	 Shift+Control      	 BackSpace  	 zoomreset 	\n\r\
All	 Shift+Control      	 Delete     	 set_fontwidth 	\n\r\
All	 Shift+Control      	 End        	 set_fontheight 	\n\r\
All	 Shift+Control      	 Home       	 set_fontheight 	\n\r\
All	 Shift+Control      	 Insert     	 set_fontwidth 	\n\r\
All	 Shift+Control      	 Page_Down  	 zoom 	\n\r\
All	 Shift+Control      	 Page_Up    	 zoom 	\n\r\
Help	 All                	 ALL_KEYS   	 dummy 	\n\r\
Help	 All                	 Down       	 kscrolldown 	\n\r\
Help	 All                	 End        	 scrolltobottom 	\n\r\
Help	 All                	 Escape     	 quithelp 	\n\r\
Help	 All                	 Home       	 scrolltotop 	\n\r\
Help	 All                	 Page_Down  	 kscrolldown 	\n\r\
Help	 All                	 Page_Up    	 kscrollup 	\n\r\
Help	 All                	 Up         	 kscrollup 	\n\r\
Help	 All                	 q          	 quithelp 	\n\r\
Help	 All                	 space      	 kscrolldown 	\n\r\
Less	 All                	 0          	 retmark 	\n\r\
Less	 All                	 1          	 retmark 	\n\r\
Less	 All                	 2          	 retmark 	\n\r\
Less	 All                	 3          	 retmark 	\n\r\
Less	 All                	 4          	 retmark 	\n\r\
Less	 All                	 5          	 retmark 	\n\r\
Less	 All                	 6          	 retmark 	\n\r\
Less	 All                	 7          	 retmark 	\n\r\
Less	 All                	 8          	 retmark 	\n\r\
Less	 All                	 9          	 retmark 	\n\r\
Less	 All                	 BackSpace  	 retmark 	\n\r\
Less	 All                	 Down       	 kscrolldown 	\n\r\
Less	 All                	 End        	 scrolltobottom 	\n\r\
Less	 All                	 Escape     	 lessmode_toggle 	\n\r\
Less	 All                	 Home       	 scrolltotop 	\n\r\
Less	 All                	 Page_Down  	 kscrolldown 	\n\r\
Less	 All                	 Page_Up    	 kscrollup 	\n\r\
Less	 All                	 Tab        	 retmark 	\n\r\
Less	 All                	 Up         	 kscrollup 	\n\r\
Less	 All                	 q          	 lessmode_toggle 	\n\r\
Less	 All                	 space      	 kscrolldown 	\n\r\
Less	 Shift              	 Return     	 lessmode_toggle 	\n\r\
MODE_DEFAULT	 Control+Shift|Alt|Win 	 I          	 dump_terminfo 	\n\r\
\n\r\
\n\r\
[36m\n\r\
===============================================================================\n\r\
\n\r\
[37m(2019-2025 miSc, Michael 147, started with the suckless st sources)\n\r\
[01;30m\n\r\
License: MIT\n\r\
Permission is hereby granted, free of charge, to any person obtaining a copy\n\r\
of this software and associated documentation files (the Software), to deal\n\r\
in the Software without restriction, including without limitation the rights\n\r\
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n\r\
copies of the Software, and to permit persons to whom the Software is\n\r\
furnished to do so, subject to the following conditions:\n\r\
\n\r\
The above copyright notice and this permission notice shall be included in all\n\r\
copies or substantial portions of the Software.\n\r\
\n\r\
THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n\r\
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n\r\
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n\r\
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n\r\
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n\r\
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n\r\
SOFTWARE.\n\r\
\n\r\
";
