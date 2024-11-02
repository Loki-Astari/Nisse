#ifndef THORSANVIL_NISSE_CONTEXT_H
#define THORSANVIL_NISSE_CONTEXT_H

#include "NisseConfig.h"
#include "NisseUtil.h"
#include "EventHandlerLibEvent.h"
#include <ThorsSocket/Socket.h>

namespace TASock = ThorsAnvil::ThorsSocket;

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
        void registerLocalSocketStream(TASock::SocketStream& stream, EventType initialWait);
        void unregisterLocalSocketStream(TASock::SocketStream& stream);
        void registerLocalSocket(TASock::Socket& socket, EventType initialWait);
        void unregisterLocalSocket(TASock::Socket& socket);

        Yield&      getYield()  {return yield;}
    private:
        void registerYield(int fd, EventType initialWait, TASock::Socket& socket, TASock::YieldFunc&& readYield, TASock::YieldFunc&& writeYield);
        void unregisterYield(int fd);
};

}

#endif
