#
# File:
#    Makefile
#
# Description:
#    Makefile for the JLAB examples.
#
#
# $Date$
# $Rev$
#

CROSS_COMPILE		=
CC			= $(CROSS_COMPILE)gcc
AR                      = ar
RANLIB                  = ranlib
CFLAGS			= -Wall -O2 -I. -I${HOME}/include_devel -I/usr/include \
			  -L${HOME}/lib_devel -L/usr/lib/gef -L.

OBJS			= tirLib.o

LIBS			= libtir.a

all: $(LIBS)

libtir.a: tirLib.o
	$(CC) -fpic -shared $(CFLAGS) -o libtir.so tirLib.c
	$(AR) ruv libtir.a tirLib.o
	$(RANLIB) libtir.a


clean distclean:
	@rm -f $(OBJS) $(LIBS)

%: %.c libtir.a 
	$(CC) $(CFLAGS) -o $@ $(@:%=%.c) $(LIBS_$@) -lrt -lvme -ltir 

.PHONY: all clean distclean
