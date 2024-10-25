#include "PyntHTTP.h"
#include "Response.h"
#include "Request.h"

using namespace ThorsAnvil::Nisse::HTTP;

ThorsAnvil::Nisse::Server::PyntResult PyntHTTP::handleRequest(TAS::SocketStream& stream, Server::Context& context)
{
    Request     request(context, stream.getSocket().protocol(), stream);
    if (!request.isValidRequest())
    {
        Response    clientError(stream, request.getVersion(), 400);
        clientError.addHeaders(request.failHeader());
        return Server::PyntResult::Done;
    }

    Response    response(stream, request.getVersion());
    this->processRequest(request, response);
    return Server::PyntResult::More;
}
