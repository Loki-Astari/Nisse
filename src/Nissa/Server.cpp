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
    // When an accepted socket is available.
    // Run this function. Which simply delegates the work to the pint.
    // If the pint return PintResult::More then yield back to the server
    // indicating you are waiting for more data to read when it arrives
    // re-call the pint.
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
    // When a connection is accepted
    // This method is run.
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
                // If everything worked then create a stream connection (see above)
                // Passing the "Pint" as the object that will handle the request.
                // Note: The "Pint" functionality is not run yet. The socket must be available to use.
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
