#ifndef THORSANVIL_NISSE_PYNT_CONTROL_H
#define THORSANVIL_NISSE_PYNT_CONTROL_H

/*
 * Server Control Line:
 * Version 1:
 */

#include "NisseConfig.h"
#include "NisseServer.h"
#include "Pynt.h"

namespace ThorsAnvil::Nisse
{

class PyntControl: public Pynt
{
    NisseServer&    server;
    public:
        PyntControl(NisseServer& server);
        virtual PyntResult handleRequest(std::iostream& stream) override;
};

}

#endif
