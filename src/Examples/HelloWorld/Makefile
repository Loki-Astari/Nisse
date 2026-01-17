THORSANVIL_ROOT				?= $(realpath ../../../)

TARGET						= HelloWorld.prog
LINK_LIBS					+= Nisse
LDLIBS_EXTERN_BUILD			= ThorSerialize
LDLIBS						+= -levent -levent_pthreads -lboost_coroutine -lboost_context
FILE_WARNING_FLAGS			+= -Wno-missing-braces

include $(THORSANVIL_ROOT)/build/tools/Makefile
