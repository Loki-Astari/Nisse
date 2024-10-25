#include "NisseServer/NisseServer.h"
#include "NisseServer/PyntControl.h"
#include "NisseServer/AsyncStream.h"
#include "NisseHTTP/HTTPHandler.h"
#include "NisseHTTP/Request.h"
#include "NisseHTTP/Response.h"
#include <ThorsLogging/ThorsLogging.h>
#include <ThorsSocket/Server.h>
#include <filesystem>

namespace TAS       = ThorsAnvil::ThorsSocket;
namespace NServer   = ThorsAnvil::Nisse::Server;
namespace NHTTP     = ThorsAnvil::Nisse::HTTP;
namespace FS        = std::filesystem;

TAS::ServerInit getSSLInit(FS::path certPath, int port)
{
    TAS::CertificateInfo        certificate{FS::canonical(certPath /= "fullchain.pem"),
                                            FS::canonical(certPath /= "privkey.pem")
                                           };
    TAS::SSLctx                 ctx{TAS::SSLMethodType::Server, certificate};
    return TAS::SServerInfo{port, ctx};
}

TAS::ServerInit getNormalInit(int port)
{
    return TAS::ServerInfo{port};
}

int main(int argc, char* argv[])
{
    if (argc != 5 && argc != 4)
    {
        std::cerr << "Usage: ReverseProxy <port> <serviceHost> <servicePort> [<certificateDirectory>]\n";
        return 1;
    }
    try
    {
        int             port        = std::stoi(argv[1]);
        std::string     dest        = argv[2];
        int             destPort    = std::stoi(argv[3]);
        TAS::ServerInit serverInit  = (argc == 4) ? getNormalInit(port) : getSSLInit(argv[4], port);

        std::cout << "Nisse ReverseProxy: Port: " << port << " Destination: >" << dest << "< DestPort: >" << destPort << "< Certificate Path: >" << (argc == 4 ? "NONE" : argv[4]) << "<\n";

        NHTTP::HTTPHandler   http;

        http.addPath(NHTTP::All::Method, "/{command}", [&](NHTTP::Request& request, NHTTP::Response& response)
        {
            TAS::SocketInfo         init{dest, destPort};
            TAS::SocketStream       socket{TAS::Socket{init, TAS::Blocking::No}};
            NServer::AsyncStream    async(socket, request.getContext(), NServer::EventType::Write);

            if (!socket)
            {
                NHTTP::HeaderResponse    header;

                response.setStatus(404);
                response.addHeaders(header);
            }

            // Step 1:
            // Forward the request.
            socket << request
                   << request.body().rdbuf()
                   << std::flush;

            // Step 2: Read the reply and return.
            socket >> response;
            NHTTP::HeaderPassThrough    headers(socket);
            NHTTP::StreamInput          body(socket, headers.getEncoding());

            // Step 3: Send the reply back to the originator.
            response.addHeaders(headers);
            response.body(headers.getEncoding()) << body.rdbuf();
        });

        NServer::NisseServer   server;
        server.listen(serverInit, http);

        NServer::PyntControl  control(server);
        server.listen(TAS::ServerInfo{port+2}, control);

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
