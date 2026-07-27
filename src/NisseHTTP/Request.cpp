#include "Request.h"
#include <algorithm>
#include <cstddef>
#include <vector>
#include <array>
#include <map>
#include <charconv>

using namespace ThorsAnvil::Nisse::HTTP;

NISSE_HEADER_ONLY_INCLUDE
Request::Request(std::string_view proto, std::istream& stream)
    : context(nullptr)
    , version{Version::Unknown}
    , method{Method::Other}
{
    init(proto, stream);
}

NISSE_HEADER_ONLY_INCLUDE
Request::Request(Server::Context& context, std::string_view proto, std::istream& stream)
    : context(&context)
    , version{Version::Unknown}
    , method{Method::Other}
{
    init(proto, stream);
}

NISSE_HEADER_ONLY_INCLUDE
void Request::init(std::string_view proto, std::istream& stream)
{
    ThorsLogTrack("ThorsAnvil::Nisse::HTTP::Request", "init", "Creating a request");
    std::string_view path = readFirstLine(stream);
    if (path.size() != 0)
    {
        readHeaders(head, stream)   &&
        buildURL(proto, path)       &&
        buildStream(stream);
    }
}

NISSE_HEADER_ONLY_INCLUDE
std::string_view Request::readFirstLine(std::istream& stream)
{
    // Read the first line
    std::getline(stream, messageHeader);
    if (messageHeader.size() > 0 && messageHeader[messageHeader.size() - 1] == '\r') {
        messageHeader.resize(messageHeader.size() - 1);
    }
    else
    {
        ThorsLogInfo("ThorsAnvil::Nisse::HTTP::Request", "readFirstLine", ": Header not \\r\\n terminated");
        failResponse.insert_or_assign("error", "Invalid HTTP Request");
        failResponse.insert_or_assign("reason", "Header Not terminated with <CR><LF>");
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

    ThorsLogTrack("ThorsAnvil::Nisse::HTTP::Request", "readFirstLine >", messageHeader, "< Request: Method:", method, " Path:", path, " Protocol:", prot, " Version:", version);
    if (meth.size() == 0 || path.size() == 0 || pv.size() == 0 || version == Version::Unknown || method == Method::Other)
    {
        ThorsLogTrack("ThorsAnvil::Nisse::HTTP::Request", "readFirstLine", ": Bad Request: ", "Method: >", meth, "< Path: >", path, "< Proto: >", pv, "<");
        failResponse.insert_or_assign("error", "Invalid HTTP Request");
        failResponse.insert_or_assign("method", meth);
        failResponse.insert_or_assign("path", path);
        failResponse.insert_or_assign("proto", pv);
        return "";
    }

    return path;
}

NISSE_HEADER_ONLY_INCLUDE
bool Request::readHeaders(HeaderRequest& dst, std::istream& stream)
{
    std::size_t                     headerSize  = 0;
    std::array<char, maxHeaderSize> lineBuffer;
    while (stream.getline(lineBuffer.data(), maxHeaderSize))
    {
        headerSize += stream.gcount();
        if (headerSize > maxTotalHeaderSize) {
            ThorsLogTrack("ThorsAnvil::Nisse::HTTP::Request", "readHeaders", ": exceeds header limit size");
            return false;
        }
        std::string_view line{lineBuffer.data(), lineBuffer.data() + stream.gcount() - 1};
        if (line == "\r") {
            break;
        }
        auto split = line.find(':');
        if (line.size() == 0 || line[line.size() - 1] != '\r' || split == std::string::npos)
        {
            ThorsLogTrack("ThorsAnvil::Nisse::HTTP::Request", "readHeaders", ": Bad Request Header: ", line);
            failResponse.insert_or_assign("error", "Invalid HTTP Header");
            failResponse.insert_or_assign("header", line);
            return false;
        }
        dst.add({&line[0], &line[0] + split}, {&line[0] + split + 1, &line[0] + line.size() - 1});
    }
    return stream.good();
}

NISSE_HEADER_ONLY_INCLUDE
bool Request::buildURL(std::string_view proto, std::string_view path)
{
    using std::literals::operator""sv;
    std::vector<std::string> const& hostValues = head.getHeader("host"sv);
    if (hostValues.size() == 0)
    {
        ThorsLogInfo("ThorsAnvil::Nisse::HTTP::Request", "buildURL", ": Bad Request No Host Field: ");
        failResponse.insert_or_assign("error", "Invalid HTTP Request- No Host header");
        return false;
    }
    url = URL{proto, hostValues[0], path};
    return true;
}

NISSE_HEADER_ONLY_INCLUDE
std::string_view Request::preloadStreamIntoBuffer(bool force) const
{
    return input.preloadStreamIntoBuffer(force);
}

NISSE_HEADER_ONLY_INCLUDE
bool Request::buildStream(std::istream& stream)
{
    auto&   contentLength    = head.getHeader("content-length");
    auto&   transferEncoding = head.getHeader("transfer-encoding");

    /* If no content set an input stream with zero size */
    if (contentLength.size() + transferEncoding.size() == 0) {
        input.addBuffer(StreamBufInput(stream, 0, [&stream](std::ios_base::iostate state){stream.setstate(state);}));
        return true;
    }

    /* Check there we have a valid content definition */
    if (contentLength.size() + transferEncoding.size() != 1)
    {
        ThorsLogInfo("ThorsAnvil::Nisse::HTTP::Request", "buildStream", ": Bad Request: Includes more than one 'content-length' and 'transfer-encoding'");
        failResponse.insert_or_assign("error", "Invalid HTTP Request- Includes multiple 'content-length' or 'transfer-encoding'");
        for (auto const& v: contentLength) {
            failResponse.insert_or_assign("value-content-length", v);
        }
        for (auto const& v: transferEncoding) {
            failResponse.insert_or_assign("value-transfer-encoding", v);
        }
        return false;
    }

    if (transferEncoding.size() == 0)
    {
        // The header specifies a content Length.
        if (contentLength[0].size() == 0) {
            ThorsLogInfo("ThorsAnvil::Nisse::HTTP::Request", "buildStream", ": Invalid content-length: Empty");
            failResponse.insert_or_assign("error", "Invalid HTTP Request- Malformed content-length: <empty>");
            return false;
        }
        std::streamsize bodySize = 0;
        char const*     first = &contentLength[0][0];
        char const*     last  = first + contentLength[0].size();
        auto result = std::from_chars(first, last, bodySize);
        if (result.ec != std::errc() || result.ptr != last || bodySize < 0) {
            ThorsLogInfo("ThorsAnvil::Nisse::HTTP::Request", "buildStream", ": Invalid content-length: ", contentLength[0]);
            failResponse.insert_or_assign("error", "Invalid HTTP Request- Malformed content-length: " + contentLength[0]);
            return false;
        }

        // Valid content length. Set fixed size input stream.
        input.addBuffer(StreamBufInput(stream, bodySize, [&stream](std::ios_base::iostate state){stream.setstate(state);}));
    }
    else
    {
        // The header specifies a body encoding/
        if (transferEncoding[0] != "chunked") {
            /* TODO: Understand other transfer-encoding and add support for these */
            ThorsLogInfo("ThorsAnvil::Nisse::HTTP::Request", "buildStream", ": Invalid transfer-encoding: ", transferEncoding[0]);
            failResponse.insert_or_assign("error", "Invalid HTTP Request- Unsupported Transer Encoding: " + transferEncoding[0]);
            return false;
        }

        // Valid transfer encoding. Chunked.
        // Set an input stream that decodes a Chunked input stream.
        input.addBuffer(StreamBufInput(stream,
                                       Encoding::Chunked,
                                       [&](std::ios_base::iostate state){stream.setstate(state);if (stream){readHeaders(tail, stream);}}));
    }

    // Good input.
    return true;
}

NISSE_HEADER_ONLY_INCLUDE
std::istream& Request::body() const
{
    return input;
}

NISSE_HEADER_ONLY_INCLUDE
void Request::print(std::ostream& stream) const
{
    stream << messageHeader << "\r\n"
           << head
           << "\r\n";
}
