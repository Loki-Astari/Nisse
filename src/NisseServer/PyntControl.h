#ifndef THORSANVIL_NISSE_PYNT_CONTROL_H
#define THORSANVIL_NISSE_PYNT_CONTROL_H

/*
 * Server Control Line:
 * Version 1:
 */

#include "NisseConfig.h"
#include "NisseServer.h"
#include "Pynt.h"
#include <ThorsSocket/SocketStream.h>

namespace TASock = ThorsAnvil::ThorsSocket;

namespace ThorsAnvil::Nisse::Server
{

class PyntControl: public Pynt
{
    NisseServer&    server;
    public:
        PyntControl(NisseServer& server);
        virtual PyntResult handleRequest(TASock::SocketStream& stream, Context& context) override;
};

}

#if defined(THORS_SERIALIZER_HEADER_ONLY) && THORS_SERIALIZER_HEADER_ONLY == 1
#include "PyntControl.source"
#endif

#endif
