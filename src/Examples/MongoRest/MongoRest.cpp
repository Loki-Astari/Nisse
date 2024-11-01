#include "MongoServer.h"
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
namespace MRest     = ThorsAnvil::Nisse::Examples::MongoRest;
namespace FS        = std::filesystem;

class MongoRest: public TANS::NisseServer
{
    TANH::HTTPHandler       http;
    TANS::PyntControl       control;
    MRest::MongoServer&     mongoServer;
    FS::path                contentDir;

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

    void sendPage(TANH::Request& request, TANH::Response& response)
    {
        std::error_code ec;
        FS::path        requestPath = FS::path{request.variables()["page"]}.lexically_normal();
        FS::path        filePath = FS::canonical(FS::path{contentDir} /= requestPath, ec);
        if (requestPath.empty() || (*requestPath.begin()) == ".." || ec || !FS::is_regular_file(filePath)) {
            return response.error(404, "No File Found At Path");
        }

        TASock::SocketStream    file{TASock::Socket{TASock::FileInfo{filePath.string(), TASock::FileMode::Read}, TASock::Blocking::No}};
        TANS::AsyncStream       async(file.getSocket(), request.getContext(), TANS::EventType::Read);

        response.body(TANH::Encoding::Chunked) << file.rdbuf();
    }
    public:
        MongoRest(std::size_t poolSize, int port, FS::path contentDir, std::optional<FS::path> certPath, MRest::MongoServer& ms)
            : TANS::NisseServer{poolSize}
            , control{*this}
            , mongoServer{ms}
            , contentDir{contentDir}
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

            // WebInterface
            http.addPath(TANH::Method::GET,    "/{page}",         [&](TANH::Request& request, TANH::Response& response) {sendPage(request, response);});

            listen(getServerInit(certPath, port), http);
            listen(TASock::ServerInfo{port+2}, control);
        }
};


int main(int argc, char* argv[])
{
#if 0
    loguru::g_stderr_verbosity = 9;
#endif

    if (argc != 7 && argc != 8)
    {
        std::cerr << "Usage: MongoRest <port> <contentDir> <MongoHost> <MongoUser> <MongoPass> <MongoDB> [<certificateDirectory>]\n";
        return 1;
    }
    try
    {
        int                     port        = std::stoi(argv[1]);
        FS::path                contentDir  = FS::canonical(argv[2]);
        std::string             mongoHost   = argv[3];
        std::string             mongoUser   = argv[4];
        std::string             mongoPass   = argv[5];
        std::string             mongoDB     = argv[6];
        std::optional<FS::path> certPath;
        if (argc == 8) {
            certPath = FS::canonical(argv[7]);
        }

        static constexpr std::size_t poolSize             = 6;
        static constexpr std::size_t mongoConnectionCount = 12;
        std::cout << "Nisse MongoRest: Port: " << port << " ConentDir: " << contentDir << " MongoHost: >" << mongoHost << "< Mongo User: >" << mongoUser << "< MongoPass: >" << mongoPass << "< MongoDB: >" << mongoDB << "< Certificate Path: >" << (argc == 7 ? "NONE" : argv[7]) << "<\n";

        MRest::MongoServer  mongoServer{mongoConnectionCount, mongoHost, 27017, mongoUser, mongoPass, mongoDB};
        MongoRest           server{poolSize, port, contentDir, certPath, mongoServer};
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
