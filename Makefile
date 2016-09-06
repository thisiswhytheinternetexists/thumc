# Makefile
# Makefile for THUM shared library and client programs
# $Revision: 1.7 $
# $Name:  $
# $Date: 2006/09/17 23:50:57 $

all: lib thumctl

lib:
	gcc -fPIC -g -c -Wall THUM.c
	gcc -shared -Wl,-soname,libTHUM.so.1 -o libTHUM.so.1 THUM.o -lc -lm

thumctl:
	gcc -Wall -w -o thumctl thumctl.c -lTHUM

install:
	cp -f libTHUM.so.1 /usr/lib
	ln -sf /usr/lib/libTHUM.so.1 /usr/lib/libTHUM.so
	/sbin/ldconfig

uninstall:
	rm -f /usr/lib/libTHUM.so
	rm -f /usr/lib/libTHUM.so.1
	/sbin/ldconfig

dist: all
	mkdir -p ../dist
	cp -f libTHUM.so.1 ../dist
	cp -f thumctl ../dist
	cp -f thumchk.pl ../dist
	chmod 755 ../dist/thumchk.pl
	cp -f thumchk.conf ../dist
	cp -f setup/install ../dist
	chmod 755 ../dist/install
	cp -f setup/uninstall ../dist
	chmod 755 ../dist/uninstall
	mkdir ../dist/src
	cp -f thumctl.c ../dist/src
	cp -f THUM.h ../dist/src
	cp -f Makefile_thumctl ../dist/src/Makefile
	cp -f ../doc/README.txt ../dist

clean:
	rm -rf THUM.o libTHUM.so.1 thumctl ../dist/* *~
