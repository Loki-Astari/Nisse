#include "AsyncStream.h"

using namespace ThorsAnvil::Nisse::Server;

AsyncStream::AsyncStream(TAS::Socket& socket, Context& context, EventType initialWait)
    : socket{socket}
    , context{context}
{
    context.registerLocalSocket(socket, initialWait);
    socket.deferredAccept();
}

AsyncStream::~AsyncStream()
{
    context.unregisterLocalSocket(socket);
}
