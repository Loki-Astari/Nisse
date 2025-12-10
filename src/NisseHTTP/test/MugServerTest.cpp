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
        std::cerr << "HTTPResponse 1\n";
            std::string     version;
            if (!(stream >> version >> code && std::getline(stream, message) && version == "HTTP/1.1")) {
                std::cerr << "Bad Input\n";
                return;
            }
        std::cerr << "HTTPResponse 2\n";
            // Remove the '\r'
            message.resize(message.size() - 1);

        std::cerr << "HTTPResponse 3\n";
            std::string     line;
            std::string     header;
            std::string     value;
        std::cerr << "HTTPResponse 4\n";
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
        std::cerr << "HTTPResponse 5\n";
            std::string    length   = std::string((*this)["content-length"]);
            if (length.size() != 0) {
                readData(stream, std::stoul(length));
            }
        std::cerr << "HTTPResponse 6\n";
            std::string_view    encoding = (*this)["transfer-encoding"];
            if (encoding == "chunked") {
                readChunked(stream);
            }
        std::cerr << "HTTPResponse DONE\n";
        }
    private:
        void readData(std::istream& stream, std::size_t size)
        {
        std::cerr << "Read Data: " << size << "\n";
            body.resize(size);
        std::cerr << "Reading\n";
            stream.read(&body[0], size);
        std::cerr << "DONE\n";
        }
        void readChunked(std::istream& stream)
        {
        std::cerr << "Read Chunked\n";
            std::string     chunkSizeStr;
            while (std::getline(stream, chunkSizeStr) && chunkSizeStr.back() == '\r') {
        std::cerr << "\nGot Chunk: >\n" << chunkSizeStr << "<\n\nS: " << chunkSizeStr.size() << "\n";
        for (int loop = 0; loop < chunkSizeStr.size(); ++loop) {
            std::cerr << "C: " << loop << " " << ((int)chunkSizeStr[loop]) << "\n";
        }
                std::size_t     chunkSize = std::stoul(chunkSizeStr, nullptr, 16);
        std::cerr << "Chunk Size: " << chunkSize << "\n";
                std::size_t     bodySize = body.size();
                body.resize(bodySize + chunkSize);
        std::cerr << "About to read Chunk: " << chunkSize << "\n";
        if (chunkSize != 0) {
                    stream.read(&body[bodySize], chunkSize);
        }
        std::cerr << "Read Done: >" << std::string(&body[bodySize], &body[bodySize + chunkSize]) << "<\n";
        std::cerr << "Avail: " << stream.rdbuf()->in_avail() << "\n";
        char buf[2];
                stream.read(buf, 2);
        std::cerr << "Ignored the new line\n";
                if (chunkSize == 0) {
            std::cerr << "Breaking\n";
                    break;
                }
            }
        std::cerr << "Read Chunk DONE\n";
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
        std::error_code ec;
        FS::path   filePath = FS::canonical(FS::path{"./test/data/pages/page1"}, ec);

        TASock::SocketStream    file{TASock::Socket{TASock::FileInfo{filePath.string(), TASock::FileMode::Read}, TASock::Blocking::Yes}};
       // TASock::SocketStream    file{TASock::Socket{TASock::FileInfo{filePath.string(), TASock::FileMode::Read}, TASock::Blocking::No}};
       // NisServer::AsyncStream  async(file, request.getContext(), NisServer::EventType::Read);

        std::ostream& output = response.body(NisHttp::Encoding::Chunked);
        std::string line;
        std::cerr << "Reading Lines\n";
        while (std::getline(file, line)) {
            std::cerr << "Got Line: >" << line << "<\n";
            output << line << "\n";
        }
        std::cerr << "FLUSHING\n";
        output << std::flush;
        std::cerr << "DONE\n";
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
                                   {handleRequestPath(request, response);}
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
        std::cerr << "Run Exit\n";
        waitForExit.count_down();
        std::cerr << "Latch Signalled\n";
    };

    LocalJthread     serverThread(work);
std::cerr << "ServiceRunAddServerWithFileValidateWorks: 1\n";
    latch.wait();

    // Talk to server.
std::cerr << "ServiceRunAddServerWithFileValidateWorks: 2\n";
    ThorsAnvil::ThorsSocket::SocketStream socketData({"localhost", 8070});

std::cerr << "ServiceRunAddServerWithFileValidateWorks: 3\n";
    socketData << HTTPSend(SendType::GET, SendVersion::HTTP1_1, "localhost", "/files/page1");

std::cerr << "ServiceRunAddServerWithFileValidateWorks: 4\n";
    HTTPResponse   response;
std::cerr << "ServiceRunAddServerWithFileValidateWorks: 5\n";
    socketData >> response;

std::cerr << "ServiceRunAddServerWithFileValidateWorks: 6\n";
    ASSERT_EQ("Data for page 1\n", response.getBody());

    // Touch the control point to shut down the server.
std::cerr << "ServiceRunAddServerWithFileValidateWorks: 7\n";
    ThorsAnvil::ThorsSocket::SocketStream       socket({"localhost", 8079});
std::cerr << "ServiceRunAddServerWithFileValidateWorks: 8\n";
    ThorsAnvil::Nisse::HTTP::HeaderResponse   headers;
std::cerr << "ServiceRunAddServerWithFileValidateWorks: 9\n";
    headers.add("host", "localhost");
std::cerr << "ServiceRunAddServerWithFileValidateWorks: 10\n";
    headers.add("content-length", "0");
std::cerr << "ServiceRunAddServerWithFileValidateWorks: 11\n";
    ThorsAnvil::Nisse::HTTP::ClientRequest  request(socket, "localhost:/?command=stophard");
std::cerr << "ServiceRunAddServerWithFileValidateWorks: 12\n";
    request.addHeaders(headers);
std::cerr << "ServiceRunAddServerWithFileValidateWorks: 13\n";
    request.flushRequest();
std::cerr << "ServiceRunAddServerWithFileValidateWorks: 14\n";
    std::cerr << "Request Flushed\n";
std::cerr << "ServiceRunAddServerWithFileValidateWorks: 15\n";
    waitForExit.wait();
std::cerr << "ServiceRunAddServerWithFileValidateWorks: 16\n";
    std::cerr << "Latch removed\n";
}

