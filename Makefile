# slterm - simple terminal
# See LICENSE file for copyright and license details.
.POSIX:


all: man
	cd src && $(MAKE)

man: slterm.1

slterm.1: slterm.1.rst
	rst2man slterm.1.rst > slterm.1 || rst2man.py slterm.1.rst > slterm.1

install: src/slterm slterm.1

.PHONY: all install

