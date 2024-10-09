#include "Nissa.h"
#include <iostream>

#include "ThorsSocket/Server.h"
#include "ThorsSocket/Socket.h"
#include "ThorsSocket/SocketStream.h"
#include "ThorsLogging/ThorsLogging.h"
#include <charconv>

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

    bool            finished = false;
    CertificateInfo certificate{"/etc/letsencrypt/live/thorsanvil.dev/fullchain.pem",
                                "/etc/letsencrypt/live/thorsanvil.dev/privkey.pem"
                               };
    SSLctx          ctx{SSLMethodType::Server, certificate};
    Server          server{SServerInfo{8080, ctx}};

    while (!finished)
    {
        Socket          accept = server.accept(Blocking::No);
        SocketStream    stream(std::move(accept));
        bool            anotherPage;
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
