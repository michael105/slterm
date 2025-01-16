# build configuration 
# More options to configure are in src/config.h
# This is a makefile, included by src/Makefile

# Set to 0
SHOWCONFIGINFO := 1

# Dump debugging info
# Values: 0 (off), 1 (on)
ENABLEDEBUG := 0

# Dump all available debug
FULLDEBUG := 0

# version
VERSION := 0.99.5.1

# Set to 1 enable Xresource configuration
# (in addition, slterm has to be started with the option "-x on")
XRESOURCES := 0

# embed the terminfo and manual page (+12k)
EMBEDRESOURCES := 1

# Embed fonts into the binary
EMBEDFONT := 0

# utf8-support (currently abandoned. Will not work )
#UTF8 := 0

# Length of history, in bits, -> log(size in lines) ~ bits */
# 8 equals 1<<8 = 256 lines, 9 = 512, 10 = 1024, ..
HISTSIZEBITS := 16
#HISTSIZEBITS = 7

# opt Flag. -O2 might be save, -O3 sometimes gives troubles
OPT_FLAG := -Os -Wall
#OPT_FLAG = -g

# Linker Flags
#LINKER_FLAG = -g
LINKER_FLAG := -Os -s 

# paths
PREFIX := /usr/local
MANPREFIX := $(PREFIX)/share/man

X11INC := /usr/X11R6/include
X11LIB := /usr/X11R6/lib

# Executables

# pkg-config
PKG_CONFIG := pkg-config

# compiler and linker
CC := gcc



