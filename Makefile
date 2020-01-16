
all: src/config.h.in
	cd src && $(MAKE)



src/config.h.in: config.h.in
	cp config.h.in ./src/config.h.in
