#ifndef THORSANVIL_NISSE_HTTP_PYNTHTTP_H
#define THORSANVIL_NISSE_HTTP_PYNTHTTP_H

/*
 * An HTTP implementation of Pynt
 * Version 1:
 */

#include "HTTPConfig.h"
#include "NisseServer/Pynt.h"
#include <ThorsSocket/SocketStream.h>
#include <string_view>

namespace TAS = ThorsAnvil::ThorsSocket;
namespace ThorsAnvil::Nisse::HTTP
{

class Request;
class Response;

class PyntHTTPV1: public Pynt
{
    public:
        virtual PyntResult handleRequest(TAS::SocketStream& stream) override;
};

}

#endif
