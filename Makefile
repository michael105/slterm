# st-asc - simple terminal
# See LICENSE file for copyright and license details.
.POSIX:

# To edit the configuration,
# please edit config.h.in

include config.h.in



ifeq "$(strip $(XRESOURCES))" "1"
		XRESOURCESFLAG=-DXRESOURCES
endif


ifeq "$(strip $(UTF8F))" "1"
		UTF8FLAG=-DUTF8
else
ifeq "$(strip $(UTF8))" "1"
		UTF8FLAG=-DUTF8
endif
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

ifeq "$(strip $(SHOWCONFIGINFO))" "1"
$(info Unconfigured - trying default config)
$(info If this doesn't work, please edit config.h.in for the configuration.)
$(info After that run as usually)
$(info 'make')
$(info 'sudo make install')
$(info The installation goes into /usr/local by default)
$(info and might be neccessary for the installation of the terminfo file)
$(info )
endif




SRC = st.c x.c
HEADER = st.h config.h win.h arg.h

all: st-asc

options:
	@echo  build options:
	@echo "CFLAGS  = $(STCFLAGS)"
	@echo "LDFLAGS = $(STLDFLAGS)"
	@echo "CC      = $(CC)"

config.h:
# cp config.def.h config.h # utterly useless and confusing

with-utf8:
	UTF8F=1 make st-asc

#st: $(OBJ)
# better go in one run. Gives way more room to the compiler for optimizations.
st-asc: $(SRC) $(HEADER)
	$(CC) -o st-asc $(SRC) $(STCFLAGS) $(STLDFLAGS)
	
shared: $(SRC) $(HEADER) libst-asc.so
	gcc -o st-asc.shared st-asc.c -lst-asc $(OPT_FLAG) 

libst-asc.so: $(SRC) $(HEADER) x.h
	$(CC) -o libst-asc.so -shared -fpic $(SRC) $(STCFLAGS) $(STLDFLAGS) -Dshared


clean:
	rm -f st-asc $(OBJ) st-asc-$(VERSION).tar.gz

dist: clean
	mkdir -p st-$(VERSION)
	cp -R FAQ LEGACY TODO LICENSE Makefile README \
		config.h.in st.info st-asc.1 arg.h st.h win.h $(SRC)\
		st-$(VERSION)
	tar -cf - st-asc-$(VERSION) | gzip > st-asc-$(VERSION).tar.gz
	rm -rf st-asc-$(VERSION)

install: st-asc
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f st-asc $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/st-asc
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	sed "s/VERSION/$(VERSION)/g" < st-asc.1 > $(DESTDIR)$(MANPREFIX)/man1/st-asc.1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/st-asc.1
	tic -sx st-asc.info
	@echo Please see the README file regarding the terminfo entry of st.

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/st-asc
	rm -f $(DESTDIR)$(MANPREFIX)/man1/st-asc.1

.PHONY: all options clean dist install uninstall
