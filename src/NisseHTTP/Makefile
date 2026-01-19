CONFIG_NAME					= Nisse
THORSANVIL_ROOT				?= $(realpath ../../)

TARGET						= NisseHTTP.defer
NAMESPACE					= ThorsAnvil::Nisse::NisseHTTP
DEFER_NAME					= Nisse
LDLIBS_EXTERN_BUILD			= ThorSerialize MagicEnum
FILE_WARNING_FLAGS			+= -Wno-missing-braces

BOOSTLIB_EXT				= $(BOOSTLIB_EXT_$(PLATFORM))
BOOSTLIB_EXT_Darwin			=
BOOSTLIB_EXT_Linux			=
BOOSTLIB_EXT_MSYS_NT		= -mt
BOOSTLIB_EXT_MINGW64_NT		= -mt
EVENT_THREADS_LIB			= $(EVENT_THREADS_LIB_$(PLATFORM))
EVENT_THREADS_LIB_Darwin	= -levent_pthreads
EVENT_THREADS_LIB_Linux		= -levent_pthreads
EVENT_THREADS_LIB_MSYS_NT	= -lws2_32
EVENT_THREADS_LIB_MING64_NT	= -lws2_32
UNITTEST_LDLIBS				+= -L ../../NisseServer/debug -L ../../NisseServer/release -levent $(EVENT_THREADS_LIB) -lboost_coroutine$(BOOSTLIB_EXT) -lboost_context$(BOOSTLIB_EXT)
UNITTEST_LINK_LIBS			+= NisseServer
UNITTEST_LDLIBS_HEADERONLY	= -lcrypto -lssl

%/MugServerTest.o:		FILE_WARNING_FLAGS += -Wno-unused-variable

include $(THORSANVIL_ROOT)/build/tools/Makefile
