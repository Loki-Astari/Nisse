#include "PyntHTTP.h"
#include "Response.h"
#include "Request.h"

#include "ThorsLogging/ThorsLogging.h"

namespace TASock = ThorsAnvil::ThorsSocket;

using namespace ThorsAnvil::Nisse::HTTP;

ThorsAnvil::Nisse::Server::PyntResult PyntHTTP::handleRequest(TASock::SocketStream& stream, Server::Context& context)
{
    ThorsLogDebug("ThorsAnvil::Nisse::Server::PyntHTTP", "handleRequest", "Enter");
    Request     request(context, stream.getSocket().protocol(), stream);
    if (!request.isValidRequest())
    {
        Response    clientError(stream, request.getVersion(), 400);
        clientError.addHeaders(request.failHeader());
        ThorsLogDebug("ThorsAnvil::Nisse::Server::PyntHTTP", "handleRequest", "Invalid Request: ", clientError.getCode().code, " => ", clientError.getCode().message);
        return Server::PyntResult::Done;
    }

    Response    response(stream, request.getVersion());
    this->processRequest(request, response);
    ThorsLogDebug("ThorsAnvil::Nisse::Server::PyntHTTP", "handleRequest", "Good Request: ", response.getCode().code, " => ", response.getCode().message);
    return Server::PyntResult::More;
}
