#include "MongoServer.h"
#include "NisseServer/NisseServer.h"
#include "NisseServer/PyntControl.h"
#include "NisseHTTP/HTTPHandler.h"
#include "NisseHTTP/Request.h"
#include "NisseHTTP/Response.h"
#include <ThorsLogging/ThorsLogging.h>
#include <ThorsSocket/Server.h>
#include <filesystem>


namespace TASock    = ThorsAnvil::ThorsSocket;
namespace TANS      = ThorsAnvil::Nisse::Server;
namespace TANH      = ThorsAnvil::Nisse::HTTP;
namespace MRest     = ThorsAnvil::Nisse::Examples::MongoRest;
namespace FS        = std::filesystem;

class MongoRest: public TANS::NisseServer
{
    TANH::HTTPHandler       http;
    TANS::PyntControl       control;
    MRest::MongoServer&     mongoServer;

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

    public:
        MongoRest(int port, std::optional<FS::path> certPath, MRest::MongoServer& ms)
            : control(*this)
            , mongoServer(ms)
        {
            // CRUD Person Interface
            http.addPath(TANH::Method::POST,   "/person/",        [&](TANH::Request& request, TANH::Response& response) {mongoServer.personCreate(request, response);});
            http.addPath(TANH::Method::GET,    "/person/Id-{id}", [&](TANH::Request& request, TANH::Response& response) {mongoServer.personGet(request, response);});
            http.addPath(TANH::Method::PUT,    "/person/Id-{id}", [&](TANH::Request& request, TANH::Response& response) {mongoServer.personUpdate(request, response);});
            http.addPath(TANH::Method::DELETE, "/person/Id-{id}", [&](TANH::Request& request, TANH::Response& response) {mongoServer.personDelete(request, response);});

            // Search Person Interface
            http.addPath(TANH::Method::GET,    "/person/findByName/{first}/{last}",[&](TANH::Request& request, TANH::Response& response) {mongoServer.personFindByName(request, response);});
            http.addPath(TANH::Method::GET,    "/person/findByTel/{tel}",          [&](TANH::Request& request, TANH::Response& response) {mongoServer.personFindByTel(request, response);});
            http.addPath(TANH::Method::GET,    "/person/findByZip/{zip}",          [&](TANH::Request& request, TANH::Response& response) {mongoServer.personFindByZip(request, response);});

            listen(getServerInit(certPath, port), http);
            listen(TASock::ServerInfo{port+2}, control);
        }
};


int main(int argc, char* argv[])
{
    if (argc != 6 && argc != 7)
    {
        std::cerr << "Usage: MongoRest <port> <MongoHost> <MongoUser> <MongoPass> <MongoDB> [<certificateDirectory>]\n";
        return 1;
    }
    try
    {
        int                     port        = std::stoi(argv[1]);
        std::string             mongoHost   = argv[2];
        std::string             mongoUser   = argv[3];
        std::string             mongoPass   = argv[4];
        std::string             mongoDB     = argv[5];
        std::optional<FS::path> certPath;
        if (argc == 7) {
            certPath = FS::canonical(argv[6]);
        }

        std::cout << "Nisse MongoRest: Port: " << port << " MongoHost: >" << mongoHost << "< Mongo User: >" << mongoUser << "< MongoPass: >" << mongoPass << "< MongoDB: >" << mongoDB << "< Certificate Path: >" << (argc == 6 ? "NONE" : argv[6]) << "<\n";

        MRest::MongoServer  mongoServer{mongoHost, 27017, mongoUser, mongoPass, mongoDB};
        MongoRest           server(port, certPath, mongoServer);
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
