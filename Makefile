
all: src/config.h src/config.make
	cd src && $(MAKE)

with-utf8:
	cd src && $(MAKE) with-utf8


install: all
	cd src && $(MAKE) install

src/config.h: config.h.in
	sed -ne '/^..start config.h/,$$p' config.h.in > src/config.h
 
src/config.make: config.h.in
	sed -ne '/#makefilestart/,/#makefileend/p' config.h.in > src/config.make


devlog: 
	gitlog2md > DEVLOG.md

devlog.html: DEVLOG.md
	pandoc -f markdown -t html DEVLOG.md > DEVLOG.html
