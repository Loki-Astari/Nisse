#include "Request.h"
#include "StreamInput.h"
#include <ThorsLogging/ThorsLogging.h>
#include <map>
#include <iostream>

using namespace ThorsAnvil::Nisse::HTTP;

Request::Request(std::string_view proto, std::istream& stream)
    : version{Version::Unknown}
    , method{Method::Other}
{
    std::string_view path = readFirstLine(stream);
    if (path.size() != 0)
    {
        readHeaders(head, stream)   &&
        buildURL(proto, path)       &&
        buildStream(stream);
    }
}

std::string_view Request::readFirstLine(std::istream& stream)
{
    // Read the first line
    std::getline(stream, messageHeader);
    if (messageHeader.size() > 0 && messageHeader[messageHeader.size() - 1] == '\r') {
        messageHeader.resize(messageHeader.size() - 1);
    }
    else
    {
        ThorsLogInfo("ThorsAnvil::Nisse::PyntHTTP::Request", "readFirstLine", ": Header not \r\n terminated");
        failResponse.add("error", "Invalid HTTP Request");
        failResponse.add("rason", "Header Not terminated with <CR><LF>");
        return "";
    }

    // Extract the Method
    auto methStart  = std::min(messageHeader.size(), messageHeader.find_first_not_of(" ", 0));
    auto methEnd    = std::min(messageHeader.size(), messageHeader.find_first_of(' ', methStart));

    // Path
    auto pathStart  = std::min(messageHeader.size(), messageHeader.find_first_not_of(" ", methEnd));
    auto pathEnd    = std::min(messageHeader.size(), messageHeader.find_first_of(" ", pathStart));

    // Proto
    auto protStart = std::min(messageHeader.size(), messageHeader.find_first_not_of(" ", pathEnd));
    auto protEnd   = std::min(messageHeader.size(), messageHeader.find_first_of("/", protStart));

    // Version
    auto versStart  = std::min(messageHeader.size(), protEnd + 1);
    auto versEnd    = std::min(messageHeader.size(), messageHeader.find_first_of(" \r", versStart));

    std::string_view meth(messageHeader.begin() + methStart, messageHeader.begin() + methEnd);
    std::string_view path(messageHeader.begin() + pathStart, messageHeader.begin() + pathEnd);
    std::string_view prot(messageHeader.begin() + protStart, messageHeader.begin() + protEnd);
    std::string_view vers(messageHeader.begin() + versStart, messageHeader.begin() + versEnd);
    std::string_view pv  (messageHeader.begin() + protStart, messageHeader.begin() + versEnd);

    version = findVersion(pv);
    method = findMethod(meth);

    if (meth.size() == 0 || path.size() == 0 || pv.size() == 0 || version == Version::Unknown || method == Method::Other)
    {
        ThorsLogInfo("ThorsAnvil::Nisse::PyntHTTP::Request", "readFirstLine", ": Bad Request: ", "Method: >", meth, "< Path: >", path, "< Proto: >", pv, "<");
        failResponse.add("error", "Invalid HTTP Request");
        failResponse.add("method", meth);
        failResponse.add("path", path);
        failResponse.add("proto", pv);
        return "";
    }

    return path;
}

bool Request::readHeaders(HeaderRequest& dst, std::istream& stream)
{
    std::string line;
    while (std::getline(stream, line))
    {
        if (line == "\r") {
            break;
        }
        auto split = line.find(':');
        if (line.size() == 0 || line[line.size() - 1] != '\r' || split == std::string::npos)
        {
            ThorsLogInfo("ThorsAnvil::Nisse::PyntHTTP::Request", "readHeaders", ": Bad Request Header: ", line);
            failResponse.add("error", "Invalid HTTP Header");
            failResponse.add("header", line);
            return false;
        }
        dst.add({&line[0], &line[0] + split}, {&line[0] + split + 1, &line[0] + line.size() - 1});
    }
    return true;
}

bool Request::buildURL(std::string_view proto, std::string_view path)
{
    using std::literals::operator""sv;
    std::vector<std::string> const& hostValues = head.getHeader("host"sv);
    if (hostValues.size() == 0)
    {
        ThorsLogInfo("ThorsAnvil::Nisse::PyntHTTP::Request", "buildURL", ": Bad Request No Host Field: ");
        failResponse.add("error", "Invalid HTTP Request- No Host header");
        return false;
    }
    std::string_view                hostValue  = hostValues.size() == 0 ? ""sv : hostValues[0];
    url = URL{proto, hostValue, path};
    return true;
}

Version Request::findVersion(std::string_view pv)
{
    static const std::map<std::string_view, Version>  versionMap {  {"HTTP/1.0", Version::HTTP1_0},
                                                                    {"HTTP/1.1", Version::HTTP1_1},
                                                                    {"HTTP/2",   Version::HTTP2},
                                                                    {"HTTP/3",   Version::HTTP3}
                                                                 };
    auto find = versionMap.find(pv);
    if (find != versionMap.end()) {
        return find->second;
    }
    return Version::Unknown;
}

Method Request::findMethod(std::string_view method)
{
    static const std::map<std::string_view, Method>  methodMap  {   {"GET",     Method::GET},
                                                                    {"HEAD",    Method::HEAD},
                                                                    {"OPTIONS", Method::OPTIONS},
                                                                    {"TRACE",   Method::TRACE},
                                                                    {"PUT",     Method::PUT},
                                                                    {"DELETE",  Method::DELETE},
                                                                    {"POST",    Method::POST},
                                                                    {"PATCH",   Method::PATCH},
                                                                    {"CONNECT", Method::CONNECT}
                                                                };
    auto find = methodMap.find(method);
    if (find != methodMap.end()) {
        return find->second;
    }
    return Method::Other;
}

bool Request::buildStream(std::istream& stream)
{
    auto&   contentLength    = head.getHeader("content-length");
    auto&   transferEncoding = head.getHeader("transfer-encoding");
    if (contentLength.size() == 1 && transferEncoding.size() != 0)
    {
        ThorsLogInfo("ThorsAnvil::Nisse::PyntHTTP::Request", "buildStream", ": Bad Request: Includes both 'content-length' and 'transfer-encoding'");
        failResponse.add("error", "Invalid HTTP Request- Includes both 'content-length' and 'transfer-encoding'");
        failResponse.add("value-content-length", contentLength[0]);
        for (auto const& v: transferEncoding) {
            failResponse.add("value-transfer-encoding", v);
        }
        return false;
    }


    if (contentLength.size() == 1)
    {
        input.addBuffer(StreamBufInput(stream, std::stoi(contentLength[0])));
        return true;
    }
    if (transferEncoding.size() != 0 && transferEncoding.size() == 1 && transferEncoding[0] == "chunked")
    {
        input.addBuffer(StreamBufInput(stream,
                                       Encoding::Chunked,
                                       [&](){readHeaders(tail, stream);}));
        return true;
    }
    // TODO Handle other transfer encoding.
    // Currently what will happen is that you can not read from the input stream.
    // Which means POST/PUT etc commands can not transfer data.
    ThorsLogInfo("ThorsAnvil::Nisse::PyntHTTP::Request", "buildStream", ": Bad Request: Unsupported Transfer Encoding.");
    failResponse.add("error", "Invalid HTTP Request- Unsupported Transer Encoding");
    for (auto const& v: transferEncoding) {
        failResponse.add("value-transfer-encoding", v);
    }
    failResponse.add("supported-transfer-encoding", "chunked");
    return false;
}

std::istream& Request::body()
{
    return input;
}
