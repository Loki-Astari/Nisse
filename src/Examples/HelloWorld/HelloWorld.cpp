#include "NisseServer/NisseServer.h"
#include "NisseServer/PyntControl.h"
#include "NisseHTTP/HTTPHandler.h"
#include "NisseHTTP/Request.h"
#include "NisseHTTP/Response.h"
#include <ThorsSocket/Server.h>
#include <filesystem>

namespace TASock    = ThorsAnvil::ThorsSocket;
namespace NisServer = ThorsAnvil::Nisse::Server;
namespace NisHttp   = ThorsAnvil::Nisse::HTTP;
namespace FS        = std::filesystem;

class HelloWorld: public NisServer::NisseServer
{
    NisHttp::HTTPHandler    http;
    NisServer::PyntControl  control;

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

    void handleRequestLenght(NisHttp::Request& request, NisHttp::Response& response)
    {
        std::string who  = request.variables()["Who"];
        std::string data = R"(
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html>
<head><title>Nisse server 1.1</title></head>
<body>Hello world: )"  + who + R"(</body>
</html>
)";
        response.body(data.size()) << data;
    }
    void handleRequestChunked(NisHttp::Request& request, NisHttp::Response& response)
    {
        response.body(NisHttp::Encoding::Chunked) << R"(
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html>
<head><title>Nisse server 1.1</title></head>
<body>Hello world: )" << request.variables()["Who"] << R"(</body>
</html>
)";
    }

    public:
        HelloWorld(int port, std::optional<FS::path> certPath)
            : control(*this)
        {
            http.addPath(NisHttp::Method::GET, "/HW{Who}.html", [&](NisHttp::Request& request, NisHttp::Response& response){handleRequestLenght(request, response);});
            http.addPath(NisHttp::Method::GET, "/CK{Who}.html", [&](NisHttp::Request& request, NisHttp::Response& response){handleRequestChunked(request, response);});
            listen(getServerInit(certPath, port), http);
            listen(TASock::ServerInfo{port+2}, control);
        }
};

int main(int argc, char* argv[])
{
#if 0
    loguru::g_stderr_verbosity = 9;
#endif
    if (argc != 2 && argc != 3)
    {
        std::cerr << "Usage: HelloWorld <port> [<certificateDirectory>]\n";
        return 1;
    }
    try
    {
        int                     port        = std::stoi(argv[1]);
        std::optional<FS::path> certPath;
        if (argc == 3) {
            certPath = FS::canonical(argv[2]);
        }

        std::cout << "Nisse Hello World: Port: " << port << " Certificate Path: >" << (argc == 2 ? "NONE" : argv[2]) << "<\n";

        HelloWorld      server{port, certPath};
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
