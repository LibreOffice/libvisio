ACLOCAL_AMFLAGS = -I m4

pkgconfdir = $(libdir)/pkgconfig

pkgconf_DATA = libvisio-@VSD_MAJOR_VERSION@.@VSD_MINOR_VERSION@.pc

SUBDIRS = build inc src docs

EXTRA_DIST = \
	AUTHORS \
	COPYING.MPL \
	NEWS \
	README \
	autogen.sh \
	libvisio.pc.in

distclean-local:
	rm -rf *.cache *~ *.out *.pc tags

dist-hook:
	git log --date=short --pretty="format:@%cd  %an  <%ae>  [%H]%n%n%s%n%n%e%b" | sed -e "s|^\([^@]\)|\t\1|" -e "s|^@||" >$(distdir)/ChangeLog

tags:
	ctags --c++-kinds=+p --fields=+iaS --extra=+q -R --totals=yes *

astyle:
	astyle --options=astyle.options \*.cpp \*.h
