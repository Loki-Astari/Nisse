AC_INIT([Nisse], [0.1], [Loki.Astari+Nisse@gmail.com])
AC_PREREQ([2.65])

AC_CONFIG_MACRO_DIR([build/autotools/m4])
AC_CONFIG_AUX_DIR([build/autotools/build])

AX_THOR_FUNC_INIT_BUILD([Nisse], [src/NisseServer/NisseServer.cpp], [20])


#
# Add extra configuration here
#


LT_INIT

AX_THOR_FUNC_TERM_BUILD([NISSE], [src/NisseServer/NisseConfig.h:config.h.in src/PyntHTTP/PyntHTTPConfig.h:config.h.in])

AC_OUTPUT
