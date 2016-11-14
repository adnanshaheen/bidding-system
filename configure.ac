AC_PREREQ([2.65])
AC_INIT([project0], [1.0], [adshahee@my.bridgeport.edu])
AC_CONFIG_SRCDIR([src/main.cpp])
AM_INIT_AUTOMAKE([${PACKAGE_NAME}], [${PACKAGE_VERSION}])
AC_CONFIG_HEADERS([config.h])

# Options
AC_ARG_ENABLE(
	[debug],
	[AS_HELP_STRING([--enable-debug], [enable debugging code and output])],
	,
	[enable_debug="no"]
)

AC_ARG_ENABLE(
	[warnings],
	[AS_HELP_STRING([--enable-warnings],[enable compiler warnings])],
	,
	[enable_warnings="no"]
)

AC_ARG_ENABLE(
	[ipv6],
	[AS_HELP_STRING([--enable-ipv6], [enable ipv6 for networking])],
	,
	[enable_ipv6="no"]
)

# Test options
if test "${enable_warnings}" = "yes"; then
	CXXFLAGS="${CFLAGS} -W -Wall -Waggregate-return -Wbad-function-cast -Wcast-align -Wcast-qual -Wdisabled-optimization -Wdiv-by-zero -Wfloat-equal -Winline -Wmissing-declarations -Wmissing-format-attribute -Wmissing-noreturn -Wmissing-prototypes -Wmultichar -Wnested-externs -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-compare -Wstrict-prototypes -Wundef -Wwrite-strings -Wformat -Wformat-security -Wuninitialized"
fi

if test "${enable_debug}" = "yes"; then
	CXXFLAGS="${CFLAGS} -ggdb3 -DDEBUG"
	AC_DEFINE(
		[ENABLE_DEBUG],
		[1],
		[Define to 1 if debug should be enabled]
	)
fi

if test "${enable_ipv6}" = "yes"; then
	AC_DEFINE(
		[ENABLE_IPV6],
		[1],
		[Define to 1 if ipv6 should be enabled]
	)
fi

# Check programs
AC_PROG_CPP
AC_PROG_CXX

# Check libraries

# Check headers
AC_CHECK_HEADERS([unistd.h \
		  stdlib.h \
		  stdio.h \
		  stdarg.h \
		  string.h \
		  errno.h \
		  assert.h \
		  signal.h \
		  getopt.h \
		  netdb.h \
		  fcntl.h \
		  sys/stat.h \
		  arpa/inet.h \
		  sys/time.h \
		  sys/wait.h \
		  netinet/in.h \
		  sys/socket.h])

# Check for typedefs, structures, and compiler characteristics

# Check for library functions

# Output files
AC_CONFIG_FILES([Makefile
		 src/Makefile])
AC_OUTPUT

