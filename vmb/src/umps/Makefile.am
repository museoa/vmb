SUBDIRS = src support 

AUTOMAKE_OPTIONS = foreign 1.9

EXTRA_DIST = LICENSE VERSION

AM_CFLAGS = -Wall
AM_CXXFLAGS = -Wall

umpsrcdir = /etc

umpsrc_DATA = ./support/umpsrc

install-data-local:
	- cp -r *example* ${DESTDIR}/${prefix}/share/${PACKAGE_NAME}/
	- find ${DESTDIR}/${prefix}/share/${PACKAGE_NAME}/ -type d | xargs chmod 755
	- find ${DESTDIR}/${prefix}/share/${PACKAGE_NAME}/ -type f | xargs chmod 644
	- chown -R root ${DESTDIR}/${prefix}/share/${PACKAGE_NAME}/
	- chgrp -R staff ${DESTDIR}/${prefix}/share/${PACKAGE_NAME}/

uninstall-local:
	- rm -rf ${DESTDIR}/${prefix}/share/${PACKAGE_NAME}/*example*

extraclean: distclean
	rm -rf aclocal.m4 autom4te.cache autoscan.log configure.scan config.h.in* configure depcomp install-sh Makefile.in config.guess config.sub ltmain.sh missing `find -name Makefile.in` vde-*.tar.gz
