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

Response::Response(std::ostream& stream, Version version, int responseCode)
    : version{version}
    , statusCode{standardCodes[responseCode]}
    , headerSent{false}
    , baseStream{stream}
{}

Response::~Response()
{
    if (stream.rdbuf() == nullptr)
    {
        std::cerr << "\tSending minimum required data\n";
        baseStream << version << " " << statusCode << "\r\n"
                   << "content-length: 0\r\n"
                   << "\r\n"
                   << std::flush;
    }
}

void Response::setStatus(int newStatusCode)
{
    statusCode = standardCodes[newStatusCode];
}

std::ostream& Response::addHeaders(HeaderResponse const& headers, Encoding type)
{
    return addHeaders(headers, StreamBufOutput{baseStream, type}, "transfer-encoding: chunked\r\n");
}

std::ostream& Response::addHeaders(HeaderResponse const& headers, std::streamsize length)
{
    return addHeaders(headers, StreamBufOutput{baseStream, length}, "content-length: "s + std::to_string(length) + "\r\n");
}

std::ostream& Response::addHeaders(HeaderResponse const& headers, StreamBufOutput&& buffer, std::string_view extraHeader)
{
    if (headerSent) {
        ThorsLogAndThrowLogical("ThorsAnvil::Nisse::Response", "addHeaders", "Headers have already been sent");
    }
    baseStream << version << " " << statusCode << "\r\n"
               << headers
               << extraHeader
               << "\r\n"
               << std::flush;
    headerSent = true;

    stream.addBuffer(std::move(buffer));
    return stream;
}
