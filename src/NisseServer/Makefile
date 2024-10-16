THORSANVIL_ROOT             = $(realpath ../../)

TARGET						= NisseServer.defer
NAMESPACE					= ThorsAnvil::Nisse
DEFER_NAME					= Nisse

XLDLIBS						+= -levent -levent_pthreads -lboost_coroutine-mt -lboost_context-mt
XLINK_LIBS					+= ThorsSocket ThorsLogging
FILE_WARNING_FLAGS			+= -Wno-missing-braces -Wno-unused-parameter

include $(THORSANVIL_ROOT)/build/tools/Makefile
