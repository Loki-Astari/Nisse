#include "PyntHTTP.h"
#include "Response.h"
#include "Request.h"

using namespace ThorsAnvil::Nisse::NisseHTTP;

ThorsAnvil::Nisse::PyntResult PyntHTTP::handleRequest(TAS::SocketStream& stream)
{
    Request     request(stream.getSocket().protocol(), stream);
    if (!request.isValidRequest())
    {
        Response    clientError(stream, 400);
        clientError.addHeaders(request.failHeader(), 0);
        return PyntResult::More;
    }

    Response    response(stream);
    this->processRequest(request, response);
    return PyntResult::More;
}
