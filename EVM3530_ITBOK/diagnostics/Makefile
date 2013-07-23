# Makefile - Makefile for building Diagnostic libarry
#
# Copyright Mistral software Pvt. Ltd.
#
#

include $(TOPDIR)/config.mk

# Diagnostic files
LIB = libdiagnostic.a

COBJS = src/dg_ram.o

OBJS = $(AOBJS) $(COBJS)

CFLAGS += -I../common/inc -Iinc

all: $(LIB)
	$(AR) crv $(LIB) $(OBJS)

$(LIB): .depend $(OBJS)

.depend:	Makefile $(AOBJS:.o=.S) $(COBJS:.o=.c)
	$(CC) -M $(CFLAGS) $(AOBJS:.o=.S) $(COBJS:.o=.c) > $@

sinclude .depend
