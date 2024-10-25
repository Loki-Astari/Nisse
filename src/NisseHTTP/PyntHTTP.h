#ifndef THORSANVIL_NISSE_NISSEHTTP_PYNTHTTP_H
#define THORSANVIL_NISSE_NISSEHTTP_PYNTHTTP_H

/*
 * An HTTP implementation of Pynt
 * Version 1:
 */

#include "NisseHTTPConfig.h"
#include "NisseServer/Pynt.h"
#include <ThorsSocket/SocketStream.h>
#include <string_view>

namespace TAS = ThorsAnvil::ThorsSocket;
namespace ThorsAnvil::Nisse::HTTP
{

class Request;
class Response;

class PyntHTTP: public Server::Pynt
{
    public:
        virtual Server::PyntResult  handleRequest(TAS::SocketStream& stream, Server::Context& context) override;
        virtual void                processRequest(Request& request, Response& response) = 0;
};

}

#endif
