#ifndef THORSANVIL_NISSE_ASYNC_STREAM_H
#define THORSANVIL_NISSE_ASYNC_STREAM_H

#include "NisseConfig.h"
#include "Context.h"
#include <ThorsSocket/SocketStream.h>

namespace TASock = ThorsAnvil::ThorsSocket;

namespace ThorsAnvil::Nisse::Server
{

class AsyncStream
{
    TASock::Socket&        socket;
    Context&            context;
    public:
        AsyncStream(TASock::Socket& socket, Context& context, EventType initialWait);
        ~AsyncStream();
};

}

#endif
