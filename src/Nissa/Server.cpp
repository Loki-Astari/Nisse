#include "Server.h"
#include "EventHandler.h"
#include <charconv>

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

WorkAction Server::createHttpJob(int socketId)
{
    return [&, socketId](ThorsAnvil::ThorsSocket::SocketStream&& streamData)
    {
        SocketStream stream = std::move(streamData);

        std::size_t bodySize = 0;
        bool        closeSocket = true;
        std::string line;
        while (std::getline(stream, line))
        {
            std::cout << "Request: " << line << "\n";
            if (line == "\r") {
                break;
            }
            if (line == "Connection: keep-alive\r") {
                closeSocket = false;
            }
            if (line.compare("Content-Length: ") == 0) {
                std::from_chars(&line[0] + 16, &line[0] + line.size(), bodySize);
            }
        }
        stream.ignore(bodySize);

        if (stream)
        {
            stream << "HTTP/1.1 200 OK\r\n"
                   << "Content-Length: 135\r\n"
                   << "\r\n"
                   << R"(<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html>
<head><title>Nissa server 1.1</title></head>
<body>Hello world</body>
</html>)";
            stream.flush();
        }
        std::cerr << "Done\n";
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
                jobQueue.addJob(createHttpJob(socketId), std::move(streamRef.value));
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

        Socket          accept = listeners[serverId].accept(Blocking::No);
        if (accept.isConnected())
        {
            int socketId = accept.socketId();
            eventHandler.add(socketId, EventType::Read, [&, acceptRef = MoveOnCopy{std::move(accept)}, socketId](bool streamOk)
            {
                if (!streamOk)
                {
                    eventHandler.remove(socketId, EventType::Read);
                    return;
                }
                jobQueue.addJob(createHttpJob(socketId), ThorsAnvil::ThorsSocket::SocketStream{std::move(acceptRef.value)});
            });
            eventHandler.restore(listeners[serverId].socketId(), EventType::Read);
        }
    };
}

void Server::listen(int port)
{
    // This is not safe to use after run() is called.
    // While the background workers can accesses listeners this should not be called.
    using ThorsAnvil::ThorsSocket::SServerInfo;
    listeners.emplace_back(SServerInfo{port, ctx});

    eventHandler.add(listeners.back().socketId(), EventType::Read, [&, serverId = listeners.size() - 1](bool)
    {
        jobQueue.addJob(createAcceptJob(serverId), {});
        std::cerr << "Next Connection\n";
    });
}
