THORSANVIL_ROOT             = $(realpath ../../../)

TARGET                      = WebServer.prog
LINK_LIBS					+= Nisse ThorsSocket ThorsLogging
LDLIBS						+= -levent -levent_pthreads -lboost_coroutine -lboost_context
FILE_WARNING_FLAGS			+= -Wno-missing-braces

include $(THORSANVIL_ROOT)/build/tools/Makefile
