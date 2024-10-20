#include "PyntHTTPV1.h"
#include <charconv>

using namespace ThorsAnvil::Nisse::HTTP;

ThorsAnvil::Nisse::PyntResult PyntHTTPV1::handleRequest(TAS::SocketStream& stream)
{
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
<head><title>Nisse server 1.1</title></head>
<body>Hello world</body>
</html>)";
        stream.flush();
    }
    std::cout << "Done\n";
    return closeSocket ? PyntResult::Done : PyntResult::More;
    return PyntResult::More;
}
