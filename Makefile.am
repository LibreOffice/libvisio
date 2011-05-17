pkgconfdir = $(libdir)/pkgconfig

pkgconf_DATA = libvisio-@VSD_MAJOR_VERSION@.@VSD_MINOR_VERSION@.pc

SUBDIRS = src build docs

EXTRA_DIST = \
	AUTHORS \
	COPYING \
	NEWS \
	README \
	autogen.sh \
	libvisio-@VSD_MAJOR_VERSION@.@VSD_MINOR_VERSION@.pc.in \
	libvisio-zip.in \
	libvisio.spec \
	libvisio.spec.in

distclean-local:
	rm -rf *.cache *~

rpm: dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz

zip: all install
	sh libvisio-zip 

dist-hook:
	git log --date=short --pretty="format:@%cd  %an  <%ae>  [%H]%n%n%s%n%n%e%b" | sed -e "s|^\([^@]\)|\t\1|" -e "s|^@||" >$(distdir)/ChangeLog

