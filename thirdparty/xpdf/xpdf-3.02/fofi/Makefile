#========================================================================
#
# FoFi library Makefile
#
# Copyright 2003 Glyph & Cog, LLC
#
#========================================================================

SHELL = /bin/sh

srcdir = .


GOOSRCDIR = $(srcdir)/../goo
GOOLIBDIR = ../goo

CXXFLAGS = -g -O2 -DHAVE_CONFIG_H -I.. -I$(GOOSRCDIR) -I$(srcdir)

CXX = g++
AR = ar rc
RANLIB = ranlib

LIBPREFIX = lib

#------------------------------------------------------------------------

.SUFFIXES: .cc

.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

#------------------------------------------------------------------------

CXX_SRC = \
	$(srcdir)/FoFiBase.cc \
	$(srcdir)/FoFiEncodings.cc \
	$(srcdir)/FoFiTrueType.cc \
	$(srcdir)/FoFiType1.cc \
	$(srcdir)/FoFiType1C.cc

#------------------------------------------------------------------------

all: $(LIBPREFIX)fofi.a

#------------------------------------------------------------------------

FOFI_OBJS = \
	FoFiBase.o \
	FoFiEncodings.o \
	FoFiTrueType.o \
	FoFiType1.o \
	FoFiType1C.o

$(LIBPREFIX)fofi.a: $(FOFI_OBJS)
	rm -f $(LIBPREFIX)fofi.a
	$(AR) $(LIBPREFIX)fofi.a $(FOFI_OBJS)
	$(RANLIB) $(LIBPREFIX)fofi.a

#------------------------------------------------------------------------

clean:
	rm -f $(FOFI_OBJS) $(LIBPREFIX)fofi.a

#------------------------------------------------------------------------

depend:
	$(CXX) $(CXXFLAGS) -MM $(CXX_SRC) >Makefile.dep

include Makefile.dep
