#ifndef THORSANVIL_NISSE_ASYNC_STREAM_H
#define THORSANVIL_NISSE_ASYNC_STREAM_H

#include "NisseConfig.h"
#include "Context.h"
#include <ThorsSocket/SocketStream.h>

namespace ThorsAnvil::DB::Mongo
{
    class ThorsMongo;
}

namespace ThorsAnvil::Nisse::Server
{

class AsyncStream
{
    TAS::Socket&        socket;
    Context&            context;
    public:
        AsyncStream(ThorsAnvil::ThorsSocket::SocketStream& stream, Context& context, EventType initialWait);
        AsyncStream(ThorsAnvil::DB::Mongo::ThorsMongo& stream, Context& context, EventType initialWait);
        ~AsyncStream();
};

}

#endif
