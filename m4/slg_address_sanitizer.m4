################################################################
## AddressSanitizer support
# https://github.com/libMesh/libmesh/issues/1396
AC_DEFUN([TSK_ADDRESS_SANITIZER], [
  AC_ARG_ENABLE(
    [address-sanitizer],
    [AS_HELP_STRING(
      [--enable-address-sanitizer],
      [enabled AddressSanitizer support for detecting memory allocation and deallocation errors]
    )],
    [
      AC_DEFINE(HAVE_ADDRESS_SANITIZER, 1, [enable AddressSanitizer])
      address_sanitizer="yes"
      CFLAGS="$CFLAGS     -fno-omit-frame-pointer -fsanitize=address -fsanitize-address-use-after-scope"
      CXXFLAGS="$CXXFLAGS -fno-omit-frame-pointer -fsanitize=address -fsanitize-address-use-after-scope"
      LDFLAGS="$LDFLAGS -fsanitize=address -g"
    ],
    []
  )
])

AC_DEFUN([TSK_THREAD_SANITIZER], [
  AC_ARG_ENABLE(
    [thread-sanitizer],
    [AS_HELP_STRING(
      [--enable-thread-sanitizer],
      [enabled ThreadSanitizer support for detecting thread interlocking errors]
    )],
    [
      AC_DEFINE(HAVE_THREAD_SANITIZER, 1, [enable ThreadSanitizer])
      thread_sanitizer="yes"
      CFLAGS="$CFLAGS -fsanitize=thread"
      CXXFLAGS="$CXXFLAGS -fsanitize=thread"
      LDFLAGS="$LDFLAGS -fsanitize=thread -g"
    ],
    []
  )
])

AC_DEFUN([TSK_UNDEFINED_SANITIZER], [
  AC_ARG_ENABLE(
    [undefined-sanitizer],
    [AS_HELP_STRING(
      [--enable-undefined-sanitizer],
      [enabled UndefinedSanitizer support for detecting use before assignment]
    )],
    [
      AC_DEFINE(HAVE_UNDEFINED_SANITIZER, 1, [enable UndefinedSanitizer])
      undefined_sanitizer="yes"
      CFLAGS="$CFLAGS -fsanitize=undefined"
      CXXFLAGS="$CXXFLAGS -fsanitize=undefined"
      LDFLAGS="$LDFLAGS -fsanitize=undefined -g"
    ],
    []
  )
])
