#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([mnid], [0.2.10], [mnid-devel@org3.net])
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AC_PREREQ([2.59]) 

AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_DISABLE_STATIC

# Checks for libraries.
# Checks for glib-2.0
AM_PATH_GLIB_2_0(2.4.0,, AC_MSG_ERROR(Test for GLib failed. See the 'INSTALL' for help.))
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

# Checks for gettext
ALL_LINGUAS="ja"
GETTEXT_PACKAGE=$PACKAGE
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Define text domain.])

#AM_GNU_GETTEXT_VERSION(0.10.40)
#AM_GNU_GETTEXT
AM_GLIB_GNU_GETTEXT
localedir='${prefix}/${DATADIRNAME}/locale'
AC_SUBST(localedir)

# Checks for iconv
AM_ICONV
AC_CHECK_FUNCS(bind_textdomain_codeset)

# Checks for gtk-2.0
AM_PATH_GTK_2_0(2.4.0,, AC_MSG_ERROR(Test for Gtk failed. See the 'INSTALL' for help.))
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

# Checks for libxml2
AM_PATH_XML2(2.6.0,, AC_MSG_ERROR(Test for libxml2 failed. See the 'INSTALL' for help.))

AC_SUBST(XML_CPPFLAGS)
AC_SUBST(XML_LIBS)

# Checks for gpgme
AC_ARG_ENABLE(gpgme,
	[  --enable-gpgme          Enable GnuPG support using GPGME [default=no]],
	[ac_cv_enable_gpgme=$enableval], [ac_cv_enable_gpgme=no])
AC_MSG_CHECKING([whether to use GPGME])
if test $ac_cv_enable_gpgme = yes; then
	AC_MSG_RESULT(yes)
	AM_PATH_GPGME(0.4.5, AC_DEFINE(USE_GPGME, 1, Define if you use GPGME to support OpenPGP.),
		      [use_gpgme=no
		       ac_cv_enable_gpgme=no])
else
	AC_MSG_RESULT(no)
fi

# Checks for openssl
AC_MSG_CHECKING([for openssl])
LIBS="$LIBS -lssl -lcrypto"
AC_TRY_LINK([
#include <openssl/opensslv.h>
],	[ return OPENSSL_VERSION_NUMBER; ],
	[ AC_MSG_RESULT(yes) ],
	[ AC_MSG_RESULT(no)
	  AC_MSG_ERROR(Test for OpenSSL failed. See the 'INSTALL' for help.)])

# Checks for cracklib
AC_SUBST(LIBCRACK)
AC_ARG_ENABLE(crack,
	[  --enable-crack          Enable CRACK support [default=no]],
	[ac_cv_enable_crack=$enableval], [ac_cv_enable_crack=no])
AC_MSG_CHECKING([whether to use CRACK])
if test "$ac_cv_enable_crack" = yes; then
	AC_MSG_RESULT(yes)

	AC_CHECK_LIB(crack, FascistCheck, LIBS="$LIBS -lcrack")
	AC_CHECK_HEADER(packer.h,
			[ac_cv_enable_carck=yes],
			[ac_cv_enable_carck=no])
	AC_MSG_CHECKING([whether crack library is available])
	AC_MSG_RESULT($ac_cv_enable_crack)

	if test "$ac_cv_enable_crack" = yes; then
		AC_DEFINE(USE_CRACK, 1, Define if you want CRACK support.)
	fi
else
	AC_MSG_RESULT(no)
fi

# Checks for cracklib dict
AC_MSG_CHECKING([where cracklib_dict is located])
AC_ARG_ENABLE(cracklib_dict,
	[  --with-cracklib-dict    Dicrectory where cracklib_dict is located],
	[ac_cv_enable_cracklib_dict=$enableval], [ac_cv_enable_cracklib_dict=""])
if test "$ac_cv_enable_cracklib_dict" = ""; then
	DICT_PREFIX_CANDIDATES="/usr /var /sw /usr/local"
	DICT_DIR_CANDIDATES="lib share share/lib share/cracklib cracklib"
	DICT_FILE_CANDIDATES="pw_dict cracklib_dict"
	CRACKLIB_DICTPATH=""
	for p in $DICT_PREFIX_CANDIDATES; do
		for d in $DICT_DIR_CANDIDATES; do
			for f in $DICT_FILE_CANDIDATES; do
				if test -r $p/$d/$f.hwm; then
					CRACKLIB_DICTPATH=$p/$d/$f
					break 3
				elif test -r $p/$d/dict/$f.hwm; then
					CRACKLIB_DICTPATH=$p/$d/dict/$f
					break 3
				fi
			done
		done
	done
else
	CRACKLIB_DICTPATH=$ac_cv_enable_cracklib_dict
fi
if test -z "$CRACKLIB_DICTPATH"; then
	AC_MSG_RESULT(no)
else
	AC_MSG_RESULT($CRACKLIB_DICTPATH)
fi
AC_SUBST(CRACKLIB_DICTPATH)

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([libintl.h getopt.h unistd.h sys/utsname.h sys/time.h fcntl.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_STRUCT_TM

AC_SYS_LARGEFILE
AC_FUNC_FSEEKO

# Checks for library functions.
AC_FUNC_FORK
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_LSTAT
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([memmove memset mkdir strcasecmp strncasecmp strchr putenv setlocale uname])

#AC_CONFIG_FILES([Makefile])
AC_OUTPUT([
	   Makefile
	   mnid.spec
	   po/Makefile.in
	   src/version.h
	   src/Makefile
	   pixmaps/Makefile
])

