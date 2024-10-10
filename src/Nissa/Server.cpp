#include "Server.h"
#include "EventHandler.h"

using namespace ThorsAnvil::Nissa;

Server::Server(Certificate certificate, int workerCount)
    : ctx{ThorsAnvil::ThorsSocket::SSLMethodType::Server, certificate}
    , jobQueue(workerCount)
{}

void Server::run()
{
    eventHandler.run();
}

// Move on copy used to move an object
// into a lambda capture when it can not be copied.
template<typename T>
class MoveOnCopy
{
    public:
        mutable T   value;
        MoveOnCopy(T&& move)
            : value(std::move(move))
        {}
        MoveOnCopy(MoveOnCopy const& copy)
            : value(std::move(copy.value))
        {}

        MoveOnCopy& operator=(MoveOnCopy const&)    = delete;
        MoveOnCopy& operator=(MoveOnCopy&&)         = delete;
};

WorkAction Server::createStreamJob(int socketId, Pint& pint)
{
    return [&, socketId](ThorsAnvil::ThorsSocket::SocketStream&& streamData)
    {
        ThorsAnvil::ThorsSocket::SocketStream stream = std::move(streamData);
        bool closeSocket = pint.handleRequest(stream);
        if (!closeSocket)
        {
            std::cerr << "Restore Socket\n";
            eventHandler.add(socketId, EventType::Read, [&, streamRef = MoveOnCopy{std::move(stream)}, socketId](bool streamOk)
            {
                if (!streamOk)
                {
                    std::cerr << "Remove Socket\n";
                    eventHandler.remove(socketId, EventType::Read);
                    return;
                }
                jobQueue.addJob(createStreamJob(socketId, pint), std::move(streamRef.value));
            });
        }
        else
        {
            std::cerr << "Remove Socket\n";
            eventHandler.remove(socketId, EventType::Read);
        }
    };
}

WorkAction Server::createAcceptJob(int serverId)
{
    return [&, serverId](ThorsAnvil::ThorsSocket::SocketStream&&)
    {
        using ThorsAnvil::ThorsSocket::Socket;
        using ThorsAnvil::ThorsSocket::Blocking;

        Socket          accept = listeners[serverId].server.accept(Blocking::No);
        if (accept.isConnected())
        {
            int socketId = accept.socketId();
            eventHandler.add(socketId, EventType::Read, [&, acceptRef = MoveOnCopy{std::move(accept)}, socketId](bool streamOk)
            {
                if (!streamOk)
                {
                    std::cerr << "Remove Socket\n";
                    eventHandler.remove(socketId, EventType::Read);
                    return;
                }
                jobQueue.addJob(createStreamJob(socketId, listeners[serverId].pint), ThorsAnvil::ThorsSocket::SocketStream{std::move(acceptRef.value)});
            });
            eventHandler.restore(listeners[serverId].server.socketId(), EventType::Read);
        }
    };
}

void Server::listen(int port, Pint& pint)
{
    // This is not safe to use after run() is called.
    // While the background workers can accesses listeners this should not be called.
    using ThorsAnvil::ThorsSocket::SServerInfo;
    listeners.emplace_back(SServerInfo{port, ctx}, pint);

    eventHandler.add(listeners.back().server.socketId(), EventType::Read, [&, serverId = listeners.size() - 1](bool)
    {
        jobQueue.addJob(createAcceptJob(serverId), {});
        std::cerr << "Next Connection\n";
    });
}
