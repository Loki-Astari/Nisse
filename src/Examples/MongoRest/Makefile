THORSANVIL_ROOT             = $(realpath ../../../)

TARGET                      = MongoRest.prog
LINK_LIBS					+= Nisse ThorsSocket ThorsLogging ThorSerialize ThorsMongo
LDLIBS						+= -levent -levent_pthreads -lboost_coroutine-mt -lboost_context-mt
FILE_WARNING_FLAGS			+= -Wno-missing-braces -Wno-unused-parameter -Wno-dollar-in-identifier-extension

include $(THORSANVIL_ROOT)/build/tools/Makefile
