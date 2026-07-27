#include "NisseHTTP/Util.h"
#include "NisseServer/Server.h"
#include "NisseServer/PyntControl.h"
#include "NisseServer/Context.h"
#include "NisseHTTP/HTTPHandler.h"
#include "NisseHTTP/Request.h"
#include "NisseHTTP/Response.h"
#include <ThorsSocket/Server.h>
#include <filesystem>

namespace TASock    = ThorsAnvil::ThorsSocket;
namespace NisServer = ThorsAnvil::Nisse::Server;
namespace NisHttp   = ThorsAnvil::Nisse::HTTP;
namespace FS        = std::filesystem;

class ReverseProxy: public NisServer::Server
{
    NisHttp::HTTPHandler    http;
    NisServer::PyntControl  control;
    std::string             dest;
    int                     destPort;

    TASock::ServerInit getServerInit(std::optional<FS::path> certPath, int port)
    {
        if (!certPath.has_value()) {
            return TASock::ServerInfo{port};
        }

        TASock::CertificateInfo     certificate{FS::canonical(FS::path(*certPath) /= "fullchain.pem"),
                                                FS::canonical(FS::path(*certPath) /= "privkey.pem")
                                               };
        TASock::SSLctx              ctx{TASock::SSLMethodType::Server, certificate};
        return TASock::SServerInfo{port, std::move(ctx)};
    }

    bool isHeader(std::string_view headerLine, std::string_view check)
    {
        headerLine.remove_prefix(std::min(headerLine.size(), headerLine.find_first_not_of(" \r\t\v")));
        auto find = std::min(headerLine.size(), headerLine.find(':'));
        headerLine.remove_suffix(headerLine.size() - find);

        return std::ranges::equal(headerLine, check, NisHttp::ichar_equals);
    }
    std::size_t getContentLength(std::string_view line)
    {
        auto find = std::min(line.size(), line.find(':'));
        find = line.find_first_not_of(" \r\t\v", find + 1);
        line.remove_prefix(find);
        line.remove_suffix(1);
        int result = 0;
        std::from_chars(line.data(), line.data() + line.size(), result);
        return result;
    }

    bool handleRequest(NisHttp::Request const& request, NisHttp::Response& response)
    {
        TASock::SocketInfo      init{dest, destPort};
        TASock::SocketStream    stream{TASock::Socket{init, TASock::Blocking::No}};
        NisServer::AsyncStream  async(stream, request.getContext(), NisServer::EventType::Write);

        if (!stream) {
            response.error(500, "Failed to open socket");
            return false;
        }

        // Step 1:
        // Forward the request.
        stream << request
               << request.body().rdbuf()
               << std::flush;

        // Step 2: Read the reply status line and headers
        stream >> response;
        NisHttp::BodyEncoding       encoding = 0;
        std::string                 line;
        while (std::getline(stream, line)) {

        while (std::getline(stream, line)) {
            if (line == "\r") {
                // The back slash r back slash n will be added when we call body()
                break;
            }
            // content-length or transfer-encoding is handled when we call body() below.
            // Note: It is also filtered out by addHeader() so no point in explicitly adding these
            using namespace std::string_view_literals;
            if (isHeader(line, "content-length"sv)) {
                encoding = getContentLength(line);
                continue;
            }
            if (isHeader(line, "transfer-encoding")) {
                encoding = NisHttp::Encoding::Chunked;
                continue;
            }
            // Calculate the header name and value.
            auto breakPoint = std::min(std::size(line), line.find(':'));
            auto valueStart = std::min(std::size(line), breakPoint + 1);
            auto headerName = line.substr(0, breakPoint);
            auto headerValue = line.substr(valueStart);
            response.addHeader(headerName, headerValue);
        }

        // Step 3: Send the reply back to the originator.
        NisHttp::StreamInput        body(stream, encoding);
        response.body(encoding) << body.rdbuf();
        return true;
    }

    public:
        ReverseProxy(int port, std::string const& dest, int destPort, std::optional<FS::path> certPath)
            : control(*this)
            , dest(dest)
            , destPort(destPort)
        {
            http.addPath(NisHttp::All::Method, "/{command}", [&](NisHttp::Request const& request, NisHttp::Response& response){return handleRequest(request, response);});
            listen(getServerInit(certPath, port), http);

            listen(TASock::ServerInfo{port+2}, control);
        }
};

int main(int argc, char* argv[])
{
#if 0
    ThorsLogLevel(9);
#endif
    if (argc != 5 && argc != 4)
    {
        std::cerr << "Usage: ReverseProxy <port> <serviceHost> <servicePort> [<certificateDirectory>]\n";
        return 1;
    }
    try
    {
        int                     port        = std::stoi(argv[1]);
        std::string             dest        = argv[2];
        int                     destPort    = std::stoi(argv[3]);
        std::optional<FS::path> certPath;
        if (argc == 5) {
            certPath = FS::canonical(argv[4]);
        }

        std::cout << "Nisse ReverseProxy: Port: " << port << " Destination: >" << dest << "< DestPort: >" << destPort << "< Certificate Path: >" << (argc == 4 ? "NONE" : argv[4]) << "<\n";

        ReverseProxy   server(port, dest, destPort, certPath);;
        server.run();
    }
    catch (std::exception const& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
        throw;
    }
    catch (...)
    {
        std::cerr << "Exception: Unknown\n";
        throw;
    }
}
