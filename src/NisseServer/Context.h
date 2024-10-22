#ifndef THORSANVIL_NISSE_CONTEXT_H
#define THORSANVIL_NISSE_CONTEXT_H

#include "NisseConfig.h"
#include "NisseUtil.h"

namespace ThorsAnvil::Nisse
{

class NisseServer;

class Context
{
    NisseServer&    server;
    Yield&          yield;
    int             owner;
    public:
        Context(NisseServer& server, Yield& yield, int owner)
            : server{server}
            , yield{yield}
            , owner{owner}
        {}
};

}

#endif
