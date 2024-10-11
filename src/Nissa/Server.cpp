#include "Server.h"
#include "EventHandler.h"

using namespace ThorsAnvil::Nissa;

Server::Server(Certificate certificate, int workerCount)
    : ctx{ThorsAnvil::ThorsSocket::SSLMethodType::Server, certificate}
    , jobQueue(workerCount)
    , eventHandler(jobQueue)
{}

void Server::run()
{
    eventHandler.run();
}

EventAction Server::createStreamJob(Pint& pint)
{
    return [&pint](ThorsAnvil::ThorsSocket::SocketStream& stream, Yield& yield)
    {
        PintResult result = pint.handleRequest(stream);
        while (result == PintResult::More)
        {
            yield(EventTask::RestoreRead);
            result = pint.handleRequest(stream);
        }
    };
}

EventAction Server::createAcceptJob(int serverId)
{
    return [&, serverId](ThorsAnvil::ThorsSocket::SocketStream&, Yield& yield)
    {
        while (true)
        {
            using ThorsAnvil::ThorsSocket::Socket;
            using ThorsAnvil::ThorsSocket::Blocking;

            Socket          accept = listeners[serverId].server.accept(Blocking::No);
            if (accept.isConnected())
            {
                int socketId = accept.socketId();
                eventHandler.add(socketId, ThorsAnvil::ThorsSocket::SocketStream{std::move(accept)}, createStreamJob(listeners[serverId].pint));
            }
            yield(EventTask::RestoreRead);
        }
    };
}

void Server::listen(int port, Pint& pint)
{
    // This is not safe to use after run() is called.
    // While the background workers can accesses listeners this should not be called.
    using ThorsAnvil::ThorsSocket::SServerInfo;
    listeners.emplace_back(SServerInfo{port, ctx}, pint);

    eventHandler.add(listeners.back().server.socketId(), ThorsAnvil::ThorsSocket::SocketStream{}, createAcceptJob(listeners.size() - 1));
}
