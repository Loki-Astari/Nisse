CONFIG_NAME					= Nisse
THORSANVIL_ROOT             = $(realpath ../../)

TARGET						= NisseHTTP.defer
NAMESPACE					= ThorsAnvil::Nisse::NisseHTTP
DEFER_NAME					= Nisse
LDLIBS_EXTERN_BUILD			= ThorSerialize MagicEnum
FILE_WARNING_FLAGS			+= -Wno-missing-braces

UNITTEST_LDLIBS				+= -L ../../NisseServer/debug -L ../../NisseServer/release -levent -levent_pthreads -lboost_coroutine -lboost_context
UNITTEST_LINK_LIBS			+= NisseServer
UNITTEST_LDLIBS_HEADERONLY	= -lcrypto -lssl


include $(THORSANVIL_ROOT)/build/tools/Makefile
