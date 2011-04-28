#========================================================================
#
# Main xpdf Makefile.
#
# Copyright 1996-2003 Glyph & Cog, LLC
#
#========================================================================

SHELL = /bin/sh

DESTDIR =

prefix = /usr/local
exec_prefix = ${prefix}
srcdir = .

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644

EXE = 

all:
	cd goo; $(MAKE)
	cd fofi; $(MAKE)
	cd splash; $(MAKE)
	cd xpdf; $(MAKE) all-no-x

all-no-x:
	cd goo; $(MAKE)
	cd fofi; $(MAKE)
	cd xpdf; $(MAKE) all-no-x

xpdf: dummy
	cd goo; $(MAKE)
	cd fofi; $(MAKE)
	cd splash; $(MAKE)
	cd xpdf; $(MAKE) xpdf$(EXE)

pdftops: dummy
	cd goo; $(MAKE)
	cd fofi; $(MAKE)
	cd splash; $(MAKE)
	cd xpdf; $(MAKE) pdftops$(EXE)

pdftotext: dummy
	cd goo; $(MAKE)
	cd fofi; $(MAKE)
	cd splash; $(MAKE)
	cd xpdf; $(MAKE) pdftotext$(EXE)

pdfinfo:
	cd goo; $(MAKE)
	cd fofi; $(MAKE)
	cd splash; $(MAKE)
	cd xpdf; $(MAKE) pdfinfo$(EXE)

pdffonts:
	cd goo; $(MAKE)
	cd fofi; $(MAKE)
	cd splash; $(MAKE)
	cd xpdf; $(MAKE) pdffonts$(EXE)

pdftoppm:
	cd goo; $(MAKE)
	cd fofi; $(MAKE)
	cd splash; $(MAKE)
	cd xpdf; $(MAKE) pdftoppm$(EXE)

pdfimages:
	cd goo; $(MAKE)
	cd fofi; $(MAKE)
	cd splash; $(MAKE)
	cd xpdf; $(MAKE) pdfimages$(EXE)

dummy:

install: dummy
	-mkdir -p $(DESTDIR)${exec_prefix}/bin
#	$(INSTALL_PROGRAM) xpdf/xpdf$(EXE) $(DESTDIR)${exec_prefix}/bin/xpdf$(EXE)
	$(INSTALL_PROGRAM) xpdf/pdftops$(EXE) $(DESTDIR)${exec_prefix}/bin/pdftops$(EXE)
	$(INSTALL_PROGRAM) xpdf/pdftotext$(EXE) $(DESTDIR)${exec_prefix}/bin/pdftotext$(EXE)
	$(INSTALL_PROGRAM) xpdf/pdfinfo$(EXE) $(DESTDIR)${exec_prefix}/bin/pdfinfo$(EXE)
	$(INSTALL_PROGRAM) xpdf/pdffonts$(EXE) $(DESTDIR)${exec_prefix}/bin/pdffonts$(EXE)
#	$(INSTALL_PROGRAM) xpdf/pdftoppm$(EXE) $(DESTDIR)${exec_prefix}/bin/pdftoppm$(EXE)
	$(INSTALL_PROGRAM) xpdf/pdfimages$(EXE) $(DESTDIR)${exec_prefix}/bin/pdfimages$(EXE)
	-mkdir -p $(DESTDIR)${prefix}/man/man1
#	$(INSTALL_DATA) $(srcdir)/doc/xpdf.1 $(DESTDIR)${prefix}/man/man1/xpdf.1
	$(INSTALL_DATA) $(srcdir)/doc/pdftops.1 $(DESTDIR)${prefix}/man/man1/pdftops.1
	$(INSTALL_DATA) $(srcdir)/doc/pdftotext.1 $(DESTDIR)${prefix}/man/man1/pdftotext.1
	$(INSTALL_DATA) $(srcdir)/doc/pdfinfo.1 $(DESTDIR)${prefix}/man/man1/pdfinfo.1
	$(INSTALL_DATA) $(srcdir)/doc/pdffonts.1 $(DESTDIR)${prefix}/man/man1/pdffonts.1
#	$(INSTALL_DATA) $(srcdir)/doc/pdftoppm.1 $(DESTDIR)${prefix}/man/man1/pdftoppm.1
	$(INSTALL_DATA) $(srcdir)/doc/pdfimages.1 $(DESTDIR)${prefix}/man/man1/pdfimages.1
	-mkdir -p $(DESTDIR)${prefix}/man/man5
	$(INSTALL_DATA) $(srcdir)/doc/xpdfrc.5 $(DESTDIR)${prefix}/man/man5/xpdfrc.5
	-mkdir -p $(DESTDIR)${prefix}/etc
	@if test ! -f $(DESTDIR)${prefix}/etc/xpdfrc; then \
		echo "$(INSTALL_DATA) $(srcdir)/doc/sample-xpdfrc $(DESTDIR)${prefix}/etc/xpdfrc"; \
		$(INSTALL_DATA) $(srcdir)/doc/sample-xpdfrc $(DESTDIR)${prefix}/etc/xpdfrc; \
	else \
		echo "# not overwriting the existing $(DESTDIR)${prefix}/etc/xpdfrc"; \
	fi

clean:
	-cd goo; $(MAKE) clean
	-cd fofi; $(MAKE) clean
	-cd splash; $(MAKE) clean
	-cd xpdf; $(MAKE) clean

distclean: clean
	rm -f config.log config.status config.cache
	rm -f aconf.h
	rm -f Makefile goo/Makefile xpdf/Makefile
	rm -f goo/Makefile.dep fofi/Makefile.dep splash/Makefile.dep xpdf/Makefile.dep
	rm -f goo/Makefile.in.bak fofi/Makefile.in.bak splash/Makefile.in.bak xpdf/Makefile.in.bak
	touch goo/Makefile.dep
	touch fofi/Makefile.dep
	touch splash/Makefile.dep
	touch xpdf/Makefile.dep

depend:
	cd goo; $(MAKE) depend
	cd fofi; $(MAKE) depend
	cd splash; $(MAKE) depend
	cd xpdf; $(MAKE) depend
