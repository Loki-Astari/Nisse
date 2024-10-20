AC_INIT([Nisse], [0.1], [Loki.Astari+Nisse@gmail.com])
AC_PREREQ([2.65])

AC_CONFIG_MACRO_DIR([build/autotools/m4])
AC_CONFIG_AUX_DIR([build/autotools/build])

AX_THOR_FUNC_INIT_BUILD([Nisse], [src/NisseServer/NisseServer.cpp], [20])


#
# Add extra configuration here
#
AX_THOR_CHECK_USE_EVENT
AX_THOR_CHECK_USE_BOOST([1.70], [], [
    AC_MSG_ERROR([
        Nisse requires boost CoRoutine2
        The minimum version we support is via boost 1.70 please upgrade your boost libraries to this (or later).
    ])
])


LT_INIT

AX_THOR_FUNC_TERM_BUILD([NISSE], [src/NisseServer/NisseConfig.h:config.h.in src/PyntHTTP/HTTPConfig.h:config.h.in])

AC_OUTPUT
