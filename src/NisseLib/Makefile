THORSANVIL_ROOT             = $(realpath ../../)

TARGET						= Nisse.lib
DEFER_LIBS					= NisseServer PyntHTTP

LDLIBS						+= -levent -levent_pthreads -lboost_coroutine-mt -lboost_context-mt
LINK_LIBS					+= ThorsSocket ThorsLogging
FILE_WARNING_FLAGS			+= -Wno-missing-braces -Wno-unused-parameter

include $(THORSANVIL_ROOT)/build/tools/Makefile
