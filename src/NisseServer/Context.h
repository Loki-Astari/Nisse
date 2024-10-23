#ifndef THORSANVIL_NISSE_CONTEXT_H
#define THORSANVIL_NISSE_CONTEXT_H

#include "NisseConfig.h"
#include "NisseUtil.h"
#include "EventHandlerLibEvent.h"
#include <ThorsSocket/Socket.h>

namespace TAS = ThorsAnvil::ThorsSocket;
namespace ThorsAnvil::Nisse::Server
{

class NisseServer;

class Context
{
    NisseServer&    server;
    Yield&          yield;
    int             owner;
    public:
        Context(NisseServer& server, Yield& yield, int owner);
        void registerLocalSocket(TAS::Socket& socket, EventType initialWait);
        void unregisterLocalSocket(TAS::Socket& socket);
};

}

#endif
