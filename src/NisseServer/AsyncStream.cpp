#include "AsyncStream.h"

using namespace ThorsAnvil::Nisse::Server;

AsyncStream::AsyncStream(TAS::SocketStream& stream, Context& context, EventType initialWait)
    : stream{stream}
    , context{context}
{
    context.registerLocalSocket(stream.getSocket(), initialWait);
}

AsyncStream::~AsyncStream()
{
    context.unregisterLocalSocket(stream.getSocket());
}
