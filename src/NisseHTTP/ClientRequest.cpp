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
    flushRequest();
}

void ClientRequest::flushHeaderIfNeeded()
{
    if (!headerSent)
    {
        ThorsLogDebug("ThorsAnvil::Nisse::HTTP::ClientRequest", "flushHeaderIfNeeded", "Sending header: ", method, " ", url, " ", version);
        baseStream << method << " " << getRequest(url) << " " << version << "\r\n"
                   << "Host: " << getHost(url) << "\r\n";

        headerSent = true;
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
        ThorsLogAndThrowWarning(std::runtime_error, "ThorsAnvil::Nisse::HTTP::ClientRequest", "addHeaders", "Headers can not be sent after the body has been started");
    }

    flushHeaderIfNeeded();
    baseStream << headers;
}

std::ostream& ClientRequest::body(BodyEncoding bodyEncoding)
{
    ThorsLogDebug("ThorsAnvil::Nisse::HTTP::ClientRequest", "body", "Adding body");
    if (version > Version::HTTP1_1 && std::holds_alternative<Encoding>(bodyEncoding) && std::get<Encoding>(bodyEncoding) == Encoding::Chunked) {
        ThorsLogFatal("ThorsAnvil::Nisse::HTTP::ClientRequest", "body", "Invalid encoding requested. Chunked encoding not supported in HTTP 2 or 3");
    }
    flushHeaderIfNeeded();
    ThorsLogDebug("ThorsAnvil::Nisse::HTTP::ClientRequest", "body", "adding body to stream");
    baseStream << bodyEncoding
               << "\r\n";
               // << std::flush; // Do we need to flush?

    stream.addBuffer(StreamBufOutput{baseStream, bodyEncoding});
    return stream;
}

void ClientRequest::flushRequest()
{
    if (stream.rdbuf() == nullptr) {
        body(0);
    }
    stream << std::flush;

}
