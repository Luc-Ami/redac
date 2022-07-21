### INITIALIZATION #############################################################
# In this section we do preparations for initializing our build system.

AC_INIT([redac], [0.2.3], [amiluc_bis@yahoo.fr])
AC_CONFIG_SRCDIR([src/main.c])
AM_INIT_AUTOMAKE([-W foreign])
AM_SILENT_RULES([no])
AC_CONFIG_HEADERS([config.h])  # rune ./autogeb.sh then ./configure to obtain a config.h file
### SHOPPING LIST ##############################################################
# In this section we notify autoconf of what tools we will be using.

AC_PROG_CC
PKG_PROG_PKG_CONFIG
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.1])
IT_PROG_INTLTOOL([0.40])

AC_SEARCH_LIBS([sqrt], [m])


### LIBRARIES ##################################################################
# In this section we use pkg-config to configure the libraries we are using.

PKG_CHECK_MODULES([REDAC], [
	glib-2.0
	gtk+-3.0
	gstreamer-1.0
	gtkspell3-3.0
	poppler-glib
])

### VARIABLES ##################################################################
# Here, we deal with optional packages and substitution variables.

# Needed by intltool
GETTEXT_PACKAGE=${PACKAGE_TARNAME}
AC_SUBST([GETTEXT_PACKAGE])
AC_SUBST(ALL_LINGUAS) # veriy !
AC_SUBST(LDFLAGS) 
### OUTPUT #####################################################################
# After checking all this stuff, we must output our results to the rest of the
# build system. That is what this section does.

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	pixmaps/Makefile
	po/Makefile.in
	src/icons/Makefile
	ui/Makefile
])
AC_OUTPUT
