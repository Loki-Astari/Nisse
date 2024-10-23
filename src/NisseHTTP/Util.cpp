#include "Util.h"
#include <iostream>
#include <string>

using namespace ThorsAnvil::Nisse::HTTP;

namespace ThorsAnvil::Nisse::HTTP
{
std::ostream& operator<<(std::ostream& stream, Version const& v)
{
    switch (v)
    {
        case Version::HTTP1_0:  stream << "HTTP/1.0";break;
        case Version::HTTP1_1:  stream << "HTTP/1.1";break;
        case Version::HTTP2:    stream << "HTTP/2";break;
        case Version::HTTP3:    stream << "HTTP/3";break;
        case Version::Unknown:  break;
    }
    return stream;
}

std::istream& operator>>(std::istream& stream, Version& v)
{
    static const std::map<std::string, Version> versionMap = {{"HTTP/1.0", Version::HTTP1_0}, {"HTTP/1.1", Version::HTTP1_1}, {"HTTP/2", Version::HTTP2}, {"HTTP/3", Version::HTTP3}};

    std::string version;
    if (stream >> version)
    {
        auto find = versionMap.find(version);
        v = Version::Unknown;
        if (find != versionMap.end()) {
            v = find->second;
        }
    }
    return stream;
}

std::ostream& operator<<(std::ostream& stream, Encoding const& /*encoding (Always chunked)*/)
{
    return stream << "chunked";
}

std::ostream& operator<<(std::ostream& stream, BodyEncoding const& bodyEncoding)
{
    struct BodyEncodingStream
    {
        std::ostream& stream;
        BodyEncodingStream(std::ostream& stream)
            : stream(stream)
        {}
        std::ostream& operator()(std::streamsize contentLength)     {return stream << "content-length: " << contentLength << "\r\n";}
        std::ostream& operator()(Encoding encoding)                 {return stream << "transfer-encoding: " << encoding << "\r\n";}
    };
    return std::visit(BodyEncodingStream{stream}, bodyEncoding);
}

}
