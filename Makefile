
all: src/config.h.in
	cd src && $(MAKE)



src/config.h.in: config.h.in
	cp config.h.in ./src/config.h.in


devlog: 
	git log | sed -e '/^commit/d;/^Author/{h;d};/^Date:/{p;x}' |\
			sed -E "s/^Date:/####/;\
				s/Author: *(.*)/(\1)/;\
				s/ *.misc.myer\@zoho.com. *// ;\
				s/^Notes: */&\n/" \
	> DEVLOG.md

devlog.html: DEVLOG.md
	pandoc -f markdown -t html DEVLOG.md > DEVLOG.html
