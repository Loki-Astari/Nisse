#include "ClientHTTP.h"

#include <algorithm>

using namespace ThorsAnvil::Nisse::HTTP;

NISSE_HEADER_ONLY_INCLUDE
bool ClientHTTPResponse::readFirstLine(std::iostream& stream)
{
    std::string messageHeader;
    std::getline(stream, messageHeader);

    if (messageHeader.size() > 0 && messageHeader[messageHeader.size() - 1] == '\r') {
        messageHeader.resize(messageHeader.size() - 1);
    }
    else
    {
        ThorsLogInfo("ThorsAnvil::Nisse::HTTP::ClientHTTPResponse", "readFirstLine", ": Header not \\r\\n terminated");
        status = 500;
        message = "Invalid HTTP Response received from Server. First Line no '\\r'. This message generated client side";
        return false;
    }

    // Extract the TTP-Method
    auto versStart  = std::min(messageHeader.size(), messageHeader.find_first_not_of(" ", 0));
    auto versEnd    = std::min(messageHeader.size(), messageHeader.find_first_of(' ', versStart));

    // Status-Code
    auto codeStart  = std::min(messageHeader.size(), messageHeader.find_first_not_of(" ", versEnd));
    auto codeEnd    = std::min(messageHeader.size(), messageHeader.find_first_of(" ", codeStart));

    // Reason-Phrase
    auto reasStart = std::min(messageHeader.size(), messageHeader.find_first_not_of(" ", codeEnd));
    auto reasEnd   = messageHeader.size();

    std::string_view vers(messageHeader.begin() + versStart, messageHeader.begin() + versEnd);
    std::string_view code(messageHeader.begin() + codeStart, messageHeader.begin() + codeEnd);
    std::string_view reas(messageHeader.begin() + reasStart, messageHeader.begin() + reasEnd);

    version = findVersion(vers);
    std::from_chars(code.data(), code.data() + code.size(), status);
    message = std::string(reas);

    ThorsLogTrack("ThorsAnvil::Nisse::HTTP::ClientHTTPResponse", "readFirstLine", "Response: Version:", version, " Code:", status, " Message:", message);
    if (vers.size() == 0 || code.size() == 0 || reas.size() == 0 || version == Version::Unknown)
    {
        ThorsLogTrack("ThorsAnvil::Nisse::HTTP::ClientHTTPResponse", "readFirstLine", ": Bad Request: ", "Version: >", version, "< Code: >", code, "< Reason: >", reas, "<");
        status = 500;
        message = "Invalid HTTP Response received from Server. First Line no invalid format. This message generated client side";
        return false;
    }
    std::string header;
    while (std::getline(stream, header)) {
        if (header == "\r") {
            break;
        }
        auto keysStart = std::min(header.size(), header.find_first_not_of(" ", 0));
        auto keysEnd   = std::min(header.size(), header.find(':'));
        auto valuStart = std::min(header.size(), header.find_first_not_of(" ", keysEnd + 1));
        auto valuEnd   = std::min(header.size(), header.find_first_of(" \r", valuStart));

        std::string_view    keys{std::begin(header) + keysStart, std::begin(header) + keysEnd};
        std::string_view    valu{std::begin(header) + valuStart, std::begin(header) + valuEnd};

        ThorsLogTrack("ThorsAnvil::Nisse::HTTP::ClientHTTPResponse", "readFirstLine", "Header:", keys, " : ", valu);
        headers.add(keys, valu);
    }
    return true;
}

NISSE_HEADER_ONLY_INCLUDE
bool ClientHTTPResponse::buildStream(std::iostream& stream)
{
    auto&   contentLength    = headers.getHeader("content-length");
    auto&   transferEncoding = headers.getHeader("transfer-encoding");

    /* If no content set an input stream with zero size */
    if (contentLength.size() + transferEncoding.size() == 0)
    {
        input.addBuffer(StreamBufInput(stream, 0, [&stream](std::ios_base::iostate state){stream.setstate(state);}));
        return true;
    }

    /* Check there we have a valid content definition */
    if (contentLength.size() + transferEncoding.size() != 1)
    {
        ThorsLogInfo("ThorsAnvil::Nisse::HTTP::ClientHTTPResponse", "buildStream", ": Bad Request: Includes more than one 'content-length' and 'transfer-encoding'");
        return false;
    }

    if (transferEncoding.size() == 0)
    {
        // The header specifies a content Length.
        if (contentLength[0].size() == 0) {
            ThorsLogInfo("ThorsAnvil::Nisse::HTTP::ClientHTTPResponse", "buildStream", ": Invalid content-length: Empty");
            return false;
        }
        std::streamsize bodySize = 0;
        char const*     first = &contentLength[0][0];
        char const*     last  = first + contentLength[0].size();
        auto result = std::from_chars(first, last, bodySize);
        if (result.ec != std::errc() || result.ptr != last || bodySize < 0) {
            ThorsLogInfo("ThorsAnvil::Nisse::HTTP::ClientHTTPResponse", "buildStream", ": Invalid content-length: ", contentLength[0]);
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
            ThorsLogInfo("ThorsAnvil::Nisse::HTTP::ClientHTTPResponse", "buildStream", ": Invalid transfer-encoding: ", transferEncoding[0]);
            return false;
        }

        // Valid transfer encoding. Chunked.
        // Set an input stream that decodes a Chunked input stream.
        input.addBuffer(StreamBufInput(stream, Encoding::Chunked, [&stream](std::ios_base::iostate state){stream.setstate(state);}));
    }

    // Good input.
    return true;
}

NISSE_HEADER_ONLY_INCLUDE
void ClientHTTPBase::send(Method method, ClientRequest const& request, BodyEncoding encoding, std::function<void(StreamOutput& action)>&& action) const
{
    // Send to the server a correctly encoded HTTP request.
    // With the minumum headers.
    stream << method << " " << request.path << " " << version << "\r\n"
           << "host: " << host() << "\r\n"
           << encoding;

    // Add the user requested header.
    for (auto const& header: request.headers) {
        for (auto const& value: header.second) {
            stream << header.first << ": " << value << "\r\n";
        }
    }

    // Empty line marking end of headers.
    stream << "\r\n";

    /*
     * Create a stream that knows how to enforce the encoding specified.
     *
     * If you specified an exact size (via integer) then the stream will set the bad bit if you send
     * more data than the allowed value. But if you don't send enough will fill the output stream in
     * the destructor.
     *
     * If the encoding is chunked it will buffer internally until you hit the buffer size or manually flush;
     * at which point it will send the hex encoded size followed by the data in the buffer.
     * The destructor will then make sure then stream is correctly terminated with an empty block.
     *
     * The same pattern for other encodings. The stream will automatically apply the encoding.
     * And the destructor of `output` will make sure the stream is correctly terminated and flushed.
     */
    StreamOutput    output(stream, encoding);
    action(output);
}

NISSE_HEADER_ONLY_INCLUDE
void ClientHTTPBase::processResp(std::function<void(ClientHTTPResponse const&)>&& action) const
{
    // Reads the status line and header information from the stream.
    // Internally it will create a stream object that decodes the input based on the headers).
    // So your code can read directly from the input.
    ClientHTTPResponse  response{stream};
    if (!response.isValid()) {
        close();
    }
    action(response);
}
