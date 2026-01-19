CONFIG_NAME					= Nisse
THORSANVIL_ROOT				?= $(realpath ../../)

LDLIBS_EXTERN_BUILD			= ThorSerialize
TARGET						= Nisse.lib
DEFER_LIBS					= NisseServer NisseHTTP

BOOSTLIB_EXT				= $(BOOSTLIB_EXT_$(PLATFORM))
BOOSTLIB_EXT_Darwin			=
BOOSTLIB_EXT_Linux			=
BOOSTLIB_EXT_MSYS_NT		= -mt
BOOSTLIB_EXT_MINGW64_NT		= -mt
EVENT_THREADS_LIB			= $(EVENT_THREADS_LIB_$(PLATFORM))
EVENT_THREADS_LIB_Darwin	= -levent_pthreads
EVENT_THREADS_LIB_Linux		= -levent_pthreads
EVENT_THREADS_LIB_MSYS_NT	= -lws2_32
EVENT_THREADS_LIB_MINGW64_NT= -lws2_32
LDLIBS						+= -levent $(EVENT_THREADS_LIB) -lboost_coroutine$(BOOSTLIB_EXT) -lboost_context$(BOOSTLIB_EXT)

include $(THORSANVIL_ROOT)/build/tools/Makefile
