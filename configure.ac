AC_INIT([Corbenik], [0.1.2], [https://github.com/chaoskagami/corbenik], [])
AM_INIT_AUTOMAKE([foreign no-dist-gzip dist-xz subdir-objects])

AC_CONFIG_MACRO_DIRS([m4])

# Find AR and RANLIB for lto support before initializing libtool
AC_CHECK_TOOL([AR],gcc-ar)
AC_CHECK_TOOL([RANLIB],gcc-ranlib)
LT_INIT

AC_PROG_SED
AC_PROG_CC
AC_PROG_CXX
AM_PROG_AS
AC_CHECK_TOOL([OBJCOPY],objcopy)

AC_ARG_ENABLE([chainloader],
	AS_HELP_STRING([--disable-chainloader], [Disable chainloading of external programs]))

AC_PREFIX_DEFAULT([/corbenik])

CHAINLOADER=1
test "$enable_chainloader" = "no" && CHAINLOADER=0

AC_DEFINE_UNQUOTED([CHAINLOADER], [$CHAINLOADER])

AC_CONFIG_FILES([Makefile source/Makefile external/Makefile include/Makefile])

AC_CONFIG_SUBDIRS([external/libctr9])

echo "
Corbenik $VERSION
========================
Prefix:          ${prefix}

cc:              ${CC}
cxx:             ${CXX}
ld:              ${LD}
oc:              ${OBJCOPY}

sed:             ${SED}

cflags:          ${CFLAGS}
cxxflags:        ${CXXFLAGS}
ldflags:         ${LDFLAGS}
ocflags:         ${OCFLAGS}

Chainloader:     ${CHAINLOADER}
"

AC_OUTPUT
