#include "Server.h"
#include "EventHandler.h"
#include <charconv>

using namespace ThorsAnvil::Nissa;

Server::Server(Certificate certificate)
    : ctx{ThorsAnvil::ThorsSocket::SSLMethodType::Server, certificate}
    , finished{false}
{
    workers.emplace_back(&Server::connectionHandler, this);
}

void Server::run(int port)
{
    using ThorsAnvil::ThorsSocket::Server;
    using ThorsAnvil::ThorsSocket::SServerInfo;

    Server  server{SServerInfo{port, ctx}};
    eventHandler.add(server.socketId(), EventType::Read, [&]()
    {
        using ThorsAnvil::ThorsSocket::Socket;
        using ThorsAnvil::ThorsSocket::Blocking;

        Socket          accept = server.accept(Blocking::No);
        std::unique_lock    lock(connectionMutex);
        connections.emplace(std::move(accept));
        connectionCV.notify_one();
    });
    eventHandler.run();
}

Server::SocketStream Server::getNextStream()
{
    std::unique_lock    lock(connectionMutex);
    connectionCV.wait(lock, [&connections = this->connections](){return !connections.empty();});

    SocketStream        socket = std::move(connections.front());
    connections.pop();
    return socket;
}

void Server::connectionHandler()
{
    while (!finished)
    {
        SocketStream stream = getNextStream();

        bool         anotherPage;
        do
        {
            anotherPage = false;
            std::size_t bodySize = 0;
            std::string line;
            while (std::getline(stream, line))
            {
                std::cout << "Request: " << line << "\n";
                if (line == "\r") {
                    break;
                }
                if (line == "Connection: keep-alive\r") {
                    anotherPage = true;
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
        }
        while (anotherPage && stream.good());
        std::cerr << "Next Connection\n";
    }

}
