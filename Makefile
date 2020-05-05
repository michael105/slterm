
all: src/config.h src/config.make
	cd src && $(MAKE)



src/config.h: config.h.in
	sed -ne '/^..start config.h/,$$p' config.h.in > src/config.h
 
src/config.make: config.h.in
	sed -ne '/#makefilestart/,/#makefileend/p' config.h.in > src/config.make


devlog: 
	git log | sed -e '/^commit/d;/^Author/{h;d};/^Date:/{p;x}' |\
			sed -E "s/^Date:/####/;\
				s/Author: *(.*)/(\1)/;\
				s/ *.misc.myer\@zoho.com. *// ;\
				s/^Notes: */&\n/" \
	> DEVLOG.md

devlog.html: DEVLOG.md
	pandoc -f markdown -t html DEVLOG.md > DEVLOG.html
