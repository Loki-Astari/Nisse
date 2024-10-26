#include "NisseServer/NisseServer.h"
#include "NisseServer/PyntControl.h"
#include "NisseServer/AsyncStream.h"
#include "NisseHTTP/HTTPHandler.h"
#include "NisseHTTP/Request.h"
#include "NisseHTTP/Response.h"
#include <ThorsSocket/Server.h>
#include <filesystem>

namespace TASock    = ThorsAnvil::ThorsSocket;
namespace TANS      = ThorsAnvil::Nisse::Server;
namespace TANH      = ThorsAnvil::Nisse::HTTP;
namespace FS        = std::filesystem;

class ReverseProxy: public TANS::NisseServer
{
    TANH::HTTPHandler       http;
    TANS::PyntControl       control;
    std::string             dest;
    int                     destPort;

    TASock::ServerInit getServerInit(std::optional<FS::path> certPath, int port)
    {
        if (!certPath.has_value()) {
            return TASock::ServerInfo{port};
        }

        TASock::CertificateInfo     certificate{FS::canonical(*certPath /= "fullchain.pem"),
                                                FS::canonical(*certPath /= "privkey.pem")
                                               };
        TASock::SSLctx              ctx{TASock::SSLMethodType::Server, certificate};
        return TASock::SServerInfo{port, ctx};
    }

    void handleRequest(TANH::Request& request, TANH::Response& response)
    {
        TASock::SocketInfo      init{dest, destPort};
        TASock::SocketStream    socket{TASock::Socket{init, TASock::Blocking::No}};
        TANS::AsyncStream       async(socket, request.getContext(), TANS::EventType::Write);

        if (!socket) {
            return response.error(500, "Failed to open socket");
        }

        // Step 1:
        // Forward the request.
        socket << request
               << request.body().rdbuf()
               << std::flush;

        // Step 2: Read the reply and return.
        socket >> response;
        TANH::HeaderPassThrough headers(socket);
        TANH::StreamInput       body(socket, headers.getEncoding());

        // Step 3: Send the reply back to the originator.
        response.addHeaders(headers);
        response.body(headers.getEncoding()) << body.rdbuf();
    }

    public:
        ReverseProxy(int port, std::string const& dest, int destPort, std::optional<FS::path> certPath)
            : control(*this)
            , dest(dest)
            , destPort(destPort)
        {
            http.addPath(TANH::All::Method, "/{command}", [&](TANH::Request& request, TANH::Response& response){handleRequest(request, response);});
            listen(getServerInit(certPath, port), http);

            listen(TASock::ServerInfo{port+2}, control);
        }
};

int main(int argc, char* argv[])
{
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
