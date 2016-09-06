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
    gcc -Wall -w -o thumctl libTHUM.so.1 THUM.o -lc -lm thumctl.c

install:
    cp -f libTHUM.so.1 /usr/lib
    ln -sf /usr/lib/libTHUM.so.1 /usr/lib/libTHUM.so
    /sbin/ldconfig

uninstall:
    rm -f /usr/lib/libTHUM.so
    rm -f /usr/lib/libTHUM.so.1
    /sbin/ldconfig

clean:
    rm -rf THUM.o libTHUM.so.1 thumctl