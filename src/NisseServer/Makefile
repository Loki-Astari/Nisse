THORSANVIL_ROOT             = $(realpath ../../)

TARGET						= NisseServer.defer
NAMESPACE					= ThorsAnvil::Nisse
DEFER_NAME					= Nisse
LDLIBS_EXTERN_BUILD			= ThorSerialize
FILE_WARNING_FLAGS			+= -Wno-missing-braces

UNITTEST_LDLIBS				+= -L $(THORSANVIL_ROOT)/src/NisseServer/debug -L $(THORSANVIL_ROOT)/src/NisseServer/release -levent -levent_pthreads -lboost_coroutine -lboost_context

include $(THORSANVIL_ROOT)/build/tools/Makefile
