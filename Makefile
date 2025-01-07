# slterm - simple terminal
# See LICENSE file for copyright and license details.
.POSIX:


all: man
	cd tools && $(MAKE)
	cd src && $(MAKE)

static: 
	cd tools && $(MAKE)
	cd src && $(MAKE) static EMBEDFONT=1
	
#local (use config.in.loc)
l: cd src && $(MAKE)

man: slterm.1

slterm.1: slterm.1.rst
	rst2man slterm.1.rst > slterm.1 || rst2man.py slterm.1.rst > slterm.1


install:
	cd src && make install


loc-install: src/slterm slterm.1
	su -c 'sh -c "mv /usr/local/bin/st /usr/local/bin/st.bak; cp src/slterm /usr/local/bin/st; chmod a+rx /usr/local/bin/st" '


.PHONY: all install

