#include "Nissa.h"
#include <iostream>

#include "ThorsSocket/Server.h"
#include "ThorsSocket/Socket.h"
#include "ThorsSocket/SocketStream.h"

int main()
{
    std::cerr << PACKAGE_STRING << " Server\n";

    using ThorsAnvil::ThorsSocket::Server;
    using ThorsAnvil::ThorsSocket::Socket;
    using ThorsAnvil::ThorsSocket::SocketStream;
    using ThorsAnvil::ThorsSocket::ServerInfo;
    using ThorsAnvil::ThorsSocket::Blocking;

    bool            finished = false;
    Server          server{ServerInfo{8080}};

    while (!finished)
    {
        Socket          accept = server.accept(Blocking::No);
        SocketStream    stream(std::move(accept));

        std::string line;
        while (std::getline(stream, line))
        {
            std::cout << "Request: " << line << "\n";
            if (line == "\r") {
                break;
            }
        }

        stream << "HTTP/1.1 200 OK\r\n"
               << "Content-Length: 135\r\n"
               << "\r\n"
               << R"(<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html>
<head><title>Nissa server 1.1</title></head>
<body>Hello world</body>
</html>)";
        stream.flush();
        std::cerr << "Done\n";
    }

}
