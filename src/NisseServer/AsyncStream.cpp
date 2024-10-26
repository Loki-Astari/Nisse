#include "AsyncStream.h"
#include <ThorsMongo/ThorsMongo.h>

using namespace ThorsAnvil::Nisse::Server;

AsyncStream::AsyncStream(TAS::SocketStream& stream, Context& context, EventType initialWait)
    : socket{stream.getSocket()}
    , context{context}
{
    context.registerLocalSocket(socket, initialWait);
}

AsyncStream::AsyncStream(ThorsAnvil::DB::Mongo::ThorsMongo& connection, Context& context, EventType initialWait)
    : socket{connection.getStream().getSocket()}
    , context{context}
{
    context.registerLocalSocket(socket, initialWait);
}

AsyncStream::~AsyncStream()
{
    context.unregisterLocalSocket(socket);
}
