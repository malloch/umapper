DISTDIR 	:= .
LIBDIR   := ../libmapper-install/lib/
LIBSRC := ./../libmapper/src/.libs
INCLUDE_DIR := ../libmapper-install/include/mapper-0/
CFLAGS		:= -O3 -Wall -fomit-frame-pointer -fstrength-reduce -funroll-loops -ffast-math -c -fPIC -DPIC
LDFLAGS		:=	$(LIBSRC)/*.o -llo -lz
INCLUDE		:= 	-I. -I$(INCLUDE_DIR) -I./../libmapper/src -L./$(LIBDIR)

FILES 	:= umapper

all: $(FILES)

umapper: umapper.o  umapper-verbose.o umapper-connections.o
	gcc $(LDFLAGS) \
	   umapper.o \
	  umapper-connections.o \
	  umapper-verbose.o -o \
	  $(DISTDIR)/umapper

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
