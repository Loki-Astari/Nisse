#include "NisseServer/NisseServer.h"
#include "NisseServer/PyntControl.h"
#include "NisseServer/Context.h"
#include "NisseHTTP/HTTPHandler.h"
#include "NisseHTTP/Request.h"
#include "NisseHTTP/Response.h"
#include <filesystem>

namespace TASock    = ThorsAnvil::ThorsSocket;
namespace NisServer = ThorsAnvil::Nisse::Server;
namespace NisHttp   = ThorsAnvil::Nisse::HTTP;
namespace FS        = std::filesystem;

class WebServer: public NisServer::NisseServer
{
    NisHttp::HTTPHandler    http;
    NisServer::PyntControl  control;
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

    void handleRequest(NisHttp::Request& request, NisHttp::Response& response)
    {
        FS::path        requestPath = FS::path{request.variables()["path"]}.lexically_normal();
        if (requestPath.empty() || (*requestPath.begin()) == "..") {
            return response.error(400, "Invalid Request Path");
        }

        std::error_code ec;
        FS::path        filePath = FS::canonical(FS::path{contentDir} /= requestPath, ec);
        if (!ec && FS::is_directory(filePath)) {
            filePath = FS::canonical(filePath /= "index.html", ec);
        }
        if (ec || !FS::is_regular_file(filePath)) {
            return response.error(404, "No File Found At Path");
        }

        TASock::SocketStream    file{TASock::Socket{TASock::FileInfo{filePath.string(), TASock::FileMode::Read}, TASock::Blocking::No}};
        NisServer::AsyncStream  async(file, request.getContext(), NisServer::EventType::Read);

        response.body(NisHttp::Encoding::Chunked) << file.rdbuf();
    }

    public:
        WebServer(int port, FS::path& contentDir, std::optional<FS::path> certPath)
            : control(*this)
            , contentDir(contentDir)
        {
            http.addPath(NisHttp::Method::GET, "/{path}", [&](NisHttp::Request& request, NisHttp::Response& response){handleRequest(request, response);});
            listen(getServerInit(certPath, port), http);
            listen(TASock::ServerInfo{port+2}, control);
        }
};


int main(int argc, char* argv[])
{
#if 0
    loguru::g_stderr_verbosity = 9;
#endif
    if (argc != 3 && argc != 4)
    {
        std::cerr << "Usage: WebServer <port> <contentDirectory> [<certificateDirectory>]\n";
        return 1;
    }
    try
    {
        int                     port        = std::stoi(argv[1]);
        FS::path                contentDir  = FS::canonical(argv[2]);
        std::optional<FS::path> certPath;
        if (argc == 4) {
            certPath = FS::canonical(argv[3]);
        }

        std::cout << "Nisse WebServer: Port: " << port << " ContentDir: >" << contentDir << "< Certificate Path: >" << (argc == 3 ? "NONE" : argv[3]) << "<\n";

        WebServer       server(port, contentDir, certPath);
        server.run();
    }
    catch (std::exception const& e)
    {
        // Try catch forces the application to correctly unwind the stack.
        std::cerr << "Exception: " << e.what() << "\n";
        // Re-throw the exception so the OS know something went wrong.
        // And does OS appropriate logging.
        throw;
    }
    catch (...)
    {
        std::cerr << "Exception: Unknown\n";
        throw;
    }
}
