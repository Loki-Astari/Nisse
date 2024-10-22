#include "PyntHTTP.h"
#include "Response.h"
#include "Request.h"

using namespace ThorsAnvil::Nisse::NisseHTTP;

ThorsAnvil::Nisse::PyntResult PyntHTTP::handleRequest(TAS::SocketStream& stream, ThorsAnvil::Nisse::Context& context)
{
    Request     request(context, stream.getSocket().protocol(), stream);
    if (!request.isValidRequest())
    {
        Response    clientError(stream, request.getVersion(), 400);
        clientError.addHeaders(request.failHeader(), 0);
        return PyntResult::More;
    }

    Response    response(stream, request.getVersion());
    this->processRequest(request, response);
    return PyntResult::More;
}
