#include "AsyncStream.h"

namespace TASock    = ThorsAnvil::ThorsSocket;

using namespace ThorsAnvil::Nisse::Server;

AsyncStream::AsyncStream(TASock::SocketStream& stream, Context& context, EventType initialWait)
    : stream{stream}
    , context{context}
{
    context.registerLocalSocketStream(stream, initialWait);
    stream.getSocket().deferInit();
}

AsyncStream::~AsyncStream()
{
    context.unregisterLocalSocketStream(stream);
}

AsyncSocket::AsyncSocket(TASock::Socket& socket, Context& context, EventType initialWait)
    : socket{socket}
    , context{context}
{
    context.registerLocalSocket(socket, initialWait);
    socket.deferInit();
}

AsyncSocket::~AsyncSocket()
{
    context.unregisterLocalSocket(socket);
}
