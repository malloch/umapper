# PKG_CONFIG_PATH must be exported to find libmpr.pc

DISTDIR 	:= .
LMCFLAGS    := $(shell pkg-config --cflags libmpr)
LMLIBS      := $(shell pkg-config --libs libmpr)
CFLAGS		:= -O3 -Wall -fomit-frame-pointer -funroll-loops -ffast-math -c -fPIC -DPIC
LDFLAGS		:=	$(LMLIBS) -lz
INCLUDE		:= 	-I. $(LMCFLAGS)

FILES 	:= umapper

all: $(FILES)

umapper: umapper.o  umapper-verbose.o
	gcc \
	  umapper.o \
	  umapper-verbose.o -o \
	  $(DISTDIR)/umapper \
	  $(LDFLAGS)

umapper.o: umapper.c
	gcc $(INCLUDE) umapper.c -c -o $(DISTDIR)/umapper.o $(CFLAGS)

umapper-verbose.o: umapper-verbose.c
	gcc $(INCLUDE) umapper-verbose.c -c -o $(DISTDIR)/umapper-verbose.o $(CFLAGS)

clean:
	rm -rf umapper.o
	rm -rf umapper-verbose.o
	rm -rf umapper
