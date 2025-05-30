dnl -*- Autoconf -*-
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])

AC_INIT([sleuthkit], [4.13.1])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([tsk/base/tsk_base.h])
AC_CONFIG_HEADERS([tsk/tsk_config.h])
AM_INIT_AUTOMAKE([foreign subdir-objects tar-ustar parallel-tests])
LT_INIT
AM_MAINTAINER_MODE


dnl Checks for programs.
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX([17], [noext], [mandatory])
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PATH_PROG(PERL, perl)

TSK_CHECK_PROG_PKGCONFIG

dnl Checks for header files.
dnl AC_HEADER_MAJOR
dnl AC_HEADER_SYS_WAIT
dnl AC_CHECK_HEADERS([fcntl.h inttypes.h limits.h locale.h memory.h netinet/in.h stdint.h stdlib.h string.h sys/ioctl.h sys/param.h sys/time.h unistd.h utime.h wchar.h wctype.h])
AC_CHECK_HEADERS([err.h inttypes.h unistd.h stdint.h sys/param.h sys/resource.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
dnl AC_CHECK_MEMBERS([struct stat.st_rdev])
dnl AC_STRUCT_TM

dnl check for large file support
AC_SYS_LARGEFILE

dnl Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_ERROR_AT_LINE
dnl AC_FUNC_FORK
AC_FUNC_FSEEKO
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
dnl AC_FUNC_MALLOC
dnl AC_FUNC_MBRTOWC
dnl AC_FUNC_MEMCMP
dnl AC_FUNC_MKTIME
dnl AC_FUNC_MMAP
dnl AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
dnl AC_FUNC_STAT
AC_FUNC_UTIME_NULL
AC_FUNC_VPRINTF
dnl AC_CHECK_FUNCS([dup2 gethostname isascii iswprint memset munmap regcomp select setlocale strcasecmp strchr strdup strerror strndup strrchr strtol strtoul strtoull utime wcwidth])
AC_CHECK_FUNCS([ishexnumber err errx warn warnx vasprintf getrusage])
AC_CHECK_FUNCS([strlcpy strlcat strnlen])

AX_PTHREAD([
    AC_DEFINE(HAVE_PTHREAD, 1, [Define if you have POSIX threads libraries and header files.])
    CC="$PTHREAD_CC"
    CXX="$PTHREAD_CXX"], [])

dnl Permit single-threaded builds
AC_ARG_ENABLE([multithreading],
    [AS_HELP_STRING([--disable-multithreading], [Build without multithreading support])])

dnl Enable multithreading by default in the presence of pthread
AS_IF([test "x$ax_pthread_ok" = "xyes" && test "x$enable_multithreading" != "xno"], [ax_multithread=yes], [ax_multithread=no])

dnl Add an option to build the Java (JNI) bindings
AC_ARG_ENABLE([java],
    [AS_HELP_STRING([--enable-java], [Build the Java (JNI) bindings @<:@default=no@:>@])],
    [enable_java=$enableval], [enable_java=no])

dnl Checks for libraries.

dnl Some platforms will complain about missing included functions if libstdc++ is not included.
AC_CHECK_LIB(stdc++, main, [], AC_MSG_ERROR([missing libstdc++]))
AC_SUBST([STDCPP_LIBS], ['-lstdc++'])

dnl Check for sqlite and its dependencies
AS_IF([test "x$ac_cv_prog_PKGCONFIG" = "xyes"],
  [
    SAVED_AX_PACKAGE_REQUIRES_PRIVATE="$AX_PACKAGE_REQUIRES_PRIVATE"
    TSK_PKG_CHECK_MODULES([SQLITE3], [], [sqlite3],
    [
      SQLITE3_CXXFLAGS="$SQLITE3_CFLAGS"
    ],
    [
      AX_PACKAGE_REQUIRES_PRIVATE="$SAVED_AX_PACKAGE_REQUIRES_PRIVATE"
      ax_sqlite3=no
    ]
  )]
)

dnl check again for sqlite3 if not found by pkgconfig
AS_IF([test "x$ax_sqlite3" != "xyes"],
  [
    AC_CHECK_HEADERS(
      [sqlite3.h],
      [
        AC_CHECK_LIB([dl], [dlopen], [ax_dl=yes], [])
        AC_CHECK_LIB([sqlite3], [sqlite3_open], [ax_sqlite3=yes], [])
      ]
    )
  ]
)

dnl Compile the bundled sqlite if there is no system one installed
AC_MSG_CHECKING(which sqlite3 to use)
AS_IF([test "x$ax_sqlite3" = "xyes"],
      [
        AC_MSG_RESULT([system])
        PACKAGE_LIBS_PRIVATE="$PACKAGE_LIBS_PRIVATE -lsqlite3"
        SQLITE3_LIBS='-lsqlite3'
        AS_IF([test "x$ax_dl" = "xyes"], [SQLITE3_LIBS="$SQLITE3_LIBS -ldl"])
      ],
      [AC_MSG_RESULT([bundled])
       SQLITE3_CPPFLAGS="-DSQLITE_OMIT_LOAD_EXTENSION"]
)
AM_CONDITIONAL([HAVE_LIBSQLITE3], [test "x$ax_sqlite3" = "xyes"])

AC_SUBST([SQLITE3_CPPFLAGS])
AC_SUBST([SQLITE3_CXXFLAGS])
AC_SUBST([SQLITE3_LIBS])

dnl Check if we should link with OpenSSL
TSK_OPT_DEP_CHECK([libcrypto], [CRYPTO], [libcrypto], [openssl/sha.h], [crypto], [SHA1])
dnl Check if we should link with afflib
dnl note that afflib/utils.h explicitly checks for openssl/pem.h, so we need to test for it also
TSK_OPT_DEP_CHECK([afflib], [AFFLIB], [], [afflib/afflib.h], [afflib], [af_open])
AC_CHECK_HEADERS([openssl/pem.h])
dnl Check if we should link with libaff4
TSK_OPT_DEP_CHECK([libaff4], [AFF4], [], [aff4/libaff4-c.h], [aff4], [AFF4_version])
dnl Check if we should link with zlib
TSK_OPT_DEP_CHECK([zlib], [ZLIB], [zlib], [zlib.h], [z], [inflate])
dnl Check if we should link with libbfio
TSK_OPT_DEP_CHECK([libbfio], [BFIO], [libbfio], [libbfio.h], [bfio], [libbfio_get_version])

dnl Check if we should link with libewf
TSK_OPT_DEP_CHECK([libewf], [EWF], [libewf], [libewf.h], [ewf], [libewf_get_version])
dnl Check for libewf_handle_read_buffer_at_offset, in newer versions of libewf
AS_IF(
  [test "x$ax_libewf" != "xno"],
  [AC_CHECK_LIB(
    [ewf],
    [libewf_handle_read_buffer_at_offset],
    [AC_DEFINE([HAVE_LIBEWF_HANDLE_READ_BUFFER_AT_OFFSET], [1], [Define to 1 if you have the libewf_handle_read_buffer_at_offset function.])]
  )]
)

dnl Check if we should link with libqcow
TSK_OPT_DEP_CHECK([libqcow], [QCOW], [libqcow], [libqcow.h], [qcow], [libqcow_get_version])
dnl Check if we should link with libvhdi
TSK_OPT_DEP_CHECK([libvhdi], [VHDI], [libvhdi], [libvhdi.h], [vhdi], [libvhdi_get_version])
dnl Check if we should link with libvmdk
TSK_OPT_DEP_CHECK([libvmdk], [VMDK], [libvmdk], [libvmdk.h], [vmdk], [libvmdk_get_version])

dnl Check that libbfio is available if libvslvm was requested
AS_IF(
  [test "x$ax_libbfio" = "xno" -a "x$with_libvslvm" != "xno" -a -n "$with_libvslvm"],
  [AC_MSG_FAILURE([libvslvm requires libbfio])]
)

dnl Check if we should link with libvslvm
TSK_OPT_DEP_CHECK([libvslvm], [VSLVM], [libvslvm], [libvslvm.h], [vslvm], [libvslvm_get_version])

dnl check for user online input
AC_ARG_ENABLE([offline],
    [ AS_HELP_STRING([--enable-offline],[Turn on offline mode])],
    [case "${enableval}" in
	yes) offline=true ;;
	no) offline=false ;;
	*)
	    offline=true
	    default_jar_location="${enableval}"
	    ;;
     esac],[offline=false])

AM_CONDITIONAL([OFFLINE], [test "x$offline" = xtrue])

AM_CONDITIONAL([CUSTOM_DEFAULT_JAR_LOCATION], [test "x$default_jar_location" != "x"])
AM_COND_IF([CUSTOM_DEFAULT_JAR_LOCATION],
    [AC_SUBST([DEFAULT_JAR_LOCATION], [$default_jar_location])]
)

dnl Test for the various Java things that we need for bindings
dnl First, test is ant is available since we now use it for the build system
dnl But don't make it required
AC_PATH_PROG([ANT], [ant], [no])
AM_CONDITIONAL([ANT_DETECTED], [test "x$ANT" != "xno"])
AS_IF([test "x$enable_java" != "xno"], [
    dnl javac is needed to compile the JAR file
    AX_PROG_JAVAC
    AS_IF(
        [test "x$JAVAC" = x],
        [enable_java=no],
        [
	AX_JNI_INCLUDE_DIR
        for JNI_INCLUDE_DIR in $JNI_INCLUDE_DIRS
        do
            JNI_CPPFLAGS="$JNI_CPPFLAGS -I$JNI_INCLUDE_DIR"
        done
        dnl Export the paths so that the Makefile gets them
        AC_SUBST(JNI_CPPFLAGS, $JNI_CPPFLAGS)

        dnl java is needed by ant
        dnl we had one report of a system with javac and not java
        AX_PROG_JAVA

        if test "x$ANT" = "xno"; then
            AC_MSG_ERROR([Apache Ant is required but was not found. Install Ant and try again.])
        fi
        AC_SUBST(ANT)
	]
    )
]) dnl test enable_java



dnl if we found everything we need, set ax_java_support for the
dnl status message and set X_JNI for use in Makefile
AS_IF([test "x$JNI_CPPFLAGS" != x && test "x$ANT" != x && test "x$JAVA" != x], [ax_java_support=yes], [ax_java_support=no])
AM_CONDITIONAL([X_JNI],[test "x$ax_java_support" = "xyes"])

dnl Dependencies for OSS-Fuzz
AM_CONDITIONAL(HAVE_LIB_FUZZING_ENGINE, [test "x${LIB_FUZZING_ENGINE}" != x])
AC_SUBST([LIB_FUZZING_ENGINE], ["${LIB_FUZZING_ENGINE}"])

AC_CONFIG_COMMANDS([tsk/tsk_incs.h],
    [echo "#ifndef _TSK_INCS_H" > tsk/tsk_incs.h
    echo "#define _TSK_INCS_H" >> tsk/tsk_incs.h
    echo "// automatically by ./configure" >> tsk/tsk_incs.h
    echo "// Contains the config.h data needed by programs that use libtsk" >> tsk/tsk_incs.h
    echo "" >> tsk/tsk_incs.h
    if test x$ac_cv_header_unistd_h = xyes; then
        echo "#include <unistd.h>" >> tsk/tsk_incs.h
    fi
    if test x$ac_cv_header_inttypes_h = xyes; then
        echo "#ifndef __STDC_FORMAT_MACROS" >> tsk/tsk_incs.h
        echo "#define  __STDC_FORMAT_MACROS" >> tsk/tsk_incs.h
        echo "#endif" >> tsk/tsk_incs.h
        echo "#include <inttypes.h>" >> tsk/tsk_incs.h
    fi
    if test x$ac_cv_header_sys_param_h = xyes; then
        echo "#include <sys/param.h>" >> tsk/tsk_incs.h
    fi

    if test x$ax_multithread = xyes; then
        echo "#define TSK_MULTITHREAD_LIB // enable multithreading" >> tsk/tsk_incs.h
    fi

    echo "" >> tsk/tsk_incs.h
    echo "#endif" >> tsk/tsk_incs.h],

    [ac_cv_header_unistd_h=$ac_cv_header_unistd_h
    ac_cv_header_inttypes_h=$ac_cv_header_inttypes_h
    ac_cv_header_sys_param_h=$ac_cv_header_sys_param_h
    ax_multithread=$ax_multithread])

dnl ################################################################
dnl ## Set TSK_LIB as appropriate for the host
dnl ##
AC_CANONICAL_HOST

case "$host_os" in
    darwin*)
        TSK_LIB="libtsk.dylib"
        ;;
    linux*)
        TSK_LIB="libtsk.so"
        ;;
    *)
        TSK_LIB="libtsk.la"  # Default for unknown platforms
        ;;
esac
AC_SUBST(TSK_LIB)
dnl ################


dnl ################################################################
dnl ## set per-host libtool flags

AC_MSG_CHECKING([if libtool needs -no-undefined flag to build shared libraries])
case "$host" in
*-*-mingw*)
  dnl Add -no-undefined flag to LDFLAGS to let libtool build DLLs.
  AC_MSG_RESULT([yes])
  LIBTSK_LDFLAGS="-no-undefined"
  AC_SUBST([LIBTSK_LDFLAGS])
  ;;
*)
  dnl No additional flags needed.
  AC_MSG_RESULT([no])
  ;;
esac

dnl Run test executables in Wine if cross-compiling with mingw
case "$host" in
*-*-mingw*)
  case "$build" in
  *-*-cygwin*|*-*-mingw*)
    ;;
  *)
    use_wine=yes
    ;;
  esac
  ;;
esac

AM_CONDITIONAL([USE_WINE], [test "x$use_wine" = "xyes"])

dnl ensure static linkage of tools during static builds
if test "x$enable_shared" != "xyes"; then
  CXXLD="$CXX -all-static"
else
  CXXLD="$CXX"
fi

AM_CONDITIONAL([BUILD_STATIC], [test "x$enable_shared" != "xyes"])
AC_SUBST([CXXLD])

dnl Dependencies for fiwalk
AC_CHECK_FUNCS([getline])

TSK_ADDRESS_SANITIZER
TSK_THREAD_SANITIZER
TSK_UNDEFINED_SANITIZER

dnl Defines for supported image types
AM_CONDITIONAL([HAVE_AFF], [test "x$ax_afflib" != "xno"])
AM_CONDITIONAL([HAVE_E01], [test "x$ax_libewf" != "xno"])
AM_CONDITIONAL([HAVE_QCOW], [test "x$ax_libqcow" != "xno"])
AM_CONDITIONAL([HAVE_VHD], [test "x$ax_libvhdi" != "xno"])
AM_CONDITIONAL([HAVE_VMDK], [test "x$ax_libvmdk" != "xno"])

AC_CONFIG_FILES([
    Makefile
    tsk/tsk.pc])

AC_OUTPUT

dnl Print a summary
AC_MSG_NOTICE([
Building:
   afflib support:                        $ax_afflib
   libaff4 support:                       $ax_libaff4
   libbfio support:                       $ax_libqcow
   libewf support:                        $ax_libewf
   libqcow support:                       $ax_libbfio
   libvhdi support:                       $ax_libvhdi
   libvmdk support:                       $ax_libvmdk
   libvslvm support:                      $ax_libvslvm

   openssl support:                       $ax_libcrypto
   zlib support:                          $ax_zlib

Features:
   Java/JNI support:                      $ax_java_support
   Multithreading:                        $ax_multithread
])
