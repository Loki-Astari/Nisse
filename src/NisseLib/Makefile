CONFIG_NAME					= Nisse
THORSANVIL_ROOT				?= $(realpath ../../)

LDLIBS_EXTERN_BUILD			= ThorSerialize event
TARGET						= Nisse.lib
DEFER_LIBS					= NisseServer NisseHTTP

BOOSTLIB_EXT				= $(BOOSTLIB_EXT_$(PLATFORM))
BOOSTLIB_EXT_Darwin			=
BOOSTLIB_EXT_Linux			=
BOOSTLIB_EXT_MSYS_NT		= -mt
BOOSTLIB_EXT_MINGW64_NT		= -mt
LDLIBS						+= -lboost_coroutine$(BOOSTLIB_EXT) -lboost_context$(BOOSTLIB_EXT)

include $(THORSANVIL_ROOT)/build/tools/Makefile
