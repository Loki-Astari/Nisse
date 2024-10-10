#include "Nissa.h"
#include <iostream>

#include "ThorsSocket/Server.h"
#include "ThorsSocket/Socket.h"
#include "ThorsSocket/SocketStream.h"
#include "ThorsLogging/ThorsLogging.h"
#include <charconv>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

using Connections = std::queue<ThorsAnvil::ThorsSocket::SocketStream>;
std::vector<std::thread>    workers;
std::mutex                  connectionMutex;
std::condition_variable     connectionCV;
Connections                 connections;
bool                        finished = false;

void connectionHandler();

int main()
{
    loguru::g_stderr_verbosity = 9;

    std::cerr << PACKAGE_STRING << " Server\n";

    using ThorsAnvil::ThorsSocket::Server;
    using ThorsAnvil::ThorsSocket::Socket;
    using ThorsAnvil::ThorsSocket::SocketStream;
    using ThorsAnvil::ThorsSocket::CertificateInfo;
    using ThorsAnvil::ThorsSocket::SSLMethodType;
    using ThorsAnvil::ThorsSocket::SSLctx;
    using ThorsAnvil::ThorsSocket::SServerInfo;
    using ThorsAnvil::ThorsSocket::Blocking;

    CertificateInfo certificate{"/etc/letsencrypt/live/thorsanvil.dev/fullchain.pem",
                                "/etc/letsencrypt/live/thorsanvil.dev/privkey.pem"
                               };
    SSLctx          ctx{SSLMethodType::Server, certificate};
    Server          server{SServerInfo{8080, ctx}};


    workers.emplace_back(connectionHandler);

    while (!finished)
    {
        Socket          accept = server.accept(Blocking::No);
        std::unique_lock    lock(connectionMutex);
        connections.emplace(std::move(accept));
        connectionCV.notify_one();
    }
}

ThorsAnvil::ThorsSocket::SocketStream getNextStream()
{
    using ThorsAnvil::ThorsSocket::SocketStream;

    std::unique_lock    lock(connectionMutex);
    connectionCV.wait(lock, [](){return !connections.empty();});
    SocketStream socket = std::move(connections.front());
    connections.pop();
    return socket;
}

void connectionHandler()
{
    using ThorsAnvil::ThorsSocket::SocketStream;

    while(!finished)
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
<body>Hello World</body>
</html>)";
                stream.flush();
            }
            std::cerr << "Done\n";
        }
        while(anotherPage && stream.good());
        std::cerr << "Next Connection\n";
    }

}
