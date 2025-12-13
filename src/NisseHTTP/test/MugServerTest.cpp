#include "gtest/gtest.h"
#include "NisseHTTPConfig.h"

#include "ClientStream.h"
#include "ClientRequest.h"
#include "PyntHTTPControl.h"
#include "HTTPHandler.h"
#include "HeaderResponse.h"
#include "Util.h"
#include "Request.h"
#include "Response.h"
#include "NisseServer/NisseServer.h"
#include "ThorSerialize/JsonThor.h"
#include "ThorsSocket/Socket.h"
#include "ThorsSocket/SocketStream.h"

#include <sstream>
#include <iostream>
#include <iomanip>

#include <latch>
#include <thread>

#include <map>
#include <vector>
#include <string>
#include <string_view>
#include <optional>

#include <utility>
#include <filesystem>
#include <algorithm>
#include <ratio>
#include <chrono>

#include <cctype>
#include <cstddef>


namespace FS        = std::filesystem;
namespace NisServer = ThorsAnvil::Nisse::Server;
namespace NisHttp   = ThorsAnvil::Nisse::HTTP;

enum class ActionType {File, Lib};

/*
 * Some locations were we build do not currently support std::jthread.
 * This is a simplified version just for testing purposes.
 */
//    std::jthread
class LocalJthread: public std::thread
{
    public:
        using std::thread::thread;
        ~LocalJthread()
        {
            join();
        }
};


enum SendType       {GET, POST};
enum SendVersion    {HTTP1_1};

inline
std::ostream& operator<<(std::ostream& stream, SendType type)
{
    switch (type)
    {
        case GET:   return stream << "GET";
        case POST:  return stream << "POST";
    }
}

inline
std::ostream& operator<<(std::ostream& stream, SendVersion version)
{
    switch (version)
    {
        case HTTP1_1:   return stream << "HTTP/1.1";
    }
}


class HTTPSend
{
    SendType            type;
    SendVersion         version;
    std::string_view    path;
    std::string_view    mimeType;
    std::string_view    host;
    std::string_view    body;

    public:
        HTTPSend(SendType type, SendVersion version, std::string_view host, std::string_view path, std::string_view mimeType = "application/json", std::string_view body = "")
            : type(type)
            , version(version)
            , path(path)
            , mimeType(mimeType)
            , host(host)
            , body(body)
        {}

        void send(std::ostream& stream) const
        {
            stream << type << " " << path << " " << version << "\r\n"
                   << "Host: " << host << "\r\n"
                   << "Accept: " << mimeType << "\r\n";
            if (body.size() != 0) {
                stream << "Content-Length: " << body.size() << "\r\n";
            }
            stream << "\r\n" << body << std::flush;
        }

        friend std::ostream& operator<<(std::ostream& stream, HTTPSend const& data) {data.send(stream);return stream;}
};

class HTTPResponse
{
    using Headers = std::map<std::string, std::string>;

    int         code;
    std::string message;
    Headers     headers;
    std::string body;

    public:

        int                 getCode()       const {return code;}
        std::string_view    getMessage()    const {return message;}
        std::string_view    getBody()       const {return body;}
        std::string_view    operator[](std::string_view header) const
        {
            auto find = headers.find(std::string(header));
            if (find == std::end(headers)) {
                return {""};
            }
            return find->second;
        }

        void recv(std::istream& stream)
        {
            std::string     version;
            if (!(stream >> version >> code && std::getline(stream, message) && version == "HTTP/1.1")) {
                ThorsLogDebug("HTTPResponse", "recv", "Bad Input");
                return;
            }
            // Remove the '\r'
            message.resize(message.size() - 1);

            std::string     line;
            std::string     header;
            std::string     value;
            while (std::getline(stream, line)) {
                if (line == "\r") {
                    break;
                }
                std::stringstream   lineStream(std::move(line));


                while (std::getline(lineStream, header, ':') && (lineStream >> std::ws) && std::getline(lineStream, value)) {
                    // Remove the '\r'
                    value.resize(value.size() - 1);
                    std::transform(std::begin(header), std::end(header), std::begin(header), []( char x){return std::tolower(x);});
                    headers[header] = value;
                }
            }
            std::string    length   = std::string((*this)["content-length"]);
            if (length.size() != 0) {
                readData(stream, std::stoul(length));
            }
            std::string_view    encoding = (*this)["transfer-encoding"];
            if (encoding == "chunked") {
                readChunked(stream);
            }
        }
    private:
        void readData(std::istream& stream, std::size_t size)
        {
            body.resize(size);
            stream.read(&body[0], size);
        }
        void readChunked(std::istream& stream)
        {
            std::string     chunkSizeStr;
            while (std::getline(stream, chunkSizeStr) && chunkSizeStr.back() == '\r') {
                std::size_t     chunkSize = std::stoul(chunkSizeStr, nullptr, 16);
                std::size_t     bodySize = body.size();
                body.resize(bodySize + chunkSize);
                if (chunkSize != 0) {
                    stream.read(&body[bodySize], chunkSize);
                }
                char buf[2];
                stream.read(buf, 2);
                if (chunkSize == 0) {
                    break;
                }
            }
        }


        friend std::istream& operator>>(std::istream& stream, HTTPResponse& data) {data.recv(stream);return stream;}
};

class MugServer: public NisServer::NisseServer
{
    using Hanlders = std::vector<NisHttp::HTTPHandler>;

    // PyntControl create access point that can be used to cleanly shut down server.
    NisHttp::PyntHTTPControl    control;
    // HTTPHandler
    // DLLibMap                    libraries;
    // LibraryChecker              libraryChecker;
    Hanlders                    servers;

    void handleRequestPath(NisHttp::Request& request, NisHttp::Response& response)
    {
        std::error_code         ec;
        FS::path                filePath = FS::canonical(FS::path{"./test/data/pages/page1"}, ec);
        TASock::SocketStream    file{TASock::Socket{TASock::FileInfo{filePath.string(), TASock::FileMode::Read}, TASock::Blocking::No}};

        if (!file) {
            // Can't open file for some reason.
            // Then we indicate a 404.
            response.setStatus(404);
            return;
        }
        // Otherwise mark the file stream as Async (thus if the disk is slow we will be de-shcheduled for other work)
        // and simply stream the file to the output stream.
        // TODO
        // Issue on linux.
        //    The stream was closed before it is registered for async use.
        //    This causes issues and an exception is thrown.
        NisServer::AsyncStream  async(file, request.getContext(), NisServer::EventType::Read);
        response.body(NisHttp::Encoding::Chunked) << file.rdbuf();
    }

    public:
        MugServer()
            : NisseServer(4)
            , control(*this)
        {
            ThorsLogDebug("MugServer", "MugServer", " Create Server");
            ThorsLogDebug("MugServer", "MugServer", " Adding Server: ", 8070);
            ThorsLogDebug("MugServer", "MugServer", " Adding Control Port: ", 8079);
            ThorsLogDebug("MugServer", "MugServer", " Adding Action: ", "/files");
            ThorsLogDebug("MugServer", "MugServer", " File Listener: ", "/files/{FilePath}");
            servers.emplace_back();

            servers.back().addPath(NisHttp::Method::GET,
                                   FS::path("/files/{FilePath}").lexically_normal(),
                                   [&](NisHttp::Request& request, NisHttp::Response& response)
                                   {handleRequestPath(request, response);return true;}
                                  );
            listen(TASock::ServerInfo{8070}, servers.back());
            listen(TASock::ServerInfo{8079}, control);
        }
};

TEST(MugServer, ServiceRunAddServerWithFileValidateWorks)
{
    MugServer     server;
    std::latch    latch(1);
    std::latch    waitForExit(1);

    auto work = [&]() {
        server.run(
                [&latch](){latch.count_down();}
        );
        waitForExit.count_down();
    };

    LocalJthread     serverThread(work);
    latch.wait();

    // Talk to server.
    ThorsAnvil::ThorsSocket::SocketStream socketData({"localhost", 8070});

    socketData << HTTPSend(SendType::GET, SendVersion::HTTP1_1, "localhost", "/files/page1");

    HTTPResponse   response;
    socketData >> response;

    ASSERT_EQ("Data for page 1\n", response.getBody());

    // Touch the control point to shut down the server.
    ThorsAnvil::ThorsSocket::SocketStream       socket({"localhost", 8079});
    ThorsAnvil::Nisse::HTTP::HeaderResponse   headers;
    headers.add("host", "localhost");
    headers.add("content-length", "0");
    ThorsAnvil::Nisse::HTTP::ClientRequest  request(socket, "localhost:/?command=stophard");
    request.addHeaders(headers);
    request.flushRequest();
    std::cerr << "Request Flushed\n";
    waitForExit.wait();
    std::cerr << "Latch removed\n";
}

