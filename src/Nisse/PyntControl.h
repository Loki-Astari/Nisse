#ifndef THORSANVIL_NISSE_PYNT_CONTROL_H
#define THORSANVIL_NISSE_PYNT_CONTROL_H

/*
 * Server Control Line:
 * Version 1:
 */

#include "NisseConfig.h"
#include "NisseUtil.h"
#include "NisseServer.h"
#include "Pynt.h"

namespace TAS   = ThorsAnvil::ThorsSocket;
namespace ThorsAnvil::Nisse
{

class PyntControl: public Pynt
{
    NisseServer&    server;
    public:
        PyntControl(NisseServer& server);
        virtual PyntResult handleRequest(TAS::SocketStream& stream) override;
};

}

#endif
