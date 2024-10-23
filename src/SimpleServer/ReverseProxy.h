#include "NisseHTTP/HTTPHandler.h"
#include "NisseHTTP/Request.h"
#include "NisseHTTP/Response.h"
#include "NisseServer/AsyncStream.h"
#include <string>

namespace TAS       = ThorsAnvil::ThorsSocket;
namespace NServer   = ThorsAnvil::Nisse::Server;
namespace NHTTP     = ThorsAnvil::Nisse::HTTP;

inline
void addReverseProxy(NHTTP::HTTPHandler& http)
{
    http.addPath("/proxy/{host}:{port}", [](NHTTP::Request& request, NHTTP::Response& response)
    {
        TAS::SocketInfo         init{request.variables()["host"], std::stoi(request.variables()["port"])};
        TAS::SocketStream       socket{TAS::Socket{init, TAS::Blocking::No}};
        NServer::AsyncStream    async(socket, request.getContext(), NServer::EventType::Write);

        if (socket)
        {
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
        }
        else
        {
            NHTTP::HeaderResponse    header;

            response.setStatus(404);
            response.addHeaders(header);
        }
    });
}
