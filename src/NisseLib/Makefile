CONFIG_NAME					= Nisse
THORSANVIL_ROOT             = $(realpath ../../)

LDLIBS_EXTERN_BUILD			= ThorSerialize
TARGET						= Nisse.lib
DEFER_LIBS					= NisseServer NisseHTTP

LDLIBS						+= -levent -levent_pthreads -lboost_coroutine -lboost_context

include $(THORSANVIL_ROOT)/build/tools/Makefile
