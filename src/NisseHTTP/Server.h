#ifndef THORSANVIL_NISSE_HTTP_NISSEHTTP_SERVER_H
#define THORSANVIL_NISSE_HTTP_NISSEHTTP_SERVER_H

#include "Request.h"
#include "Response.h"
#include "PyntHTTPControl.h"
#include "HTTPHandler.h"

#include "NisseServer/Server.h"
#include "ThorsSocket/Server.h"

#include <filesystem>
#include <string>

namespace ThorsAnvil::Nisse::HTTP
{

class Server: public ThorsAnvil::Nisse::Server::Server
{
    HTTPHandler             handler;
    PyntHTTPControl         control;

    public:
        Server(std::size_t workerCount, TASock::ServerInit&& handlerInit, TASock::ServerInit&& controlInit)
            : ThorsAnvil::Nisse::Server::Server(workerCount)
            , control(*this)
        {
            listen(std::move(handlerInit), handler);
            listen(std::move(controlInit), control);
        }
        ~Server()
        {
            stopHard();
        }

        void addPath(MethodChoice method, std::string const& path, HTTPAction&& action, HTTPValidate&& val = [](Request const&){return true;})
        {
            handler.addPath(method, path, std::forward<HTTPAction>(action), std::forward<HTTPValidate>(val));
        }
};

namespace UnitTest
{

namespace FS        = std::filesystem;


class WebServer: public Server
{
    void handleRequestPath(Request const& request, Response& response)
    {
        using namespace std::string_literals;
        std::error_code         ec;
        std::string             fileName = request.variables()["FilePath"];
        FS::path                filePath = FS::canonical(FS::path{"./test/data/pages/"s + fileName}, ec);
        TASock::SocketStream    file{TASock::Socket{TASock::FileInfo{filePath.string(), TASock::FileMode::Read}, TASock::Blocking::No}};

        if (!file) {
            // Can't open file for some reason.
            // Then we indicate a 404.
            response.setStatus(404);
            return;
        }
        // Otherwise mark the file stream as Async (thus if the disk is slow we will be de-shcheduled for other work)
        // and simply stream the file to the output stream.
        // TODO
        // Issue on linux.
        //    The stream was closed before it is registered for async use.
        //    This causes issues and an exception is thrown.
        ThorsAnvil::Nisse::Server::AsyncStream  async(file, request.getContext(), ThorsAnvil::Nisse::Server::EventType::Read);
        response.body(Encoding::Chunked) << file.rdbuf();
    }

    public:
        WebServer(std::size_t workerCount = 4, TASock::ServerInit&& handlerInit = TASock::ServerInfo{8070}, TASock::ServerInit&& controlInit = TASock::ServerInfo{8079})
            : Server{workerCount, std::forward<TASock::ServerInit>(handlerInit), std::forward<TASock::ServerInit>(controlInit)}
        {
            addPath(Method::GET,
                    "/files/{FilePath}",
                    [&](Request const& request, Response& response)
                    {
                        handleRequestPath(request, response);
                        return true;
                    });
        }
};
}

}

#endif
