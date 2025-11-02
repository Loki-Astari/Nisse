#include "Util.h"
#include <iostream>
#include <string>

using namespace ThorsAnvil::Nisse::HTTP;

namespace ThorsAnvil::Nisse::HTTP
{
std::ostream& operator<<(std::ostream& stream, Version const& v)
{
    static const std::map<Version, std::string> versionMap = {{Version::HTTP1_0, "HTTP/1.0"}, {Version::HTTP1_1, "HTTP/1.1"}, {Version::HTTP2, "HTTP/2"}, {Version::HTTP3, "HTTP/3"}};

    auto find = versionMap.find(v);
    if (find != versionMap.end()) {
        stream << find->second;
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
        std::ostream& operator()(std::size_t contentLength)         {return stream << "content-length: " << contentLength << "\r\n";}
        std::ostream& operator()(std::streamsize contentLength)     {return stream << "content-length: " << contentLength << "\r\n";}
        std::ostream& operator()(Encoding encoding)                 {return stream << "transfer-encoding: " << encoding << "\r\n";}
    };
    return std::visit(BodyEncodingStream{stream}, bodyEncoding);
}

std::ostream& operator<<(std::ostream& stream, Method const& method)
{
    static char const* out[] = {"GET", "HEAD", "OPTIONS", "TRACE", "PUT", "DELETE", "POST", "PATCH", "CONNECT", "BAD"};
    return stream << out[static_cast<int>(method)];
}

}
