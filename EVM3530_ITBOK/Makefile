# Makefile - Makefile for building Diagnostic libarry
#
# Copyright Mistral software Pvt. Ltd.
#
#

#include $(TOPDIR)/config.mk

ifeq ($(BUILD_DIR),)
    LIB = libitbok.a
else
    LIB = $(BUILD_DIR)/EVM3530_ITBOK/libitbok.a
endif

LIB_DIAGS = diagnostics/

all: $(LIB_DIAGS) 
	$(MAKE) -C $(LIB_DIAGS)
	$(AR) crv $(LIB) `find . -name *.o`
