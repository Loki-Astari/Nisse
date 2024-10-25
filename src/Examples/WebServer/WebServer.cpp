#include "NisseServer/NisseServer.h"
#include "NisseServer/PyntControl.h"
#include "NisseServer/AsyncStream.h"
#include "NisseHTTP/HTTPHandler.h"
#include "NisseHTTP/Request.h"
#include "NisseHTTP/Response.h"
#include "NisseHTTP/HeaderResponse.h"
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
    if (argc != 3 && argc != 4)
    {
        std::cerr << "Usage: WebServer <port> <contentDirectory> [<certificateDirectory>]\n";
        return 1;
    }
    try
    {
        int             port        = std::stoi(argv[1]);
        FS::path        contentDir  = FS::canonical(argv[2]);
        TAS::ServerInit serverInit  = (argc == 3) ? getNormalInit(port) : getSSLInit(argv[3], port);

        std::cout << "Nisse WebServer: Port: " << port << " ContentDir: >" << contentDir << "< Certificate Path: >" << (argc == 3 ? "NONE" : argv[3]) << "<\n";

        NHTTP::HTTPHandler   http;
        http.addPath(NHTTP::Method::GET, "/{path}", [&](NHTTP::Request& request, NHTTP::Response& response)
        {
            NHTTP::HeaderResponse    header;

            std::error_code ec;
            FS::path        requestPath = FS::path{request.variables()["path"]}.lexically_normal();
            if (requestPath.empty() || (*requestPath.begin()) == "..")
            {
                response.setStatus(400);
                header.add("Error", "Invalid Request Path");
                response.addHeaders(header);
                return;
            }
            FS::path        filePath    = contentDir /= requestPath;
            FS::path        filePathCan = FS::canonical(filePath, ec);
            if (!ec && FS::is_directory(filePathCan)) {
                filePathCan = FS::canonical(filePathCan /= "index.html", ec);
            }
            if (ec || !FS::is_regular_file(filePathCan))
            {
                response.setStatus(404);
                header.add("Error", "No File Found At Path");
                response.addHeaders(header);
                return;
            }

            TAS::SocketStream       file{TAS::Socket{TAS::FileInfo{filePathCan.string(), TAS::FileMode::Read}, TAS::Blocking::No}};
            NServer::AsyncStream    async(file, request.getContext(), NServer::EventType::Read);

            response.addHeaders(header);
            response.body(NHTTP::Encoding::Chunked) << file.rdbuf();
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
