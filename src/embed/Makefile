
all:slterm_font_ttf.o slterm_font_italic_ttf.o


slterm_font_ttf.o: slterm_font_ttf.c
	gcc -c slterm_font_ttf.c

slterm_font_ttf.c: slterm_font.ttf
	xxd -i slterm_font.ttf > slterm_font_ttf.c


slterm_font_italic_ttf.o: slterm_font_italic_ttf.c
	gcc -c slterm_font_italic_ttf.c

slterm_font_italic_ttf.c: slterm_font_italic.ttf
	xxd -i slterm_font_italic.ttf > slterm_font_italic_ttf.c


