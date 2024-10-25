THORSANVIL_ROOT             = $(realpath ../../)

TARGET						= Nisse.lib
DEFER_LIBS					= NisseServer NisseHTTP

LDLIBS						+= -levent -levent_pthreads -lboost_coroutine-mt -lboost_context-mt
LINK_LIBS					+= ThorsSocket ThorsLogging

include $(THORSANVIL_ROOT)/build/tools/Makefile
