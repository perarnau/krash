ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src doc tests

EXTRA_DIST = autogen.sh version.sh ChangeLog samples debian README INSTALL COPYING

# custom rule to create a .deb file
# most of this code is extracted from the distcheck rule
# of autotools generated root Makefile
deb: dist
	case '$(DIST_ARCHIVES)' in \
	*.tar.gz*) \
	  GZIP=$(GZIP_ENV) gunzip -c $(distdir).tar.gz | $(am__untar) ;;\
	*.tar.bz2*) \
	  bunzip2 -c $(distdir).tar.bz2 | $(am__untar) ;;\
	*.tar.lzma*) \
	  unlzma -c $(distdir).tar.lzma | $(am__untar) ;;\
	*.tar.xz*) \
	  xz -dc $(distdir).tar.xz | $(am__untar) ;;\
	*.tar.Z*) \
	  uncompress -c $(distdir).tar.Z | $(am__untar) ;;\
	*.shar.gz*) \
	  GZIP=$(GZIP_ENV) gunzip -c $(distdir).shar.gz | unshar ;;\
	*.zip*) \
	  unzip $(distdir).zip ;;\
	esac
	$(am__cd) $(distdir) && \
	dpkg-buildpackage
	$(am__remove_distdir)
