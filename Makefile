#
# File:
#    Makefile
#
# Description:
#    Makefile for the TIR Library using a VME Controller running Linux
#
#
# $Date$
# $Rev$
#
# Uncomment DEBUG line, to include some debugging info ( -g and -Wall)
#export DEBUG=1
#


ifndef LINUXVME_LIB
	LINUXVME_LIB	= $CODA/extensions/linuxvme/libs
endif
ifndef LINUXVME_INC
	LINUXVME_INC	= $CODA/extensions/linuxvme/include
endif

CROSS_COMPILE		=
CC			= $(CROSS_COMPILE)gcc
AR                      = ar
RANLIB                  = ranlib
CFLAGS			= -O2 -I. -I${LINUXVME_INC} -I/usr/include \
			  -L${LINUXVME_LIB} -L.
ifdef DEBUG
CFLAGS			+= -Wall -g
endif

OBJS			= tirLib.o

LIBS			= libtir.a

all: $(LIBS)

libtir.a: tirLib.o
	$(CC) -fpic -shared $(CFLAGS) -o libtir.so tirLib.c
	$(AR) ruv libtir.a tirLib.o
	$(RANLIB) libtir.a

clean distclean:
	@rm -f $(OBJS) $(LIBS) *.so *~

links: libtir.a
	ln -sf $(PWD)/libtir.a $(LINUXVME_LIB)/libtir.a
	ln -sf $(PWD)/libtir.so $(LINUXVME_LIB)/libtir.so
	ln -sf $(PWD)/tirLib.h $(LINUXVME_INC)/tirLib.h

%.o: %.c 
	$(CC) $(CFLAGS) -c $<

.PHONY: all clean distclean
