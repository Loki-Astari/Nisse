#ifndef THORSANVIL_NISSE_ASYNC_STREAM_H
#define THORSANVIL_NISSE_ASYNC_STREAM_H

#include "NisseConfig.h"
#include "Context.h"
#include <ThorsSocket/SocketStream.h>

namespace TAS = ThorsAnvil::ThorsSocket;

namespace ThorsAnvil::Nisse::Server
{

class AsyncStream
{
    TAS::Socket&        socket;
    Context&            context;
    public:
        AsyncStream(TAS::Socket& socket, Context& context, EventType initialWait);
        ~AsyncStream();
};

}

#endif
