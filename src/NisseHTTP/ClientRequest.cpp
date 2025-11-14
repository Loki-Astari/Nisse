#include "ClientRequest.h"
#include "ThorsLogging/ThorsLogging.h"

using namespace ThorsAnvil::Nisse::HTTP;

std::string ClientRequest::getHost(std::string const& url)
{
    auto find = url.find("://");
    if (find == std::string::npos) {
        find = 0;
    }
    else {
        find += 3;
    }
    auto path = std::min(std::size(url), url.find('/', find));
    auto port = url.find(':', find);
    auto end  = std::min(path, port);
    return url.substr(find, (end - find));
}

std::string_view ClientRequest::getRequest(std::string const& url)
{
    auto find = url.find("://");
    if (find == std::string::npos) {
        find = 0;
    }
    else {
        find += 3;
    }
    auto path = std::min(std::size(url), url.find('/', find));
    return {std::begin(url) + path, std::end(url)};
}

ClientRequest::ClientRequest(std::ostream& baseStream, std::string url, Method method, Version version)
    : method(method)
    , version(version)
    , url(std::move(url))
    , baseStream(baseStream)
    , headerSent(false)
{}

ClientRequest::~ClientRequest()
{
    ThorsLogDebug("ThorsAnvil::Nisse::HTTP::ClientRequest", "~ClientRequest", "Responce object destruction");
    if (stream.rdbuf() == nullptr)
    {
        if (!headerSent)
        {
            ThorsLogDebug("ThorsAnvil::Nisse::HTTP::ClientRequest", "~ClientRequest", "Sending header: ", method, " ", url, " ", version);
            baseStream << method << " " << getRequest(url) << " " << version << "\r\n"
                       << "Host: " << getHost(url) << "\r\n";

            headerSent = true;
        }
        ThorsLogDebug("ThorsAnvil::Nisse::HTTP::ClientRequest", "~ClientRequest", "Setting content length to zero and flushing");
        baseStream << "content-length: 0\r\n"
            << "\r\n"
            << std::flush;
    }
}

namespace ThorsAnvil::Nisse::HTTP
{
    // Forward declare as it currently is not in header file.
    // TODO FIX
    std::ostream& operator<<(std::ostream& stream, Header const& header);
}

void ClientRequest::addHeaders(Header const& headers)
{
    if (stream.rdbuf() != nullptr) {
        ThorsLogAndThrowWarning(std::runtime_error, "ThorsAnvil::Nisse::ClientRequest", "addHeaders", "Headers can not be sent after the body has been started");
    }

    if (!headerSent)
    {
        ThorsLogDebug("ThorsAnvil::Nisse::HTTP::ClientRequest", "addHeaders", "Sending header: ", method, " ", url, " ", version);
        baseStream << method << " " << getRequest(url) << " " << version << "\r\n"
                   << "Host: " << getHost(url) << "\r\n";
        headerSent = true;
    }

    baseStream << headers;
}

std::ostream& ClientRequest::body(BodyEncoding bodyEncoding)
{
    ThorsLogDebug("ThorsAnvil::Nisse::HTTP::ClientRequest", "body", "Adding body");
    if (version > Version::HTTP1_1 && std::holds_alternative<Encoding>(bodyEncoding) && std::get<Encoding>(bodyEncoding) == Encoding::Chunked) {
        ThorsLogFatal("ThorsAnvil::Nisse::HTTP::ClientRequest", "body", "Invalid encoding requested. Chunked encoding not supported in HTTP 2 or 3");
    }
    if (!headerSent)
    {
        ThorsLogDebug("ThorsAnvil::Nisse::HTTP::ClientRequest", "body", "Sending header: ", method, " ", url, " ", version);
        baseStream << method << " " << getRequest(url) << " " << version << "\r\n"
                   << "Host: " << getHost(url) << "\r\n";
        headerSent = true;
    }
    ThorsLogDebug("ThorsAnvil::Nisse::HTTP::ClientRequest", "body", "adding body to stream");
    baseStream << bodyEncoding
               << "\r\n"
               << std::flush;

    stream.addBuffer(StreamBufOutput{baseStream, bodyEncoding});
    return stream;
}
