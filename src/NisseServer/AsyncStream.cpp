#include "AsyncStream.h"

namespace TASock    = ThorsAnvil::ThorsSocket;

using namespace ThorsAnvil::Nisse::Server;

AsyncStream::AsyncStream(TASock::Socket& socket, Context& context, EventType initialWait)
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
