

CC:=gcc -Os -Wall -Wno-unused-value -static -s

all: cpfilter ttfz ttfdz

cpfilter: cpfilter.c ../src/charmaps.h
	${CC} -o cpfilter cpfilter.c


ttfz: ttfz.c sdefl.h
	gcc -Os -s -o ttfz ttfz.c

ttfdz: ttfdz.c sdefl.h
	gcc -Os -s -o ttfdz ttfdz.c




