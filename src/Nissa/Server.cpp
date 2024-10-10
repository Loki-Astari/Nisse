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

Work Server::createHttpJob(SocketStream&& acceptStream)
{
    return [&, streamRef = MoveOnCopy{std::move(acceptStream)}]()
    {
        SocketStream& stream = streamRef.value;

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
        if (!closeSocket) {
            jobQueue.addJob(createHttpJob(std::move(stream)));
        }
    };
}

void Server::listen(int port)
{
    using ThorsAnvil::ThorsSocket::SServerInfo;
    listeners.emplace_back(SServerInfo{port, ctx});

    eventHandler.add(listeners.back().socketId(), EventType::Read, [&, serverId = listeners.size() - 1]()
    {
        using ThorsAnvil::ThorsSocket::Socket;
        using ThorsAnvil::ThorsSocket::Blocking;

        Socket          accept = listeners[serverId].accept(Blocking::No);
        if (accept.isConnected())
        {
            SocketStream    stream{std::move(accept)};
            jobQueue.addJob(createHttpJob(std::move(stream)));
        }
        std::cerr << "Next Connection\n";
    });
}
