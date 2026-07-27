#include "PyntHTTP.h"
#include "Response.h"
#include "Request.h"

#include "ThorsLogging/ThorsLogging.h"

namespace TASock = ThorsAnvil::ThorsSocket;

using namespace ThorsAnvil::Nisse::HTTP;

NISSE_HEADER_ONLY_INCLUDE
ThorsAnvil::Nisse::Server::PyntResult PyntHTTP::handleRequest(TASock::SocketStream& stream, Server::Context& context)
{
    ThorsLogTrack("ThorsAnvil::Nisse::HTTP::PyntHTTP", "handleRequest", "Enter");
    Request     request(context, stream.getSocket().protocol(), stream);

    // Check if we have a valid request.
    // If there is something wrong in the HTTP request or any of the header parameters or the construction of the body stream
    // then the request is invalid and we will immediately return and close the connection.
    if (!request.isValidRequest())
    {
        Response    clientError(stream, request.getVersion(), 400);
        for (auto const& fail: request.failHeader()) {
            clientError.addHeader(fail.first, fail.second);
        }
        ThorsLogTrack("ThorsAnvil::Nisse::HTTP::PyntHTTP", "handleRequest", "Invalid Request: ", clientError.getCode().code, " => ", clientError.getCode().message);
        return Server::PyntResult::Done;
    }

    Response    response(stream, request.getVersion());
    this->processRequest(request, response);


    ThorsLogTrack("ThorsAnvil::Nisse::HTTP::PyntHTTP", "handleRequest", "Good Request: ", response.getCode().code, " => ", response.getCode().message);
    // By default we want to Keep-Alive
    // So we will only close if all the "connection" values are "close"
    std::vector<std::string> const& connection = request.headers().getHeader("connection");
    bool isClose = false;
    bool isKeepAlive = false;
    for (auto const& c: connection) {
        if (c == "close") {
            isClose = true;
        }
        if (c == "keep-alive") {
            isKeepAlive = true;
        }
    }

    Server::PyntResult result;
    if (isClose) {
        // If I see one close then we will force a close no
        // matter what the other connection values are.
        result = Server::PyntResult::Done;
    }
    else {
        // HTTP 1.0 the connections are closed by default (unless explicitly kept open)
        // HTTP 1.1 the connections are kept open by default.
        result = isKeepAlive || request.getVersion() != Version::HTTP1_0 ? Server::PyntResult::More : Server::PyntResult::Done;
    }
    return result;
}
