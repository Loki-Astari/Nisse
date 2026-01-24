CONFIG_NAME					= Nisse
THORSANVIL_ROOT				?= $(realpath ../../)
TARGET						= NisseHTTP.defer
NAMESPACE					= ThorsAnvil::Nisse::NisseHTTP
DEFER_NAME					= Nisse


LDLIBS_EXTERN_BUILD			+= ThorSerialize MagicEnum event BoostCoRoutine
FILE_WARNING_FLAGS			+= -Wno-missing-braces


UNITTEST_LDLIBS				+= -L ../../NisseServer/debug -L ../../NisseServer/release
UNITTEST_LINK_LIBS			+= NisseServer
UNITTEST_LDLIBS_HEADERONLY	+= -lcrypto -lssl


%/MugServerTest.o:		FILE_WARNING_FLAGS += -Wno-unused-variable

include $(THORSANVIL_ROOT)/build/tools/Makefile
