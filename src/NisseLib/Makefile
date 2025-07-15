THORSANVIL_ROOT             = $(realpath ../../)

TARGET						= Nisse.lib
DEFER_LIBS					= NisseServer NisseHTTP

LDLIBS						+= -levent -levent_pthreads -lboost_coroutine -lboost_context
LINK_LIBS					+= ThorsSocket ThorsLogging

include $(THORSANVIL_ROOT)/build/tools/Makefile
