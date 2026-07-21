#ifndef THORSANVIL_NISSE_PYNT_CONTROL_H
#define THORSANVIL_NISSE_PYNT_CONTROL_H

/*
 * Server Control Line:
 * Version 1:
 */

#include "NisseServerConfig.h"
#include "Pynt.h"
#include <ThorsSocket/SocketStream.h>

namespace TASock = ThorsAnvil::ThorsSocket;

namespace ThorsAnvil::Nisse::Server
{

class Server;

class PyntControl: public Pynt
{
    Server&    server;
    public:
        PyntControl(Server& server);
        virtual PyntResult handleRequest(TASock::SocketStream& stream, Context& context) override;
};

}

#if defined(NISSE_HEADER_ONLY) && NISSE_HEADER_ONLY == 1
#endif

#endif
