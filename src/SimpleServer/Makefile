THORSANVIL_ROOT             = $(realpath ../../)

TARGET                      = Nisse.prog
LINK_LIBS					+= Nisse ThorsSocket ThorsLogging
LDLIBS						+= -levent -levent_pthreads -lboost_coroutine-mt -lboost_context-mt
FILE_WARNING_FLAGS			+= -Wno-missing-braces -Wno-unused-parameter
FILE_WARNING_FLAGS			+= -Wno-unused-private-field

include $(THORSANVIL_ROOT)/build/tools/Makefile
