#ifndef THORSANVIL_NISSE_PYNT_HTTP_H
#define THORSANVIL_NISSE_PYNT_HTTP_H

/*
 * An HTTP implementation of Pynt
 * Version 1:
 */

#include "NisseConfig.h"
#include "NisseUtil.h"
#include "Pynt.h"

namespace ThorsAnvil::Nisse
{

class PyntHTTP: public Pynt
{
    public:
        virtual PyntResult handleRequest(TAS::SocketStream& stream) override;
};

}

#endif
