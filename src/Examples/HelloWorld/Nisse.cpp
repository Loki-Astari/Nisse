#include "NisseServer/NisseServer.h"
#include "NisseServer/PyntControl.h"
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
    if (argc != 2 && argc != 3)
    {
        std::cerr << "Usage: HelloWorld <port> [<certificateDirectory>]\n";
        return 1;
    }
    try
    {
        int             port        = std::stoi(argv[1]);
        TAS::ServerInit serverInit  = (argc == 2) ? getNormalInit(port) : getSSLInit(argv[2], port);

        std::cout << "Nisse Hello World: Port: " << port << " Certificate Path: >" << (argc == 2 ? "NONE" : argv[2]) << "<\n";

        // Part 1:
        // =======
        // Object to processes HTTP connections with.
        // Register with listen() below.
        NHTTP::HTTPHandler   http;
        http.addPath("/HW{Who}.html", [](NHTTP::Request& request, NHTTP::Response& response)
        {
            std::string who  = request.variables()["Who"];
            std::string data = R"(
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html>
<head><title>Nisse server 1.1</title></head>
<body>Hello world: )"  + who + R"(</body>
</html>
)";
            NHTTP::HeaderResponse   header;
            response.addHeaders(header);
            response.body(data.size()) << data;
        });
        http.addPath("/CK{Who}.html", [](NHTTP::Request& request, NHTTP::Response& response)
        {
            NHTTP::HeaderResponse   header;
            response.addHeaders(header);
            response.body(NHTTP::Encoding::Chunked) << R"(
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html>
<head><title>Nisse server 1.1</title></head>
<body>Hello world: )" << request.variables()["Who"] << R"(</body>
</html>
)";
        });

        // Part 2:
        // =======
        // Set up the server
        NServer::NisseServer              server;
        server.listen(serverInit, http);

        // This interface does nothing.
        // But if you connect to it (port+2) it will cleanly shutdown the server.
        // But you can hit ctrl-c will usually work.
        NServer::PyntControl  control(server);
        server.listen(TAS::ServerInfo{port+2}, control);


        // Part 3:
        // =======
        // Let the server run.
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
