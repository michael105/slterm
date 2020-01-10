# st-asc - simple terminal
# See LICENSE file for copyright and license details.
.POSIX:

ifdef UNDEF

######### edit values below, and in config.h, 
# to change the config

# version
VERSION = asc-0.9rc2

# Customize below 

# Uncomment to enable Xresource configuration
# (in addition, st-asc has to be started with the option "-x on")
# XRESOURCES=-DXRESOURCES

# Uncomment to compile with utf8-support
# UTF8=-DUTF8

# opt Flag. -O2 might be save, -O3 works here also (ArchLinux 64bit)
OPT_FLAG = -O2
LINKER_FLAG = -s

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

PKG_CONFIG = pkg-config

# compiler and linker
CC = gcc

endif

include config.h.in

ifeq "$(strip $(XRESOURCES))" "1"
		XRESOURCESFLAG=-DXRESOURCES
endif

ifeq "$(strip $(UTF8))" "1"
		UTF8FLAG=-DUTF8
endif



# includes and libs
INCS = -I$(X11INC) \
       `$(PKG_CONFIG) --cflags fontconfig` \
       `$(PKG_CONFIG) --cflags freetype2`

LIBS = -L$(X11LIB) -lm -lrt -lX11 -lutil -lXft \
       `$(PKG_CONFIG) --libs fontconfig` \
       `$(PKG_CONFIG) --libs freetype2`

# flags
STCPPFLAGS = -DVERSION=\"$(VERSION)\" -D_XOPEN_SOURCE=600
STCFLAGS = $(INCS) $(STCPPFLAGS) $(CPPFLAGS) $(CFLAGS) $(XRESOURCESFLAG) $(OPT_FLAG) $(UTF8FLAG) -DHISTSIZEBITS=$(HISTSIZEBITS)
STLDFLAGS = $(LIBS) $(LDFLAGS) $(LINKER_FLAG)

# OpenBSD:
#CPPFLAGS = -DVERSION=\"$(VERSION)\" -D_XOPEN_SOURCE=600 -D_BSD_SOURCE
#LIBS = -L$(X11LIB) -lm -lX11 -lutil -lXft \
#       `pkg-config --libs fontconfig` \
#       `pkg-config --libs freetype2`


######### end of config options


SRC = st.c x.c
HEADER = st.h config.h win.h arg.h

all: options st

options:
	@echo st build options:
	@echo "CFLAGS  = $(STCFLAGS)"
	@echo "LDFLAGS = $(STLDFLAGS)"
	@echo "CC      = $(CC)"

config.h:
# cp config.def.h config.h # utterly useless and confusing

with-utf8:
	UTF8=-DUTF8 make st

#st: $(OBJ)
# better go in one run. Gives way more room to the compiler for optimizations.
st: $(SRC) $(HEADER)
	$(CC) -o st $(SRC) $(STCFLAGS) $(STLDFLAGS)
	
shared: $(SRC) $(HEADER) st-asc.so
	gcc -o st-asc st-asc.c -lst-asc $(OPT_FLAG) 

st-asc.so: $(SRC) $(HEADER) x.h
	$(CC) -o libst-asc.so -shared -fpic $(SRC) $(STCFLAGS) $(STLDFLAGS) -Dshared


clean:
	rm -f st $(OBJ) st-$(VERSION).tar.gz

dist: clean
	mkdir -p st-$(VERSION)
	cp -R FAQ LEGACY TODO LICENSE Makefile README \
		config.def.h st.info st.1 arg.h st.h win.h $(SRC)\
		st-$(VERSION)
	tar -cf - st-$(VERSION) | gzip > st-$(VERSION).tar.gz
	rm -rf st-$(VERSION)

install: st
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f st $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/st
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	sed "s/VERSION/$(VERSION)/g" < st.1 > $(DESTDIR)$(MANPREFIX)/man1/st.1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/st.1
	tic -sx st.info
	@echo Please see the README file regarding the terminfo entry of st.

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/st
	rm -f $(DESTDIR)$(MANPREFIX)/man1/st.1

.PHONY: all options clean dist install uninstall
