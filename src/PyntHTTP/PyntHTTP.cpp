#include "PyntHTTP.h"
#include "Response.h"
#include "Request.h"

using namespace ThorsAnvil::Nisse::PyntHTTP;

ThorsAnvil::Nisse::PyntResult PyntHTTP::handleRequest(TAS::SocketStream& stream)
{
    Request     request(stream.getSocket().protocol(), stream);
    if (!request.isOk())
    {
        Response    clientError(stream, request.getVersion(), standardCodes[400]);
        clientError.addHeaders(request.failHeader());
        clientError.done();
        return PyntResult::More;
    }

    Response    response(stream, request.getVersion());
    this->processRequest(request, response);
    return PyntResult::More;
}
