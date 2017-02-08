AC_INIT([Corbenik], [0.3.0], [https://github.com/chaoskagami/corbenik], [corbenik])

export CONFIGURE_OPTIONS="$@"

AM_INIT_AUTOMAKE([foreign no-dist-gzip dist-xz subdir-objects])

# Find AR and RANLIB for lto support before initializing libtool
AC_CHECK_TOOL([AR],gcc-ar)
AC_CHECK_TOOL([RANLIB],gcc-ranlib)

AC_PROG_SED
AC_PROG_CC
AC_PROG_CXX
AM_PROG_AS

AC_CHECK_TOOL([OBJCOPY],objcopy)

LT_INIT

AC_ARG_ENABLE([chainloader],
	AS_HELP_STRING([--disable-chainloader], [Disable chainloading of external programs]))

AC_PREFIX_DEFAULT([/corbenik])

CHAINLOADER=1
test "$enable_chainloader" = "no" && CHAINLOADER=0
AC_DEFINE_UNQUOTED([CHAINLOADER], [$CHAINLOADER])

COMPILER_ID="$($CC --version | head -n1)"
AC_DEFINE_UNQUOTED([COMPILER_ID], [$COMPILER_ID])

AC_DEFINE_UNQUOTED([CONFIGURE_OPTIONS], [$CONFIGURE_OPTIONS])

AC_CONFIG_FILES([Makefile boot/Makefile external/Makefile include/Makefile bootstub/Makefile])

AC_CONFIG_SUBDIRS([external/libctr9])

AC_OUTPUT

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

Configure:       ${CONFIGURE_OPTIONS}
Compiler ID:     ${COMPILER_ID}
"

