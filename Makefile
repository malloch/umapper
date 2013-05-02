# PKG_CONFIG_PATH must be exported to find libmapper-0.pc

DISTDIR 	:= .
LMCFLAGS    := $(shell pkg-config --cflags libmapper-0)
LMLIBS      := $(shell pkg-config --libs libmapper-0)
CFLAGS		:= -O3 -Wall -fomit-frame-pointer -fstrength-reduce -funroll-loops -ffast-math -c -fPIC -DPIC
LDFLAGS		:=	$(LMLIBS) -lz
INCLUDE		:= 	-I. $(LMCFLAGS)

FILES 	:= umapper

all: $(FILES)

umapper: umapper.o  umapper-verbose.o umapper-connections.o
	gcc \
	   umapper.o \
	  umapper-connections.o \
	  umapper-verbose.o -o \
	  $(DISTDIR)/umapper \
	  $(LDFLAGS)

umapper.o: umapper.c
	gcc $(INCLUDE) umapper.c -c -gstabs -o $(DISTDIR)/umapper.o $(CFLAGS)

umapper-verbose.o: umapper-verbose.c
	gcc $(INCLUDE) umapper-verbose.c -c -gstabs -o $(DISTDIR)/umapper-verbose.o $(CFLAGS)

umapper-connections.o: umapper-connections.c
	gcc $(INCLUDE) umapper-connections.c -c -gstabs -o $(DISTDIR)/umapper-connections.o $(CFLAGS)

clean:
	rm -rf umapper.o
	rm -rf umapper-verbose.o
	rm -rf umapper-connections.o
	rm -rf umapper
