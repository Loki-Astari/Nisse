#include "Request.h"
#include <ThorsLogging/ThorsLogging.h>
#include <map>
#include <iostream>

using namespace ThorsAnvil::Nisse::PyntHTTP;

Request::Request(std::string_view proto, std::istream& stream)
    : version(Version::Unknown)
    , method(Method::Other)
{
    std::string_view path = readFirstLine(stream);
    readHeaders(head, stream);
    buildURL(proto, path);
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
        ThorsLog("ThorsAnvil::Nisse::PyntHTTP::Request", "readFirstLine", ": Header not \r\n terminated");
    }
    auto end = std::min(messageHeader.size(), messageHeader.find_last_not_of("\r"));

    messageHeader.erase(end + 1);

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

    return path;
}

void Request::readHeaders(Header& dst, std::istream& stream)
{
    std::string line;
    while (std::getline(stream, line))
    {
        if (line == "\r") {
            break;
        }
        auto split = line.find(':');
        if (split == std::string::npos) {   // Log BAD Header.
            continue;
        }
        std::string_view header = getValue({&line[0], &line[0] + split});
        std::string_view value  = getValue({&line[0] + split + 1, &line[0] + line.size()});

        dst.add(header, value);
    }
}

void Request::buildURL(std::string_view proto, std::string_view path)
{
    using std::literals::operator""sv;
std::vector<std::string>& hostValues = head.getHeader("host"sv);
std::string_view          hostValue  = hostValues.size() == 0 ? ""sv : hostValues[0];
    url = URL{proto, hostValue, path};
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

std::string_view Request::getValue(std::string_view input)
{
    // Remove leading and trailing white space.
    input.remove_prefix(std::min(input.size(), input.find_first_not_of(" \r\t\v")));
    input.remove_suffix(input.size() - std::min(input.size(), input.find_last_not_of(" \r\t\v")) - 1);
    return input;
}

std::istream& Request::body()
{
    return std::cin;
}
