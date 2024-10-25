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
    if (argc != 6 && argc != 7)
    {
        std::cerr << "Usage: MongoRest <port> <MongoHost> <MongoUser> <MongoPass> <MongoDB> [<certificateDirectory>]\n";
        return 1;
    }
    try
    {
        int             port        = std::stoi(argv[1]);
        std::string     mongoHost   = argv[2];
        std::string     mongoUser   = argv[3];
        std::string     mongoPass   = argv[4];
        std::string     mongoDB     = argv[5];
        TAS::ServerInit serverInit  = (argc == 6) ? getNormalInit(port) : getSSLInit(argv[6], port);

        std::cout << "Nisse MongoRest: Port: " << port << " MongoHost: >" << mongoHost << "< Certificate Path: >" << (argc == 3 ? "NONE" : argv[3]) << "<\n";

        MRest::MongoServer  mongoServer{mongoHost, 27017, mongoUser, mongoPass, mongoDB};

        NHTTP::HTTPHandler  http;
        // CRUD Person Interface
        http.addPath(NHTTP::Method::POST,   "/person/",        [&](NHTTP::Request& request, NHTTP::Response& response) {mongoServer.personCreate(request, response);});
        http.addPath(NHTTP::Method::GET,    "/person/Id-{id}", [&](NHTTP::Request& request, NHTTP::Response& response) {mongoServer.personGet(request, response);});
        http.addPath(NHTTP::Method::PUT,    "/person/Id-{id}", [&](NHTTP::Request& request, NHTTP::Response& response) {mongoServer.personUpdate(request, response);});
        http.addPath(NHTTP::Method::DELETE, "/person/Id-{id}", [&](NHTTP::Request& request, NHTTP::Response& response) {mongoServer.personDelete(request, response);});

        // Search Person Interface
        http.addPath(NHTTP::Method::GET,    "/person/find",    [&](NHTTP::Request& request, NHTTP::Response& response) {mongoServer.personFind(request, response);});

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
