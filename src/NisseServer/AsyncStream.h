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
    TASock::SocketStream&   stream;
    Context&                context;
    public:
        AsyncStream(TASock::SocketStream& stream, Context& context, EventType initialWait);
        ~AsyncStream();
};
class AsyncSocket
{
    TASock::Socket&         socket;
    Context&                context;
    public:
        AsyncSocket(TASock::Socket& socket, Context& context, EventType initialWait);
        ~AsyncSocket();
};

}

#endif
