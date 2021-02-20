# st-asc - simple terminal
# See LICENSE file for copyright and license details.
.POSIX:


all: man
	cd src && $(MAKE)

man: st-asc.1

st-asc.1: st-asc.1.rst
	rst2man st-asc.1.rst > st-asc.1 || rst2man.py st-asc.1.rst > st-asc.1

install: src/st-asc st-asc.1

.PHONY: all install

