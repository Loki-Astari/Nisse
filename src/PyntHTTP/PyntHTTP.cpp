#include "PyntHTTP.h"
#include "Response.h"
#include "Request.h"

using namespace ThorsAnvil::Nisse::PyntHTTP;

ThorsAnvil::Nisse::PyntResult PyntHTTP::handleRequest(TAS::SocketStream& stream)
{
    Request     request(stream.getSocket().protocol(), stream);
    Response    response(stream);

    this->processRequest(request, response);
    return PyntResult::More;
}
