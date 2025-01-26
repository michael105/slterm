# slterm - simple terminal
# See LICENSE file for copyright and license details.
.POSIX:


define HELP
Makefile help

for the build configuration, edit config.make
further configuration and preferences can be set in src/config.h

make targets
------------
slterm: (default) build slterm
static: build slterm as static binary
static_embedfont: static build, embed fonts into the binary

install: install slterm

endef



all: man tools
	cd src && $(MAKE)


help:
	$(info $(HELP))


slterm: man tools
	cd src && $(MAKE) 
	
static: man tools
	cd src && $(MAKE) static

static_embedfont: man tools
	cd src && $(MAKE) static EMBEDFONT=1

tools:
	cd tools && $(MAKE) 
	
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


