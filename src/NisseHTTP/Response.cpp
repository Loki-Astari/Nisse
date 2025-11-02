#include "Response.h"
#include "ThorsLogging/ThorsLogging.h"
#include <iostream>
#include <string>

using namespace ThorsAnvil::Nisse::HTTP;

using std::literals::string_literals::operator""s;
using std::literals::string_view_literals::operator""sv;

StandardStatusCodeMap::StatusCodeMap const StandardStatusCodeMap::standardCodes
{
    {
        {100, "Continue"sv}, {101, "Switching Protocols"sv}, {102, "Processing Deprecated"sv}, {103, "Early Hints"sv},

        {200, "OK"sv}, {201, "Created"sv}, {202, "Accepted"sv}, {203, "Non-Authoritative Information"sv},
        {204, "No Content"sv}, {205, "Reset Content"sv}, {206, "Partial Content"sv}, {207, "Multi-Status (WebDAV)"sv},
        {208, "Already Reported (WebDAV)"sv}, {226, "IM Used (HTTP Delta encoding)"sv},

        {300, "Multiple Choices"sv}, {301, "Moved Permanently"sv}, {302, "Found"sv}, {303, "See Other"sv},
        {304, "Not Modified"sv}, {305, "Use Proxy Deprecated"sv}, {306, "unused"sv}, {307, "Temporary Redirect"sv},
        {308, "Permanent Redirect"sv},

        {400, "Bad Request"sv}, {401, "Unauthorized"sv}, {402, "Payment Required"sv}, {403, "Forbidden"sv},
        {404, "Not Found"sv}, {405, "Method Not Allowed"sv}, {406, "Not Acceptable"sv},
        {407, "Proxy Authentication Required"sv}, {408, "Request Timeout"sv}, {409, "Conflict"sv}, {410, "Gone"sv},
        {411, "Length Required"sv}, {412, "Precondition Failed"sv}, {413, "Content Too Large"sv}, {414, "URI Too Long"sv},
        {415, "Unsupported Media Type"sv}, {416, "Range Not Satisfiable"sv}, {417, "Expectation Failed"sv},
        {418, "I'm a teapot"sv}, {421, "Misdirected Request"sv}, {422, "Unprocessable Content (WebDAV)"sv},
        {423, "Locked (WebDAV)"sv}, {424, "Failed Dependency (WebDAV)"sv}, {425, "Too Early Experimental"sv},
        {426, "Upgrade Required"sv}, {428, "Precondition Required"sv}, {429, "Too Many Requests"sv},
        {431, "Request Header Fields Too Large"sv}, {451, "Unavailable For Legal Reasons"sv},

        {500, "Internal Server Error"sv}, {501, "Not Implemented"sv}, {502, "Bad Gateway"sv},
        {503, "Service Unavailable"sv}, {504, "Gateway Timeout"sv}, {505, "HTTP Version Not Supported"sv},
        {506, "Variant Also Negotiates"sv}, {507, "Insufficient Storage (WebDAV)"sv}, {508, "Loop Detected (WebDAV)"sv},
        {510, "Not Extended"sv}, {511, "Network Authentication Required"sv}
    },
    [](StatusCode const& lhs, StatusCode const& rhs){return lhs.code < rhs.code;}
};

StatusCode const& StandardStatusCodeMap::operator[](int code)
{
    static StatusCode  unknown{500, "Internal Server Error"sv};

    auto find = standardCodes.find({code, ""sv});
    return find == standardCodes.end() ? unknown : *find;
}

StandardStatusCodeMap standardCodes;

void StatusCode::print(std::ostream& stream) const
{
    stream << code << " " << message;
}

Response::Response(std::ostream& stream, Version version, int responseCode)
    : version{version}
    , statusCode{standardCodes[responseCode]}
    , headerSent{false}
    , baseStream{stream}
{}

Response::~Response()
{
    ThorsLogDebug("ThorsAnvil::Nisse::HTTP::Response", "~Response", "Responce object destruction");
    if (stream.rdbuf() == nullptr)
    {
        if (!headerSent)
        {
            ThorsLogDebug("ThorsAnvil::Nisse::HTTP::Response", "~Response", "Sending header: ", statusCode);
            baseStream << version << " " << statusCode << "\r\n";
            headerSent = true;
        }
        ThorsLogDebug("ThorsAnvil::Nisse::HTTP::Response", "~Response", "Setting content length to zero and flushing");
        baseStream << "content-length: 0\r\n"
                   << "\r\n"
                   << std::flush;
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
        ThorsLogAndThrowWarning(std::runtime_error, "ThorsAnvil::Nisse::Response", "addHeaders", "Headers can not be sent after the body has been started");
    }

    if (!headerSent)
    {
        baseStream << version << " " << statusCode << "\r\n";
        headerSent = true;
    }

    baseStream << headers;
}

std::ostream& Response::body(BodyEncoding bodyEncoding)
{
    ThorsLogDebug("ThorsAnvil::Nisse::HTTP::Response", "body", "adding body");
    if (!headerSent)
    {
        ThorsLogDebug("ThorsAnvil::Nisse::HTTP::Response", "body", "sending header");
        baseStream << version << " " << statusCode << "\r\n";
        headerSent = true;
    }
    ThorsLogDebug("ThorsAnvil::Nisse::HTTP::Response", "body", "adding body to stream");
    baseStream << bodyEncoding
               << "\r\n"
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
