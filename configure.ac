AC_INIT([Nissa], [0.1], [Loki.Astari+Nissa@gmail.com])
AC_PREREQ([2.65])

AC_CONFIG_MACRO_DIR([build/autotools/m4])
AC_CONFIG_AUX_DIR([build/autotools/build])

AX_THOR_FUNC_INIT_BUILD([Nissa], [src/Nissa/Nissa.cpp], [20])


#
# Add extra configuration here
#


LT_INIT

AX_THOR_FUNC_TERM_BUILD([NISSA], [src/Nissa/NissaConfig.h:config.h.in])

AC_OUTPUT
