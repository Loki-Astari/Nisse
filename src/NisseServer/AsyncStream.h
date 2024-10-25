#ifndef THORSANVIL_NISSE_ASYNC_STREAM_H
#define THORSANVIL_NISSE_ASYNC_STREAM_H

#include "NisseConfig.h"
#include "Context.h"
#include "ThorsSocket/Socket.h"
#include "ThorsSocket/SocketStream.h"

namespace TAS = ThorsAnvil::ThorsSocket;
namespace ThorsAnvil::Nisse::Server
{

class AsyncStream
{
    TAS::SocketStream&  stream;
    Context&            context;
    public:
        AsyncStream(TAS::SocketStream& stream, Context& context, EventType initialWait);
        ~AsyncStream();
};

}

#endif
