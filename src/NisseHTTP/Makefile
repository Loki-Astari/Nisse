THORSANVIL_ROOT             = $(realpath ../../)

TARGET						= NisseHTTP.defer
NAMESPACE					= ThorsAnvil::Nisse::NisseHTTP
DEFER_NAME					= Nisse
LDLIBS_EXTERN_BUILD			= ThorSerialize
FILE_WARNING_FLAGS			+= -Wno-missing-braces

UNITTEST_LDLIBS				+= -L $(THORSANVIL_ROOT)/src/NisseServer/debug -L $(THORSANVIL_ROOT)/src/NisseServer/release
UNITTEST_LINK_LIBS			+= NisseServer


include $(THORSANVIL_ROOT)/build/tools/Makefile
