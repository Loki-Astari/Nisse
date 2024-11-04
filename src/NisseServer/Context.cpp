#include "Context.h"
#include "NisseServer.h"
#include "EventHandlerLibEvent.h"

namespace TASock = ThorsAnvil::ThorsSocket;

using namespace ThorsAnvil::Nisse::Server;

Context::Context(NisseServer& server, Yield& yield, int owner)
    : server{server}
    , yield{yield}
    , owner{owner}
{}

void Context::registerOwnedSocketStream(TASock::SocketStream& stream, EventType initialWait)
{
    TASock::Socket& socket      = stream.getSocket();
    int             fd          = socket.socketId();
    Yield&          localYield  = yield;

    registerYield(
        fd,
        initialWait,
        stream.getSocket(),
        [&localYield, &stream, fd]()
        {
            std::ios_base::iostate e = stream.exceptions();
            stream.exceptions(std::ios::badbit);
            localYield({TaskYieldState::RestoreRead, fd});
            stream.exceptions(e);
            return true;
        },
        [&localYield, &stream, fd]()
        {
            std::ios_base::iostate e = stream.exceptions();
            stream.exceptions(std::ios::badbit);
            localYield({TaskYieldState::RestoreWrite, fd});
            stream.exceptions(e);
            return true;
        }
    );
}

void Context::unregisterOwnedSocketStream(TASock::SocketStream& stream)
{
    unregisterYield(stream.getSocket().socketId());
}

void Context::registerOwnedSocket(TASock::Socket& socket, EventType initialWait)
{
    int     fd          = socket.socketId();
    Yield&  localYield  = yield;

    registerYield(
        fd,
        initialWait,
        socket,
        [&localYield, fd]()
        {
            localYield({TaskYieldState::RestoreRead, fd});
            return true;
        },
        [&localYield, fd]()
        {
            localYield({TaskYieldState::RestoreWrite, fd});
            return true;
        }
    );
}

void Context::unregisterOwnedSocket(TASock::Socket& socket)
{
    unregisterYield(socket.socketId());
}

void Context::registerSharedSocket(NisseServer& server, TASock::Socket& socket)
{
    server.eventHandler.addSharedFD(socket.socketId());
}

void Context::unregisterSharedSocket(NisseServer& server, TASock::Socket& socket)
{
    server.eventHandler.remSharedFD(socket.socketId());
}

void Context::registerYield(int fd, EventType initialWait, TASock::Socket& socket, TASock::YieldFunc&& readYield, TASock::YieldFunc&& writeYield)
{
    server.eventHandler.addOwnedFD(fd, owner, initialWait);
    TaskYieldState  yieldType = initialWait == EventType::Read ? TaskYieldState::RestoreRead : TaskYieldState::RestoreWrite;
    yield({yieldType, fd});

    socket.setReadYield(std::move(readYield));
    socket.setWriteYield(std::move(writeYield));
}

void Context::unregisterYield(int fd)
{
    server.eventHandler.remOwnedFD(fd);
}

AsyncStream::AsyncStream(TASock::SocketStream& stream, Context& context, EventType initialWait)
    : stream{stream}
    , context{context}
{
    context.registerOwnedSocketStream(stream, initialWait);
    stream.getSocket().deferInit();
}

AsyncStream::~AsyncStream()
{
    context.unregisterOwnedSocketStream(stream);
}

AsyncSocket::AsyncSocket(TASock::Socket& socket, Context& context, EventType initialWait)
    : socket{socket}
    , context{context}
{
    context.registerOwnedSocket(socket, initialWait);
    socket.deferInit();
}

AsyncSocket::~AsyncSocket()
{
    context.unregisterOwnedSocket(socket);
}

AsyncSharedSocket::AsyncSharedSocket(TASock::Socket& socket, NisseServer& server)
    : socket{socket}
    , server{server}
{
    Context::registerSharedSocket(server, socket);
    socket.deferInit();
}

AsyncSharedSocket::~AsyncSharedSocket()
{
    Context::unregisterSharedSocket(server, socket);
}
