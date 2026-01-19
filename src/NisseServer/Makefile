CONFIG_NAME					= Nisse
THORSANVIL_ROOT				?= $(realpath ../../)
COVERAGE_REQUIRED			= 10

TARGET						= NisseServer.defer
NAMESPACE					= ThorsAnvil::Nisse
DEFER_NAME					= Nisse
LDLIBS_EXTERN_BUILD			= ThorSerialize

FILE_WARNING_FLAGS_Darwin		=
FILE_WARNING_FLAGS_Linux		=
FILE_WARNING_FLAGS_MSYS_NT		= -Wno-uninitialized
FILE_WARNING_FLAGS_MINGW64_NT	= -Wno-uninitialized
FILE_WARNING_FLAGS				+= -Wno-missing-braces  $(FILE_WARNING_FLAGS_$(PLATFORM))

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
UNITTEST_LDLIBS				+= -L $(THORSANVIL_ROOT)/src/NisseServer/debug -L $(THORSANVIL_ROOT)/src/NisseServer/release -levent $(EVENT_THREADS_LIB) -lboost_coroutine$(BOOSTLIB_EXT) -lboost_context$(BOOSTLIB_EXT)
UNITTEST_LDLIBS_HEADERONLY	= -lcrypto -lssl

%/NisseServerTest.o:		FILE_WARNING_FLAGS += -Wno-unused-variable


include $(THORSANVIL_ROOT)/build/tools/Makefile
