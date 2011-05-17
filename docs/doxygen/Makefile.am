EXTRA_DIST =	doxygen.cfg \
		footer.html.in \
		header.html.in

if WITH_LIBVISIO_DOCS

install-data-am:
	mkdir -p $(DESTDIR)$(docdir)/html
	$(INSTALL_DATA) html/*.html $(DESTDIR)$(docdir)/html/
	$(INSTALL_DATA) html/*.png $(DESTDIR)$(docdir)/html/
	$(INSTALL_DATA) html/*.css $(DESTDIR)$(docdir)/html/

uninstall-am:
	-rm -rf $(DESTDIR)$(docdir)/html

all:
	test -f header.html.in || $(LN_S) $(srcdir)/header.html.in ./
	test -f footer.html.in || $(LN_S) $(srcdir)/footer.html.in ./
	doxygen $(srcdir)/doxygen.cfg

else

all:

endif

distclean-local: clean-local

clean-local:
	rm -rf html
