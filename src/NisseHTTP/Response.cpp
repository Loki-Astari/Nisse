#include "Response.h"
#include "Util.h"
#include "HeaderResponse.h"
#include "HeaderPassThrough.h"
#include "ThorsLogging/ThorsLogging.h"

#include <limits>

using namespace ThorsAnvil::Nisse::HTTP;

StandardStatusCodeMap standardCodes;

Response::Response(std::ostream& stream, Version version, int responseCode)
    : version{version}
    , statusCode{standardCodes[responseCode]}
    , headerSent{false}
    , baseStream{stream}
    , checkPoint(std::chrono::high_resolution_clock::now())
{}

Response::~Response()
{
    if (stream.rdbuf() == nullptr)
    {
        sendHeaderIfNotSent();
        ThorsLogTrack("ThorsAnvil::Nisse::HTTP::Response", "~Response", "Setting content length to zero and flushing");
        baseStream << "content-length: 0\r\n"
                   << "\r\n"
                   << std::flush;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - checkPoint;
    ThorsLogInfo("ThorsAnvil::Nisse::HTTP::Response", "~Response", "Response Time: ", std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count(), "ms");
}

void Response::sendHeaderIfNotSent()
{
    if (!headerSent)
    {
        ThorsLogTrack("ThorsAnvil::Nisse::HTTP::Response", "sendHeaderIfNotSent", "Sending header: ", statusCode);
        baseStream << version << " " << statusCode << "\r\n";
        headerSent = true;
    }
}

struct IgnoreLine
{
    friend std::istream& operator>>(std::istream& stream, IgnoreLine const&)
    {
        return stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
};

void Response::read(std::istream& stream)
{
    int code;
    if (stream >> version >> code >> IgnoreLine{})
    {
        statusCode = standardCodes[code];
    }
}

namespace ThorsAnvil::Nisse::HTTP
{
    std::ostream& operator<<(std::ostream& stream, Header const& header)
    {
        struct HeaderStream
        {
            std::ostream& stream;
            HeaderStream(std::ostream& stream)
                : stream(stream)
            {}
            std::ostream& operator()(HeaderResponse const& header)    {return stream << header;}
            std::ostream& operator()(HeaderPassThrough const& header) {return stream << header;}
        };
        return std::visit(HeaderStream{stream}, header);
    }

}

void Response::setStatus(int newStatusCode)
{
    statusCode = standardCodes[newStatusCode];
}

void Response::addHeaders(Header const& headers)
{
    if (stream.rdbuf() != nullptr) {
        ThorsLogAndThrowWarning(std::runtime_error, "ThorsAnvil::Nisse::HTTP::Response", "addHeaders", "Headers can not be sent after the body has been started");
    }

    sendHeaderIfNotSent();
    baseStream << headers;
}

std::ostream& Response::body(BodyEncoding bodyEncoding)
{
    ThorsLogTrack("ThorsAnvil::Nisse::HTTP::Response", "body", "adding body");
    sendHeaderIfNotSent();
    baseStream << bodyEncoding
               << "\r\n"
               // TODO:  Do I really want to force a flush here.
               //        Could be more efficient to add body first before flushing.
               //        But, I think the StreamOutput depends on it.
               << std::flush;

    stream.addBuffer(StreamBufOutput{baseStream, bodyEncoding});
    return stream;
}

void Response::error(int code, std::string_view errorMessage)
{
    setStatus(code);
    HeaderResponse  header;
    header.add("Error", errorMessage);
    addHeaders(header);
}
