THORSANVIL_ROOT             = $(realpath ../../)

TARGET						= NisseHTTP.defer
NAMESPACE					= ThorsAnvil::Nisse::NisseHTTP
DEFER_NAME					= Nisse
LINK_LIBS					+= ThorsLogging
FILE_WARNING_FLAGS			+= -Wno-missing-braces

UNITTEST_LDLIBS				+= -L $(THORSANVIL_ROOT)/src/NisseServer/debug -L $(THORSANVIL_ROOT)/src/NisseServer/release
UNITTEST_LINK_LIBS			+= ThorsSocket NisseServer


include $(THORSANVIL_ROOT)/build/tools/Makefile
