AC_INIT([Corbchain], [0.1.2], [https://github.com/chaoskagami/corbenik], [corbenik])
AM_INIT_AUTOMAKE([foreign no-dist-gzip dist-xz subdir-objects])

AC_CONFIG_MACRO_DIRS([m4])

# Find AR and RANLIB for lto support before initializing libtool
AC_CHECK_TOOL([AR],gcc-ar)
AC_CHECK_TOOL([RANLIB],gcc-ranlib)

AC_PROG_SED
AC_PROG_CC
AC_PROG_CXX
AM_PROG_AS

AC_CHECK_TOOL([OBJCOPY],objcopy)

LT_INIT

AC_PREFIX_DEFAULT([])

CHAINLOADER=1
AC_DEFINE_UNQUOTED([CHAINLOADER], [$CHAINLOADER])

AC_CONFIG_FILES([Makefile source/Makefile external/Makefile include/Makefile])

AC_CONFIG_SUBDIRS([external/libctr9])

echo "
Corbenik (chainloader only) $VERSION
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
"

AC_OUTPUT
