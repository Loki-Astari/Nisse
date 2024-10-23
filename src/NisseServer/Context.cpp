#include "Context.h"
#include "NisseServer.h"
#include "EventHandlerLibEvent.h"

using namespace ThorsAnvil::Nisse::Server;

Context::Context(NisseServer& server, Yield& yield, int owner)
    : server{server}
    , yield{yield}
    , owner{owner}
{}

void Context::registerLocalSocket(TAS::Socket& socket, EventType initialWait)
{
    int fd = socket.socketId();
    server.eventHandler.addLinkedStream(fd, owner, initialWait);
    TaskYieldState  yieldType = initialWait == EventType::Read ? TaskYieldState::RestoreRead : TaskYieldState::RestoreWrite;
    yield({yieldType, fd});

    Yield& localYield = yield;
    socket.setReadYield([&localYield, fd](){localYield({TaskYieldState::RestoreRead, fd});return true;});
    socket.setWriteYield([&localYield, fd](){localYield({TaskYieldState::RestoreWrite, fd});return true;});
}

void Context::unregisterLocalSocket(TAS::Socket& socket)
{
    server.eventHandler.remLinkedStream(socket.socketId());
}