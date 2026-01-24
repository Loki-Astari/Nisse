CONFIG_NAME						= Nisse
THORSANVIL_ROOT					?= $(realpath ../../)
COVERAGE_REQUIRED				= 10
TARGET							= NisseServer.defer
NAMESPACE						= ThorsAnvil::Nisse
DEFER_NAME						= Nisse


LDLIBS_EXTERN_BUILD				+= ThorSerialize event BoostCoRoutine


FILE_WARNING_FLAGS_Darwin		=
FILE_WARNING_FLAGS_Linux		=
FILE_WARNING_FLAGS_MSYS_NT		= -Wno-uninitialized
FILE_WARNING_FLAGS_MINGW64_NT	= -Wno-uninitialized
FILE_WARNING_FLAGS				+= -Wno-missing-braces  $(FILE_WARNING_FLAGS_$(PLATFORM))

UNITTEST_LDLIBS					+= -L $(THORSANVIL_ROOT)/src/NisseServer/debug -L $(THORSANVIL_ROOT)/src/NisseServer/release
UNITTEST_LDLIBS_HEADERONLY		+= -lcrypto -lssl

%/NisseServerTest.o:		FILE_WARNING_FLAGS += -Wno-unused-variable


include $(THORSANVIL_ROOT)/build/tools/Makefile
