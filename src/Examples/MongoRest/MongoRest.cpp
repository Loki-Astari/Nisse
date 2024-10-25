#include "MongoServer.h"
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
namespace MRest     = ThorsAnvil::Nisse::Examples::MongoRest;

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
    if (argc != 3 && argc != 4)
    {
        std::cerr << "Usage: MongoRest <port> <MongoHost> [<certificateDirectory>]\n";
        return 1;
    }
    try
    {
        int             port        = std::stoi(argv[1]);
        std::string     mongoHost   = argv[2];
        TAS::ServerInit serverInit  = (argc == 3) ? getNormalInit(port) : getSSLInit(argv[3], port);

        std::cout << "Nisse MongoRest: Port: " << port << " MongoHost: >" << mongoHost << "< Certificate Path: >" << (argc == 3 ? "NONE" : argv[3]) << "<\n";

        MRest::MongoServer  mongoServer;

        NHTTP::HTTPHandler  http;
        // CRUD Person Interface
        http.addPath(NHTTP::Method::POST,   "/person/",     [&](NHTTP::Request& request, NHTTP::Response& response) {mongoServer.personCreate(request, response);});
        http.addPath(NHTTP::Method::GET,    "/person/{id}", [&](NHTTP::Request& request, NHTTP::Response& response) {mongoServer.personGet(request, response);});
        http.addPath(NHTTP::Method::PUT,    "/person/{id}", [&](NHTTP::Request& request, NHTTP::Response& response) {mongoServer.personUpdate(request, response);});
        http.addPath(NHTTP::Method::DELETE, "/person/{id}", [&](NHTTP::Request& request, NHTTP::Response& response) {mongoServer.personDelete(request, response);});

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
