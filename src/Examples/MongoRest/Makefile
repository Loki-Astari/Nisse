THORSANVIL_ROOT				?= $(realpath ../../../)
TARGET						= MongoRest.prog


LINK_LIBS					+= Nisse
LDLIBS_EXTERN_BUILD			+= ThorSerialize event BoostCoRoutine
FILE_WARNING_FLAGS			+= -Wno-missing-braces -Wno-unused-parameter -Wno-dollar-in-identifier-extension


include $(THORSANVIL_ROOT)/build/tools/Makefile
