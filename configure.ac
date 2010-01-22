# fetch version number
m4_include([version.m4])
# init autoconf
AC_INIT([krash],[VERSION_NUMBER],[krash-commits@ligforge.imag.fr])
# are we in the right source dir ?
AC_CONFIG_SRCDIR([src/krash.c])
# build tools in separate dir
AC_CONFIG_AUX_DIR([build-aux])
# macro in srcdir/m4
AC_CONFIG_MACRO_DIR([m4])

# configuration output in config.h
AC_CONFIG_HEADERS([src/config.h])

# automake should fail on any error
AM_INIT_AUTOMAKE([-Wall -Werror gnu])
# check for programs
AC_LANG_C
AC_PROG_CC
#check for libcgroup
AC_CHECK_LIB([cgroup],[cgroup_init],[],AC_MSG_ERROR("could not find the cgroup library"))

#DX_HTML_FEATURE(ON)
#DX_CHM_FEATURE(OFF)
#DX_CHI_FEATURE(OFF)
#DX_MAN_FEATURE(OFF)
#DX_RTF_FEATURE(OFF)
#DX_XML_FEATURE(OFF)
#DX_PDF_FEATURE(OFF)
#DX_PS_FEATURE(OFF)
#DX_INIT_DOXYGEN(ggen,doxygen.cfg,doxygen-doc)


# output makefiles
AC_CONFIG_FILES([Makefile src/Makefile])
# do the output
AC_OUTPUT
