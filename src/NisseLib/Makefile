CONFIG_NAME					= Nisse
THORSANVIL_ROOT				?= $(realpath ../../)
TARGET						= Nisse.lib


LDLIBS_EXTERN_BUILD			+= ThorSerialize event BoostCoRoutine
DEFER_LIBS					+= NisseServer NisseHTTP


include $(THORSANVIL_ROOT)/build/tools/Makefile
