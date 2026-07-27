#include "Response.h"
#include "ThorsLogging/ThorsLogging.h"
#include <limits>
#include <stdexcept>


using namespace ThorsAnvil::Nisse::HTTP;

NISSE_HEADER_ONLY_INCLUDE
Response::Response(std::ostream& stream, Version version, int responseCode)
    : version{version}
    , statusCode{StandardStatusCodeMap::getStandardStatusCodeMap()[responseCode]}
    , headerSent{false}
    , baseStream{stream}
    , checkPoint(std::chrono::high_resolution_clock::now())
{}

NISSE_HEADER_ONLY_INCLUDE
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

NISSE_HEADER_ONLY_INCLUDE
void Response::sendHeaderIfNotSent()
{
    if (!headerSent)
    {
        ThorsLogTrack("ThorsAnvil::Nisse::HTTP::Response", "sendHeaderIfNotSent", "Sending header: ", version, " ", statusCode);
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

NISSE_HEADER_ONLY_INCLUDE
void Response::read(std::istream& stream)
{
    int code;
    if (stream >> version >> code >> IgnoreLine{})
    {
        statusCode = StandardStatusCodeMap::getStandardStatusCodeMap()[code];
    }
}

NISSE_HEADER_ONLY_INCLUDE
Response& Response::setStatus(int newStatusCode)
{
    if (headerSent) {
        ThorsLogAndThrowError(std::runtime_error, "ThorsAnvil::Nisse::HTTP::Response", "setStatus", "Setting status after headers already sent");
    }
    statusCode = StandardStatusCodeMap::getStandardStatusCodeMap()[newStatusCode];
    return *this;
}

NISSE_HEADER_ONLY_INCLUDE
Response& Response::addHeader(std::string_view head, std::string_view value)
{
    if (stream.rdbuf() != nullptr) {
        ThorsLogAndThrowWarning(std::runtime_error, "ThorsAnvil::Nisse::HTTP::Response", "addHeaders", "Headers can not be sent after the body has been started");
    }

    using namespace std::string_view_literals;

    /*
     * Ignore content Length and transfer encoding.
     * These are set when you call body
     */
    if (std::ranges::equal(head, "content-length"sv, ichar_equals)) {
        return *this;
    }
    if (std::ranges::equal(head, "transfer-encoding"sv, ichar_equals)) {
        return *this;
    }
    sendHeaderIfNotSent();
    baseStream << head << ": "<< value << "\r\n";
    return *this;
}

NISSE_HEADER_ONLY_INCLUDE
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

NISSE_HEADER_ONLY_INCLUDE
void Response::error(int code, std::string_view errorMessage)
{
    setStatus(code).addHeader("Error", errorMessage);
}
